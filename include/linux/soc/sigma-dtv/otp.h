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

#ifndef __ARCH_SIGMA_DTV_OTP_H__
#define __ARCH_SIGMA_DTV_OTP_H__


#define OTP_RSA_KEY_NBYTES 0x100

#ifndef __ASSEMBLY__
/*
 * get security boot state from OTP
 * return value
 * 	false	-  security boot disabled (default)
 * 	true	-  security boot enabled
 */
bool otp_get_security_boot_state(void);

/*
 * get new NAND selection state from OTP
 * return value
 * 	false	-  legacy NAND controller (default)
 * 	true	-  new NAND controller
 */
bool otp_get_new_nand_sel_state(void);

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

/*
 * get value of bits [<s>+<nbits>-1:<s>] of fuse <ofs>
 * inputs:
 * 	ofs     -  specify fuse offset
 * 	s	-  start bit (begin at 0)
 * 	nb	-  number of bits to get against
 * outputs:
 * 	ptr	-  pointer of buffer to load fuse value on success
 * return value:
 * 	0 on success with value of specified bits stored in buffer
 * 	pointed by <ptr>. Otherwise errors.
 */
int otp_get_fuse_bits(const uint32_t ofs, const unsigned s, const unsigned nb, uint32_t *ptr);

#endif /*__ASSEMBLY__*/

#endif /* __ARCH_SIGMA_DTV_OTP_H__ */
