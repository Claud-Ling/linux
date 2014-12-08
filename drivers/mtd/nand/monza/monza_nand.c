#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/err.h>        
#include <linux/slab.h>
#include <linux/clk.h>        
#include <linux/interrupt.h>  
#include <linux/sched.h>      
#include <linux/ctype.h>      
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <linux/mtd/mtd.h>    
#include <linux/mtd/nand.h>   
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/dma-mapping.h>

#include <asm/io.h>
#include <asm/setup.h> 
#include "../../mtdcore.h"
#include "monza_nand.h"
#include "monza_ecc.h"

#define TRIX_NAND_DEV_NAME "trix-nand"

#ifdef CONFIG_SIGMA_SOC_SX7   
#define TURING_REG_BASE (0xF1040000)
#define TURING_REG_SIZE         SZ_8K   /*actually turing reg space is 128k, but 8k is enough for use here*/
static void __iomem* turing_reg_vbase = NULL;                                                                                       
# define TURING_SETUP() do{                                                                     \
                                turing_reg_vbase = ioremap(TURING_REG_BASE, TURING_REG_SIZE);   \
                                if (!turing_reg_vbase)                                          \
                                        pr_err("turing iomap failed!\n");                       \
                        }while(0)
# define TURING_CLEANUP() do{                                                                   \
                                if (turing_reg_vbase) iounmap(turing_reg_vbase);                \
                                turing_reg_vbase = NULL;                                        \
                        }while(0)
# define TURING_READL(a) ({     								\
			   	long _ret = 0;							\
                                if (turing_reg_vbase && ((a) > 0 && (a) < TURING_REG_SIZE))    	\
                                        _ret = readl((volatile void*)(turing_reg_vbase + (a))); \
                                else                                                            \
                                        pr_warn("failed to read turing reg:%x\n", (a));         \
                                _ret;                                                           \
                        })    
#define OTP_FC2 (0x110C)        
#define BIT_OTP_FC2_NANDCtrlSel  12
/* 0: old NAND controller; 1: new NAND controlle*/
#define otp_NandCtrlSel() ((TURING_READL(OTP_FC2) >> BIT_OTP_FC2_NANDCtrlSel)&0x1)                                                  
#endif 

#define FLASH_SUPPORT_RNDOUT_READ

#if defined(CONFIG_MTD_HIDTV_PARTS)
extern char *saved_command_line;
static char mtd_cmd_line[COMMAND_LINE_SIZE];
#elif defined(CONFIG_MTD_CMDLINE_PARTS)
static const char *part_probes[] = { "cmdlinepart", NULL };
#else
static struct mtd_partition trix_partitions[] = {
	{
		name	:	TRIX_NAND_DEV_NAME,
		offset	:	0,
		size	:	MTDPART_SIZ_FULL
	}
};
#endif

/*
 * Module stuff
 */
static char ecc_tmp_read[250]  __attribute__ ((aligned (4))); 
static char ecc_tmp_write[250] __attribute__ ((aligned (4)));

/* for 2KB page, 64OOB, we use 8bit ECC*/
static struct nand_ecclayout nand_oob_bch_gf14_64_8 = {
        .eccbytes = 56,
        .eccpos = {
                8,9,10,11,12,13,14,15,16,17,18,19,20,21,
                22,23,24,25,26,27,28,29,30,31,32,33,34,35,
                36,37,38,39,40,41,42,43,44,45,46,47,48,49,
                50,51,52,53,54,55,56,57,58,59,60,61,62,63 },                                                                        
        .oobfree = {
                {.offset = 3, 
                 .length = 5}}
};

/*for 4k page 224OOB ,we use 24bit ECC*/
static struct nand_ecclayout nand_oob_bch_gf14_224_24 = {
    .eccbytes = 168,
    .eccpos = {
         56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,
         67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
         78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,
         89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
        100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
        111,
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
        123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
        134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
        145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
        156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
        167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
        178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
        189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
        222, 223},
        .oobfree = {
            {.offset = 3,
             .length = 53}}
};
/*for 8K page 448OOB, we use 24bit ECC*/
static struct nand_ecclayout nand_oob_bch_gf14_448_24 = {
        .eccbytes = 336,//24*7/4 *8,
        .eccpos = {
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
        123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
        134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
        145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
        156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
        167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
        178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
        189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
        222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232,
        233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243,
        244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
        255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265,
        266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276,
        277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287,
        288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298,
        299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
        310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320,
        321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331,
        332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342,
        343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353,
        354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364,
        365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375,
        376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386,
        387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397,
        398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408,
        409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419,
        420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430,
        431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441,
        442, 443, 444, 445, 446, 447 },
        .oobfree = {
                {.offset = 3,
                 .length = 109}}
};
/***********************************************************
 *nand oob layout info for SX7 new nand controller
 * *********************************************************/
/* for 2KB page, 64OOB, we use 8bit ECC*/
static struct nand_ecclayout nand_oob_bch_gf15_64_8 = {
        .eccbytes = 60,
        .eccpos = {
		4,5,6,7,
                8,9,10,11,12,13,14,15,16,17,18,19,20,21,
                22,23,24,25,26,27,28,29,30,31,32,33,34,35,
                36,37,38,39,40,41,42,43,44,45,46,47,48,49,
                50,51,52,53,54,55,56,57,58,59,60,61,62,63 },
        .oobfree = {
                {.offset = 3,
                 .length = 1}}
};

/*for 4k page 224OOB ,we use 24bit ECC*/
static struct nand_ecclayout nand_oob_bch_gf15_224_24 = {
    .eccbytes = 180,
    .eccpos = {
	 44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,55,
         56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,
         67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
         78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,
         89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
        100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
        111,
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
        123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
        134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
        145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
        156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
        167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
        178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
        189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
        222, 223},
        .oobfree = {
            {.offset = 3,
             .length = 41}}
};
/*for 8K page 448OOB, we use 24bit ECC*/
static struct nand_ecclayout nand_oob_bch_gf15_448_24 = {
        .eccbytes = 360,
        .eccpos = {
	88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,
	104,105,106,107,108,109,110,111,
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
        123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
        134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
        145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
        156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
        167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
        178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
        189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
        222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232,
        233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243,
        244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
        255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265,
        266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276,
        277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287,
        288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298,
        299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
        310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320,
        321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331,
        332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342,
        343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353,
        354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364,
        365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375,
        376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386,
        387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397,
        398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408,
        409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419,
        420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430,
        431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441,
        442, 443, 444, 445, 446, 447 },
        .oobfree = {
                {.offset = 3,
                 .length = 109}}
};


static void enable_nce(struct monza_nand_info *info ,int enabled)
{
	unsigned int val;
	
	val = nand_readl(info,NAND_BOOTROM_R_TIMG);
	if(!enabled){
		nand_writel(info, NAND_BOOTROM_R_TIMG, val | 0x80);
	}
	else{
		nand_writel(info, NAND_BOOTROM_R_TIMG, val & ~0x80);
	}
}

static void monza_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct monza_nand_info *info = container_of(mtd,
					struct monza_nand_info, mtd);
	static unsigned long IO_ADDR_W = 0;
	
	switch (ctrl) {
	case NAND_CTRL_CHANGE:
		enable_nce(info, 0);
		break;
	
	default:
		enable_nce(info, 1);

		if(ctrl & NAND_CTRL_CHANGE)
		{
			if(ctrl == NAND_NCE)
				IO_ADDR_W = ((unsigned long)info->nand.IO_ADDR_W);
			else if(ctrl & NAND_ALE)
				IO_ADDR_W = ((unsigned long)info->nand.IO_ADDR_W) + (1 << 14);
			else if(ctrl & NAND_CLE)
				IO_ADDR_W = ((unsigned long)info->nand.IO_ADDR_W) + (1 << 13);
			else
				IO_ADDR_W = ((unsigned long)info->nand.IO_ADDR_W);
		}
		
		break;
	}
	
	if (cmd != NAND_CMD_NONE)
	{
		WriteRegByte((void *)IO_ADDR_W, cmd);
	}
}

static int monza_nand_ready(struct mtd_info *mtd)
{
	struct monza_nand_info *info = container_of(mtd,
					struct monza_nand_info, mtd);

	return (nand_readl(info,NAND_BOOTROM_R_TIMG) & 0x40) != 0;
}
/*
 * set nand controller work mode
 * 	mode: NAND_HW_MODE / NAND_SW_MODE
 */
void set_nand_mode(struct monza_nand_info *info,int mode)
{
	if( likely(MONZA_NAND_NEW == info->nand_ctrler) )
		MWriteRegWord((void *)(info->host_base + NAND_CCR), mode<<24, 0xFF000000);
	else if( MONZA_NAND_OLD == info->nand_ctrler)
		MWriteRegWord((void *)info->host_base + NAND_DCR, mode<<24, 0xFF000000);
}

/* Allocate the DMA buffers */ 
static int monza_nand_dma_init(struct monza_nand_info *info)
{
	if( MONZA_NAND_PIO == info->xfer_type )
		return 0;

        info->data_buff_virt     = dma_alloc_coherent(NULL, DMAUNIT,
                                       &info->data_buff_phys, GFP_KERNEL); 
        
	info->dma_int_descr_virt = dma_alloc_coherent(NULL, 4,
                                       &info->dma_int_descr_phys, GFP_KERNEL);


	if( ( NULL ==  info->data_buff_virt ) || ( NULL ==  info->dma_int_descr_virt ) ) {
		printk("dma alloc fail, free resource\n");
		goto out_free_mem ;
	}
	pr_debug("data_buff_virt:%08x data_buff_phys: %08x\n",\
				(unsigned int)info->data_buff_virt, info->data_buff_phys);
	pr_debug("dma_int_descr_virt:%08x dma_int_descr_phys: %08x\n",\
				(unsigned int)info->dma_int_descr_virt, info->dma_int_descr_phys);
	memset(info->data_buff_virt, 0, DMAUNIT);
	memset((unsigned char *)(info->dma_int_descr_virt), 0, 4);
	
	return 0;
out_free_mem:
	if( info->data_buff_virt != NULL ) {
		dma_free_coherent( NULL,
				DMAUNIT,
				info->data_buff_virt,
				info->data_buff_phys ) ;
	}

	if( info->dma_int_descr_virt != NULL ) {
		dma_free_coherent( NULL,
				4,
				info->dma_int_descr_virt,
				info->dma_int_descr_phys ) ;
	}
	
	/*Force to PIO mode*/
	info->xfer_type = MONZA_NAND_PIO;

	return -ENOMEM;
}


void hidtv_nand_inithw(struct monza_nand_info *info)
{
	unsigned char val8;
	unsigned int val32;
        
	/* switch the nand source clock from 100Hz to 200Hz for this version of NAND mode setting
        * if the souce clock is not match the NAND mode, will cause the DMA read time out error */

        val8 = ReadRegByte((void*)0xf500e849);
        val8 &= 0xcf;
        val8 |= 0x20;
        WriteRegByte((void*)0xf500e849,val8);
	
        /* switch the nand source clock 200Hz clock for write, 
        if the clock is not match the NAND init setting such like 1b002004 = 0x21, will cause WP*/
        nand_writel(info, NAND_FCR, 0x00000067);

        val32 = nand_readl(info, NAND_MLC_ECC_CTRL);
        val32 |= 0x100;
        nand_writel(info, NAND_MLC_ECC_CTRL,val32);

        val32 = nand_readl(info, NAND_SPI_DLY_CTRL);
        val32 &= 0xffff3f3f;
        nand_writel(info, NAND_SPI_DLY_CTRL,val32);

        nand_writel(info, NAND_TIMG, 0x40193333);

	/* ECC control reset */	
	nand_writel(info, NAND_ECC_CTRL, (nand_readl(info, NAND_ECC_CTRL)&(~0x4)));
	nand_writel(info, NAND_ECC_CTRL, 0x1000);
	while((nand_readl(info, NAND_ECC_CTRL)&0x1000)!=0);                      
	nand_writel(info, NAND_ECC_CTRL, 0x02); 

	nand_writel(info, NAND_DCR , 0x00000002);
	nand_writel(info, NAND_CTRL, 0x00000420);
	set_nand_mode(info, NAND_SW_MODE);
	return;
}

#ifdef CONFIG_MTD_HIDTV_PARTS
#define MTD_MAX_COUNT 10
static const char MTD_NAME[] = "MTD_NAME=";

static unsigned long long size_parse (char *ptr, char **retptr)
{
	unsigned long long ret = simple_strtoull (ptr, retptr, 16);

	switch (**retptr) {
	case 'G':
	case 'g':
		ret <<= 10;
	case 'M':
	case 'm':
		ret <<= 10;
	case 'K':
	case 'k':
		ret <<= 10;
		(*retptr)++;
	default:
		break;
	}
	return ret;
}

// The format for the command line is as follows:
// mtd_parts	:= partdef[ partdef]
// partdef		:= MTD_NAME=name,start,offset
// name			:= <a-z>[a-z,0-9]
// start		:= standard linux memsize
// offset		:= standard linux memsize
// 
// example: MTD_NAME=KERNEL,0,6M MTD_NAME=CRAMFS,6M,8M MTD_NAME=EXT2,eM,2M
static inline int parse_mtd_cmdline(struct mtd_info *mtd)
{
	char *p, *from = mtd_cmd_line;
	int mtd_count = 0;
	struct mtd_partition mtd_part[MTD_MAX_COUNT];

	memset(mtd_part, 0, sizeof(mtd_part));
	pr_debug("%s:%d saved_command_line:%s\n", __func__, __LINE__, saved_command_line);
	strncpy(mtd_cmd_line, saved_command_line, sizeof mtd_cmd_line);
	mtd_cmd_line[sizeof mtd_cmd_line - 1] = 0;

	
	while((from < mtd_cmd_line + COMMAND_LINE_SIZE) && 
			((from = strstr(from, MTD_NAME)) != NULL))
	{
		if(mtd_count >= MTD_MAX_COUNT)
			break;
		if(!(p = strchr(from, ',')))
			break;

		*p = '\0';
		mtd_part[mtd_count].name = from + sizeof(MTD_NAME) - 1;
		mtd_part[mtd_count].offset = size_parse(p + 1, &p);
		mtd_part[mtd_count].size = size_parse(p + 1, &p);
		mtd_count++;
		from = p + 1;
	}

	if(mtd_count > 0){
		printk(KERN_NOTICE
		       "Using partition definition from KERNEL command line parameters\n");
		add_mtd_partitions(mtd, mtd_part, mtd_count);
	}

	return mtd_count;
}

static int hidtv_nand_add_partition(struct monza_nand_info *info)
{	
	int nr_parts = 0;
	nr_parts = parse_mtd_cmdline(&(info->mtd));
	return nr_parts;
}
#endif

/*PIO Mode*/
static void monza_pio_read_buf32(struct mtd_info *mtd, uint8_t *buf, int len)
{
        int i;
        struct nand_chip *chip = mtd->priv;
        u32 *p = (u32 *) buf; 
	
	for (i = 0; i < (len >> 2); i++)              
		p[i] = ReadRegWord(chip->IO_ADDR_R);

}

static void monza_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
        struct nand_chip *chip = mtd->priv;
        register volatile void* hwaddr = chip->IO_ADDR_W;
        register u32 *p = (u32 *) buf;                                                                                              

        len >>= 2;
        if (len == 0)         
                return;       
        do {
                WriteRegWord(hwaddr,*p++);
        } while(--len);

}

/*
 * @param: is_read - 0 DMA read;1 DMA write
 * @return: 0 success;others fail.
 *
 * NOTE:dma write not support so far,design for future.
 */
static int monza_nand_dma_op(struct mtd_info *mtd, void *buf, int len,
                               int is_read)
{
	struct monza_nand_info *info = container_of(mtd,
					struct monza_nand_info, mtd);
        struct nand_chip *chip       = mtd->priv;
	
	int err = 0;
	dma_addr_t dma_src_addr, dma_dst_addr;
	unsigned short val16;
	unsigned int prev,cur;
	unsigned long dma_timeout = jiffies + MONZA_DMA_TRANS_TIMEOUT;
	
	if (is_read) {
		dma_src_addr = (dma_addr_t)chip->IO_ADDR_R;  
		dma_dst_addr = info->data_buff_phys;
	} else {
		dma_src_addr = info->data_buff_phys;      
		dma_dst_addr = (dma_addr_t)chip->IO_ADDR_W;
	}
	/*set up dma*/
	nand_writel(info, NAND_FDMA_SRC_ADDR ,dma_src_addr);
	nand_writel(info, NAND_FDMA_DEST_ADDR,dma_dst_addr);
	nand_writel(info, NAND_FDMA_INT_ADDR ,info->dma_int_descr_phys);
	nand_writel(info, NAND_FDMA_LEN_ADDR ,(len - 1) & 0xFFFF );
	
	prev = readl_relaxed(info->dma_int_descr_virt);
	
	/* enable flash DMA read access */
	val16 = ReadRegHWord((void *)0xfb000006);
	WriteRegHWord((void *)0xfb000006, val16 |0x1);
        do{
		/*
 		 * readl_relaxed here is a must!
 		 * it guarantee that memory in ddr is strongly order accessed.
 		 */
		cur = readl_relaxed(info->dma_int_descr_virt);
		if(cur > prev ){
			/*dma complete*/
			break;
		}
        }while (!time_after_eq(jiffies, dma_timeout));

        if (time_after_eq(jiffies, dma_timeout))
        {
                printk("warning:%s, DMA not ready.prev:%08x. current: %08x, xfer_type force to PIO mode\n",__func__,prev,cur);
		/* 
 		 * DMA fatal,force to PIO mode. 
		 */
		err = -ETIMEDOUT;
		info->xfer_type = MONZA_NAND_PIO;
	}

	/* disable flash DMA read access */  
	WriteRegHWord((void *)0xfb000006, val16 & 0xfffe);
	memcpy(buf, info->data_buff_virt, len);	
	return err;
}

static void monza_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct monza_nand_info *info = container_of(mtd,
					struct monza_nand_info, mtd);
	unsigned long nand_reg = 0x0; 
        
        if(likely(MONZA_NAND_NEW == info->nand_ctrler))
                nand_reg = NAND_CCR;   
        else if( MONZA_NAND_OLD == info->nand_ctrler)
                nand_reg = NAND_CTRL;  
        set_nand_mode(info, NAND_HW_MODE);      /*set HC as hw mode to fast read*/
        MWriteRegWord((void *)(info->host_base + NAND_CTRL), len<<16, 0xFFFF0000);
        MWriteRegWord((void *)(info->host_base + nand_reg), 1, 0x1);
	
	/* only use DMA for bigger than oob size: better performances */	
	if( (MONZA_NAND_DMA == info->xfer_type) && (len > mtd->oobsize) )
	{
		if(monza_nand_dma_op(mtd, buf, len, 1) == 0)
			goto xfer_complete;
		else
			goto xfer_err;
	}
	monza_pio_read_buf32(mtd, buf ,len);	

xfer_complete:
        set_nand_mode(info, NAND_SW_MODE);      /*must exit hw mode*/
        MWriteRegWord((void *)(info->host_base + NAND_CTRL), 0<<16, 0xFFFF0000);
        MWriteRegWord((void *)(info->host_base + nand_reg), 0, 0x1);
	return;

xfer_err:
	/*
	 * fatal during dma transfer
	 * xfer_type will be force to MONZA_NAND_PIO from now.
	 */ 
	set_nand_mode(info, NAND_SW_MODE);
	nand_writel(info, NAND_CTRL, 0x00000420);
	monza_pio_read_buf32(mtd, buf ,len);
	return;	
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int monza_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)                                                     
{
        int i;                
        struct nand_chip *chip = mtd->priv;
        u32 *p = (u32 *) buf; 

        len >>= 2;
        for (i = 0; i < len; i++)
                if (p[i] != ReadRegWord(chip->IO_ADDR_R))
                        return -EFAULT;

        return 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int monza_read_page_bch(struct mtd_info *mtd, struct nand_chip *chip,
		uint8_t *buf,int page_addr)
#else
static int monza_read_page_bch(struct mtd_info *mtd, struct nand_chip *chip,
		uint8_t *buf, int oob_required, int page_addr)
#endif
{
	int i,  eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	u_char *p = buf;
	u_char *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	int stat = 0;
	
	/* Read the OOB area first */
	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page_addr);//read oob first

	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

#ifdef FLASH_SUPPORT_RNDOUT_READ
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, 0x00, -1);
#else
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page_addr);
#endif
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
	{
		chip->ecc.hwctl(mtd, NAND_ECC_READ|(i<<8));
		chip->read_buf(mtd, p, eccsize);
		stat = chip->ecc.correct(mtd, p, NULL, NULL); 
		if(stat)
			break;
	}

	return stat;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static void monza_write_page_bch(struct mtd_info *mtd, struct nand_chip *chip,
							const uint8_t *buf)
#else
static int monza_write_page_bch(struct mtd_info *mtd, struct nand_chip *chip,const uint8_t *buf, int oob_required)

#endif
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	u_char *ecc_calc = chip->buffers->ecccalc;
	int *eccpos = chip->ecc.layout->eccpos;
	const uint8_t *p = buf;
	int stat = 0;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		stat = chip->ecc.calculate(mtd, p, (uint8_t *)ecc_tmp_write);
		if(stat)
		{
			printk("BCH: write page error %d\n",stat);
			goto exit;
		}
		memcpy(&ecc_calc[i] ,ecc_tmp_write,chip->ecc.bytes);
	}

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
exit:	
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	return;
#else
	return 0;
#endif
}

/* stubs for ECC functions not used by the NAND core */
static int monza_ecc_calculate(struct mtd_info *mtd, const uint8_t *data,
                                uint8_t *ecc_code)             
{
	int stat;
	struct nand_chip *chip = mtd->priv;
	
	stat = bch_encode_end((unsigned int*)ecc_code,chip);
        
	return stat;
}

static int monza_ecc_correct(struct mtd_info *mtd, uint8_t *data,
                                uint8_t *read_ecc, uint8_t *calc_ecc)
{
	int stat = 0;
	struct nand_chip *chip = mtd->priv;
	struct monza_nand_info *info = container_of(mtd,
					struct monza_nand_info, mtd);
        
	if( likely(MONZA_NAND_NEW == info->nand_ctrler) )
                stat = bch_decode_end_correct_new(mtd,data,chip); 
        else if( MONZA_NAND_OLD == info->nand_ctrler) 
                stat = bch_decode_end_correct_old(mtd,data,chip);
	
	return stat;
}

static void monza_ecc_hwctl(struct mtd_info *mtd, int param)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t *ecc_code = chip->buffers->ecccode; 
	
	int mode = param & 0xFF;
	int offset = (param >> 8) & 0xFF;

	switch(mode)
	{
	case NAND_ECC_READ:
		memcpy(ecc_tmp_read,&ecc_code[offset],chip->ecc.bytes);
		bch_decode_start((unsigned int*)ecc_tmp_read,chip);
		break;
	case NAND_ECC_WRITE:
		bch_encode_start(chip);
		break;
	}

	return;
}

static void monza_set_nand_ctrler(struct monza_nand_info *info)
{
#ifdef CONFIG_SIGMA_SOC_SX7
	TURING_SETUP();
	info->nand_ctrler = otp_NandCtrlSel()?MONZA_NAND_NEW:MONZA_NAND_OLD;
	TURING_CLEANUP();
#else
	info->nand_ctrler = MONZA_NAND_OLD;
#endif	
	printk("Nand ctrler type: %d\n",info->nand_ctrler);
	return;
}

static int monza_nand_probe(struct platform_device *pdev)
{
	struct monza_nand_info           *info;
	struct monza_nand_platform_data  *pdata;
	int err = 0;	

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data defined\n");
		err =  -ENODEV;
		goto exit;
	}

	info = kzalloc(sizeof(struct monza_nand_info), GFP_KERNEL); 
	if(!info) {
		err =  -ENOMEM;
		goto exit;
	}
	platform_set_drvdata(pdev, info);

	monza_set_nand_ctrler(info);
	info->host_base  = pdata->host_base;
	info->io_base    = pdata->io_base;
	info->xfer_type  = pdata->xfer_type;
	
	memset((char *)&(info->mtd), 0, sizeof(struct mtd_info));
	memset((char *)&(info->nand), 0, sizeof(struct nand_chip));
	
	/*basic hardware initialisation*/
	hidtv_nand_inithw(info);
	monza_nand_dma_init(info);
	info->mtd.priv       = &(info->nand);	
	info->mtd.owner      = THIS_MODULE;	
	info->mtd.name       = TRIX_NAND_DEV_NAME;

	info->nand.IO_ADDR_R = info->io_base;
	info->nand.IO_ADDR_W = info->io_base;
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	info->nand.options     = NAND_NO_AUTOINCR/*NAND_SKIP_BBTSCAN|NAND_BBT_USE_FLASH|NAND_BBT_NO_OOB*/;
#endif
	//info->nand.bbt_options = NAND_BBT_USE_FLASH|NAND_BBT_NO_OOB;

	info->nand.chip_delay  = 1000; //fix me
	info->nand.cmd_ctrl    = monza_nand_hwcontrol;	
	info->nand.dev_ready   = monza_nand_ready;
	
	info->nand.write_buf   = monza_write_buf;
	info->nand.read_buf    = monza_read_buf;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	info->nand.verify_buf  = monza_verify_buf;
#endif
	info->nand.ecc.mode     = NAND_ECC_HW; 	
	if (nand_scan_ident(&(info->mtd), 1, NULL)) { 
		err =  -ENXIO;
		goto exit;
	}
	
	info->nand.options |= NAND_NO_SUBPAGE_WRITE;	/*we not support subpage in UBI*/
	info->mtd.subpage_sft = 0; 
	
	switch (info->mtd.writesize)
	{
		case 2048:
			info->nand.ecc.strength = 8;
			info->nand.ecc.size     = 512;
			info->nand.ecc.layout	= info->nand_ctrler?&nand_oob_bch_gf15_64_8:&nand_oob_bch_gf14_64_8;
			break;
		case 4096:
			info->nand.ecc.strength = 24;
			info->nand.ecc.size     = 1024;
			info->nand.ecc.layout	= info->nand_ctrler?&nand_oob_bch_gf15_224_24:&nand_oob_bch_gf14_224_24;
			break;
		case 8192:
			info->nand.ecc.strength = 24;
			info->nand.ecc.size     = 1024;
			info->nand.ecc.layout	= info->nand_ctrler?&nand_oob_bch_gf15_448_24:&nand_oob_bch_gf14_448_24;
			break;
		default:
			printk("ERROR: unsupport NAND device.\n");
			return -ENXIO;
	}

        /*ECC bytes per step*/
        if( likely(MONZA_NAND_NEW == info->nand_ctrler) )
                /*BCH GF 15*/
                info->nand.ecc.bytes = (15*info->nand.ecc.strength + 7)>>3;
        else if( MONZA_NAND_OLD == info->nand_ctrler)
                /*BCH BF 14*/
                info->nand.ecc.bytes = (14*info->nand.ecc.strength + 7)>>3;
	/*
 	 * sanity checks 
 	 * NOTE:it's important!
 	 */
	BUG_ON((info->nand.ecc.size != 512)&&(info->nand.ecc.size != 1024));	
	BUG_ON((info->nand.ecc.strength < 4)||(info->nand.ecc.strength > 60)||(info->nand.ecc.strength % 2 ));

	if (!info->nand.ecc.read_page)
		info->nand.ecc.read_page = monza_read_page_bch;
	if (!info->nand.ecc.write_page)
		info->nand.ecc.write_page = monza_write_page_bch;
	
	info->nand.ecc.calculate = monza_ecc_calculate;
	info->nand.ecc.hwctl     = monza_ecc_hwctl;
	info->nand.ecc.correct   = monza_ecc_correct;
	
	/* second phase scan */
	if (nand_scan_tail(&(info->mtd))) { 
		err =  -ENXIO;
		goto exit;
	}

	printk("Detected NAND, %lldMiB, erasesize %dKiB, pagesize %dB,oobsize %dB, \
	oobavail %dB\n",
	info->mtd.size >> 20, 
	info->mtd.erasesize >> 10, 
	info->mtd.writesize,
	info->mtd.oobsize, 
	info->mtd.oobavail);

	printk("Nand ECC info:Strength=%d,ecc size=%d ecc_bytes=%d\n",
	info->nand.ecc.strength,
	info->nand.ecc.size,
	info->nand.ecc.bytes);

#ifdef CONFIG_MTD_HIDTV_PARTS
	hidtv_nand_add_partition(info);
#elif defined(CONFIG_MTD_CMDLINE_PARTS)
	mtd_device_parse_register(&info->mtd, part_probes, NULL, NULL, 0);
#else
	mtd_device_register(&info->mtd, trix_partitions, 1);	/*static*/
#endif
exit:
	return err;	
}
static int monza_nand_remove(struct platform_device *pdev)
{
	struct monza_nand_info *info = platform_get_drvdata(pdev);
	/* Unregister the device */
	del_mtd_device(&(info->mtd));

	kfree(info);
	
	return 0;
}

#ifdef CONFIG_PM
static int monza_nand_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int monza_nand_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define monza_nand_suspend     NULL
#define monza_nand_resume      NULL
#endif

static struct platform_driver monza_nand_driver = {
        .driver = {
                .name   = "monza-nand",       
        },
        .probe          = monza_nand_probe,
        .remove         = monza_nand_remove,
        .suspend        = monza_nand_suspend,         
        .resume         = monza_nand_resume,
};

module_platform_driver(monza_nand_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SigmaDesigns kernel team");
MODULE_DESCRIPTION("driver for NAND on HiDTV board");

