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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/usb/otg.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/version.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#include <linux/irqchip/arm-gic.h>
#else
#include <asm/hardware/gic.h>
#endif

#include "common.h"

static void sigma_arch_reset(char mode, const char *cmd)
{
	printk("reset, loop... Fix me %s\n",__func__);
	while(1);
}
void (*arch_reset)(char, const char *) = sigma_arch_reset; 


static void __init sx6_init_early(void)
{
	printk("[%d]%s\n",__LINE__,__func__);
}

#if 0
static void sx6_display_init(void)
{
	printk("[%d]%s\n",__LINE__,__func__);
	return;
}
#endif

static void __init sx6_init(void)
{
	printk("[%d]%s\n",__LINE__,__func__);
}

static void sx6_reserve(void)
{
	printk("[%d]%s\n",__LINE__,__func__);
}

static void sx6_restart(char mode, const char *cmd)
{
	printk("SX6 restart\n");

	mcu_send_rest();

	while (1); /* wait forever */

	return;
}

MACHINE_START(SIGMA_SX6, "Sigma sx6 board")
	/* Maintainer: Sigma Designs, Inc. */
	.smp		= smp_ops(sx6_smp_ops),
	.reserve	= sx6_reserve,
	.map_io		= sx6_map_io,
	.init_early	= sx6_init_early,
	.init_irq	= sx6_init_irq,
	.init_machine	= sx6_init,
	.init_time	= sx6_timer_init,
	.restart	= sx6_restart,
MACHINE_END
