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
 */
#define pr_fmt(fmt) "otp: " fmt
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

#include "otp.h"

const struct trix_otp_soc sxx_fuse_soc;

#define OTP_READ_ADDR 		0x1020
#define OTP_READ_STATUS		0x1024
#define OTP_READ_DATA0		0x1028
#define OTP_READ_DATA1		0x102c
#define OTP_READ_DATA2		0x1030
#define OTP_READ_DATA3		0x1034

#define OTP_FUSE_BASE		0x1100
#define FUSE_OFS_FC_0		0x004	/*Fuction Control 0*/
#define FUSE_OFS_FC_1		0x008	/*Fuction Control 1*/
#define FUSE_OFS_FC_2		0x00c	/*Fuction Control 2*/

#define FUSE_FC2_BOOT_ROM	BIT(0)
#define FUSE_FC2_SEC_BOOT_EN	BIT(1)
#define FUSE_FC2_USER_ID_SHIFT	2
#define FUSE_FC2_USER_ID_MASK	(0x1f << FUSE_FC2_USER_ID_SHIFT)
#define FUSE_FC2_NEW_NAND_SEL	BIT(12)

#define FUSE_OFS_FC_3		0x010	/*Fuction Control 3
					 * bit [0]	SEC_DIS
					 * bit [1]	BOOT_FROM_ROM_DIS
					 */
#define FUSE_OFS_DIE_ID_0	0x038	/*DIE_ID_0
					 * bit [7:0]	DESIGN_ID (0x00-A0; 0x01-A1; 0x02-A2; 0x03-A3; 0x04-A4;...0x05-A5
					 */
#define OTP_RSA_KEY_NBYTES 0x100

#ifdef CONFIG_SIGMA_SOC_SX8_OF
# define OTP_RSA_KEY_OFFSET 0xd50
#else
# define OTP_RSA_KEY_OFFSET 0x290
#endif

/*
 * read generic fuse
 */
static int read_fuse(struct trix_otp *host, const uint32_t offset, uint32_t *pval)
{
	BUG_ON(pval == NULL);

	*pval = OTP_READL(host, OTP_FUSE_BASE + offset);
	return 0;
}

/** read fuse data from data-addr register
 *@param bQuadWord 0: one word to be read out; 1: 4 words to be read out
 */
static int read_fuse_data(struct trix_otp *host, uint32_t fuseOffset, uint32_t bQuadWord, uint32_t* buf)
{
	int addr_reg;
	int temp;

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
			while ( (OTP_READL(host, OTP_READ_ADDR)&0xFF000000) != 0x00000000);
			// attempt read, then check if command was accepted
			// i.e. busy, done or invalid
			OTP_WRITEL(host, addr_reg,OTP_READ_ADDR);
			temp = OTP_READL(host, OTP_READ_STATUS);
			// exit if read had an error and return temp
			if (temp & 0x000000A0)
				return temp;
		 }while (!(temp&0x11));  // loop again if busy or done bits are not active

		// wait for read to complete
		while (!(temp&0x00000010))
		{
			temp = OTP_READL(host, OTP_READ_STATUS);
		}
		if (temp & 0x000000A0)
			return temp;

		// copy word to data array
		buf[0] = OTP_READL(host, OTP_READ_DATA0);
		// copy three more words if performing 128-bit read
		if (bQuadWord)
		{
			buf[1] = OTP_READL(host, OTP_READ_DATA1);
			buf[2] = OTP_READL(host, OTP_READ_DATA2);
			buf[3] = OTP_READL(host, OTP_READ_DATA3);
		}
		// check done bit one more time
		// if it is still set then the correct data was read from the registers
		// if it is not set then another client’s data may have overwritten
		// data in the registers before they were read
	}while(0==(OTP_READL(host, OTP_READ_STATUS)&0x10));

	return 0;
}

static int get_fuse_array(struct trix_otp *host, uint32_t ofs, uint32_t *buf, uint32_t nbytes)
{
	int i, j;

	BUG_ON(buf == NULL);
	for(i=0,j=0; i<(nbytes>>2); i+=4,j+=16) {
		read_fuse_data(host, ofs+j, 1, buf+i);
	}
	return 0;
}

static void __init sxx_fuse_init(struct trix_otp *host)
{
	host->read = read_fuse;
	host->read_array = get_fuse_array;
	/*
	 * TODO: invoke secure firmware operation
	 *       if we are running in non-secure world
	 */
}

static bool sxx_get_security_boot_state(struct trix_otp *host)
{
	uint32_t val;
	WARN_ON(host->read(host, FUSE_OFS_FC_2, &val) < 0);
	return (val & FUSE_FC2_SEC_BOOT_EN) ? true : false;
}

static bool sxx_get_new_nand_sel_state(struct trix_otp *host)
{
	uint32_t val;
	WARN_ON(host->read(host, FUSE_OFS_FC_2, &val) < 0);
	return (val & FUSE_FC2_NEW_NAND_SEL) ? true : false;
}

static int  sxx_get_rsa_key_index(struct trix_otp *host)
{
	uint32_t val;
	WARN_ON(host->read(host, FUSE_OFS_FC_2, &val) < 0);
	return (int)((val && FUSE_FC2_USER_ID_MASK) >> FUSE_FC2_USER_ID_SHIFT);
}

static uint32_t sxx_get_rsa_key(struct trix_otp *host, uint32_t *buf, uint32_t nbytes)
{
	uint32_t otp_rsa_key_off = OTP_RSA_KEY_OFFSET;

	if (nbytes < OTP_RSA_KEY_NBYTES) {
		pr_warn("small buffer for rsa key: %d\n", nbytes);
		return 1;
	}

	return host->read_array(host, otp_rsa_key_off, buf, OTP_RSA_KEY_NBYTES);
}

const struct trix_otp_soc sxx_fuse_soc = {
	.init	= sxx_fuse_init,
	.get_security_boot_state = sxx_get_security_boot_state,
	.get_new_nand_sel_state  = sxx_get_new_nand_sel_state,
	.get_rsa_key_index       = sxx_get_rsa_key_index,
	.get_rsa_key             = sxx_get_rsa_key,
};

