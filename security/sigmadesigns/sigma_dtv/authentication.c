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
#include <linux/vmalloc.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <asm/div64.h>
#include <asm/io.h>
#include <linux/mmc/card.h> 
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/blkdev.h>

#include "./inc/rsa.h"
#include "./inc/sha2.h"
#include "./inc/bignum.h"
#include "./inc/bn_mul.h"
#include "./inc/fw_env.h"
#include "./inc/klib.h"

#include "./inc/mmc_priv.h"

#ifdef CONFIG_PROC_FS
#include "proc_security.c"
#endif

extern void mcu_send_rest( void );
//#define SIG_DEBUG 1

#ifdef SIG_DEBUG
#define PDEBUG(fmt, args...) printk(fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif
#define PERROR(fmt, args...) printk(fmt, ## args)

#define HASHLEN  32     //in bytes
#define SIGNLEN  256    //signature len in bytes
#define KEY_SIG_BUF_LEN 0x600 //signuate and key buffer length
#define KEY_LEN  256

#define SECURITY_FAIL 127

#define TURING_FUSE_STATE	0x110C
#define TURING_REG_SIZE		SZ_8K	/*actually turing reg space is 128k, but 8k is enough for use here */

#ifdef CONFIG_SIGMA_SOC_SX6
# define TURING_REG_BASE 	0xf5020000
# define TURING_SETUP()		do{}while(0)
# define TURING_CLEANUP()	do{}while(0)
# define TURING_READL(a) readl((void*)(a))
#else /*CONFIG_SIGMA_SOC_SX6*/
static void __iomem* turing_reg_vbase = NULL;
# define TURING_REG_BASE 0xf1040000
# define TURING_SETUP()	do{									\
				turing_reg_vbase = ioremap(TURING_REG_BASE, TURING_REG_SIZE);	\
				if (!turing_reg_vbase)						\
					pr_err("turing iomap failed!\n");			\
			}while(0)
# define TURING_CLEANUP() do{									\
				if (turing_reg_vbase) iounmap(turing_reg_vbase);		\
				turing_reg_vbase = NULL;					\
			}while(0)
# define TURING_READL(a) ({									\
				long _ret = 0;							\
				if (turing_reg_vbase && ((a)>0 && (a) < TURING_REG_SIZE))	\
					_ret = readl((void*)(turing_reg_vbase + a));		\
				else								\
					pr_warn("failed to read turing reg:%x\n", (a));		\
				_ret;								\
			})
#endif /*CONFIG_SIGMA_SOC_SX6*/

int __initdata security_debug = 0;  /*0 = normal mode ,1 = force debug, 2 = force disable*/
#ifndef SECURITY_BUILD_AS_MODULE
static int __init sx6_security_debug(char *str)
{
	security_debug = simple_strtol(str,NULL,0) & 0x3;
	return 1;
}
#endif
__setup("security_debug=", sx6_security_debug);


static int is_secure_enable(void)
{
	int security_otp;
	unsigned long val;	
	
	val = TURING_READL(TURING_FUSE_STATE);
	//PDEBUG("Secure OTP val:0x%08lx\n", val);
	security_otp = (val&0x2)?1:0;

	/*
	 * we check the kernel param "security_debug" for debug mode:
 	 * 1.if security_debug=1,authenticating is force enable	
 	 * 2.if security_check=2,authenticating is force disable
 	 * 3.depending on OTP register if security_debug is 0.	 	 
	 */	
	if(!security_debug)
		return security_otp;
	else
		return (security_debug == 1)?1:0;
}


static void print_mem(char *desc, unsigned char *buf, int len)
{
#ifdef SIG_DEBUG
    int i;
    int j = 0;
    PDEBUG("start print %s\n", desc);
    for (i = 0; i < len;  i ++)
    {
        PDEBUG("%02x ", buf[i]);
        j ++;
        if (j == 16)
        {
            j = 0;
            PDEBUG("\n");
        }
    }
    PDEBUG("end print %s\n", desc);
#endif
}



static unsigned long strtohex(char* str)
{
        unsigned long var=0;
        unsigned long t;
       	
	if(strlen(str) > 8)
		return 0; 
	for (;*str;str++)    
        {
                if (*str>='A' && *str <='F')   
                        t = *str-55;                   
                else if (*str>='a' && *str <='f')
                        t = *str - 87;                 
                else
                        t = *str-48;  /*0 - 9*/        
                var<<=4;
                var|=t;
        }
        return var;
}
#define EXT_CSD_BOOT_CFG_EN	0x38
#define PUBLIC_KEY_BASE	      0xF000
#define PUBLIC_KEY_APP_OFS    0x300 
static int get_pubkey(unsigned char *key,unsigned int off)
{
	char *dev_name = "/dev/mmcblk0";
	char *boot_dev = "/dev/mmcblk0bootX";
	struct file *filp = NULL;
	struct inode *inode;
	struct block_device *bdev;
	struct gendisk *disk;
	struct mmc_blk_data *md;
	struct mmc_card *card; 

	u8 part_config;

	if(NULL == (filp = klib_fopen(dev_name,O_RDONLY,0)))
	{
		PERROR("Could not open %s at line %d\n",dev_name,__LINE__);
		return 0;
	}
	inode = filp->f_mapping->host;
	bdev  = inode->i_bdev;
	disk  = bdev->bd_disk;
	md    = (struct mmc_blk_data *)(disk->private_data);
	card  = md->queue.card;

	PDEBUG("%s ext_csd part_config = %x\n",dev_name,card->ext_csd.part_config);
	part_config = card->ext_csd.part_config;
	klib_fclose(filp);
	
	switch ((part_config & EXT_CSD_BOOT_CFG_EN)>>3)
	{
        	case 0x0:
                	PDEBUG(" Not boot enable,invalid!\n");  
                	break;        
        	case 0x1:
                	PDEBUG(" Boot Partition 1 enabled\n");
			boot_dev[16] = '0';
                	break;        
        	case 0x2:             
                	PDEBUG(" Boot Partition 2 enabled\n");
			boot_dev[16] = '1';
                	break;
        	case 0x7:
                	PDEBUG(" User Area Enabled for boot,invalid!\n");
                	break;
	}
	
	if(NULL == (filp = klib_fopen(boot_dev,O_RDONLY,0)))
	{
		PERROR("Could not open %s at line %d\n",boot_dev,__LINE__);
		return 0;
	}
	klib_fseek(filp,PUBLIC_KEY_BASE + off,0);
	klib_fread(key,256,filp); 		
	klib_fclose(filp);
	
	return 0;
}

static int do_change_workdir(void)
{
	char *s;
	char new_opt[25] = "mmcblk0.APP_X";
	char *p = new_opt;
	s = fw_getenv("opt_partition");
	if(!s)
	{
		PERROR( "%s :Could not get opt_partition from env,Exit\n",__func__);
		return -1;
	}
 	//s = mmcblk0.APP_A or mmcblk0.APP_B
 	(s[12]=='A')?(p[12]='B'):(p[12]='A');
	PERROR("new_opt: %s\n",new_opt);
	fw_env_write("opt_partition",new_opt);
	return 0;
}

static int do_authenticate(char *dev,int length,unsigned char *sig,unsigned char *key)
{
	int left_len = length;
	int ret;	
	sha2_context ctx;
	struct file *filp = NULL;
	int err;
	unsigned char sha_result[HASHLEN];
	unsigned char *buf = NULL;
	unsigned long read_len = 40*1024; //read 40K per time

	char E_buf[20] = "010001";
	rsa_context rsa;
	/*
 	 *	do hash
 	 */ 	 	 
	PERROR( "%s:do_hash--------imagesize=0x%x Bytes\n",dev,length);
	
	buf = (unsigned char *)vmalloc(read_len);
	if(!buf)
	{
		err = 1;
		goto FAIL;
	}	
	if(NULL == (filp = klib_fopen(dev,O_RDONLY,0)))
	{
		err = 2;
		PERROR("Could not open %s at line %d\n",dev,__LINE__);
		goto FAIL;
	}
	filp->f_pos = 0;

	sha2_starts(&ctx, 0); 
	while(left_len>0)
	{
		if(read_len > left_len)
			read_len = left_len;
		ret = klib_fread(buf,read_len,filp);
		if(!ret)
			break;
		sha2_update(&ctx,buf,read_len);
		left_len -= ret;
		msleep(5);
	}
	sha2_finish(&ctx,sha_result);

	/*
 	 *	RSA verify
 	 */
	rsa_init( &rsa, RSA_PKCS_V15, 0, NULL, NULL );
	mpi_read_binary(&rsa.N, key, KEY_LEN);
	mpi_read_string(&rsa.E, 16, E_buf);
	
	print_mem("-------- key -------", key, KEY_LEN);
	print_mem("-------- sig -------", sig, SIGNLEN);
	print_mem("-------- hash ------", sha_result, HASHLEN);
	
	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
	if( ( rsa_pkcs1_verify( &rsa, RSA_PUBLIC, RSA_SHA256,
                                HASHLEN, sha_result, sig) ) != 0 )
	{
		PERROR( "%s: rsa_pkcs1_verify failed \n",dev);
		rsa_free( &rsa );
		err = SECURITY_FAIL;
		goto FAIL;
	}
	
	PERROR( "%s: rsa_pkcs1_verify success\n",dev);
	rsa_free(&rsa);
	vfree(buf);

	klib_fclose(filp);
	return 0;

FAIL:
	vfree(buf);
	(filp==NULL)?0:klib_fclose(filp);
	
	return err;

}

struct workqueue_struct *security_workqueue;
struct delayed_work security_check;

static void verify_signature(struct work_struct *work)
{
	int err = 0;
	char envbuf[20];
	char *s = envbuf;

	unsigned char sig[SIGNLEN];
	unsigned char key[KEY_LEN];
	
	char dev_name[40];
	char size_env[40];
	unsigned int blocknum = 0;
	unsigned int length = 0;

	struct file *filp = NULL;	

	if(-1 == fw_env_open())
	{
		goto EXIT;
	}
	/*********************************
 	 * verify bootimg.
 	********************************/
	/*	
 	err = do_authentice(dev,length,sig,key);
	if(err)
		goto EXIT;
	*/

	/*********************************
 	 * verify workdir.
 	********************************/
	s = fw_getenv("opt_partition");
	if(!s)
	{
		PERROR( "%s :Could not get opt_partition from env,Exit\n",__func__);
		goto EXIT;
	}
	/*
 	 * s = mmcblk0.APP_A or mmcblk0.APP_B
 	 */
        sprintf(dev_name,"/dev/%s",s);
	sprintf(size_env,"APP_%c_size",s[12]);
	s = fw_getenv(size_env);
	if(!s)
	{
		PERROR( "%s :Could not get %s from env,Exit\n",__func__,size_env);
		goto EXIT;
	}

	blocknum = strtohex(s);
	blocknum = blocknum - 1; //reserved 1 block for signature and public key
	length = blocknum << 9;  //length = blocknum * 512	

	if(NULL == (filp = klib_fopen(dev_name,O_RDONLY,0)))
	{
		err = 2;
		PERROR("Could not open %s at line %d\n",dev_name,__LINE__);
		goto EXIT;
	}
	filp->f_pos = length;	//signature and public key locate in the end.
	klib_fread(sig,SIGNLEN,filp);		/*get sig*/
	klib_fclose(filp);
	
	get_pubkey(key,PUBLIC_KEY_APP_OFS);	/*get public key*/
	
	err = do_authenticate(dev_name,length,sig,key);
	if(err)
		goto EXIT;
	
	fw_env_close();	
	return;
EXIT:
	if(err == SECURITY_FAIL)
	{
		do_change_workdir();
		PERROR("\r\n############swtich OPT partition,SX6 system restart##########\r\n");
		mcu_send_rest();
		while(1); /* wait forever */
	}
	fw_env_close();	
	destroy_workqueue(security_workqueue);
	return;
}

static int __init secure_check_init(void)
{
	TURING_SETUP();
#ifdef CONFIG_PROC_FS
	proc_security_init();	
#endif
	if (!is_secure_enable())
	{
		PDEBUG( "secure is not enabled\n");	
		return 0;
	}
	security_workqueue = create_singlethread_workqueue("ksecure");
	INIT_DELAYED_WORK(&security_check,verify_signature);
	/*
 	 *  1.wait for filesystem ready
 	 *  2.let application run first.
	 */
	queue_delayed_work(security_workqueue,&security_check,15*HZ);
	
	return 0;
}
#ifndef SECURITY_BUILD_AS_MODULE
late_initcall(secure_check_init);
#else
static void __exit cleanup_secure_check(void)
{
	TURING_CLEANUP();
	return;
}
module_init(secure_check_init);
module_exit(cleanup_secure_check);
MODULE_DESCRIPTION("secure check");
MODULE_LICENSE("GPL");
#endif
