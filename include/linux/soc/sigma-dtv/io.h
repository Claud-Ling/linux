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
 *  Author: Tony He, 2014.
 */

#ifndef __ARCH_SIGMA_DTV_IO_H__
#define __ARCH_SIGMA_DTV_IO_H__

#include <linux/types.h>

/*
 * ----------------------------------------------------------------------------
 * I/O mapping
 * ----------------------------------------------------------------------------
 */

#ifdef __ASSEMBLY__
#define IOMEM(x)		(x)
#else
#define IOMEM(x)		((void __force __iomem *)(x))
#endif

#define SIGMA_IO_BETWEEN(p, st, sz)   ((p) >= (st) && (p) < ((st) + (sz)))
#define SIGMA_IO_ADDRESS(n) (((phys_addr_t)(n) & 0xfffffff) | 0xf0000000)

#ifndef __ASSEMBLY__

#include <asm/io.h>

#if IS_REACHABLE(CONFIG_TRIX_DRV_HELPER)
/*
 * @fn		int io_accessor_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval);
 * @brief	read one register
 * 		user shall rarely use this API, go with sys_io_read_uintx for instead.
 * @param[in]	<mode> - specifies access mode
 * 		0 - byte
 * 		1 - halfword
 * 		2 - word
 * 		others - reserved
 * @param[in]	<pa>   - specifies reg physical addr
 * @param[out]	<pval> - pointer of buffer
 * @return	0 on success. Otherwise error code
 */
int io_accessor_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval);

/*
 * @fn		int io_accessor_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
 * @brief	write one register, supports mask
 * 		user shall rarely use this API, go with sys_io_write_uintx for instead.
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
int io_accessor_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);

#else

static inline
int io_accessor_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval)
{
	return -EIO;
}

static inline
int io_accessor_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask)
{
	return -EIO;
}

#endif /* CONFIG_TRIX_DRV_HELPER */

#define io_accessor_read_generic(pa, m, t) ({					\
				uint32_t _tmp;					\
				io_accessor_read_reg(m, (uint32_t)pa, &_tmp);	\
				(t)_tmp;					\
})
#define sys_io_read_uint8(pa)	io_accessor_read_generic(pa, 0, uint8_t)
#define sys_io_read_uint16(pa)	io_accessor_read_generic(pa, 1, uint16_t)
#define sys_io_read_uint32(pa)	io_accessor_read_generic(pa, 2, uint32_t)
#define sys_io_write_uint8(pa, val, m)	io_accessor_write_reg(0, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define sys_io_write_uint16(pa, val, m)	io_accessor_write_reg(1, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define sys_io_write_uint32(pa, val, m)	io_accessor_write_reg(2, (uint32_t)pa, (uint32_t)v, (uint32_t)m)


/*
 *  I/O helpers for modules
 */

static inline unsigned char ReadRegByte(uint32_t pa)
{
	return sys_io_read_uint8(pa);
}

static inline unsigned short ReadRegHWord(uint32_t pa)
{
	return sys_io_read_uint16(pa);
}

static inline unsigned int ReadRegWord(uint32_t pa)
{
	return sys_io_read_uint32(pa);
}

static inline void WriteRegByte(uint32_t pa, unsigned char v)
{
	sys_io_write_uint8(pa, v, U8_MAX);
}

static inline void WriteRegHWord(uint32_t pa, unsigned short v)
{
	sys_io_write_uint16(pa, v, U16_MAX);
}

static inline void WriteRegWord(uint32_t pa, unsigned int v)
{
	sys_io_write_uint32(pa, v, U32_MAX);
}


#define MaskWriteReg(op, va, v, m)	do{			\
		typeof(v) __tmp;				\
		__tmp = __raw_read##op(va);	\
		__tmp &= ~(m);					\
		__tmp |= ((v) & (m));				\
		__raw_write##op(__tmp, va);	\
	}while(0)

static inline void MWriteRegByte(uint32_t pa, unsigned char v, unsigned char m)
{
	sys_io_write_uint8(pa, v, m);
}

static inline void MWriteRegHWord(uint32_t pa, unsigned short v, unsigned short m)
{
	sys_io_write_uint16(pa, v, m);
}

static inline void MWriteRegWord(uint32_t pa, unsigned int v, unsigned int m)
{
	sys_io_write_uint32(pa, v, m);
}


#endif /* __ASSEMBLY__ */

#endif /*__ARCH_SIGMA_DTV_IO_H__*/
