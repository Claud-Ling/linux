/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2015.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <mach/otp.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif
#ifdef CONFIG_SIGMA_SMC
#include "smc.h"
#endif

static void __iomem *otp_base = NULL;

#define OTP_READL(o) readl_relaxed(otp_base + (o))
#define OTP_WRITEL(v, o) writel_relaxed(v, otp_base + (o))


#ifdef CONFIG_SIGMA_OTP_SYS

#ifdef CONFIG_SIGMA_SOC_SX6
# define OTP_REG_BASE 	0xf5100000
#elif defined(CONFIG_SIGMA_SOC_SX7)
# define OTP_REG_BASE	0xf1040000
#else
# error"unknown chip set"
#endif

#define OTP_REG_LENGTH	0x2000 /*8K is enough here for otp access*/

#define OTP_READ_ADDR 		0x1020
#define OTP_READ_STATUS		0x1024
#define OTP_READ_DATA0		0x1028
#define OTP_READ_DATA1		0x102c
#define OTP_READ_DATA2		0x1030
#define OTP_READ_DATA3		0x1034

#define OTP_FUSE_BASE		0x1100
#define FUSE_OFS_FC_0		0x004	/*Fuction Control 0*/
#define FUSE_OFS_FC_1		0x008	/*Fuction Control 1*/
#define FUSE_OFS_FC_2		0x00c	/*Fuction Control 2
					 * bit [0]	SEC_BOOT_FROM_ROM
					 * bit [1]	SEC_BOOT_EN
					 * bit [6:2]	BOOT_VAL_USER_ID
					 */
#define FUSE_OFS_FC_3		0x010	/*Fuction Control 3
					 * bit [0]	SEC_DIS
					 * bit [1]	BOOT_FROM_ROM_DIS
					 */
#define FUSE_OFS_DIE_ID_0	0x038	/*DIE_ID_0
					 * bit [7:0]	DESIGN_ID (0x00-A0; 0x01-A1; 0x02-A2; 0x03-A3; 0x04-A4;...0x05-A5
					 */

/*
 * read generic fuse
 */
static int read_fuse(const uint32_t offset, uint32_t *pval)
{
	BUG_ON(pval == NULL);
#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		return secure_otp_get_fuse_mirror(offset, pval);
#endif
	*pval = OTP_READL(OTP_FUSE_BASE + offset);
	return 0;
}

/** read fuse data from data-addr register
 *@param bQuadWord 0: one word to be read out; 1: 4 words to be read out
 */
static int read_fuse_data(uint32_t fuseOffset, uint32_t bQuadWord, uint32_t* buf)
{
	int addr_reg;
	int temp;
	BUG_ON(otp_base == NULL);

	// setup addr_reg for read command
	addr_reg = 0xC2000000 | // set interlock to 0xC2 to indicate the start of read operation
		(fuseOffset & 0x0000FFFF); // set fuse offset to addr
	// loop until read is successful
	do
	{
		// initiate read command
		do
		{
			// wait for read interface to be idle
			while ( (OTP_READL(OTP_READ_ADDR)&0xFF000000) != 0x00000000);
			// attempt read, then check if command was accepted
			// i.e. busy, done or invalid
			OTP_WRITEL(addr_reg,OTP_READ_ADDR);
			temp = OTP_READL(OTP_READ_STATUS);
			// exit if read had an error and return temp
			if (temp & 0x000000A0)
				return temp;
		 }while (!(temp&0x11));  // loop again if busy or done bits are not active

		// wait for read to complete
		while (!(temp&0x00000010))
		{
			temp = OTP_READL(OTP_READ_STATUS);
		}
		if (temp & 0x000000A0)
			return temp;

		// copy word to data array
		buf[0] = OTP_READL(OTP_READ_DATA0);
		// copy three more words if performing 128-bit read
		if (bQuadWord)
		{
			buf[1] = OTP_READL(OTP_READ_DATA1);
			buf[2] = OTP_READL(OTP_READ_DATA2);
			buf[3] = OTP_READL(OTP_READ_DATA3);
		}
		// check done bit one more time
		// if it is still set then the correct data was read from the registers
		// if it is not set then another client’s data may have overwritten
		// data in the registers before they were read
	}while(0==(OTP_READL(OTP_READ_STATUS)&0x10));

	return 0;
}

static uint32_t get_fuse_array(uint32_t ofs, uint32_t *buf, uint32_t nbytes)
{
	int i, j;

#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		return secure_otp_get_fuse_array(ofs, buf, nbytes);
#endif

	BUG_ON(buf == NULL);
	for(i=0,j=0; i<(nbytes>>2); i+=4,j+=16) {
		read_fuse_data(ofs+j, 1, buf+i);
	}
	return 0;
}

/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
static bool get_security_boot_state(void)
{
	uint32_t val;
	WARN_ON(read_fuse(FUSE_OFS_FC_2, &val) < 0);
	return (val & 0x2) ? 1 : 0;
}

/*
 * get index value of current using RSA public key
 * return value
 * 	0       -  use OTP RSA public key
 * 	1 ~ 16  -  index to ROM embedded RSA public key that is in use
 */
static int get_rsa_key_index(void)
{
	uint32_t val;
	WARN_ON(read_fuse(FUSE_OFS_FC_2, &val) < 0);
	return (int)((val >> 2) & 0x1f);
}

/*
 * read RSA public key from OTP
 * inputs:
 * 	buf     -  point to a buffer
 * 	nbytes  -  length of buffer, it's at least OTP_RSA_KEY_NBYTES long
 * return value:
 * 	0 on success. Otherwise non-zero
 */
static uint32_t get_rsa_key(uint32_t *buf, uint32_t nbytes)
{
	uint32_t otp_rsa_key_off = 0x290;
	BUG_ON(otp_base == NULL);
	if (nbytes < OTP_RSA_KEY_NBYTES) {
		printk(KERN_WARNING "small buffer for rsa key: %d\n", nbytes);
		return 1;
	}

	return get_fuse_array(otp_rsa_key_off, buf, OTP_RSA_KEY_NBYTES);
}

#elif defined(CONFIG_SIGMA_OTP_EFUSE) /*CONFIG_SIGMA_OTP_SYS*/

#ifndef CONFIG_SIGMA_SOC_UXLB
#error "only support efuse on UXLB by now"
#endif

#define OTP_REG_BASE 0xf0002000
#define OTP_REG_LENGTH	0x2000 /*8K is enough here for otp access*/

#define FC_ADDR_REG		0x0000
#define FC_WRITE_DATA_REG	0x0004
#define FC_MODE_REG		0x0008
#define FC_CMD_REG		0x000c
#define FC_READ_DATA_REG	0x0010
#define FC_UNLOCK_CODE_REG	0x0040
#define FC_UNLOCK_STATUS_REG	0x0044
#define FC_WRITE_ENABLE_REG	0x0048
#define FC_READ_ENABLE_REG	0x004c

#define OTP_FUSE_BASE		0x0200
#define FUSE_OFS_SECURITY_BOOT	0x038	/* Security boot reg
					 * bit [15:8]	0~7 RSA key select code
					 * bit [16]	SEC_BOOT_EN
					 * bit [23]	RSA_KEY_IN_OTP, 1 in OTP or 0 in ROM
					 * bit [31:24]	8~15 RSA key select code
					 */

#define FC_UNLOCK_CODE		0x01234567

/*
 * read generic fuse
 */
static int read_fuse(const uint32_t offset, uint32_t *pval)
{
	BUG_ON(pval == NULL);
#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		return secure_otp_get_fuse_mirror(offset, pval);
#endif
	*pval = OTP_READL(OTP_FUSE_BASE + offset);
	return 0;
}

/*
 * read fuse data from data-addr register
 */
static int read_fuse_data (uint32_t addr, uint32_t *buf)
{
	int addr_reg, temp;
	BUG_ON(otp_base == NULL);

	// enable OTP read/write throught register access
	OTP_WRITEL(0x1, FC_WRITE_ENABLE_REG);
	OTP_WRITEL(0x1, FC_READ_ENABLE_REG);

	// unlock the programming interface
	temp = 0;
	while (!temp) {
		OTP_WRITEL(FC_UNLOCK_CODE, FC_UNLOCK_CODE_REG);
		// verify unlock was successful
		temp = (OTP_READL(FC_UNLOCK_STATUS_REG) == 0x1);
	}

	// setup read address
	addr_reg = (addr & 0x0000ffff);
	OTP_WRITEL(addr_reg, FC_ADDR_REG);

	// setup read command
	OTP_WRITEL(0x1, FC_MODE_REG);	//1 means read operation

	// execute command
	OTP_WRITEL(0x1, FC_CMD_REG);

	// wait for command to complete
	temp = 1;
	while (temp) {
		temp = OTP_READL(FC_CMD_REG);
	}

	// copy word to data array
	*buf = OTP_READL(FC_READ_DATA_REG);

	// disable OTP read/write through register access
	OTP_WRITEL(0x0, FC_WRITE_ENABLE_REG);
	OTP_WRITEL(0x0, FC_READ_ENABLE_REG);

	return 0;
}

static uint32_t get_fuse_array(uint32_t ofs, uint32_t *buf, uint32_t nbytes)
{
	int i, j;

#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		return secure_otp_get_fuse_array(ofs, buf, nbytes);
#endif

	BUG_ON(buf == NULL);
	for(i=0,j=0; i<(nbytes>>2); i++,j+=4) {
		read_fuse_data(ofs+j, buf+i);
	}
	return 0;
}

/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
static bool get_security_boot_state(void)
{
	uint32_t val;
	WARN_ON(read_fuse(FUSE_OFS_SECURITY_BOOT, &val) < 0);
	return ((val & (1 << 16))) ? 1 : 0;
}

/*
 * get index value of current using RSA public key
 * return value
 * 	0       -  use OTP RSA public key
 * 	1 ~ 16  -  index to ROM embedded RSA public key that is in use
 */
static int get_rsa_key_index(void)
{
	int idx = 0;
	uint32_t val;
	WARN_ON(read_fuse(FUSE_OFS_SECURITY_BOOT, &val) < 0);
	if (val & (1 << 23)) {
		idx = 0;
	} else {
		printk("TODO: get_rsa_key_index for ROM!\n");
		idx = 1;	//TODO
	}
	return idx;
}

/*
 * read RSA public key from OTP
 * inputs:
 * 	buf     -  point to a buffer
 * 	nbytes  -  length of buffer, it's at least OTP_RSA_KEY_NBYTES long
 * return value:
 * 	0 on success. Otherwise non-zero
 */
static uint32_t get_rsa_key(uint32_t *buf, uint32_t nbytes)
{
	uint32_t otp_rsa_key_off = 0x400;
	BUG_ON(otp_base == NULL);
	if (nbytes < OTP_RSA_KEY_NBYTES) {
		printk(KERN_WARNING "small buffer for rsa key: %d\n", nbytes);
		return 1;
	}

	return get_fuse_array(otp_rsa_key_off, buf, OTP_RSA_KEY_NBYTES);
}

#else /*CONFIG_SIGMA_OTP_SYS*/

/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
static bool get_security_boot_state(void)
{
	return 0;
}

/*
 * get index value of current using RSA public key
 * return value
 * 	0       -  use OTP RSA public key
 * 	1 ~ 16  -  index to ROM embedded RSA public key that is in use
 */
static int get_rsa_key_index(void)
{
	return 1;
}

/*
 * read RSA public key from OTP
 * inputs:
 * 	buf     -  point to a buffer
 * 	nbytes  -  length of buffer, it's at least OTP_RSA_KEY_NBYTES long
 * return value:
 * 	0 on success. Otherwise non-zero
 */
static uint32_t get_rsa_key(uint32_t *buf, uint32_t nbytes)
{
	return 1;
}

#endif /*CONFIG_SIGMA_OTP_SYS*/

#ifdef CONFIG_PROC_FS

struct field_descriptor {
	int bit;
	int nbits;
	const char* fn;
};

struct node_descriptor {
	const char* comm;
	struct proc_dir_entry *pe;
	struct list_head items;
	int(*create)(struct node_descriptor *);
	int(*helper)(struct seq_file *);
	struct file_operations fops;
};

#ifdef GET_BITS
# undef GET_BITS
#endif
#define GET_BITS(v, s, n) (((v) & (((1 << (n)) - 1) << (s))) >> (s))

#define otp_fuse_field(s, l, fn) {s, l, #fn}
#define otp_fuse_entry(name, ofs, syntax...) 				\
static struct field_descriptor fuse_e##name##_desc[] =			\
{									\
	syntax,								\
	{-1, -1, NULL}		/*EOT*/					\
};									\
									\
static int fuse_e##name##_create(struct node_descriptor *self)		\
{									\
	if (NULL == self->pe) {						\
		if (!(self->pe = proc_create("otp/"#name, S_IRUGO, NULL,\
					&self->fops)))			\
			return -ENOMEM;					\
	}								\
	return 0;							\
}									\
									\
static int fuse_e##name##_helper(struct seq_file *m)			\
{									\
	struct field_descriptor *f = &fuse_e##name##_desc[0];		\
	seq_printf(m, "[%03x] 004 "#name"\n", ofs);			\
	for(; f->fn != NULL; f++) {					\
		if (f->nbits > 1)					\
			seq_printf(m, "\tbit[%-2d:%-2d]  - %s\n",	\
			f->bit+f->nbits-1, f->bit, f->fn);		\
		else							\
			seq_printf(m, "\tbit[%-2d]     - %s\n",		\
			f->bit, f->fn);					\
	}								\
	return 0;							\
}									\
									\
static int fuse_e##name##_parser(struct seq_file *m, uint32_t val)	\
{									\
	struct field_descriptor *f = &fuse_e##name##_desc[0];		\
	for(; f->fn != NULL; f++) {					\
		seq_printf(m, "%s: %d\n", f->fn,			\
				GET_BITS(val, f->bit, f->nbits));	\
	}								\
	return 0;							\
}									\
									\
static int fuse_e##name##_show(struct seq_file *m, void *v)		\
{									\
	int ret; uint32_t temp;						\
	if ((ret = read_fuse(ofs, &temp)) == 0)	{			\
		seq_printf(m, "%08x\n", temp);				\
		fuse_e##name##_parser(m, temp);				\
	}								\
	return ret;							\
}									\
									\
static int fuse_e##name##_open(struct inode *inode, struct file *file)	\
{									\
        return single_open(file, fuse_e##name##_show, NULL);		\
}									\
									\
static struct node_descriptor fuse_e##name = {				\
	.comm		= #name,					\
	.create		= fuse_e##name##_create,			\
	.helper		= fuse_e##name##_helper,			\
	.fops		= {						\
				.open           = fuse_e##name##_open,	\
				.read           = seq_read,		\
				.llseek         = seq_lseek,		\
				.release        = single_release,	\
			}						\
};

#define otp_fuse_generic(name, ofs, len)				\
static int fuse_g##name##_create(struct node_descriptor *self)		\
{									\
	if (NULL == self->pe) {						\
		if (!(self->pe = proc_create("otp/"#name, S_IRUGO, NULL,\
					&self->fops)))			\
			return -ENOMEM;					\
	}								\
	return 0;							\
}									\
									\
static int fuse_g##name##_helper(struct seq_file *m)			\
{									\
	seq_printf(m, "[%03x] %03x "#name"\n", ofs, len);		\
	return 0;							\
}									\
									\
static int fuse_g##name##_show(struct seq_file *m, void *v)		\
{									\
	int i, ret; char *p;						\
	uint32_t temp[(len)>>2];					\
	if ((ret = get_fuse_array(ofs, temp, len)) == 0) {		\
		for (i = 0, p = (char*)temp; i < len; i++, p++) {	\
			if (i && !(i & 0xf))				\
				seq_printf(m, "\n");			\
			seq_printf(m, "%02x", *p);			\
		}							\
		seq_printf(m, "\n");					\
	}								\
	return ret;							\
}									\
									\
static int fuse_g##name##_open(struct inode *inode, struct file *file)	\
{									\
        return single_open(file, fuse_g##name##_show, NULL);		\
}									\
									\
static struct node_descriptor fuse_g##name = {				\
	.comm		= #name,					\
	.create		= fuse_g##name##_create,			\
	.helper		= fuse_g##name##_helper,			\
	.fops		= {						\
				.open           = fuse_g##name##_open,	\
				.read           = seq_read,		\
				.llseek         = seq_lseek,		\
				.release        = single_release,	\
			}						\
};

#include "fuse_data_map.inc"
#undef otp_fuse_field
#undef otp_fuse_entry
#undef otp_fuse_generic

/*
 * local fuse node list header
 */
static LIST_HEAD(otp_fuse_list);

/*
 * otp/index node
 */
static int otp_index_create(struct node_descriptor* self)
{
	if (NULL == self->pe) {
		if (!(self->pe = proc_create("otp/index", S_IRUGO, NULL, &self->fops)))
			return -ENOMEM;
	}
	return 0;
}

static int otp_index_show(struct seq_file *m, void *v)
{
	struct list_head *pos;
	struct node_descriptor *node;
	seq_printf(m, "<ofs  len name>\n");
	list_for_each(pos, &otp_fuse_list) {
		node = list_entry(pos, struct node_descriptor, items);
		if (node->helper)
			node->helper(m);
	}
	return 0;
}

static int otp_index_open(struct inode *inode, struct file *file)
{
        return single_open(file, otp_index_show, NULL);
}

static struct node_descriptor otp_index = {
	.comm		= "index",
	.create		= otp_index_create,
	.fops		= {
				.open           = otp_index_open,
				.read           = seq_read,
				.llseek         = seq_lseek,
				.release        = single_release,
			}
};

static int otp_proc_create(void)
{
	int ret = 0;
	struct list_head *pos;
	struct node_descriptor *node;

	INIT_LIST_HEAD(&otp_fuse_list);
	list_add_tail(&otp_index.items, &otp_fuse_list);
#define otp_fuse_field(s, n, fn) s
#define otp_fuse_entry(name, ofs, syntax...) list_add_tail(&fuse_e##name.items, &otp_fuse_list);
#define otp_fuse_generic(name, ofs, len) list_add_tail(&fuse_g##name.items, &otp_fuse_list);
#include "fuse_data_map.inc"
#undef otp_fuse_field
#undef otp_fuse_entry
#undef otp_fuse_generic

	list_for_each(pos, &otp_fuse_list){
		node = list_entry(pos, struct node_descriptor, items);
		if((ret = node->create(node)) != 0)
			break;
	}

	return ret;
}
#endif /*CONFIG_PROC_FS*/

static int __init otp_init(void)
{
	otp_base = ioremap(OTP_REG_BASE, OTP_REG_LENGTH);
	if (otp_base == NULL)
		printk(KERN_ERR "ioremap otp regs failed!\n");

#ifdef CONFIG_PROC_FS
	proc_mkdir("otp", NULL);
	BUG_ON(otp_proc_create() != 0);
#endif

	return 0;
}
early_initcall(otp_init);

/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
bool otp_get_security_boot_state(void)
{
	return get_security_boot_state();
}

/*
 * get index value of current using RSA public key
 * return value
 * 	0       -  use OTP RSA public key
 * 	1 ~ 16  -  index to ROM embedded RSA public key that is in use
 */
int otp_get_rsa_key_index(void)
{
	return get_rsa_key_index();
}

/*
 * read RSA public key from OTP
 * inputs:
 * 	buf     -  point to a buffer
 * 	nbytes  -  length of buffer, it's at least OTP_RSA_KEY_NBYTES long
 * return value:
 * 	0 on success. Otherwise non-zero
 */
uint32_t otp_get_rsa_key(uint32_t *buf, uint32_t nbytes)
{
	return get_rsa_key((uint32_t*)buf, nbytes);
}

