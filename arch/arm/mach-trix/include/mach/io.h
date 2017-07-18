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

#include <plat/io.h>

#ifndef __ASM_ARCH_TRIX_MACH_IO_H__
#define __ASM_ARCH_TRIX_MACH_IO_H__

/*usb*/

#if defined(CONFIG_SIGMA_SOC_SX6) || defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX8)

#define SIGMA_EHCI_BASE     0xf502f100
#define SIGMA_EHCI_LEN      0x200

#define SIGMA_EHCI2_BASE    0xfb008100
#define SIGMA_EHCI2_LEN     0x200

#elif defined(CONFIG_SIGMA_SOC_UXLB)

#define SIGMA_EHCI_BASE     0xfb003100
#define SIGMA_EHCI_LEN      0x200

#define SIGMA_EHCI2_BASE    0xfb007100
#define SIGMA_EHCI2_LEN     0x200

#endif

//#define SIGMA_GIC_BASE		0xF2100000 
//#define SIGMA_L2C_BASE		0xF2102000 
//#define SIGMA_LOCAL_TWD_BASE 	0xF2000000
#define SIGMA_XHCI_BASE    0xf5200000
#define SIGMA_XHCI_LEN     0xd000
#define SIGMA_XHCI_PTR               IO_PTR(SIGMA_SX6_XHCI_BASE)

#if defined(CONFIG_SIGMA_SOC_SX6)
#define SIGMA_SDHCI_1_BASE             0xfb00a000
#define SIGMA_SDHCI_1_LEN              0x100
#elif defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX8)
#define SIGMA_SDHCI_1_BASE             0xfb00c000
#define SIGMA_SDHCI_1_LEN              0x100
#elif defined(CONFIG_SIGMA_SOC_UXLB)
#define SIGMA_SDHCI_1_BASE             0xfb005a00
#define SIGMA_SDHCI_1_LEN              0x100
#endif

#if defined(CONFIG_SIGMA_SOC_SX6) || defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX8)
#define SIGMA_SDHCI_2_BASE             0xfb00a000
#define SIGMA_SDHCI_2_LEN              0x100
#elif defined(CONFIG_SIGMA_SOC_UXLB)
#define SIGMA_SDHCI_2_BASE             0xfb005c00
#define SIGMA_SDHCI_2_LEN              0x100
#endif

#define SIGMA_MI2C0_BASE		0xf502c000	//I2C Master 1
#define SIGMA_MI2C1_BASE		0xf502d000	//I2C Master 2
#define SIGMA_MI2C2_BASE		0xf502e000	//I2C Master 3
#define SIGMA_MI2C3_BASE		0xf502b000	//I2C Master 4

#define SIGMA_MI2C_SIZE			0x100

#define SIGMA_GMAC0_BASE		0xf5030000
#define SIGMA_GMAC0_SIZE		0x2000

#define SIGMA_SPI_NOR_BASE		0xfb002000
#define SIGMA_SPI_NOR_SIZE		0x400

#define SIGMA_SPI_NOR_IO		0xfdbe0000	/* 0xfdc00000 - 0x20000 */
#define SIGMA_SPI_IO_SIZE		0x400000

#define SIGMA_SOC_TIMER_BASE		0xf5027000
#endif /*__ASM_ARCH_TRIX_MACH_IO_H__*/
