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
#include <linux/of.h>
#include <linux/delay.h>

#include <asm/cacheflush.h>
#include <asm/smp_scu.h>

/* Size of mapped Cortex A9 SCU address space */
#define CORTEX_A9_SCU_SIZE      0x60

#define AUX_BOOT_ID_BITS	4
#define AUX_BOOT_ID_MASK	((1 << AUX_BOOT_ID_BITS) - 1)
#define AUX_BOOT_ADDR_MASK	(~AUX_BOOT_ID_MASK)

/* Name of device node property defining secondary boot register location */
#define OF_SECONDARY_BOOT	"secondary-boot-reg"
#define OF_SECONDARY_BOOT_ID	"secondary-boot-id"

/* SCU base address */
static void __iomem *scu_base = NULL;

/* I/O address of register used to coordinate secondary core startup */ 
static u32 secondary_boot, secondary_boot_id;

static void __iomem *secondary_boot_reg = NULL;
static void __iomem *secondary_boot_id_reg = NULL;

/*
 * Secondary cores will start in sigma_secondary_startup
 * defined in "arch/arm/mach-trix-of/sigma-headsmp.S
 */
extern void sigma_secondary_startup(void);
extern volatile int pen_release;

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

static int __init scu_get_base(void)
{
	unsigned long config_base;

	if (!scu_a9_has_base()) {
		pr_err("no configuration base address register!\n");
		return -ENXIO;
	}

	/* Config base address register value is zero for uniprocessor */
	config_base = scu_a9_get_base();
	if (!config_base) {
		pr_err("hardware reports only one core\n");
		return -ENOENT;
	}

	scu_base = ioremap((phys_addr_t)config_base, CORTEX_A9_SCU_SIZE);
	if (!scu_base) {
		pr_err("failed to remap config base (%lu/%u) for SCU\n",
			config_base, CORTEX_A9_SCU_SIZE);
		return -ENOMEM;
	}

	return 0;
}

static int __init boot_register_of_parse(void)
{
	struct device_node *node;
	int ret = 0;

	/*
	 * this only happens if a "/cpus" device tree node exists
	 * and has an "enable-method" property that selects the SMP
	 * operations defined herein.
	 */
	node = of_find_node_by_path("/cpus");
	if (!node) {
		ret = -ENOENT;
		goto out;
	}

	/*
	 * Our secondary enable method requires a "secondary-boot-reg"
	 * property to specify a register address used to request the
	 * ROM code boot a secondary code.  If we have any trouble
	 * getting this we fall back to uniprocessor mode.
	 */
	if (of_property_read_u32(node, OF_SECONDARY_BOOT, &secondary_boot)) {
		pr_err("%s: missing/invalid " OF_SECONDARY_BOOT " property\n",
			node->name);
		ret = -ENOENT;
		goto out;
	}

	if (of_property_read_u32(node, OF_SECONDARY_BOOT_ID, &secondary_boot_id)) {
		pr_err("%s: missing/invalid " OF_SECONDARY_BOOT_ID " property\n",
			node->name);
		ret = -ENOENT;
		goto out;
	}

	secondary_boot_reg = ioremap_nocache((phys_addr_t)secondary_boot, sizeof(u32));
	secondary_boot_id_reg = ioremap_nocache((phys_addr_t)secondary_boot_id, sizeof(u32));

	if(!secondary_boot_reg || !secondary_boot_id_reg) {
		pr_err("unable to map boot register\n");
		return -ENOSYS;
	}
out:
	of_node_put(node);

	return ret;
}

/*kernel init stage of this cpu*/
static void __cpuinit trix_secondary_init(unsigned int cpu)
{
	unsigned int val;

	/*
	* let the primary processor know we're out of the
	* pen, then head off into the C entry point
	*/
	val = readl_relaxed(secondary_boot_id_reg);
	val &= ~AUX_BOOT_ID_MASK; /* reset boot id */
	writel_relaxed(val, secondary_boot_id_reg);

	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	raw_spin_lock(&boot_lock);
	raw_spin_unlock(&boot_lock);
}

static int __cpuinit trix_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;
	unsigned int val;

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
	val = readl_relaxed(secondary_boot_reg);
	val &= ~AUX_BOOT_ID_MASK;
	val |= cpu & AUX_BOOT_ID_MASK;
	writel_relaxed(val, secondary_boot_reg);
		
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

/*
 * CPU Nodes are passed thru DT and set_cpu_possible
 * is set by "arm_dt_init_cpu_maps"
 */
static void __init trix_smp_init_cpus(void)
{
	return;
}

static void wakeup_secondary(void *entry)
{
	phys_addr_t boot_entry;

	/*
	* Write the address of secondary startup routine into the
	* AuxCoreBoot1 where ROM code will jump and start executing
	* on secondary core once out of WFE
	* A barrier is added to ensure that write buffer is drained
	*/
	boot_entry = virt_to_phys(entry);
	WARN_ON(boot_entry & AUX_BOOT_ID_MASK);
	WARN_ON(boot_entry > (phys_addr_t)U32_MAX);

	writel_relaxed(boot_entry, secondary_boot_reg);

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
	static cpumask_t only_cpu_0 = { CPU_BITS_CPU0 };
	int i, ret = 0;

	ret = boot_register_of_parse();
	if (ret)
		goto out;

	ret = scu_get_base();
	if (ret)
		goto out;

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

out:
	if (ret) {
		/* Update the CPU present map to reflect uniprocessor mode */
		pr_warn("disabling SMP\n");
		init_cpu_present(&only_cpu_0);
	}
}

void __iomem *platform_get_scu_base(void)
{
	return scu_base;
}

struct smp_operations trix_smp_ops __initdata = {
	.smp_init_cpus		= trix_smp_init_cpus,
	.smp_prepare_cpus	= trix_smp_prepare_cpus,
	.smp_secondary_init	= trix_secondary_init,
	.smp_boot_secondary	= trix_boot_secondary,
};
CPU_METHOD_OF_DECLARE(trix_smp, "sigma,trix-smp", &trix_smp_ops);


