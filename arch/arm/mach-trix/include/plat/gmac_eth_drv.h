/*
 *  (C)Copyright 2011 Trident Microsystem ,Inc. All Rights Reserved.
 *     Jason mei  jason.mei@tridentmicro.com
 *  Version 1.0 2011/07/28
 *
 * Copyright 2010 Trident Microsystems (Far East) Ltd. 
 * All prior copyright rights in this work and the accompanying software 
 * products transferred to Trident Microsystems (Far East) Ltd. by written 
 * agreement.  All rights reserved.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __ASM_ARCH_TRIX_PLAT_GMAC_ETH_DRV_H__
#define __ASM_ARCH_TRIX_PLAT_GMAC_ETH_DRV_H__

#include <asm/io.h>
#include <asm/irq.h>
/* Information, which can vary between two ethernet IPs */
typedef struct gmac_platform_data
{
	__u32 isExternal;
	__u32 clk_csr_val;
	__u32 max_speed;
	__u8 mac_addr[6];

}gmac_platform_data_t,*pgmac_platform_data_t;

/* External PHY */
#define GMAC0_BASE		0x15030000    //base address will be remaped to 0xf5030000 in driver..
#define IRQ_GMAC0		TRIHIDTV_ETHERNET_INTERRUPT
#define IRQ_GMAC0_POWER		TRIHIDTV_ETHERNET_PMT_INTERRUPT
#define GMAC0_CLK_CSR_VAL	(0x1U )
#define GMAC0_CTRL_GLOBAL_REG	0xf5030000 
#define PINSHARE_REG_A0		0x1500e0a0
#define PINSHARE_REG_A4		0x1500e0a4
#define INTERRUPTCTL_REG	0x15031f04

#ifdef CONFIG_GMAC_MODE_RGMII
#define GMAC0_MAX_SPEED	(1000U)
#else
#define GMAC0_MAX_SPEED	(100U)
#endif

#endif /*__ASM_ARCH_TRIX_PLAT_GMAC_ETH_DRV_H__*/
