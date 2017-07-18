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

#ifndef __ASM_ARCH_TRIX_PLAT_IO_H__
#define __ASM_ARCH_TRIX_PLAT_IO_H__

#include <mach/hardware.h>
#include <plat/secure_io.h>

#define IO_SPACE_LIMIT 0xffffffff

/*
 * We don't actually have real ISA nor PCI buses, but there is so many
 * drivers out there that might just work if we fake them...
 */
#define __io(a)		__typesafe_io(a)
#define __mem_pci(a)	(a)

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

/*io register space*/
/*0xf5000000 - 0xfd000000, other IPs io space will be map by itself?*/
#define SIGMA_IO_0_BASE_PHYS	0xF5000000
#define SIGMA_IO_0_BASE_VIRT	IOMEM(0xF5000000)
#define SIGMA_IO_0_SIZE		0x08000000

/*0xfd000000 - 0xfe000000*/
//#define SIGMA_IO_1_BASE_PHYS	0xFD000000
//#define SIGMA_IO_1_BASE_VIRT	IOMEM(0xFD000000)
//#define SIGMA_IO_1_SIZE		0x01000000

/* A9 private register space*/
/*0xf2100000 - 0xf2103000*/
/*
  0xf210_0000~0xf210_00ff	SCU registers
  0xf210_0100~0xf210_01ff	Interrupt controller
  0xf210_0200~0xf210_02ff	Global timer
  0xf210_0300~0xf210_05ff	-
  0xf210_0600~0xf210_06ff	Private timers and watchdogs
  0xf210_0700~0xf210_0fff	Reserved
  0xf210_1000~0xf210_1fff	Interrupt Distributor
  0xf210_2000~0xf210_2fff	L2C
*/
#define SIGMA_IO_ARM_BASE_PHYS	0xF2100000
#define SIGMA_IO_ARM_BASE_VIRT	IOMEM(0xF2100000)
#define SIGMA_IO_ARM_SIZE	0x3000

/* AV mips uart */
#define SIGMA_IO_AVMIPS_UART_PHYS	(0xf0012000)
#define SIGMA_IO_AVMIPS_UART_VIRT	IOMEM(0xf0012000)


#define SIGMA_IO_TO_VIRT_BETWEEN(p, st, sz)   ((p) >= (st) && (p) < ((st) + (sz)))
#define SIGMA_IO_TO_VIRT_XLATE(p, pst, vst)   (((p) - (pst) + (vst)))

#define SIGMA_IO_TO_VIRT(n) ( \
        SIGMA_IO_TO_VIRT_BETWEEN((n), SIGMA_IO_0_BASE_PHYS, SIGMA_IO_0_SIZE) ?           	\
                SIGMA_IO_TO_VIRT_XLATE((n), SIGMA_IO_0_BASE_PHYS, SIGMA_IO_0_BASE_VIRT) :	\
        SIGMA_IO_TO_VIRT_BETWEEN((n), SIGMA_IO_ARM_BASE_PHYS, SIGMA_IO_ARM_SIZE) ?		\
                SIGMA_IO_TO_VIRT_XLATE((n), SIGMA_IO_ARM_BASE_PHYS, SIGMA_IO_ARM_BASE_VIRT) :	\
        NULL)

#define SIGMA_IO_ADDRESS(n) (SIGMA_IO_TO_VIRT(((unsigned int)(n) & 0xfffffff) | 0xf0000000))

/*
 * ----------------------------------------------------------------------------
 * Omap specific register access
 * ----------------------------------------------------------------------------
 */
#ifndef __ASSEMBLY__

#include <asm/io.h>
/*
 *  pa: phisical address 
 */

static inline unsigned char ReadRegByte(volatile void *pa)
{
#ifdef CONFIG_TRIX_SMC
	return secure_read_uint8((uint32_t)SIGMA_IO_ADDRESS(pa));
#else
	return __raw_readb(SIGMA_IO_ADDRESS(pa));
#endif
}

static inline unsigned short ReadRegHWord(volatile void *pa)
{
#ifdef CONFIG_TRIX_SMC
	return secure_read_uint16((uint32_t)SIGMA_IO_ADDRESS(pa));
#else
	return __raw_readw(SIGMA_IO_ADDRESS(pa));
#endif
}

static inline unsigned int ReadRegWord(volatile void *pa)
{
#ifdef CONFIG_TRIX_SMC
	return secure_read_uint32((uint32_t)SIGMA_IO_ADDRESS(pa));
#else
	return __raw_readl(SIGMA_IO_ADDRESS(pa));
#endif
}

static inline void WriteRegByte(volatile void *pa, unsigned char v)
{
#ifdef CONFIG_TRIX_SMC
	secure_write_uint8((uint32_t)SIGMA_IO_ADDRESS(pa), v, 0xff);
#else
	__raw_writeb(v, SIGMA_IO_ADDRESS(pa));
#endif
}

static inline void WriteRegHWord(volatile void *pa, unsigned short v)
{
#ifdef CONFIG_TRIX_SMC
	secure_write_uint16((uint32_t)SIGMA_IO_ADDRESS(pa), v, 0xffff);
#else
	__raw_writew(v, SIGMA_IO_ADDRESS(pa));
#endif
}

static inline void WriteRegWord(volatile void *pa, unsigned int v)
{
#ifdef CONFIG_TRIX_SMC
	secure_write_uint32((uint32_t)SIGMA_IO_ADDRESS(pa), v, 0xffffffff);
#else
	__raw_writel(v, SIGMA_IO_ADDRESS(pa));
#endif
}

#define MaskWriteReg(op, pa, v, m)	do{			\
		typeof(v) __tmp;				\
		__tmp = __raw_read##op(SIGMA_IO_ADDRESS(pa));	\
		__tmp &= ~(m);					\
		__tmp |= ((v) & (m));				\
		__raw_write##op(__tmp, SIGMA_IO_ADDRESS(pa));	\
	}while(0)

static inline void MWriteRegByte(volatile void *pa, unsigned char v, unsigned char m)
{
#ifdef CONFIG_TRIX_SMC
	secure_write_uint8((uint32_t)SIGMA_IO_ADDRESS(pa), v, m);
#else
	if (m != 0xff)
		MaskWriteReg(b, pa, v, m);
	else
		__raw_writeb(v, SIGMA_IO_ADDRESS(pa));
#endif
}

static inline void MWriteRegHWord(volatile void *pa, unsigned short v, unsigned short m)
{
#ifdef CONFIG_TRIX_SMC
	secure_write_uint16((uint32_t)SIGMA_IO_ADDRESS(pa), v, m);
#else
	if (m != 0xffff)
		MaskWriteReg(w, pa, v, m);
	else
		__raw_writew(v, SIGMA_IO_ADDRESS(pa));
#endif
}

static inline void MWriteRegWord(volatile void *pa, unsigned int v, unsigned int m)
{
#ifdef CONFIG_TRIX_SMC
	secure_write_uint32((uint32_t)SIGMA_IO_ADDRESS(pa), v, m);
#else
	if (m != 0xffffffff)
		MaskWriteReg(l, pa, v, m);
	else
		__raw_writel(v, SIGMA_IO_ADDRESS(pa));
#endif
}
#endif /* __ASSEMBLY__ */

#endif /*__ASM_ARCH_TRIX_PLAT_IO_H__*/
