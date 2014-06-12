/*
 * (C)Copyright 2011 Trident Microsystem ,Inc. All Rights Reserved.
 * Jason mei  jason.mei@tridentmicro.com
 * Version 1.0 2011/07/28
 *
 * Copyright 2010 Trident Microsystems (Far East) Ltd. 
 * All prior copyright rights in this work and the accompanying software 
 * products transferred to Trident Microsystems (Far East) Ltd. by written 
 * agreement.  All rights reserved.
 * 
 * Copyright 2009 (C) NXP BV, All Rights Reserved
 * Author: OKC <okc.helpdesk@nxp.com>
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

#include <linux/platform_device.h>
#include <linux/init.h>
#include <asm/io.h>
#include <plat/gmac_eth_drv.h>
#include <asm/setup.h>
#include <linux/module.h>

#ifdef CONFIG_SIGMA_TRIX_ENET
static gmac_platform_data_t gmac0_platform_data =
{
	0,
	GMAC0_CLK_CSR_VAL,
	GMAC0_MAX_SPEED,
	{0x00,0x06,0x37,0xFF,0xFF,0x01},
};

static struct resource gmac0_resources[] = {
	{
		.start		= GMAC0_BASE,
		.end		= GMAC0_BASE + 0x2000 - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= IRQ_GMAC0,
		.end		= IRQ_GMAC0,
		.flags		= IORESOURCE_IRQ,
	},
	{
		.start		= IRQ_GMAC0_POWER,
		.end		= IRQ_GMAC0_POWER,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device gmac0_device = {
	.name		= "SIGMA_Trix_GMAC",
	.id		= 0,
	.dev.platform_data	= &gmac0_platform_data,
	.num_resources	= ARRAY_SIZE(gmac0_resources),
	.resource	= gmac0_resources,
};
#endif

static int __init gmac_ethernet_init(void)
{
    int err = 0;

#ifdef CONFIG_SIGMA_TRIX_ENET
	gmac0_platform_data.isExternal = 1;
        /* Register GMAC0 */
        if (platform_device_register(&gmac0_device)) {
            err--;
        }
#endif

    return err;
}

arch_initcall(gmac_ethernet_init);

/* Mac address from TAG List */
unsigned char stb_mac_address[1][6];
EXPORT_SYMBOL(stb_mac_address);

