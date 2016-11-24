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
#include <mach/otp.h>

#include "./inc/rsa.h"
#include "./inc/sha2.h"
#include "./inc/bignum.h"
#include "./inc/bn_mul.h"
#include "./inc/fw_env.h"
#include "./inc/klib.h"

#include "./inc/mmc_priv.h"
#include "./inc/fw_core.h"


extern void trix_mcomm_send_reboot( void );

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

#ifdef CONFIG_PROC_FS
#include "proc_security.c"
#endif

static int  security_debug = 0;  /*0 = normal mode ,1 = force debug, 2 = force disable*/
#ifndef SECURITY_BUILD_AS_MODULE
static int __init trix_security_debug(char *str)
{
	security_debug = simple_strtol(str,NULL,0) & 0x3;
	return 1;
}
__setup("security_debug=", trix_security_debug);
#endif

static int is_secure_enable(void)
{
	int security_otp;

	security_otp = otp_get_security_boot_state();

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
#define PUBLIC_KEY_PREBOOT_OFS  0x0
#define PUBLIC_KEY_UBOOT_OFS    0x100
#define PUBLIC_KEY_KERNEL_OFS   0x200
#define PUBLIC_KEY_APP_OFS    0x300
static int get_pubkey(unsigned char *key,unsigned int off)
{
#if defined(CONFIG_SIGMA_DTV_ROM_MMC) || defined(CONFIG_SIGMA_DTV_ROM_SPI_MMC)
	char *dev_name = "/dev/mmcblk0";
	char *boot_dev = "/dev/mmcblk0bootX";
#elif defined(CONFIG_SIGMA_DTV_ROM_NAND)
	char *boot_dev="/dev/mtdblock_robbs0";
#else
	#error "Unknown ROM type!!"
#endif
	struct file *filp = NULL;

#if defined(CONFIG_SIGMA_DTV_ROM_MMC)
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
#endif	

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
	char *s = NULL;
	char new_opt[25] = { 0 };
	char *p = NULL;
	s = fw_getenv("opt_partition");
	if(!s)
	{
		PERROR( "%s :Could not get opt_partition from env,Exit\n",__func__);
		return -1;
	}
 	//s = mmcblk0.APP_A or mmcblk0.APP_B
 	strcpy(new_opt, s);
	s = strstr(s, "APP_");
	p = strstr(new_opt, "APP_");
 	(s[4]=='A')?(p[4]='B'):(p[4]='A');
	PERROR("new_opt: %s\n",new_opt);
	fw_env_write("opt_partition",new_opt);
	return 0;
}

int authenticate_buff(void *data, int len, void *key, void *sig)
{
	int err = -1;
	sha2_context ctx;
	rsa_context rsa;
	unsigned char sha_result[HASHLEN];

	if(!is_secure_enable())
	{
		PERROR( "secure is not enabled\n");
		return 0;
	}

	/*
	 *	do hash
	 */
	PERROR( "do hash at 0x%x,imagesize=0x%x Bytes\n",(int)data,len);
	sha2_starts(&ctx, 0);
	sha2_update(&ctx,data,len);
	sha2_finish(&ctx,sha_result);

	/*
	 *	RSA verify
	 */
	rsa_init( &rsa, RSA_PKCS_V15, 0, NULL, NULL );
	mpi_read_binary(&rsa.N, key, KEY_LEN);
	mpi_read_string(&rsa.E, 16,"010001"/*E_buf*/);

	print_mem("-------- public key -------", key, KEY_LEN);
	print_mem("-------- signature -------", sig, SIGNLEN);
	print_mem("-------- hash result ------", sha_result, HASHLEN);

	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
	if( ( rsa_pkcs1_verify( &rsa, RSA_PUBLIC, RSA_SHA256,
                                HASHLEN, sha_result, sig) ) != 0 )
	{
		PERROR( "rsa_pkcs1_verify failed \n");
		rsa_free( &rsa );
		err = SECURITY_FAIL;
		goto FAIL;
	}

	PERROR( "rsa_pkcs1_verify success\n");
	rsa_free(&rsa);

	return 0;

FAIL:
	return err;
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

static struct workqueue_struct *security_workqueue;
static struct delayed_work security_check;

#define for_each_vol(_vol_, _ctx_)			\
	list_for_each_entry(_vol_, &_ctx_->vols, list)

static int check_vol_signature(struct fw_ctx *ctx, struct fw_vol *vol)
{
	int ret = 0, len = 0;
	struct fw_part *part = NULL;
	char *sig =  NULL;
	char *key = NULL;
	void *data = NULL;
	char *dev = NULL;

	part = fw_vol_get_active_part(vol);

	if (!part) {
		ret = -ENODEV;
		goto fail;
	}

	sig = (char *)vmalloc(SIGNLEN);
	if (!sig) {
		ret = -ENOMEM;
		goto fail;
	}

	key = (char *)vmalloc(KEY_LEN);
	if (!key) {
		ret = -ENOMEM;
		goto fail1;
	}

	/* Verify part info signature */
	memcpy((void *)sig, (void *)&part->info->info_sig[0], SIGNLEN);
	get_pubkey(key, PUBLIC_KEY_PREBOOT_OFS);
	data = &part->info->need_sig;
	len = (int)((unsigned long)&part->info->info_sig[0] -
				(unsigned long)&part->info->need_sig - 1);

	ret = authenticate_buff(data, len, key, sig);
	if (ret) {
		PERROR("\r\n############## part(%s) info signature verify failed ##########\r\n", part->info->name);
		goto fail2;
	}

	/* Do not need signature check */
	if (!part->info->need_sig) {
		ret = 0;
		goto fail2;
	}

	dev = volume_name_to_dev(ctx, &vol->info->name[0], FW_VOL_CONTENT_INUSE);
	memcpy((void *)sig, (void *)&part->info->part_sig[0], SIGNLEN);
	get_pubkey(key, PUBLIC_KEY_PREBOOT_OFS);

	ret = do_authenticate(dev, part->info->valid_sz, sig, key);

fail2:
	vfree(key);
fail1:
	vfree(sig);
fail:
	return ret;
}

static void volume_switch_part(struct fw_vol *vol)
{
	struct fw_part *act_part = fw_vol_get_active_part(vol);
	struct fw_part *inact_part = fw_vol_get_inactive_part(vol);

	/* Impossible */
	if (!act_part || !inact_part) {
		dump_stack();
		return;
	}

	PERROR("\r\n############switch part %s -> %s##########\r\n", act_part->info->name, inact_part->info->name);

	fw_vol_set_active_part(vol, inact_part);
	return;
}

static void sigchk_with_firmware_info(struct fw_ctx *ctx)
{
	int ret = 0, status = 0;
	struct fw_vol *vol = NULL;

	for_each_vol(vol, ctx) {
		if(strncmp(&vol->info->fs[0], "squashfs", strlen("squashfs"))) {
			continue;
		}
		ret = check_vol_signature(ctx, vol);

		if (ret == SECURITY_FAIL) {
			PERROR("\r\n############voume(%s) signature verify failed##########\r\n", vol->info->name);
			status++;
			volume_switch_part(vol);

		}
	}

	if (!status)
		return;


	PERROR("\r\n############Signature check failed!, system restart##########\r\n");
	fw_ctx_save_to_flash(ctx);
	emergency_sync();
	trix_mcomm_send_reboot();
	while(1); /* wait forever */
}

static void verify_signature(struct work_struct *work)
{
	int err = 0;
	char envbuf[40] = { 0 };
	char *s = envbuf;
	char *p = NULL;
	struct fw_ctx *ctx = NULL;

	unsigned char sig[SIGNLEN];
	unsigned char key[KEY_LEN];

	char dev_name[40] = { 0 };
	char size_env[40] = { 0 };
	unsigned int blocknum = 0;
	unsigned int length = 0;

	struct file *filp = NULL;

	if(-1 == fw_env_open())
	{
		goto EXIT;
	}

	ctx = fw_get_board_ctx();

	if (ctx) {
		sigchk_with_firmware_info(ctx);
		return;
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
 	 * eMMC:
 	 * s = mmcblk0.APP_A or mmcblk0.APP_B
 	 * NAND:
 	 * s = mtdblock.APP_A or mtdblock.APP_B
 	 */
        sprintf(dev_name,"/dev/%s",s);
	p = strstr(s, "APP_");
	sprintf(size_env,"APP_%c_size", p[4]);
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
		PERROR("\r\n############swtich OPT partition, system restart##########\r\n");
		trix_mcomm_send_reboot();
		while(1); /* wait forever */
	}
	fw_env_close();
	destroy_workqueue(security_workqueue);
	return;
}

static int __init secure_check_init(void)
{
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
	return;
}
module_init(secure_check_init);
module_exit(cleanup_secure_check);
MODULE_DESCRIPTION("secure check");
MODULE_LICENSE("GPL");
#endif
