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

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/clockchips.h>
#include <asm/irq.h>
#include <asm/smp_twd.h>
#include <asm/localtimer.h>

#ifdef CONFIG_LOCAL_TIMERS

static DEFINE_TWD_LOCAL_TIMER(trix_twd_local_timer, SIGMA_TRIX_LOCAL_TWD_BASE, TRIHIDTV_LOCAL_TIMER_IRQ);

/*
 * local timer setup, driver smp tick.
 */
int __init trix_local_timer_setup(void)
{
	int err = 1;
	TRI_DBG("[%d] %s\n",__LINE__,__func__);
#ifdef CONFIG_HAVE_ARM_TWD
	err = twd_local_timer_register(&trix_twd_local_timer);
	if (err)
		printk("twd_local_timer_register failed %d\n", err);
#endif
	TRI_DBG("[%d] %s\n",__LINE__,__func__);
	return err;
}

#endif
