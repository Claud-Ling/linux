/*
 * Sigmadesigns DTV SoCs General-Purpose Timer handling
 *
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
 *  Author: Claud Ling, 2016.
 */

#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cpu_pm.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif
#include <linux/version.h>
#include <linux/sched_clock.h>
#include <linux/slab.h>

#define TIMER_TCVR_REG(idx)	(0x00 + (0xc * (idx)))	/*constant value*/
#define TIMER_TRVR_REG(idx)	(0x04 + (0xc * (idx)))	/*counter*/

#define TIMER_TCR_REG(idx)	(0x08 + (0xc * (idx)))	/*control, bit1: start or stop, bit0: reload or no*/
#define TIMER_TCR_OP_ONESHOT	(0x0)
#define TIMER_TCR_OP_REPEAT	(0x1)
#define TIMER_TCR_OP_FREERUN	(0x3)
#define TIMER_TCR_ENABLE	(2)
#define TIMER_TCR_DISABLE	(0)

#define TIMER_TIRR_REG		(0x18)		/*interrupt ack, wirte 1 to clear, bit[1:0]*/
#define TIMER_IRQ_ACK(idx)	BIT((idx))
#define TIMER_IRQ_ACK_ALL	(TIMER_IRQ_ACK(0)|TIMER_IRQ_ACK(1))

#define TIMER_TIDR_REG		(0x1c)		/*interrupt enable, set 1 to disable, bit[1:0]*/
#define TIMER_IRQ_DISABLE(idx)	BIT((idx))
#define TIMER_IRQ_DISABLE_ALL	(TIMER_IRQ_DISABLE(0)|TIMER_IRQ_DISABLE(1))


#define MAX_TIMER_NUM		2

/**
 * struct trix_timer_device - trix SoC timer descriptor
 * @base		mapped register base address
 * @freq		input frequency to the timer block
 * @ticks_per_jiffy	clock counter per jiffy
 * @src_id		indicate the channel used for clock source
 * @event_id		indicate the channel used for clock event
 * @ced			clock event device
 * @cs			clocksource free-running counter
 */
struct trix_timer_device {
	void __iomem *base;

	unsigned int freq;
	unsigned int ticks_per_jiffy;

	unsigned int source_id;
	unsigned int event_id;

	struct clock_event_device ced;
	struct clocksource cs;
};

static struct trix_timer_device *trix_timer;

static void trix_clkevt_time_stop(void)
{
	unsigned int timer = trix_timer->event_id;

	writel(TIMER_TCR_DISABLE, trix_timer->base + TIMER_TCR_REG(timer));
}

static void trix_clkevt_time_setup(unsigned long cycles)
{
	unsigned int timer = trix_timer->event_id;

	writel(cycles, trix_timer->base + TIMER_TCVR_REG(timer));
}

static void trix_clkevt_time_start(bool periodic)
{
	unsigned int timer = trix_timer->event_id;
	u32 val;

	writel(trix_timer->ticks_per_jiffy, trix_timer->base + TIMER_TCVR_REG(timer));

	if (periodic)
		val = TIMER_TCR_OP_REPEAT | TIMER_TCR_ENABLE;
	else
		val = TIMER_TCR_OP_ONESHOT | TIMER_TCR_ENABLE;

	writel(val, trix_timer->base + TIMER_TCR_REG(timer));
}

static int set_next_event(unsigned long cycles,
					 struct clock_event_device *evt)
{
	trix_clkevt_time_stop();
	trix_clkevt_time_setup(cycles);
	trix_clkevt_time_start(false);

	return 0;
}

static int set_state_periodic(struct clock_event_device *evt)
{
	/* periodic irqs; fixed rate of 1/HZ */
	trix_clkevt_time_stop();
	trix_clkevt_time_setup(trix_timer->ticks_per_jiffy);
	trix_clkevt_time_start(true);

	return 0;
}

static int set_state_shutdown(struct clock_event_device *evt)
{
	trix_clkevt_time_stop();

	return 0;
}

static void trix_clkevt_irq_enable(void)
{
	unsigned int timer = trix_timer->event_id;
	u32 val;

	val = readl(trix_timer->base + TIMER_TIDR_REG);
	val &= ~TIMER_IRQ_DISABLE(timer);
	writel(val, trix_timer->base + TIMER_TIDR_REG);
}

/*
 * SOC Timer ack
 */
static void trix_clkevt_irq_ack(void)
{
	unsigned int timer = trix_timer->event_id;
	u32 val;

	val = readl(trix_timer->base + TIMER_TIRR_REG);
	val |= TIMER_IRQ_ACK(timer);
	writel(val, trix_timer->base + TIMER_TIRR_REG);
}

static irqreturn_t trix_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	irq_enter();

	trix_clkevt_irq_ack();

	/*hrtimer_interrupt when switch to hrtimer*/
	if (evt->event_handler)
	{
		evt->event_handler(evt);
	}
	else
	{
		printk("Warning: %s \n",__func__);
	}

	irq_exit();

	return IRQ_HANDLED;
}

static void trix_timer_reset(void)
{
	/* Disable all interrupts */
	writel(TIMER_IRQ_DISABLE_ALL, trix_timer->base + TIMER_TIDR_REG);
	/* Acknowlegde all interrupts */
	writel(TIMER_IRQ_ACK_ALL, trix_timer->base + TIMER_TIRR_REG);
}

static void trix_clocksource_start(void)
{
	unsigned int timer = trix_timer->source_id;

	/* counterinng from 0xffffffff to 0 and again */
	writel(U32_MAX, trix_timer->base + TIMER_TCVR_REG(timer));

	writel(TIMER_TCR_OP_FREERUN, trix_timer->base + TIMER_TCR_REG(timer));
}

#ifdef CONFIG_PM
static void trix_clksrc_resume(struct clocksource *cs)
{
	trix_clocksource_start();

}
#endif

/**
 * trix_read_count_32 - Read the lower 32-bits of the timer 0(global counter)
 * Returns the number of cycles in the timer
 */
static inline u32 trix_read_count_32(void)
{
	return readl_relaxed(trix_timer->base + TIMER_TRVR_REG(trix_timer->source_id));
}

static cycle_t trix_read_cycles(struct clocksource *cs)
{
	return ~(cycle_t)trix_read_count_32() & cs->mask;
}

/*
 * Override the global weak sched_clock symbol with this
 * local implementation which uses the clocksource to get some
 * better resolution when scheduling the kernel.
 *
 * This is marked as notrace so it can be used by the scheduler clock.
 */
static u32 notrace trix_read_sched_clock(void)
{
	return (u32)(~trix_read_count_32());
}

static void __init trix_clocksource_init(u32 freq)
{
	trix_timer->cs.name	= "trix clocksource timer";
	trix_timer->cs.rating	= freq;
	trix_timer->cs.read	= trix_read_cycles;
	trix_timer->cs.mask	= CLOCKSOURCE_MASK(32);
	trix_timer->cs.flags	= CLOCK_SOURCE_IS_CONTINUOUS;
#ifdef CONFIG_PM
	trix_timer->cs.resume	= trix_clksrc_resume;
#endif
	trix_clocksource_start();
	clocksource_register_hz(&trix_timer->cs, freq);

	sched_clock_register((u64 (*)(void))trix_read_sched_clock, 32, freq);

	return;
}

static void __init trix_clockevent_init(u32 freq)
{
	trix_timer->ced.name = "trix clockevent timer";

	clockevents_calc_mult_shift(&trix_timer->ced, freq, 4);
	trix_timer->ced.max_delta_ns = clockevent_delta2ns(0xffffffff,&trix_timer->ced);
	trix_timer->ced.min_delta_ns = clockevent_delta2ns(0x1,&trix_timer->ced);

	trix_timer->ced.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
	trix_timer->ced.rating	= 300;
	trix_timer->ced.cpumask	= cpu_possible_mask;

	trix_timer->ced.set_state_periodic	= set_state_periodic;
	trix_timer->ced.set_state_shutdown	= set_state_shutdown;
	trix_timer->ced.set_state_oneshot	= set_state_shutdown;
	trix_timer->ced.set_next_event		= set_next_event;

	if (request_irq(trix_timer->ced.irq, trix_timer_interrupt,
			IRQF_TIMER | IRQF_IRQPOLL, "trix_timer", &trix_timer->ced)) {
		pr_err("Trix timer: failed to set up irq\n");
		return;
	}

	clockevents_register_device(&trix_timer->ced);

	trix_clkevt_irq_enable();

	return;
}

void __init
trix_timer_init(void __iomem *base, unsigned int src_id, unsigned int evt_id,
		unsigned int rate, unsigned int irq)
{
	trix_timer = kzalloc(sizeof(struct trix_timer_device), GFP_KERNEL);
	if (!trix_timer)
		return;

	trix_timer->base = base;

	trix_timer->source_id = src_id;

	trix_timer->event_id = evt_id;

	trix_timer->ced.irq = irq;

	trix_timer->freq = rate;

	trix_timer->ticks_per_jiffy = DIV_ROUND_UP(trix_timer->freq, HZ);

	trix_timer_reset();

	/* Configure timer[source_id] as clock source */
	trix_clocksource_init(trix_timer->freq);

	/* Configure timer[event_id] as clock event */
	trix_clockevent_init(trix_timer->freq);
}

#ifdef CONFIG_OF
static void __init
trix_timer_of_init(struct device_node *np)
{
	void __iomem *base;
	unsigned int source_id, event_id, irq, clk_rate;
	int ret;

	base = of_iomap(np, 0);
	if (IS_ERR(base)) {
		pr_err("Trix timer: Unable to map timer registers\n");
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * search for the usage property in DT.if it is not provided,
	 * the assignment of the two 32-bit count-down timers as follows:
	 *   - timer 0, clocksource for generic timekeeping
	 *   - timer 1, clockevent source for hrtimers
	 */
	if (of_property_read_u32(np, "clock-source", &source_id)) {
		source_id = 0;
	}

	if (of_property_read_u32(np, "clock-event", &event_id)) {
		event_id = 1;
	}

	/* sanity check on the assignment */
	WARN_ON(source_id >= MAX_TIMER_NUM);
	WARN_ON(event_id >= MAX_TIMER_NUM);
	WARN_ON(source_id == event_id);


	/* get irq of the clock event device */
	irq = irq_of_parse_and_map(np, event_id);
	if (irq <= 0) {
		pr_err("Trix timer: Unable to get IRQ from DT\n");
		ret = -EINVAL;
		goto out;
	}

	/* get the input clock rating in DT */
	if (of_property_read_u32(np, "clock-frequency", &clk_rate)) {

		/*
		 * in case property <clock-frequency> is not provided,
		 * Get clk rate through clk driver if present
		 */
		clk = of_clk_get_by_name(np, "cntclk");
		if (IS_ERR(clk)) {
			pr_err("clocks or clock-frequency not defined\n");
			ret = PTR_ERR(clk);
			goto out;
		}

		clk_prepare_enable(clk);
		clk_rate = clk_get_rate(clk);
	}

	trix_timer_init(base, source_id, event_id, clk_rate, irq);
out:
	return;
}

CLOCKSOURCE_OF_DECLARE(trix, "sigma,trix-timer", trix_timer_of_init);

#endif

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_soctimer = NULL;
static int soctimer_cycle_show(struct seq_file *m, void *v)
{
	u32 temp = trix_read_sched_clock();
	seq_printf(m, "%u\n", temp);
	return 0;
}

static int soctimer_cycle_open(struct inode *inode, struct file *file)
{
        return single_open(file, soctimer_cycle_show, NULL);
}

static const struct file_operations soctimer_cycle_ops = {
        .open           = soctimer_cycle_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int __init soctimer_proc_init(void)
{
	proc_soctimer = proc_mkdir("soctimer", NULL);
        proc_create("cycle", 0, proc_soctimer, &soctimer_cycle_ops);

	return 0;
}

late_initcall(soctimer_proc_init);
#endif

