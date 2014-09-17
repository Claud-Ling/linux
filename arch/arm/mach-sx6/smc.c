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
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/export.h>

#include <asm/hardware/cache-l2x0.h>
#include <mach/hardware.h>

#include "smc.h"


/**************************************************************************/
/* sx6 secure monitor configuration					  */
/* Give "have_armor" in cmdline to fit with secure monitor (armor), and   */
/* linux runs in non-secure then					  */
/**************************************************************************/
static int security_state = 1;	/*0 - NS, 1 - Secure*/
static int __init do_with_secure_monitor(char *str)
{
	printk(KERN_INFO "##linux start with secure monitor##\n");
	security_state = 0;
	return 0;
}
early_param("with_armor", do_with_secure_monitor);

/*
 * Return current security state
 * 0 - Non-secure, 1 - secure
 */
int get_security_state(void)
{
	return security_state;
}
EXPORT_SYMBOL(get_security_state);

/**************************************************************************/
/* sx6 l2x0 control wrapper						  */
/**************************************************************************/
#ifdef CONFIG_SIGMA_CACHE_L2X0
extern void __iomem *sx6_get_l2cache_base(void);
static void __iomem *l2c_base = NULL;
#define GET_L2C_BASE ((NULL==l2c_base) ? (l2c_base=sx6_get_l2cache_base()) : l2c_base)

void sx6_smc_l2x0_enable(void)
{
	/* Disable PL310 L2 Cache controller */
	if ( !security_state )
		sigma_dtv_smc1(ARMOR_SMCCALL_SET_L2_CONTROL, 0x1);
	else
		writel_relaxed(1, GET_L2C_BASE + L2X0_CTRL);
}
void sx6_smc_l2x0_disable(void)
{
	/* Disable PL310 L2 Cache controller */
	if ( !security_state )
		sigma_dtv_smc1(ARMOR_SMCCALL_SET_L2_CONTROL, 0x0);
	else
		writel_relaxed(0, GET_L2C_BASE + L2X0_CTRL);
}

void sx6_smc_l2x0_set_auxctrl(unsigned long val)
{
	/* Set L2 Cache auxiliary control register */
	if ( !security_state )
		sigma_dtv_smc1(ARMOR_SMCCALL_SET_L2_AUX_CONTROL, val);
	else
		writel_relaxed(val, GET_L2C_BASE + L2X0_AUX_CTRL);
}

void sx6_smc_l2x0_set_debug(unsigned long val)
{
	/* Program PL310 L2 Cache controller debug register */
	if ( !security_state )
		sigma_dtv_smc1(ARMOR_SMCCALL_SET_L2_AUX_DEBUG, val);
	else
		writel_relaxed(val, GET_L2C_BASE + L2X0_DEBUG_CTRL);
}

void sx6_smc_l2x0_set_prefetchctrl(unsigned long val)
{
	/* Program PL310 L2 Cache controller prefetch control register */
	if ( !security_state )
		;//TODO: sigma_dtv_smc1(ARMOR_SMCCALL_SET_L2_PREFETCH_CTRL, val);
	else
		writel_relaxed(val, GET_L2C_BASE + L2X0_PREFETCH_CTRL);
}
#endif

/*
 * 1. ACTLR R/W in secure state;
 * 2. ACTLR RO in Non-secure state if NSACR.NS_SMP=0;
 * 3. ACTLR R/W in Non-secure state if NSACR.NS_SMP=1. In this case all bits are write ignored except for the SMP bit
 */
void sx6_smc_set_actlr(unsigned long val)
{
	/* Program PL310 L2 Cache controller debug register */
	if ( !security_state )
		;				/*armor N/A yet*/
	else
		__asm__ __volatile__("mcr	p15, 0, %0, c1, c0, 1": : "r" (val):);
}

static int __init sigma_smc_init(void)
{
	return 0;
}
early_initcall(sigma_smc_init);
