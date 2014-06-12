//#include "fw_env.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/capability.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <linux/math64.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <asm/div64.h>
#include <mtd/ubi-user.h>
#include <mtd/mtd-abi.h>
#include <linux/vmalloc.h>
#include <linux/major.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/core.h>
#include <linux/mmc/ioctl.h>
#include "./inc/crc32_boot.h"
#include "./inc/klib.h"
struct envdev_s {
	char devname[32];		/* Device name */
	ulong devoff;			/* Device offset */
	ulong env_size;			/* environment size */
	ulong erase_size;		/* device erase size */
	ulong env_sectors;		/* number of environment sectors */
	uint8_t mtd_type;		/* type of the MTD device */
};

static struct envdev_s envdevices[2] =
{
	{
		.mtd_type = MTD_ABSENT,
	}, {
		.mtd_type = MTD_ABSENT,
	},
};
static int dev_current = 0;

#define FLASH_READ   0
#define FLASH_WRITE  1

#define HAVE_REDUND /* For systems with 2 env sectors */
#define ENV_DNAME_ANDROID "/dev/block/mmcblk0p2"
#define ENV_DNAME_GENERIC "/dev/mmcblk0p2"

#define DEVICE1_NAME      ENV_DNAME_GENERIC
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x1000	/* 4k */
#define DEVICE1_ESIZE     0x1000	/* 4k TODO */
#define DEVICE1_ENVSECTORS 8		/* 8 sectors equals to 4k */

#define DEVICE2_NAME      DEVICE1_NAME
#define DEVICE2_OFFSET    ENV1_SIZE
#define ENV2_SIZE         ENV1_SIZE
#define DEVICE2_ESIZE     ENV1_SIZE
#define DEVICE2_ENVSECTORS     DEVICE1_ENVSECTORS

#define DEVNAME(i)    envdevices[(i)].devname
#define DEVOFFSET(i)  envdevices[(i)].devoff
#define ENVSIZE(i)    envdevices[(i)].env_size
#define DEVESIZE(i)   envdevices[(i)].erase_size
#define ENVSECTORS(i) envdevices[(i)].env_sectors
#define DEVTYPE(i)    envdevices[(i)].mtd_type

#define CONFIG_ENV_SIZE ENVSIZE(dev_current)

#define ENV_SIZE      getcmdenvsize()

struct env_image_single {
	uint32_t	crc;	/* CRC32 over data bytes    */
	char		data[];
};

struct env_image_redundant {
	uint32_t	crc;	/* CRC32 over data bytes    */
	unsigned char	flags;	/* active or obsolete */
	char		data[];
};

enum flag_scheme {
	FLAG_NONE,
	FLAG_BOOLEAN,
	FLAG_INCREMENTAL,
};

struct environment {
	void			*image;
	uint32_t		*crc;
	unsigned char		*flags;
	char			*data;
	enum flag_scheme	flag_scheme;
};

static struct environment environment = {
	.image	     = NULL,
	.flag_scheme = FLAG_NONE,
};

static int HaveRedundEnv = 0;
static int double_env = 0;

static unsigned char active_flag = 1;
/* obsolete_flag must be 0 to efficiently set it on NOR flash without erasing */
static unsigned char obsolete_flag = 0;

#ifdef HAVE_REDUND
static int __init have_redund_env(char *str)
{
	HaveRedundEnv = 1;
	return 1;
}

__setup("redund_env", have_redund_env);
#endif

static int __init have_double_env(char *str)
{
	double_env = 1;
	return 1;
}
__setup("double_env", have_double_env);

static int read_extcsd(struct file *filp, __u8 *ext_csd)
{	
	int ret = 0;
	struct mmc_ioc_cmd idata;
	memset(&idata, 0, sizeof(idata));
	memset(ext_csd, 0, sizeof(__u8) * 512);
	idata.write_flag = 0;
	idata.opcode = MMC_SEND_EXT_CSD;
	idata.arg = 0;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blksz = 512;
	idata.blocks = 1;
	mmc_ioc_cmd_set_data(idata, ext_csd);

	ret = klib_ioctl(filp, MMC_IOC_CMD, (unsigned long)&idata);
	if (ret)
		printk("get extcsd ioctl error\n");

	return ret;
}
static int auto_detect_env_group(void)
{
#define ENV_BASE1 (0)
#define ENV_BASE2 (512*1024)

	int ret = -1, base = 0;
	unsigned char ext_csd[512] = { 0 };
	struct file *filp = NULL;

	if (double_env == 0) {
		goto out;
	}

	filp = klib_fopen("/dev/mmcblk0",O_RDONLY,0);
	
	if (filp == NULL) {
		printk("open dev/mmcblk0 error\n");
		goto out;
	}
	ret = read_extcsd(filp, ext_csd);
	if (ret) {
		printk("read_extcsd  error\n");
		goto out1;
	}

	base = (((ext_csd[EXT_CSD_PART_CONFIG] >> 3) & 0x7) == \
					0x1 ? ENV_BASE1 : ENV_BASE2);

out1:
	klib_fclose(filp);
out:
	return base;
#undef ENV_BASE1
#undef ENV_BASE2
}

static inline ulong getcmdenvsize (void)
{
	ulong rc = CONFIG_ENV_SIZE - sizeof(uint32_t);

	if (HaveRedundEnv)
		rc -= sizeof(unsigned char);
	return rc;
}

static int flash_read (int dev_target)
{
	int rc;
	loff_t offset = DEVOFFSET(dev_target);

	struct file * filp = klib_fopen(DEVNAME(dev_target),O_RDONLY,0);
	if(!filp) {
		printk("Can't open %s\n", DEVNAME (dev_target));
		return -1;
	}

	filp->f_pos = offset;
	rc = klib_fread(environment.image,CONFIG_ENV_SIZE,filp);
	klib_fclose(filp);	
	return (!rc) ? -1 : 0;
}

static int flash_write(int dev_target)
{
	int rc;
	loff_t offset = DEVOFFSET(dev_target);
	
	struct file * filp = klib_fopen(DEVNAME(dev_target),O_WRONLY,0);
	if(!filp) {
		printk("Can't open %s\n", DEVNAME (dev_target));
		return -1;
	}

	switch (environment.flag_scheme) {
		case FLAG_NONE:
			break;
		case FLAG_INCREMENTAL:
			(*environment.flags)++;
			break;
		case FLAG_BOOLEAN:
			*environment.flags = active_flag;
			break;
		default:
			printk("Unimplemented flash scheme %u \n",
			environment.flag_scheme);
			return -1;
	}

	pr_debug ("Writing new environment at 0x%lx on %s\n",
            DEVOFFSET (dev_target), DEVNAME (dev_target));

	filp->f_pos = offset;
	rc = klib_fwrite(environment.image,CONFIG_ENV_SIZE,filp);
	klib_fclose(filp);	
	return 0;
}


static int flash_io (int mode)
{
	int dev_target = dev_current;;
	if (mode == FLASH_READ) {
		return flash_read(dev_target);
	} else {
		if (HaveRedundEnv) {
			dev_target = !dev_current;
		}
		return flash_write(dev_target);
	}
}


static int parse_config(void)
{
	int offset = 0;
	offset = auto_detect_env_group();
	
	strcpy(DEVNAME(0), DEVICE1_NAME);
	DEVOFFSET(0) = DEVICE1_OFFSET + offset;
	ENVSIZE(0) = ENV1_SIZE;
	DEVESIZE(0) = ENVSIZE(0);
	ENVSECTORS(0) = 0x8;
	if (HaveRedundEnv) {
		strcpy(DEVNAME(1), DEVICE2_NAME);
		DEVOFFSET(1) = DEVICE2_OFFSET + offset;
		ENVSIZE(1) = ENV2_SIZE;
		DEVESIZE(1) = ENVSIZE(1);
		ENVSECTORS(1) = 0x8;
	}
	return 0;
}

static char *envmatch (char * s1, char * s2)
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return s2;
	if (*s1 == '\0' && *(s2 - 1) == '=')
		return s2;
	return NULL;
}


int fw_env_open(void)
{
	int crc0, crc0_ok;
	unsigned char flag0;
	void *addr0 = NULL;
	void *addr1 = NULL;

	struct env_image_single *single;
	struct env_image_redundant *redundant;

	if (parse_config ())		/* should fill envdevices */
		return -1;

	addr0 = vmalloc (CONFIG_ENV_SIZE);
	if (addr0 == NULL) {
		printk (
			"Not enough memory for environment (%ld bytes)\n",
			CONFIG_ENV_SIZE);
		return -1;
	}
	memset(addr0, 0, CONFIG_ENV_SIZE);

	/* read environment from FLASH to local buffer */
	environment.image = addr0;
	if (HaveRedundEnv) {
		redundant = addr0;
		environment.crc		= &redundant->crc;
		environment.flags	= &redundant->flags;
		environment.data	= redundant->data;
	} else {
		single = addr0;
		environment.crc		= &single->crc;
		environment.flags	= NULL;
		environment.data	= single->data;
	}
	
	dev_current = 0;
	if (flash_io (FLASH_READ))
	{
		printk("%s line %d: flash_io error\n",__FILE__,__LINE__);
		goto ERROR;
	}

	crc0 = crc32 (0, (uint8_t *) environment.data, ENV_SIZE);
	crc0_ok = (crc0 == *environment.crc);
	if (!HaveRedundEnv) {
		if (!crc0_ok) {
		// TODO: return appropriate error code
			printk("Warning: Bad CRC, no environment available\n");
			goto ERROR;
		}
	} else {
		int crc1, crc1_ok;
		unsigned char flag1;

		flag0 = *environment.flags;

		dev_current = 1;
		addr1 = vmalloc (CONFIG_ENV_SIZE);
		if (addr1 == NULL) {
			printk("Error: Not enough memory for environment (%ld bytes)\n",
				CONFIG_ENV_SIZE);
			goto ERROR;
		}
		memset(addr1, 0, CONFIG_ENV_SIZE);
		redundant = addr1;

		/*
		 * have to set environment.image for flash_read(), careful -
		 * other pointers in environment still point inside addr0
		 */
		environment.image = addr1;
		if (flash_io (FLASH_READ)) {
			printk("%s line %d: flash_io error\n",__FILE__,__LINE__);
			goto ERROR;
		}

		/* Check flag scheme compatibility */
		if (DEVTYPE(dev_current) == MTD_NORFLASH &&
		    DEVTYPE(!dev_current) == MTD_NORFLASH) {
			environment.flag_scheme = FLAG_BOOLEAN;
		} else if (DEVTYPE(dev_current) == MTD_NANDFLASH &&
			   DEVTYPE(!dev_current) == MTD_NANDFLASH) {
			environment.flag_scheme = FLAG_INCREMENTAL;
		} else if (DEVTYPE(dev_current) == MTD_DATAFLASH &&
			   DEVTYPE(!dev_current) == MTD_DATAFLASH) {
			environment.flag_scheme = FLAG_BOOLEAN;
		} else if (DEVTYPE(dev_current) == MTD_ABSENT &&
			   DEVTYPE(!dev_current) == MTD_ABSENT) {
			environment.flag_scheme = FLAG_INCREMENTAL;	/* mmc */
		} else {
			printk ("Error: Incompatible flash types!\n");
			goto ERROR;
		}

		crc1 = crc32 (0, (uint8_t *) redundant->data, ENV_SIZE);
		crc1_ok = (crc1 == redundant->crc);
		flag1 = redundant->flags;

		if (crc0_ok && !crc1_ok) {
			dev_current = 0;
		} else if (!crc0_ok && crc1_ok) {
			dev_current = 1;
		} else if (!crc0_ok && !crc1_ok) {
			printk ("Warning: Bad CRC, no environment available\n");
			goto ERROR;
		} else {
			switch (environment.flag_scheme) {
			case FLAG_BOOLEAN:
				if (flag0 == active_flag &&
				    flag1 == obsolete_flag) {
					dev_current = 0;
				} else if (flag0 == obsolete_flag &&
					   flag1 == active_flag) {
					dev_current = 1;
				} else if (flag0 == flag1) {
					dev_current = 0;
				} else if (flag0 == 0xFF) {
					dev_current = 0;
				} else if (flag1 == 0xFF) {
					dev_current = 1;
				} else {
					dev_current = 0;
				}
				break;
			case FLAG_INCREMENTAL:
				if (flag0 == 255 && flag1 == 0)
					dev_current = 1;
				else if ((flag1 == 255 && flag0 == 0) ||
					 flag0 >= flag1)
					dev_current = 0;
				else /* flag1 > flag0 */
					dev_current = 1;
				break;
			default:
				printk("Error: Unknown flag scheme %u \n",
					environment.flag_scheme);
				goto ERROR;
			}
		}

		/*
		 * If we are reading, we don't need the flag and the CRC any
		 * more, if we are writing, we will re-calculate CRC and update
		 * flags before writing out
		 */
		if (dev_current) {
			environment.image	= addr1;
			environment.crc		= &redundant->crc;
			environment.flags	= &redundant->flags;
			environment.data	= redundant->data;
			vfree (addr0);
			addr0 = NULL;
		} else {
			environment.image	= addr0;
			/* Other pointers are already set */
			vfree (addr1);
			addr1 = NULL;
		}
	}
	return 0;

ERROR:
	if (addr1) {vfree(addr1); addr1 = NULL;}
	if (addr0) {vfree(addr0); addr0 = NULL;}
	environment.image = NULL;
	environment.crc   = NULL;
	environment.flags = NULL;
	environment.data  = NULL;
	return -1;
}

void fw_env_close(void)
{
	if(environment.image != NULL)
		vfree(environment.image);
}



char *fw_getenv (char *name)
{
	char *env, *nxt;

	for (env = environment.data; *env; env = nxt + 1) {
		char *val;

		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				printk ("## Error: "
					"environment not terminated\n");
				return NULL;
			}
		}
		val = envmatch (name, env);
		if (!val)
			continue;
		return val;
	}
	return NULL;
}

int fw_env_write(char * name,char * value)
{
    int len;
	char *env, *nxt;
	char *oldval = NULL;

	/*
	 * search if variable with this name already exists
	 */
	for (nxt = env = environment.data; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				printk("## Error: "
					"environment not terminated\n");
				return -1;
			}
		}
		if ((oldval = envmatch (name, env)) != NULL)
		{
		    printk("old value:%s\n", oldval);
		    break;
		}
	}

	/*
	 * Delete any existing definition
	 */
	if (oldval) {
		if (*++nxt == '\0') {
			*env = '\0';
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

	/* Delete only ? */
	if (!value || !strlen(value))
		return 0;

	/*
	 * Append new definition at the end
	 */
	for (env = environment.data; *env || *(env + 1); ++env);
	if (env > environment.data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > CONFIG_ENV_SIZE - (env-environment)
	 */
	len = strlen (name) + 2;
	/* add '=' for first arg, ' ' for all others */
	len += strlen(value) + 1;

	if (len > (&environment.data[ENV_SIZE] - env)) {
		printk (
			"Error: environment overflow, \"%s\" deleted\n",
			name);
		return -1;
	}

	while ((*env = *name++) != '\0')
		env++;
	*env = '=';
	while ((*++env = *value++) != '\0')
		;

	/* end is marked with double '\0' */
	*++env = '\0';

	*environment.crc = crc32(0, (uint8_t *) environment.data, ENV_SIZE);
	flash_io(FLASH_WRITE);

	return 0;
}

/*
 * Print the current definition of one, or more, or all
 * environment variables
 */
int fw_printenv (int argc, char *argv[])
{
	char *env, *nxt;
	int i, n_flag;
	int rc = 0;

	if (fw_env_open())
		return -1;

	if (argc == 1) {		/* Print all env variables  */
		for (env = environment.data; *env; env = nxt + 1) {
			for (nxt = env; *nxt; ++nxt) {
				if (nxt >= &environment.data[ENV_SIZE]) {
					printk ("## Error: "
						"environment not terminated\n");
					return -1;
				}
			}

			printk ("%s\n", env);
		}
		return 0;
	}

	if (strcmp (argv[1], "-n") == 0) {
		n_flag = 1;
		++argv;
		--argc;
		if (argc != 2) {
			printk ("## Error: "
				"`-n' option requires exactly one argument\n");
			return -1;
		}
	} else {
		n_flag = 0;
	}

	for (i = 1; i < argc; ++i) {	/* print single env variables   */
		char *name = argv[i];
		char *val = NULL;

		for (env = environment.data; *env; env = nxt + 1) {

			for (nxt = env; *nxt; ++nxt) {
				if (nxt >= &environment.data[ENV_SIZE]) {
					printk ("## Error: "
						"environment not terminated\n");
					return -1;
				}
			}
			val = envmatch (name, env);
			if (val) {
				if (!n_flag) {
					printk ("%s=", name);
				}
				printk ("%s\n", val);
				break;
			}
		}
		if (!val) {
			printk ("## Error: \"%s\" not defined\n", name);
			rc = -1;
		}
	}

	return rc;
}
