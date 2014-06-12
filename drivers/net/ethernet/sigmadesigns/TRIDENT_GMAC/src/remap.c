/*
 *
 *  (c)copyright 2011 trident microsystem ,inc. all rights reserved.
 *     jason mei  jason.mei@tridentmicro.com
 *  version 1.0 2011/07/28
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (c) 2006-2007, LIPP Alliance
 *
 * All Rights Reserved.
 *
 *---------------------------------------------------------------------------
 * %filename:         remap.c    %
 * %pid_version:     1.2          %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  This file contains functions for remapping and unmapping 
 *               device base addresses, for registering interrupt routines
 *
 * DOCUMENT REF: 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/
#include "../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h"
#include "remap.h"
#include <asm/io.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include "gmac_drv.h"
#include <linux/interrupt.h>
/* Remap base addresses and assign to the global variables for 
** accessing the device registers
*/
__s32 remapBaseAdrs(struct platform_device *pdev)
{
    struct resource *r1;

    r1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!r1)
        return -ENODEV;

    if (r1->flags == IORESOURCE_MEM) {
        ghwTRIDENTGMACEth_Base[pdev->id].baseAddress = r1->start + 0xe0000000;
    } else {
        GMAC_PRINT_ERR("Error in mem address for unit %d\n", pdev->id);
    }
    return 0;
}

/* Register interrupt service routines for ethernet IP */
__s32 registerInterrupts(struct platform_device *pdev)
{
    struct resource *r1;

    struct net_device *dev = dev_get_drvdata(&pdev->dev);

    /* Process the IO resources */
    r1 = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (!r1)
        return -ENODEV;
        
	if(r1->flags == IORESOURCE_IRQ) {
	    /* This is not used in the driver. This stores the ethernet 
	    ** power mgmt interrupt only not the ethernet interrupt line.
	    */
	    dev->irq = (__u32) (r1->start);

	    GMAC_PRINT_ERR("Registering Ethernet ISR number %d\n",dev->irq);

	    if( request_irq( dev->irq, trident_gmacEth_isr,IRQF_SHARED,"Ethernet-GMAC", dev) < 0) {
		GMAC_PRINT_ERR("Error while installing the ISR, aborting\n");
		return -1 ;
	    }

	} else {
	    GMAC_PRINT_ERR("Error in IO resiurce, aborting\n");
	}
    return 0;

}

void unmapBaseAdrs(struct platform_device *pdev)       
{
    struct resource *r1;

    /* Process the memory resources */
    r1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!r1)
        return;
                
    if (r1->flags == IORESOURCE_MEM) {
        ghwTRIDENTGMACEth_Base[pdev->id].baseAddress = 0;
    } else {
        GMAC_PRINT_ERR("Error while unmapping the Base, aborting\n");
    }
}

void unregisterInterrupts(struct platform_device *pdev)       
{
    struct resource *r1;

    struct net_device *dev = dev_get_drvdata(&pdev->dev);

    /* Free the IO resources */
    r1 = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (!r1)
        return;
   
    if (r1->flags == IORESOURCE_IRQ) {
        dev->irq = (__u32) (r1->start);
        free_irq(dev->irq,dev);
    } else {
        GMAC_PRINT_ERR("Error while unregistering the ISR, aborting\n");
    }
}

