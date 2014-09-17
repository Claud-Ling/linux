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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#include <linux/irqchip/arm-gic.h>
#else
#include <asm/hardware/gic.h>
#endif
#include <mach/hardware.h>
#include <mach/irqs.h>

static void __iomem *gic_cpu_base 	= SIGMA_IO_ADDRESS(SIGMA_TRIX_GIC_CPU_BASE);
static void __iomem *gic_dist_base_addr	= SIGMA_IO_ADDRESS(SIGMA_TRIX_GIC_DIST_BASE);

void __init trix_init_irq(void)
{
	TRI_DBG("%d %s\n",__LINE__,__func__);

	printk("int dist iomap '%x' -> '%p'\n", SIGMA_TRIX_GIC_DIST_BASE, gic_dist_base_addr);
	BUG_ON(!gic_dist_base_addr);

	TRI_DBG("%d %s\n",__LINE__,__func__);
	printk("gic iomap '%x' -> '%p'\n", SIGMA_TRIX_GIC_CPU_BASE, gic_cpu_base);
	BUG_ON(!gic_cpu_base);

	TRI_DBG("%d %s\n",__LINE__,__func__);
	gic_init(0, TRIHIDTV_GIC_IRQ, gic_dist_base_addr, gic_cpu_base);
	TRI_DBG("%d %s\n",__LINE__,__func__);
}

