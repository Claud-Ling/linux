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

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })


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

#define ENV1_SIZE         0x1000	/* 4k */
#define ENV2_SIZE         ENV1_SIZE

/*
 * eMMC device definition
 */
#define EMMC_DEV1     ENV_DNAME_GENERIC
#define EMMC_DEV1_OFFS    0x0000
#define EMMC_DEV1_ESIZE     0x1000	/* 4k TODO */
#define EMMC_DEV1_ENVSECTORS 8		/* 8 sectors equals to 4k */

#define EMMC_DEV2      EMMC_DEV1
#define EMMC_DEV2_OFFS    0x0000
#define EMMC_DEV2_ESIZE     0x1000	/* 4k TODO */
#define EMMC_DEV2_ENVSECTORS 8		/* 8 sectors equals to 4k */

/*
 * NAND device definition
 */
#define NAND_DEV1		"/dev/mtd0"
#define NAND_DEV1_OFFS		0x600000
/* Erase size  temporary define as 4k,
 * actually, this value will be overwrite by device specified value. 
 */
#define NAND_DEV1_ESIZE		0x1000
#define NAND_DEV1_ENVSECTORS	8

#define NAND_DEV2		"/dev/mtd0"
#define NAND_DEV2_OFFS		0x900000
#define NAND_DEV2_ESIZE		NAND_DEV1_ESIZE
#define NAND_DEV2_ENVSECTORS	NAND_DEV1_ENVSECTORS

/*
 * eMMC ENV group define
 */
#define EMMC_ENV_GROUP1_OFFS	(0x00000)
#define EMMC_ENV_GROUP2_OFFS (0x80000)  /* 512k offset */ 

/*
 * NAND ENV group define
 */
#define NAND_ENV_GROUP1_OFFS	(0x00000)
#define NAND_ENV_GROUP2_OFFS (0x900000)  /* 9MB offset */


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

static struct mtd_info_user g_mtd_info = { 0 };
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
static int emmc_detect_env_group(void)
{
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
					0x1 ? \
					EMMC_ENV_GROUP1_OFFS \
					: EMMC_ENV_GROUP2_OFFS);

out1:
	klib_fclose(filp);
out:
	return base;
}
static int auto_detect_env_group(void)
{
	if(DEVTYPE(0) == MTD_ABSENT) {
		return emmc_detect_env_group();
	} else if (DEVTYPE(0) == MTD_NANDFLASH) {
		printk("Warning!! NAND doesn't support double ENV yet!\n");
		return NAND_ENV_GROUP1_OFFS;

	} else {
		printk("Warning!! Unknown flash type!!\n");
		return 0;
	}
}
static inline ulong getcmdenvsize (void)
{
	ulong rc = CONFIG_ENV_SIZE - sizeof(uint32_t);

	if (HaveRedundEnv)
		rc -= sizeof(unsigned char);
	return rc;
}

static int nand_read_buf (struct file *filp, void *buf, size_t count, 
			off_t offset, struct mtd_info_user *mtd)
{
	size_t blocklen;	/* erase / write length - one block on NAND,
				   0 on NOR */
	size_t processed = 0;	/* progress counter */
	size_t readlen = count;	/* current read length */
	off_t block_seek;	/* offset inside the current block to the start
				   of the data */
	loff_t blockstart;	/* running start of the current block -
				   MEMGETBADBLOCK needs 64 bits */
	int rc;

	blockstart = (offset / (mtd->erasesize)) * (mtd->erasesize);

	/* Offset inside a block */
	block_seek = offset - blockstart;

	/*
	 * NAND: calculate which blocks we are reading. We have
	 * to read one block at a time to skip bad blocks.
	 */
	blocklen = mtd->erasesize;


	/* Limit to one block for the first read */
	if (readlen > blocklen - block_seek)
		readlen = blocklen - block_seek;

	while (processed < count) {
		rc = klib_ioctl(filp, MEMGETBADBLOCK, (unsigned long)&blockstart);
		if (rc < 0) {
			printk("Cannot read bad block mark\n");
			return -1;
		}
		if (rc) { /* Bad block, continue */
			blockstart += blocklen;
			continue;
		}

		/*
		 * If a block is bad, we retry in the next block at the same
		 * offset - see common/env_nand.c::writeenv()
		 */
		filp->f_pos = (blockstart + block_seek);

		rc = klib_fread(buf+processed,readlen,filp);
		if (rc != readlen) {
			printk ("Read NAND error!!\n");
			return -1;
		}
#ifdef DEBUG
		printk ("Read 0x%x bytes at 0x%llx\n",
			 rc, blockstart + block_seek);
#endif
		processed += readlen;
		readlen = min (blocklen, count - processed);
		block_seek = 0;
		blockstart += blocklen;
	}

	return processed;
}
static int nand_write_buf (struct file *filp, void *buf, size_t count, 
			off_t offset, struct mtd_info_user *mtd)
{
	void *data;
	struct erase_info_user erase;
	size_t blocklen;	/* length of NAND block / NOR erase sector */
	size_t erase_len;	/* whole area that can be erased - may include
				   bad blocks */
	size_t erasesize = 0;	/* erase / write length - one block on NAND,
				   whole area on NOR */
	size_t processed = 0;	/* progress counter */
	size_t write_total;	/* total size to actually write - excluding
				   bad blocks */
	off_t erase_offset;	/* offset to the first erase block (aligned)
				   below offset */
	off_t block_seek = 0;	/* offset inside the erase block to the start
				   of the data */
	loff_t blockstart;	/* running start of the current block -
				   MEMGETBADBLOCK needs 64 bits */
	int rc;

	blocklen = mtd->erasesize;

	erase_offset = (offset / blocklen) * blocklen;

	/* Offset inside a block */
	block_seek = offset - erase_offset;

	blockstart = erase_offset;

	/*
	 * Data size we actually have to write: from the start of the block
	 * to the start of the data, then count bytes of data, and to the
	 * end of the block
	 */
	write_total = ((block_seek + count + blocklen - 1) /
						blocklen) * blocklen;
	/* Maximum area we may use */
	erase_len =  write_total;

	/*
	 * Support data anywhere within erase sectors: read out the complete
	 * area to be erased, replace the environment image, write the whole
	 * block back again.
	 */
	if (write_total > count) {
		data = kmalloc (erase_len, GFP_KERNEL);
		if (!data) {
			printk ("Cannot malloc %zu bytes\n",
				 erase_len);
			return -1;
		}

		rc = nand_read_buf (filp, data, write_total, 
				erase_offset, &g_mtd_info);
		if (write_total != rc)
			return -1;

		/* Overwrite the old environment */
		memcpy (data + block_seek, buf, count);
	} else {
		/*
		 * We get here, iff offset is block-aligned and count is a
		 * multiple of blocklen - see write_total calculation above
		 */
		data = buf;
	}


	erase.length = blocklen;

	/* This only runs once on NOR flash and SPI-dataflash */
	while (processed < write_total) {
		rc = klib_ioctl(filp, MEMGETBADBLOCK, (unsigned long)&blockstart);
		if (rc < 0) {
			printk("Cannot read bad block mark\n");
			return -1;
		}
		if (rc) { /* Bad block, continue */
			blockstart += blocklen;
			continue;
		}

		erase.start = blockstart;
		klib_ioctl (filp, MEMUNLOCK, (unsigned long)&erase);

		/* Dataflash does not need an explicit erase cycle */
		if (klib_ioctl (filp, MEMERASE, (unsigned long)&erase) != 0) {
			printk ("NAND erase error\n");
			return -1;
		}

		filp->f_pos = blockstart;

#ifdef DEBUG
		printk ("Write 0x%x bytes at 0x%llx\n", erasesize, blockstart);
#endif
		if (klib_fwrite(data + processed, erasesize, filp) != erasesize) {
			printk ("NAND write error\n");
			return -1;
		}

		klib_ioctl (filp, MEMLOCK, (unsigned long)&erase);

		processed  += blocklen;
		block_seek = 0;
		blockstart += blocklen;
	}

	if (write_total > count)
		kfree (data);

	return processed;
}
static int emmc_read_env(int dev_target)
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
static int nand_read_env(int dev_target)
{
	int rc;
	off_t offset = DEVOFFSET(dev_target);

	struct file *filp = klib_fopen(DEVNAME(dev_target), O_RDONLY, 0);
	if(!filp) {
		printk("Can't open %s\n", DEVNAME (dev_target));
		return -1;
	}
	
	filp->f_pos = offset;
	rc = nand_read_buf(filp, environment.image, 
				CONFIG_ENV_SIZE, offset, &g_mtd_info);
	rc = (rc == CONFIG_ENV_SIZE ? 0 : -1);

	klib_fclose(filp);
	return rc;	
}
static int flash_read(int dev_target)
{
	if (DEVTYPE(0) == MTD_NANDFLASH) {
		return nand_read_env(dev_target);
	} else if (DEVTYPE(0) == MTD_ABSENT) {
		return emmc_read_env(dev_target);
	} else {
		printk("Unknown flash type!!\n");
		return -1;
	}
}
static int emmc_write_env(int dev_target)
{
	int rc;
	loff_t offset = DEVOFFSET(dev_target);
	
	struct file * filp = klib_fopen(DEVNAME(dev_target),O_WRONLY,0);
	if(!filp) {
		printk("Can't open %s\n", DEVNAME (dev_target));
		return -1;
	}

	filp->f_pos = offset;
	rc = klib_fwrite(environment.image,CONFIG_ENV_SIZE,filp);
	klib_fclose(filp);	
	return 0;

}
static int nand_write_env(int dev_target)
{
	int rc;
	loff_t offset = DEVOFFSET(dev_target);

	struct file *filp = klib_fopen(DEVNAME(dev_target), O_RDWR, 0);
	if(!filp) {
		printk("Can't open %s\n", DEVNAME (dev_target));
		return -1;
	}

	filp->f_pos = offset;

	rc = nand_write_buf(filp, environment.image,
				CONFIG_ENV_SIZE, offset, &g_mtd_info);

	rc = (rc == CONFIG_ENV_SIZE ? 0 : -1);
	klib_fclose(filp);
	return rc;
}
static int flash_write(int dev_target)
{

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

	if (DEVTYPE(0) == MTD_NANDFLASH) {
		return nand_write_env(dev_target);
	} else if (DEVTYPE(0) == MTD_ABSENT) {
		return emmc_write_env(dev_target);
	} else {
		printk("Unknown flash type!!\n");
		return -1;
	}

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


static int enumerate_env_device(void)
{
	struct mtd_info_user *mtd = &g_mtd_info;
	struct file *filp = NULL;

	/*
 	 * Try with NAND device first
 	 */ 
	filp = klib_fopen(NAND_DEV1, O_RDONLY, 0);
	if (filp == NULL) {
		goto try_emmc;
	}
	
	if (klib_ioctl(filp, MEMGETINFO, (unsigned long)mtd)) {
		printk("Can't get device %s MTD info\n", NAND_DEV1);
		klib_fclose(filp);
		return -1;
	}
#ifdef DEBUG
	printk("mtd->type = %d\n", mtd->type);
	printk("mtd->size = %x\n", mtd->size);
	printk("mtd->erasesize = %x\n", mtd->erasesize);
	printk("mtd->writesize = %x\n", mtd->writesize);
	printk("mtd->oobsize = %x\n", mtd->oobsize);
#endif
	strcpy(DEVNAME(0), NAND_DEV1);
	DEVOFFSET (0) = NAND_DEV1_OFFS;
	ENVSIZE (0) = ENV1_SIZE;
	DEVESIZE (0) = mtd->erasesize;
	ENVSECTORS(0) = NAND_DEV1_ENVSECTORS;
	DEVTYPE(0) = MTD_NANDFLASH;
#ifdef HAVE_REDUND
	strcpy(DEVNAME(1), NAND_DEV2);
	DEVOFFSET (1) = NAND_DEV2_OFFS;
	ENVSIZE (1) = ENV2_SIZE;
	DEVESIZE (1) = mtd->erasesize;
	ENVSECTORS(1) = NAND_DEV2_ENVSECTORS;
	DEVTYPE(1) = MTD_NANDFLASH;
#endif
	klib_fclose(filp);
	filp = NULL;
	if (mtd->type == MTD_NORFLASH) {
		goto try_emmc;
	}
	goto end;

try_emmc:
	/*
	 * Try with eMMC device
	 */ 
	filp = klib_fopen(EMMC_DEV1, O_RDONLY, 0);
	if (filp == NULL) {
		printk("No ENV device found!\n");
		return -1;
	}
	klib_fclose(filp);
	filp = NULL;
	strcpy(DEVNAME(0), EMMC_DEV1);
	DEVOFFSET (0) = EMMC_DEV1_OFFS;
	ENVSIZE (0) = ENV1_SIZE;
	DEVESIZE (0) = ENVSIZE (0);
	ENVSECTORS(0) = EMMC_DEV1_ENVSECTORS;
#ifdef HAVE_REDUND
	strcpy(DEVNAME(1), EMMC_DEV2);
	DEVOFFSET (1) = EMMC_DEV2_OFFS;
	ENVSIZE (1) = ENV2_SIZE;
	DEVESIZE (1) = ENVSIZE (1);
	ENVSECTORS(1) = EMMC_DEV2_ENVSECTORS;
#endif
end:
	return 0;
}
static int parse_config(void)
{
	int rc = 0, offset;

	rc = enumerate_env_device();
	if (rc) {
		printk("Enumerate ENV device failed!!\n");
		return -1;
	}
	offset = auto_detect_env_group();

	DEVOFFSET(0) += offset;
#ifdef HAVE_REDUND
	DEVOFFSET(1) += offset;
#endif
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
