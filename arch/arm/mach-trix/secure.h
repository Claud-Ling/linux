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

#ifndef __ASM_ARCH_TRIX_SECURE_H__
#define __ASM_ARCH_TRIX_SECURE_H__

#include <plat/secure_io.h>

#ifndef __ASSEMBLY__

/*
 * @fn		int secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval);
 * @brief	ask armor to read fuse mirror, must from NS world
 * @param[in]	<offset>  - fuse offset
 * @param[out]	<pval>    - buffer pointer
 * @return	0 on success. Otherwise error code
 */
int secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval);

/*
 * @fn		int secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, uint32_t nbytes);
 * @brief	ask armor to read fuse array, must from NS world
 * @param[in]	<offset> - fuse offset
 * @param[out]	<buf>    - buffer pointer
 * @param[in]	<nbytes> - buffer length
 * @return	return 0 and fuse value filled in buf on success. Otherwise error code
 */
int secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, uint32_t nbytes);

#endif /*__ASSEMBLY__*/

#endif /*__ASM_ARCH_TRIX_SECURE_H__*/
