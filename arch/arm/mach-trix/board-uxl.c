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
 * This file is used to define board specific supports
 *
 * Author: Tony He, 03/02/2015.
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

#include <plat/io.h>
#include <linux/delay.h>
#include <asm/hardware/cache-l2x0.h>
#include "common.h"

/*
 * UXLB A9 sub-system register is at 0x10004000
 * 0x1000_4012[7:4] dbg_swen
 */
static void __iomem *a9_sub_regbase = NULL;
#define UXLB_A9_SUB_BASE_PHYS (0x10004000|0xf0000000)
#define sub_sys_a9_readb(regaddr)	readb((unsigned long)a9_sub_regbase + (regaddr) - UXLB_A9_SUB_BASE_PHYS)
#define sub_sys_a9_writeb(value, regaddr) writeb((value), (unsigned long)a9_sub_regbase + (regaddr) - UXLB_A9_SUB_BASE_PHYS)

static int sub_a9_io_init(void)
{
	a9_sub_regbase = ioremap_nocache(0xf0004000, 0x1000);
	if(a9_sub_regbase == NULL) {
		pr_err("%s: Fail to map uxl a9 sub-system register;this will be fatal\n",__func__);
		return -1;
	}
	pr_notice("UXLB A9 sub-system iomap '%x' -> '%p'\n", UXLB_A9_SUB_BASE_PHYS, a9_sub_regbase);
	return 0;
}

static void sub_a9_io_exit(void)
{
	iounmap(a9_sub_regbase);
	a9_sub_regbase = NULL;
}

static int uxl_set_dbg_swen(unsigned int cpu, bool state)
{
#define A9_CFG2_REG ((void*)0xf0004012)	/*
					 * UXLB: [7..4] sw debug enable (to access dbg regs), each bit for one core
					 */
#define DBG_SWEN_MASK	0xf0
#define DBG_SWEN_SHIFT	4
	u8 temp8, val = 0;
	if (-1u == cpu) {
		val = (state ? DBG_SWEN_MASK : 0);	/*enable/disable sw debug on all cores*/
	} else if (cpu < NR_CPUS) {
		val = ((state ? 1 : 0) << DBG_SWEN_SHIFT);
	} else {
		pr_warn("invalid input cpu: %d\n", cpu);
		return 1;
	}

	if( sub_a9_io_init() )
		return -1;

	temp8 = sub_sys_a9_readb(A9_CFG2_REG);
	temp8 &= ~(DBG_SWEN_MASK);
	temp8 |= val & (DBG_SWEN_MASK);
	sub_sys_a9_writeb(val, A9_CFG2_REG);	/*enable sw debug on two cores*/
	mb();		/*drain write buffer*/
	ndelay(200);	/*explicit delay (~200ns) is required here as dbg_swen is in 24M clock domain*/

	/* No longer require this region to be mapped */
	sub_a9_io_exit();
	return 0;
}

#ifdef CONFIG_CACHE_L2X0
static int uxl_make_l2x_auxctrl(u32* val, u32* mask)
{
	u32 aux_ctrl;
	/*
	* UXLB L2CC: PL310@r3p2, 128K, 16ways, 8K per ways
	* Tag laterncy: 0, Data latency: 0
	*/

	/*RTL has well configured l2x, so...*/
	aux_ctrl = ReadRegWord((void*)(SIGMA_TRIX_L2CACHE_BASE + L2X0_AUX_CTRL));
	return aux_ctrl;
}
#endif

struct trix_board_object trix_board_obj = {
	.name = "UXLB",
	.ops = {
		.set_dbg_swen = uxl_set_dbg_swen,
#ifdef CONFIG_CACHE_L2X0
		.make_l2x_auxctrl = uxl_make_l2x_auxctrl,
#endif
	}
};
