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

#ifndef __SECURE_H__
#define __SECURE_H__

#ifndef __ASSEMBLY__
/*
 * @fn		uint32_t secure_read_reg(uint32_t mode, uint32_t pa);
 * @brief	secure read one register
 * 		user shall rarely use this API, go with secure_read_uintx for instead.
 * @param[in]	<mode> - specifies access mode
 * 		0 - byte
 * 		1 - halfword
 * 		2 - word
 * 		others - reserved
 * @param[in]	<pa> - specifies reg physical addr
 * @return	value on success. Otherwise 0
 */
uint32_t secure_read_reg(uint32_t mode, uint32_t pa);

/*
 * @fn		uint32_t secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
 * @brief	secure write one register, supports mask
 * 		user shall rarely use this API, go with secure_write_uintx for instead.
 * @param[in]	<mode> - specifies access mode
 * 		0 - byte
 * 		1 - halfword
 * 		2 - word
 * 		others - reserved
 * @param[in]	<pa>   - specifies reg physical addr
 * @param[in]	<val>  - value to write
 * @param[in]	<mask> - mask for write
 * @return	void
 */
void secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
#define secure_read_uint8(pa) (char)secure_read_reg(0, (uint32_t)pa)
#define secure_read_uint16(pa) (uint16_t)secure_read_reg(1, (uint32_t)pa)
#define secure_read_uint32(pa) (uint32_t)secure_read_reg(2, (uint32_t)pa)
#define secure_write_uint8(pa, val, m) secure_write_reg(0, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define secure_write_uint16(pa, val, m) secure_write_reg(1, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define secure_write_uint32(pa, val, m) secure_write_reg(2, (uint32_t)pa, (uint32_t)v, (uint32_t)m)

/*
 * @fn		uint32_t secure_otp_get_fuse_mirror(const uint32_t offset);
 * @brief	ask armor to read fuse mirror, must from NS world
 * @param[in]	<offset>  - fuse offset
 * @return	fuse value on success
 */
uint32_t secure_otp_get_fuse_mirror(const uint32_t offset);

/*
 * @fn		uint32_t secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, uint32_t nbytes);
 * @brief	ask armor to read fuse array, must from NS world
 * @param[in]	<offset> - fuse offset
 * @param[in]	<buf>    - buffer pointer
 * @param[in]	<nbytes> - buffer length
 * @return	return 0 and fuse value filled in buf on success. Otherwise non-zero
 */

uint32_t secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, uint32_t nbytes);

#endif /*__ASSEMBLY__*/

#endif
