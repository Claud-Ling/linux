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
#define pr_fmt(fmt) "smp: " fmt

#include <linux/init.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <asm/cacheflush.h>
#include <asm/smp_scu.h>
#include <mach/hardware.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#include <linux/irqchip/arm-gic.h>
#else
#include <asm/hardware/gic.h>
#endif

#include <mach/smp.h>
#include "common.h"

/*
 * set up id in aux_boot_addr, as well as aux_boot_id
 */
#define SECURE_BOOT_AUX_CORE(reg, id) do{			\
	secure_boot_aux_core((reg), (id));			\
	if ((u32)(reg) != AUX_BOOT_ID_REG) {			\
		uint32_t _tmp = 0;				\
		if (!secure_get_aux_core_boot((reg), &_tmp)) {	\
			writel_relaxed(_tmp & AUX_BOOT_ID_MASK,	\
					(void*)AUX_BOOT_ID_REG);\
		} else {					\
			pr_err("get aux id fail, %d\n", (id));	\
		}						\
	}							\
} while(0)

/* SCU base address */
static void __iomem *scu_base = SIGMA_IO_ADDRESS(SIGMA_TRIX_SCU_BASE);

/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void write_pen_release(int val)
{
    pen_release = val;
    smp_wmb();
    __cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
#ifdef CONFIG_CACHE_L2X0
    outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
#endif
}

static DEFINE_RAW_SPINLOCK(boot_lock);

/*kernel init stage of this cpu*/
static void trix_secondary_init(unsigned int cpu)
{
	TRI_DBG("[%d] %s\n",__LINE__,__func__);

	/*
	* let the primary processor know we're out of the
	* pen, then head off into the C entry point
	*/
	MWriteRegWord((void *)AUX_BOOT_ID_REG, 0, AUX_BOOT_ID_MASK);	/*reset boot id*/
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	raw_spin_lock(&boot_lock);
	raw_spin_unlock(&boot_lock);

	TRI_DBG("[%d] %s\n",__LINE__,__func__);
}

static int trix_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;
	TRI_DBG("[%d] %s\n",__LINE__,__func__);

	/*
	* Set synchronisation state between this boot processor
	* and the secondary one
	*/
	raw_spin_lock(&boot_lock);

	/*
	* The secondary processor is waiting to be released from
	* the holding pen - release it, then wait for it to flag
	* that it has been released by resetting pen_release.
	*
	* Note that "pen_release" is the hardware CPU ID, whereas
	* "cpu" is Linux's internal ID.
	*/
	write_pen_release(cpu);

	/*
	* In case secondary processor is waiting to be release from
	* the holding pen in boot reg - release it.
	*
	* Note that: this is required when take cores back from non-OFF
	* suspend mode.
	*/
	SECURE_BOOT_AUX_CORE((void*)AUX_BOOT_ADDR_REG, cpu);

	/*
	* Send the secondary CPU a soft interrupt, thereby causing
	* the boot monitor to read the system wide flags register,
	* and branch to the address found there.
	*/
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout))
	{
		if (pen_release == -1)
			break;
		udelay(10);
	}

	/*
	* now the secondary core is starting up let it run its
	* calibrations, then wait for it to finish
	*/
	raw_spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

//extern void early_printk(const char *fmt, ...);
//#define printk early_printk
/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init trix_smp_init_cpus(void)
{
	unsigned int i, ncores = 0;

	printk("scu iomap '%x' -> '%p' \n", SIGMA_TRIX_SCU_BASE, scu_base);
	BUG_ON(!scu_base);
	ncores = scu_get_core_count(scu_base);

	/* sanity check */
	if (ncores > NR_CPUS) {
		printk(KERN_WARNING
		       "Sigma SX6: no. of cores (%d) greater than configured "
		       "maximum of %d - clipping\n",
		       ncores, NR_CPUS);
		ncores = NR_CPUS;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)
	set_smp_cross_call(gic_raise_softirq);
#endif
	TRI_DBG("[%d] %s\n",__LINE__,__func__);	
}

static void wakeup_secondary(void *entry)
{
	/*
	* Write the address of secondary startup routine into the
	* AuxCoreBoot1 where ROM code will jump and start executing
	* on secondary core once out of WFE
	* A barrier is added to ensure that write buffer is drained
	*/
	WARN_ON((virt_to_phys(entry) & (~AUX_BOOT_ADDR_MASK)) != 0 );
	secure_set_aux_core_addr((void*)AUX_BOOT_ADDR_REG, virt_to_phys(entry));

	smp_wmb();

	/*
	* Send a 'sev' to wake the secondary core from WFE.
	* Drain the outstanding writes to memory
	*/
	dsb_sev();
	mb();
}

static void __init trix_smp_prepare_cpus(unsigned int max_cpus)
{
	int i;

	TRI_DBG("[%d] %s max_cpus:%d\n",__LINE__,__func__,max_cpus);	
	scu_enable(scu_base);

	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

	/*
	 * Initialise the SCU and wake up the secondary core using
	 */
	wakeup_secondary(sigma_secondary_startup);

	TRI_DBG("[%d] %s\n",__LINE__,__func__);
}

void __iomem *platform_get_scu_base(void)
{
	return scu_base;
}

#ifdef CONFIG_PM
#include <asm/suspend.h>
void platform_smp_resume_cpus(void)
{
	int i = 0;
	unsigned int max_cpus = setup_max_cpus;
	unsigned int ncores = num_possible_cpus();

	/*
	 * are we trying to resume more cores than exist?
	 */
	if (max_cpus > ncores)
		max_cpus = ncores;
	if (ncores > 1 && max_cpus) {
		TRI_DBG(KERN_INFO "resume secondary cpus\n");
		scu_enable(scu_base);

		/*
		 * Initialise the present map, which describes the set of CPUs
		 * actually populated at the present time.
		 */
		for (i = 0; i < max_cpus; i++)
			set_cpu_present(i, true);

		/*
		 * Initialise the SCU and wake up the secondary core using
		 */
		wakeup_secondary(trix_cpu_resume);
	}
}
#endif

struct smp_operations trix_smp_ops __initdata = {
	.smp_init_cpus		= trix_smp_init_cpus,
	.smp_prepare_cpus	= trix_smp_prepare_cpus,
	.smp_secondary_init	= trix_secondary_init,
	.smp_boot_secondary	= trix_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= trix_cpu_die,
#endif
};
