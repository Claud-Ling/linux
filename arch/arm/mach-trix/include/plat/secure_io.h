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

#ifndef __ASM_ARCH_TRIX_PLAT_SECURE_H__
#define __ASM_ARCH_TRIX_PLAT_SECURE_H__

#ifdef CONFIG_TRIX_SMC

#include <linux/types.h>

#ifndef __ASSEMBLY__
/*
 * @fn		int secure_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval);
 * @brief	secure read one register
 * 		user shall rarely use this API, go with secure_read_uintx for instead.
 * @param[in]	<mode> - specifies access mode
 * 		0 - byte
 * 		1 - halfword
 * 		2 - word
 * 		others - reserved
 * @param[in]	<pa>   - specifies reg physical addr
 * @param[out]	<pval> - pointer of buffer
 * @return	0 on success. Otherwise error code
 */
int secure_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval);

/*
 * @fn		int secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
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
 * @return	0 on success. Otherwise error code
 */
int secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
#define secure_read_generic(pa, m, t) ({					\
				uint32_t _tmp;					\
				secure_read_reg(m, (uint32_t)pa, &_tmp);	\
				(t)_tmp;					\
})
#define secure_read_uint8(pa)	secure_read_generic(pa, 0, uint8_t)
#define secure_read_uint16(pa)	secure_read_generic(pa, 1, uint16_t)
#define secure_read_uint32(pa)	secure_read_generic(pa, 2, uint32_t)
#define secure_write_uint8(pa, val, m)	secure_write_reg(0, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define secure_write_uint16(pa, val, m)	secure_write_reg(1, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define secure_write_uint32(pa, val, m)	secure_write_reg(2, (uint32_t)pa, (uint32_t)v, (uint32_t)m)

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_TRIX_SMC */

#endif /*__ASM_ARCH_TRIX_PLAT_SECURE_H__*/
