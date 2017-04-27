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

/*
 * inspired by otp_ree.c in u-boot
 */

#define pr_fmt(fmt) "otp: " fmt
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/soc/sigma-dtv/security.h>

#include "otp.h"

/******************************************************************/
/*                     Fuse Data Map Start                        */
/* TODO: derive fuse data map from dtb!?                          */
/******************************************************************/
#define FUSE_OFS_FC_0		0x004	/*Fuction Control 0*/
#define FUSE_OFS_FC_1		0x008	/*Fuction Control 1*/
#define FUSE_OFS_FC_2		0x00c	/*Fuction Control 2
					 * bit [0]	SEC_BOOT_FROM_ROM
					 * bit [1]	SEC_BOOT_EN
					 * bit [6:2]	BOOT_VAL_USER_ID
					 * bit [12]	NEW_NAND_CTRL_SEL
					 */
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

#ifdef CONFIG_SIGMA_SOC_SX8_OF
# define FUSE_OFS_RSA_PUB_KEY	0xd50
#else
# define FUSE_OFS_RSA_PUB_KEY	0x290
#endif

/******************************************************************/
/*                     Fuse Data Map End                          */
/******************************************************************/

const struct trix_otp_soc sxx_fuse_soc;

static int fuse_read_mirror(struct trix_otp *otp, const uint32_t offset, uint32_t *pval, uint32_t *pprot)
{
	(void)otp;
	return secure_otp_get_fuse_mirror(offset, pval, pprot);
}

static int fuse_read_array(struct trix_otp *otp, const uint32_t offset, uint32_t *buf, const uint32_t nbytes, uint32_t *pprot)
{
	(void)otp;
	return secure_otp_get_fuse_array(offset, buf, nbytes, pprot);
}

static void __init sxx_fuse_init(struct trix_otp *host)
{
	host->read = fuse_read_mirror;
	host->read_array = fuse_read_array;
}

static bool sxx_get_security_boot_state(struct trix_otp *host)
{
	uint32_t val = 0;
	WARN_ON(host->read(host, FUSE_OFS_FC_2, &val, NULL) < 0);
	return (val & FUSE_FC2_SEC_BOOT_EN) ? true : false;
}

static bool sxx_get_new_nand_sel_state(struct trix_otp *host)
{
	uint32_t val = 0;
	WARN_ON(host->read(host, FUSE_OFS_FC_2, &val, NULL) < 0);
	return (val & FUSE_FC2_NEW_NAND_SEL) ? true : false;
}

static int  sxx_get_rsa_key_index(struct trix_otp *host)
{
	uint32_t val = 0;
	WARN_ON(host->read(host, FUSE_OFS_FC_2, &val, NULL) < 0);
	return (int)((val && FUSE_FC2_USER_ID_MASK) >> FUSE_FC2_USER_ID_SHIFT);
}

static int sxx_get_rsa_key(struct trix_otp *host, uint32_t *buf, uint32_t nbytes)
{
	return host->read_array(host, FUSE_OFS_RSA_PUB_KEY, buf, nbytes, NULL);
}

const struct trix_otp_soc sxx_fuse_soc = {
	.init	= sxx_fuse_init,
	.get_security_boot_state = sxx_get_security_boot_state,
	.get_new_nand_sel_state  = sxx_get_new_nand_sel_state,
	.get_rsa_key_index       = sxx_get_rsa_key_index,
	.get_rsa_key             = sxx_get_rsa_key,
};

