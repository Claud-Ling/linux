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
#include <asm/setup.h>
#include <asm/irq.h>
#include <linux/module.h>

#define UXL_FTMAC0_IO_BASE 0x1B004000
#define IRQ_FTMAC0	TRIHIDTV_ETHERNET_IRQ

static u64 dmamask = ~(u32)0;
static void uxl_release_dev(struct device * dev)
{
        dev->parent = NULL;
}

static struct resource uxl_eth_resources[] = {
	{
		.start		= UXL_FTMAC0_IO_BASE,
		.end		= UXL_FTMAC0_IO_BASE + 0x100 - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= IRQ_FTMAC0,
		.end		= IRQ_FTMAC0,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device uxl_eth_device = {
	.name		= "ftmac110",
	.id		= 0,
	.dev = {
		.dma_mask		= &dmamask,
		.coherent_dma_mask	= 0xffffffff,
		.release		= uxl_release_dev,
        },
	.num_resources	= ARRAY_SIZE(uxl_eth_resources),
	.resource	= uxl_eth_resources,
};

static int __init uxl_ethernet_init(void)
{
    int err = 0;

        /* Register FTMAC0 */
        if (platform_device_register(&uxl_eth_device)) {
            err--;
        }

    return err;
}

arch_initcall(uxl_ethernet_init);

