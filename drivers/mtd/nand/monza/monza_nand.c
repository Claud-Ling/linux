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

#define FLASH_SUPPORT_RNDOUT_READ
/*
 * MTD structure for HiDTV board
 */
static struct mtd_info hidtv_nand_mtd;
static struct nand_chip hidtv_nand_chip;

#define MONZA_NAND_IO_ADDR	CONFIG_MTD_NAND_IO_SPACE	/* Start of HIDTV NAND IO address space */

extern char *saved_command_line;
static char mtd_cmd_line[COMMAND_LINE_SIZE];

/*
 * Module stuff
 */

/*
 * Define partitions for flash device
 */
struct mtd_partition nand_physical_partition_info[] = {
	{
		name:"NAND",
		offset:0,
		size:256*1024*1024
	}
};

/* for 2KB page, typically with 64B OOB */ 
static struct nand_ecclayout nand_oob_mlcbch_64 = {
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

/*for 4k page*/
static struct nand_ecclayout nand_oob_mlcbch_224 = {
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
/*for 8K page*/
static struct nand_ecclayout nand_oob_mlcbch_448 = {
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


static void enable_nce(int enabled)
{
	unsigned int val;
	
	val = ReadRegWord((u32 __iomem *)0xfb002000);
	if(!enabled){
		WriteRegWord((u32 __iomem *)0xfb002000, val | 0x80);
	}
	else{
		WriteRegWord((u32 __iomem *)0xfb002000, val & ~0x80);
	}
}

static void monza_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *nand_chip = (struct nand_chip *) (mtd->priv);
	static unsigned long IO_ADDR_W = 0;
	
	switch (ctrl) {
	case NAND_CTRL_CHANGE:
		enable_nce(0);
		break;
	
	default:
		enable_nce(1);

		if(ctrl & NAND_CTRL_CHANGE)
		{
			if(ctrl == NAND_NCE)
				IO_ADDR_W = ((unsigned long)nand_chip->IO_ADDR_W);
			else if(ctrl & NAND_ALE)
				IO_ADDR_W = ((unsigned long)nand_chip->IO_ADDR_W) + (1 << 14);
			else if(ctrl & NAND_CLE)
				IO_ADDR_W = ((unsigned long)nand_chip->IO_ADDR_W) + (1 << 13);
			else
				IO_ADDR_W = ((unsigned long)nand_chip->IO_ADDR_W);
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
	return (ReadRegWord((void*)NAND_BOOTROM_R_TIMG) & 0x40) != 0;
}
/*
 * set nand controller work mode
 * 	mode: NAND_HW_MODE / NAND_SW_MODE
 */
void set_nand_mode(int mode)
{
	WriteRegWord((void *)NAND_DCR, (mode<<24)|0x00000002); 

}

void hidtv_nand_inithw(void)
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
        WriteRegWord((void*)NAND_FCR, 0x00000067);

        val32 = ReadRegWord((void*)NAND_MLC_ECC_CTRL);
        val32 |= 0x100;
        WriteRegWord((void*)NAND_MLC_ECC_CTRL,val32);

        val32 = ReadRegWord((void*)NAND_SPI_DLY_CTRL);
        val32 &= 0xffff3f3f;
        WriteRegWord((void*)NAND_SPI_DLY_CTRL,val32);

        WriteRegWord((void*)NAND_TIMG, 0x40193333);

	/* ECC control reset */	
	WriteRegWord((void *)NAND_ECC_CTRL, (ReadRegWord((void *)NAND_ECC_CTRL)&(~0x4)));
	WriteRegWord((void *)NAND_ECC_CTRL, 0x1000);
	while((ReadRegWord((void *)NAND_ECC_CTRL)&0x1000)!=0);                      
	WriteRegWord((void *)NAND_ECC_CTRL, 0x02); 

	set_nand_mode(NAND_SW_MODE);
	return;
}

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

#ifdef CONFIG_MTD_CMDLINE_PARTS
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
	printk("%s:%d saved_command_line:%s\n", __func__, __LINE__, saved_command_line);
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
#endif

static int hidtv_nand_add_partition(void)
{	
	int nr_parts = 0;
	/* register the Master.*/
	nand_physical_partition_info[0].size = hidtv_nand_mtd.size;
	add_mtd_partitions(&hidtv_nand_mtd, nand_physical_partition_info, 1);
	
#ifdef CONFIG_MTD_CMDLINE_PARTS
	nr_parts = parse_mtd_cmdline(&hidtv_nand_mtd);
#endif
	return nr_parts;
}

static void monza_write_buf32(struct mtd_info *mtd, const uint8_t *buf, int len)                     {
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

static void monza_read_buf32(struct mtd_info *mtd, uint8_t *buf, int len)                            {
        int i;
        struct nand_chip *chip = mtd->priv;
        u32 *p = (u32 *) buf; 
        
	set_nand_mode(NAND_HW_MODE);	/*set HC as hw mode to fast read*/
	WriteRegWord((void*)NAND_CTRL, (0x00000421+(len<<16)));//length and stop bit
	
	/*should implement DMA in future. wonderful :)*/
	len >>= 2;
	for (i = 0; i < len; i++)              
		p[i] = ReadRegWord(chip->IO_ADDR_R);

	set_nand_mode(NAND_SW_MODE);	/*must exit hw mode*/
	WriteRegWord((void*)NAND_CTRL, 0x00000420);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static int monza_verify_buf32(struct mtd_info *mtd, const uint8_t *buf, int len)                                                     
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
		stat = chip->ecc.calculate(mtd, p, &ecc_calc[i]);
		if(stat)
		{
			printk("BCH: write page error %d\n",stat);
			goto exit;
		}
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
	int stat;
	struct nand_chip *chip = mtd->priv;

	stat = bch_decode_end_correct(mtd,data,chip);
	
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
		bch_decode_start((unsigned int*)&ecc_code[offset],chip);
		break;
	case NAND_ECC_WRITE:
		bch_encode_start(chip);
		break;
	}

	return;
}

static int __init hidtv_nand_init (void)
{
	struct nand_chip *this;
	void* nand_io_addr;

	nand_io_addr = (void *)MONZA_NAND_IO_ADDR;
	
	memset((char *)&hidtv_nand_mtd, 0, sizeof(struct mtd_info));
	memset((char *)&hidtv_nand_chip, 0, sizeof(struct nand_chip));
	
	/*basic hardware initialisation*/
	hidtv_nand_inithw();
	
	hidtv_nand_mtd.priv = &hidtv_nand_chip;
	hidtv_nand_mtd.owner = THIS_MODULE;
	this = &hidtv_nand_chip; 	
	
	this->IO_ADDR_R   = (void __iomem *)nand_io_addr;
	this->IO_ADDR_W   = (void __iomem *)nand_io_addr;
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	this->options     = NAND_NO_AUTOINCR/*NAND_SKIP_BBTSCAN|NAND_BBT_USE_FLASH|NAND_BBT_NO_OOB*/;
#endif
	//this->bbt_options = NAND_BBT_USE_FLASH|NAND_BBT_NO_OOB;

	this->chip_delay  = 1000; //fix me
	this->cmd_ctrl    = monza_nand_hwcontrol;	
	this->dev_ready   = monza_nand_ready;
	
	this->write_buf   = monza_write_buf32;
	this->read_buf    = monza_read_buf32;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	this->verify_buf  = monza_verify_buf32;
#endif
	this->ecc.mode     = NAND_ECC_HW; 	
	if (nand_scan_ident(&hidtv_nand_mtd, 1, NULL)) { 
		return -ENXIO;
	}
	
	this->options |= NAND_NO_SUBPAGE_WRITE;	/*we not support subpage in UBI*/
	hidtv_nand_mtd.subpage_sft = 0; 
	
	switch (hidtv_nand_mtd.writesize)
	{
		case 2048:
			this->ecc.strength = 8;
			this->ecc.size     = 512;
			this->ecc.layout = &nand_oob_mlcbch_64;
			break;
		case 4096:
			this->ecc.strength = 24;
			this->ecc.size     = 1024;
			this->ecc.layout = &nand_oob_mlcbch_224;
			break;
		case 8192:
			this->ecc.strength = 24;
			this->ecc.size     = 1024;
			this->ecc.layout = &nand_oob_mlcbch_448;
			break;
		default:
			printk("ERROR: unsupport NAND device.\n");
			return -ENXIO;
	}

	/*ECC bytes per step*/
	this->ecc.bytes = (14*this->ecc.strength + 7)>>3;	

	/*
 	 * sanity checks 
 	 * NOTE:it's important!
 	 */
	BUG_ON((this->ecc.size != 512)&&(this->ecc.size != 1024));	
	BUG_ON((this->ecc.strength < 4)||(this->ecc.strength > 60)||(this->ecc.strength % 2 ));

	if (!this->ecc.read_page)
		this->ecc.read_page = monza_read_page_bch;
	if (!this->ecc.write_page)
		this->ecc.write_page = monza_write_page_bch;
	
	this->ecc.calculate = monza_ecc_calculate;
	this->ecc.hwctl     = monza_ecc_hwctl;
	this->ecc.correct   = monza_ecc_correct;
	
	/* second phase scan */
	if (nand_scan_tail(&hidtv_nand_mtd)) { 
		return -ENXIO;
	}

	printk("Detected NAND, %lldMiB, erasesize %dKiB, pagesize %dB,oobsize %dB, \
	oobavail %dB\n",
	hidtv_nand_mtd.size >> 20, hidtv_nand_mtd.erasesize >> 10, 
	hidtv_nand_mtd.writesize,hidtv_nand_mtd.oobsize, hidtv_nand_mtd.oobavail);

	printk("Nand ECC info:Strength=%d,ecc size=%d ecc_bytes=%d\n",this->ecc.strength,this->ecc.size,this->ecc.bytes);
	hidtv_nand_add_partition();
	return 0;	
}

/*
 * Clean up routine
 */

static void __exit
hidtv_nand_cleanup(void)
{
	struct nand_chip *this = &hidtv_nand_chip;
	/* Unregister the device */
	del_mtd_device(&hidtv_nand_mtd);

	/* Free the MTD device structure */
	kfree(&hidtv_nand_mtd);
	kfree(this);
}

module_init(hidtv_nand_init);
module_exit(hidtv_nand_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SigmaDesigns kernel team");
MODULE_DESCRIPTION("driver for NAND on HiDTV board");











