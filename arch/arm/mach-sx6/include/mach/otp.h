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

#ifndef __ASM_ARCH_SIGMA_TRIX_OTP_H__
#define __ASM_ARCH_SIGMA_TRIX_OTP_H__


#define OTP_RSA_KEY_NBYTES 0x100

#ifndef __ASSEMBLY__
/*
 * get security boot state from OTP
 * return value
 * 	0  -  security boot disabled (default)
 * 	1  -  security boot enabled
 */
bool otp_get_security_boot_state(void);

/*
 * get index value of current using RSA public key
 * return value
 * 	0       -  use OTP RSA public key
 * 	1 ~ 16  -  index to ROM embedded RSA public key that is in use
 */
int otp_get_rsa_key_index(void);

/*
 * read RSA public key from OTP
 * inputs:
 * 	buf     -  point to a buffer
 * 	nbytes  -  length of buffer, it's at least OTP_RSA_KEY_NBYTES long
 * return value:
 * 	0 on success. Otherwise non-zero
 */
uint32_t otp_get_rsa_key(uint32_t *buf, uint32_t nbytes);

#endif /*__ASSEMBLY__*/

#endif
