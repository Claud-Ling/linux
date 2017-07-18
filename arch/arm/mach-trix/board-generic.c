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
#include "chipver.h"

static void sigma_arch_reset(char mode, const char *cmd)
{
	printk("reset, loop... Fix me %s\n",__func__);
	while(1);
}
void (*arch_reset)(char, const char *) = sigma_arch_reset; 


static void __init trix_init_early(void)
{
	//printk("[%d]%s\n",__LINE__,__func__);
}

static void trix_poweroff(void)
{
	printk("%s power off\n", dtv_chip_name(dtv_get_chip_id()));

	trix_mcomm_send_poweroff();

	while (1); /* wait forever */

	return;
}

static void __init trix_init(void)
{
	//printk("[%d]%s\n",__LINE__,__func__);

	pm_power_off = trix_poweroff;
	regulator_has_full_constraints();
}

static void trix_reserve(void)
{
	//printk("[%d]%s\n",__LINE__,__func__);
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 10, 0)
static void trix_restart(enum reboot_mode mode, const char *cmd)
#else
static void trix_restart(char mode, const char *cmd)
#endif
{
	printk("%s restart\n", dtv_chip_name(dtv_get_chip_id()));

	trix_mcomm_send_reboot();

	while (1); /* wait forever */

	return;
}
MACHINE_START(TRIX_SXX, "Sigma "CONFIG_TRIX_SOC_NAME" board")
	/* Maintainer: Sigma Designs, Inc. */
	.smp		= smp_ops(trix_smp_ops),
	.reserve	= trix_reserve,
	.map_io		= trix_map_io,
	.init_early	= trix_init_early,
	.init_irq	= trix_init_irq,
	.init_machine	= trix_init,
	.init_time	= trix_init_timer,
	.restart	= trix_restart,
MACHINE_END

/*board specific operations*/
extern struct trix_board_object trix_board_obj;
#define TRIX_BOARD_OPS trix_board_obj.ops
const char* trix_board_name(void)
{
	return trix_board_obj.name;
}

int trix_set_dbg_swen(unsigned int cpu, bool state)
{
	BUG_ON(!TRIX_BOARD_OPS.set_dbg_swen);
	return TRIX_BOARD_OPS.set_dbg_swen(cpu, state);
}

int trix_make_l2x_auxctrl(u32* val, u32* mask)
{
	BUG_ON(!TRIX_BOARD_OPS.make_l2x_auxctrl);
	return TRIX_BOARD_OPS.make_l2x_auxctrl(val, mask);
}


