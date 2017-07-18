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
 *  Author: Tony He, 2016.
 */

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/clockchips.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/smp_twd.h>
#include "tri_dbg.h"
#include "common.h"

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(trix_twd_local_timer,
			      SIGMA_TRIX_LOCAL_TWD_BASE,
			      TRIHIDTV_LOCAL_TIMER_IRQ);
/*
 * local timer setup, driver smp tick.
 */
static void __init trix_local_twd_init(void)
{
	int err = twd_local_timer_register(&trix_twd_local_timer);
	if (err)
		pr_err("twd_local_timer_register failed %d\n", err);
}
#else
#define trix_local_twd_init() do { } while(0)
#endif


#define SOC_TIMER_FREQ          (200UL * 1000 * 1000)   // timer are clk @200MHz

enum {
	SOC_TIMER0	= 0,
	SOC_TIMER1
};

void __init trix_init_timer(void)
{
	trix_timer_init((void __iomem *)SIGMA_SOC_TIMER_BASE,
			SOC_TIMER0, SOC_TIMER1,
			SOC_TIMER_FREQ, TRIHIDTV_TIMER_1_INTERRUPT);

	trix_local_twd_init();
}
