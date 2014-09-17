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

#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cpu_pm.h>

#include <asm/mach/time.h>
#include <asm/localtimer.h>
#include <asm/sched_clock.h>
#include <asm/smp_twd.h>
#include <mach/io.h>
#include "common.h"

/*timer 0*/
#define TIMER_TCVR0            	0xf5027000  	/*constant value*/
#define TIMER_TRVR0            	0xf5027004	/*counter*/	
#define TIMER_TCR0             	0xf5027008	/*control, bit1: start or stop, bit0: reload or no*/	

/*timer 1*/
#define TIMER_TCVR1            	0xf502700c 
#define TIMER_TRVR1           	0xf5027010 
#define TIMER_TCR1             	0xf5027014	

#define TIMER_TIRR		0xf5027018	/*interrupt request,wirte 1 to clear, bit[1:0], timer1 and time0*/	
#define TIMER_TIDR		0xf502701c	/*interrupt request disable, bit[1:0], set 1 to disable, timer1 and time0*/	

/*frequency*/
#ifndef CONFIG_SIGMA_VELOCE
#define SOC_TIMER_FREQ 		(200UL * 1000 * 1000)	// timer are clk @200MHz
#else
#define SOC_TIMER_FREQ 		(60UL * 1000)		// timer are clk @60KHz for emulation
#endif

/*
 * This driver configures the 2 32-bit count-down timers as follows:
 *
 * T1: Timer 0, clocksource for generic timekeeping
 * T2: Timer 1, clockevent source for hrtimers
 *
 * The input frequency to the timer module for emulation is 60KHz which is
 * common to all the timer channels (T1, T2). The timers are clocked 
 * at 60KHz (16.7 us resolution).
 *
 * The input frequency to the timer module in silicon will be 200MHz. The timers 
 * are clocked at 200MHz (5ns resolution).
 */

static int sigma_sx6_timer_set_next_event(unsigned long cycles,
					 struct clock_event_device *evt)
{
	//printk("[%d] %s 0x%08x\n",__LINE__,__func__,(unsigned int)cycles);	

	/*stop counting*/
	WriteRegWord((void*)TIMER_TCR1, 0x0);

	/*set constant */
	WriteRegWord((void*)TIMER_TCVR1, cycles);

	/*load and start counting*/
	WriteRegWord((void*)TIMER_TCR1, 0x3);

	return 0;
}

static void sigma_sx6_timer_set_mode(enum clock_event_mode mode,
				    struct clock_event_device *evt)
{
	u32 period;

	TRI_DBG("[%d] %s mode %08x\n",__LINE__,__func__,(unsigned int)mode);	

	switch (mode) {

	case CLOCK_EVT_MODE_PERIODIC:
		TRI_DBG("%s[%d] periodic\n",__func__,__LINE__);	
		WriteRegWord((void*)TIMER_TCR1, 0x0);	//stop
		period = SOC_TIMER_FREQ / HZ;
		WriteRegWord((void*)TIMER_TCVR1, period);
		WriteRegWord((void*)TIMER_TCR1, 0x3);	//start
		break;

	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		TRI_DBG("%s[%d] oneshot\n",__func__,__LINE__);	
		WriteRegWord((void*)TIMER_TCR1, 0x0);	//stop
		break;

	case CLOCK_EVT_MODE_RESUME:
		TRI_DBG("%s[%d] resume\n",__func__,__LINE__);	
		WriteRegWord((void*)TIMER_TCR1, 0x3);	//start
		break;
	}
}

static DEFINE_PER_CPU(struct clock_event_device, soctimer_events);

static struct clock_event_device soctimer_clockevent = {
	.name		= "soc timer",
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.set_next_event	= sigma_sx6_timer_set_next_event,
	.set_mode	= sigma_sx6_timer_set_mode,
};

/*
 * SOC Timer ack 
 */
static void soctimer_ack(void)
{
	unsigned int val;

	//clear timer2 interrupt request bit here...
	val = ReadRegWord((void*)TIMER_TIRR);
	val |= 0x2;
	WriteRegWord((void*)TIMER_TIRR, val);
}

static irqreturn_t sigma_sx6_timer_interrupt(int irq, void *dev_id)
{
	int cpu = smp_processor_id();
	struct clock_event_device *evt = &per_cpu(soctimer_events, cpu);
	
	//printk("[%d] %s\n",__LINE__,__func__);	
	//printk("--------%s %08x %08x\n",__func__,evt, evt->event_handler);
	irq_enter();

	soctimer_ack();

	/*hrtimer_interrupt when switch to hrtimer*/
	if (evt->event_handler)
	{
		//printk("[%d] %s %08x\n",__LINE__,__func__,(unsigned int)evt->event_handler);	
		evt->event_handler(evt);
	}
	else
	{
		printk("Warning: %s \n",__func__);
	}

	irq_exit();
	
	return IRQ_HANDLED;
}

static struct irqaction sigma_sx6_timer_irq = {
	.name		= "soc timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= sigma_sx6_timer_interrupt,
};

static void __init sigma_sx6_clockevent_init(void)
{
	int cpu = smp_processor_id();
	struct clock_event_device *levt = &per_cpu(soctimer_events,cpu);

	TRI_DBG("[%d] %s\n",__LINE__,__func__);	
	/*register irq handler*/
	setup_irq(TRIHIDTV_TIMER_1_INTERRUPT, &sigma_sx6_timer_irq);

	TRI_DBG("[%d] %s\n",__LINE__,__func__);	
	/*register clock event*/
	clockevents_calc_mult_shift(&soctimer_clockevent, SOC_TIMER_FREQ, 4);
	soctimer_clockevent.max_delta_ns 	= clockevent_delta2ns(0xffffffff, &soctimer_clockevent);
	soctimer_clockevent.min_delta_ns 	= clockevent_delta2ns(0x1, &soctimer_clockevent);
	soctimer_clockevent.cpumask		= (struct cpumask *)&cpumask_of_cpu(cpu);
	soctimer_clockevent.rating		= 300; //fix me in case multi devices
	TRI_DBG("%#llx %#llx\n",soctimer_clockevent.min_delta_ns, soctimer_clockevent.max_delta_ns);	

	TRI_DBG("[%d] %s\n",__LINE__,__func__);	
	memcpy(levt, &soctimer_clockevent, sizeof(*levt));
	clockevents_register_device(levt);

	TRI_DBG("[%d] %s\n",__LINE__,__func__);	
}

/*
 * clocksource
 */
static cycle_t clocksource_read_cycles(struct clocksource *cs)
{
	return (cycle_t)(0xffffffff - ReadRegWord((void*)TIMER_TRVR0));
}

#ifdef CONFIG_PM
static void sx6_clksrc_resume(struct clocksource *cs)
{
	unsigned int tmpsh ;
	 /* 
	 * make timer0 starting countering 
	 * used as clock source, counterinng from 0xffffffff to 0 and again
	 */
	WriteRegWord((void*)TIMER_TCVR0, 0xffffffff);

	/*clear interrupt request*/
	tmpsh = ReadRegWord((void*)TIMER_TIDR);
	tmpsh |= 0x1;
	WriteRegWord((void*)TIMER_TIDR, tmpsh);

	/*disable interrupt request*/
	tmpsh = ReadRegWord((void*)TIMER_TIRR);
	tmpsh |= 0x1;
	WriteRegWord((void*)TIMER_TIRR, tmpsh);

	/*start counting and reload*/
	tmpsh = ReadRegWord((void*)TIMER_TCR0);
	tmpsh |= 3;
	WriteRegWord((void*)TIMER_TCR0, tmpsh); 

}
#endif
static struct clocksource clocksource_soc = {
	.name		= "soc timer",
	.rating		= 200,
	.read		= clocksource_read_cycles,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
#ifdef CONFIG_PM
	.resume         = sx6_clksrc_resume, 
#endif
};

static void soctimer_init(void)
{
	unsigned int tmpsh ;
	unsigned int timer_constant_value = (SOC_TIMER_FREQ + HZ / 2) / HZ;

	TRI_DBG("[%d] %s @%#lxHz\n",__LINE__,__func__,SOC_TIMER_FREQ);	

	/*stop timers*/
	//WriteRegWord((void*)TIMER_TCR0, 0); 
	//WriteRegWord((void*)TIMER_TCR1, 0); 

	 /* 
	 * make timer1 starting to generate the timer interrupt
	 */
	WriteRegWord((void*)TIMER_TCVR1, timer_constant_value);

	/*clear interrupt request*/
	tmpsh = ReadRegWord((void*)TIMER_TIRR);
	tmpsh |= 0x2;
	WriteRegWord((void*)TIMER_TIRR, tmpsh);

	/*enable interrupt request*/
	tmpsh = ReadRegWord((void*)TIMER_TIDR);
	tmpsh &= ~2;
	WriteRegWord((void*)TIMER_TIDR, tmpsh);

	 /* 
	 * make timer0 starting countering 
	 * used as clock source, counterinng from 0xffffffff to 0 and again
	 */
	WriteRegWord((void*)TIMER_TCVR0, 0xffffffff);

	/*clear interrupt request*/
	tmpsh = ReadRegWord((void*)TIMER_TIDR);
	tmpsh |= 0x1;
	WriteRegWord((void*)TIMER_TIDR, tmpsh);

	/*disable interrupt request*/
	tmpsh = ReadRegWord((void*)TIMER_TIRR);
	tmpsh |= 0x1;
	WriteRegWord((void*)TIMER_TIRR, tmpsh);

	/*start counting and reload*/
	tmpsh = ReadRegWord((void*)TIMER_TCR0);
	tmpsh |= 3;
	WriteRegWord((void*)TIMER_TCR0, tmpsh); 
}

/* Setup free-running counter for clocksource */
static void __init sigma_sx6_clocksource_init(void)
{
	TRI_DBG("[%d] %s\n",__LINE__,__func__);	

	clocksource_register_hz(&clocksource_soc, SOC_TIMER_FREQ);
}


void __init trix_timer_init(void)
{
	TRI_DBG("[%d] %s\n",__LINE__,__func__);	

	/*soc timer init*/
	soctimer_init();	

	sigma_sx6_clocksource_init();
	sigma_sx6_clockevent_init();

#ifdef CONFIG_LOCAL_TIMERS
	trix_local_timer_setup();
#endif

	TRI_DBG("[%d] %s end\n",__LINE__,__func__);	
}

