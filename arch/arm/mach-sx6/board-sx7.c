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
 * Author: Tony He, 09/12/2014.
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

static int sx7_set_dbg_swen(unsigned int cpu, bool state)
{
#define A9_CFG2_REG ((void*)0x1500ef12)	/*
					 * SX7: [7..4] sw debug enable (to access dbg regs), each bit for one core
					 */
#define DBG_SWEN_MASK	0xf0
#define DBG_SWEN_SHIFT	4
	u8 val = 0;
	if (-1u == cpu) {
		val = (state ? DBG_SWEN_MASK : 0);	/*enable/disable sw debug on two cores*/
	} else if (cpu < NR_CPUS) {
		val = ((state ? 1 : 0) << DBG_SWEN_SHIFT);
	} else {
		pr_warn("invalid input cpu: %d\n", cpu);
		return 1;
	}

	MWriteRegByte(A9_CFG2_REG, val, DBG_SWEN_MASK);	/*enable sw debug on two cores*/
	mb();		/*drain write buffer*/
	ndelay(200);	/*explicit delay (~200ns) is required here as dbg_swen is in 24M clock domain*/
	return 0;
}

static u32 sx7_make_l2x_auxctrl(void)
{
	u32 aux_ctrl;
	/*
	* SX7 L2CC: PL310@r3p2, 1024K, 16ways, 64K per ways
	* Tag laterncy: 0, Data latency: 0
	*/

	aux_ctrl = ((1 << L2X0_AUX_CTRL_ASSOCIATIVITY_SHIFT) |		//bit 16: 1 for 16 ways
			(0x3 << L2X0_AUX_CTRL_WAY_SIZE_SHIFT) |		//bit 19-17: 011b 64KB, 010b 32KB
			(1 << L2X0_AUX_CTRL_SHARE_OVERRIDE_SHIFT) |     //bit 22: 1 shared attribute internally ignored.
			(1 << 25) |					//bit 25: SBO/RAO
			(1 << L2X0_AUX_CTRL_NS_LOCKDOWN_SHIFT) |	//bit 26: 1 NS can write to the lockdown register
			(1 << L2X0_AUX_CTRL_NS_INT_CTRL_SHIFT) |	//bit 27: 1 Int Clear and Mask can be NS accessed
			(1 << L2X0_AUX_CTRL_DATA_PREFETCH_SHIFT) |	//bit 28: 1 data prefetching enabled
			(1 << L2X0_AUX_CTRL_INSTR_PREFETCH_SHIFT) |	//bit 29: 1 instruction prefetch enabled
			(1 << L2X0_AUX_CTRL_EARLY_BRESP_SHIFT));	//bit 30: 1 early BRESP eanbled

	return aux_ctrl;
}

struct trix_board_object trix_board_obj = {
	.name = "SX7",
	.ops = {
		.set_dbg_swen = sx7_set_dbg_swen,
		.make_l2x_auxctrl = sx7_make_l2x_auxctrl,
	}
};
