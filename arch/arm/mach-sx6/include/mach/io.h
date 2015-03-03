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
 *  Author: Qipeng Zha, 2012.
 */

#include <plat/io.h>
/*usb*/
#define SIGMA_EHCI_BASE     0xf502f100
#define SIGMA_EHCI_LEN      0x200
#define SIGMA_EHCI_PTR               IO_PTR(SIGMA_SX6_EHCI_BASE)


#define SIGMA_EHCI2_BASE    0xfb008100
#define SIGMA_EHCI2_LEN     0x200
#define SIGMA_EHCI2_PTR               IO_PTR(SIGMA_SX6_EHCI2_BASE)

//#define SIGMA_GIC_BASE		0xF2100000 
//#define SIGMA_L2C_BASE		0xF2102000 
//#define SIGMA_LOCAL_TWD_BASE 	0xF2000000
#define SIGMA_XHCI_BASE    0xf5200000
#define SIGMA_XHCI_LEN     0xc00
#define SIGMA_XHCI_PTR               IO_PTR(SIGMA_SX6_XHCI_BASE)

#if defined(CONFIG_SIGMA_SOC_SX6)
#define SIGMA_SDHCI_1_BASE             0xfb00a000
#define SIGMA_SDHCI_1_LEN              0x100
#elif defined(CONFIG_SIGMA_SOC_SX7)
#define SIGMA_SDHCI_1_BASE             0xfb00c000
#define SIGMA_SDHCI_1_LEN              0x100
#elif defined(CONFIG_SIGMA_SOC_UXLB)
#define SIGMA_SDHCI_1_BASE             0xfb005a00
#define SIGMA_SDHCI_1_LEN              0x100
#endif

#if defined(CONFIG_SIGMA_SOC_SX6) || defined(CONFIG_SIGMA_SOC_SX7)
#define SIGMA_SDHCI_2_BASE             0xfb00a000
#define SIGMA_SDHCI_2_LEN              0x100
#elif defined(CONFIG_SIGMA_SOC_UXLB)
#define SIGMA_SDHCI_2_BASE             0xfb005c00
#define SIGMA_SDHCI_2_LEN              0x100
#endif
