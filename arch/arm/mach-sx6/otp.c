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
#define FUSE_OFS_FC_2		0x00c	/*Fuction Control 2
					 * bit [1]	SEC_BOOT_EN
					 * bit [6:2]	BOOT_VAL_USER_ID
					 */

/*
 * read generic fuse
 */
static uint32_t read_fuse(const uint32_t offset)
{
#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		return secure_otp_read_fuse(offset);
	else
#endif
		return OTP_READL(OTP_FUSE_BASE + offset);
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

/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
static bool get_security_boot_state(void)
{
	return ((read_fuse(FUSE_OFS_FC_2) & 0x2)) ? 1 : 0;
}

/*
 * get index value of current using RSA public key
 * return value
 * 	0       -  use OTP RSA public key
 * 	1 ~ 16  -  index to ROM embedded RSA public key that is in use
 */
static int get_rsa_key_index(void)
{
	int idx = (read_fuse(FUSE_OFS_FC_2) >> 2) & 0x1f;
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
	int i,j;
	uint32_t otp_rsa_key_off = 0x290;
	BUG_ON(otp_base == NULL);
	BUG_ON(buf == NULL);
	if (nbytes < OTP_RSA_KEY_NBYTES) {
		printk(KERN_WARNING "small buffer for rsa key: %d\n", nbytes);
		return 1;
	}

	for(i=0,j=0; i<(OTP_RSA_KEY_NBYTES>>2); i+=4,j+=16) {
		read_fuse_data(otp_rsa_key_off+j, 1, buf+i);
	}
	return 0;
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
static uint32_t read_fuse(const uint32_t offset)
{
#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		return secure_otp_read_fuse(offset);
	else
#endif
		return OTP_READL(OTP_FUSE_BASE + offset);
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

/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
static bool get_security_boot_state(void)
{
	return ((read_fuse(FUSE_OFS_SECURITY_BOOT) & (1 << 16))) ? 1 : 0;
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
	if (read_fuse(FUSE_OFS_SECURITY_BOOT) & (1 << 23)) {
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
	int i,j;
	uint32_t otp_rsa_key_off = 0x400;
	BUG_ON(otp_base == NULL);
	BUG_ON(buf == NULL);
	if (nbytes < OTP_RSA_KEY_NBYTES) {
		printk(KERN_WARNING "small buffer for rsa key: %d\n", nbytes);
		return 1;
	}

	for(i=0,j=0; i<(OTP_RSA_KEY_NBYTES>>2); i++,j+=4) {
		read_fuse_data(otp_rsa_key_off+j, buf+i);
	}
	return 0;
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

static int __init otp_init(void)
{
	otp_base = ioremap(OTP_REG_BASE, OTP_REG_LENGTH);
	if (otp_base == NULL)
		printk(KERN_ERR "ioremap otp regs failed!\n");
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
	uint32_t (*func)(uint32_t *, uint32_t);

	func = get_rsa_key;
#ifdef CONFIG_SIGMA_SMC
	if (!get_security_state())
		func = secure_otp_read_rsa_key;
#endif
	return func((uint32_t*)buf, nbytes);
}

