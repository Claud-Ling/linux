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
 *  Author: Tony He, 2016.
 */

#ifndef __ASM_ARCH_TRIX_PLAT_SMP_H__
#define __ASM_ARCH_TRIX_PLAT_SMP_H__

/*
*0x1502_4140: undefined instruction handler address
*0x1502_4144: SWI handler address
*0x1502_4148: prefetch handler address
*0x1502_414c: abort handler address
*0x1502_4150: not used handler address
*0x1502_4154: IRQ handler address
*0x1502_4158: FIQ handler address

*0x1502_415c: CPU1 start address
*cpu1 while loop to check this register. if 0, loop. If != 0, jump to the address specified in this 
*/

/*
 * secondary boot register (32-bit)
 *
 * +--------------------------------+
 * |31                        4|3  0|
 * +---------------------------+----+
 * | physical entry address    | id |
 * +---------------------------+----+
 *
 * Note:
 * entry address must be 16-byte aligned at least
 */
#if defined(CONFIG_SIGMA_SOC_SX6) || defined(CONFIG_SIGMA_SOC_UXLB) || defined(CONFIG_SIGMA_SOC_SX7)
#define AUX_BOOT_ADDR_REG	0xf502415c
#else
#define AUX_BOOT_ADDR_REG	0xfb00f07c	/*this belongs to sectimer target,
						 *can be non-accessible in NS world
						 */
#endif
#define AUX_BOOT_ID_REG		0xf502415c	/*use it as duplicated copy of aux_boot_addr<id>
						 *field for non-secure world, as ns may need read
						 *<id> field under circumstance where smc call is
						 *not available. For instance after move core out
						 *of coherency, i.e. hotplug.
						 */
#define AUX_BOOT_ID_BITS	4
#define AUX_BOOT_ID_MASK	((1 << AUX_BOOT_ID_BITS) - 1)
#define AUX_BOOT_ADDR_MASK	(~AUX_BOOT_ID_MASK)

#ifndef __ASSEMBLY__

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#include <linux/irqchip/arm-gic.h>
#else
#include <asm/hardware/gic.h>
#endif

#endif

#endif /*__ASM_ARCH_TRIX_PLAT_SMP_H__*/
