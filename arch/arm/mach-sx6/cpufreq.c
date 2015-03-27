/*
 *  linux/arch/arm/mach-trix/cpufreq.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SoC CPUfreq Support
 *
 * Author: Tony He, 2014.
 */

#define pr_fmt(fmt) "cpufreq: " fmt

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>
#include <linux/cpu.h> 
#include <linux/version.h>

#include <asm/smp_plat.h>
#include <asm/io.h>
#include <asm/delay.h>
#include "chipver.h"
#ifdef CONFIG_SIGMA_SMC
#include "smc.h"
#endif

#define Freq_ReadRegByte(addr) ReadRegByte((void*)addr)
#define Freq_WriteRegByte(addr, val) WriteRegByte((void*)addr, val)
#define Freq_WriteRegByteMask(addr, val, mask) do{		\
		unsigned char __tmp = Freq_ReadRegByte(addr);	\
		__tmp &= ~(mask);				\
		__tmp |= ((val) & (mask));			\
		Freq_WriteRegByte(addr, __tmp);			\
	}while(0)

struct pll_config {
	unsigned int max;	/*KHz, derived from pll/user settings*/
	unsigned int min;	/*KHz, derived from pll/user settings*/
};

struct pll_operations {
	unsigned int (*getclock)(void);
	int (*setclock)(unsigned int target);
	int (*getconfig)(struct pll_config*);
	const char* (*getname)(void);
};

#ifdef CONFIG_SIGMA_SOC_SX6
# include "pll-sx6.c"
#elif defined (CONFIG_SIGMA_SOC_SX7)
# include "pll-sx7.c"
#elif defined (CONFIG_SIGMA_SOC_UXLB)
# include "pll-uxl.c"
#else
# error "unknown chip!!"
#endif

static struct cpufreq_frequency_table trix_freq_table[] = {
	//{ 0,  24000  }, //TODO: risk due to its another clk src than others. do we realy need clk lower than 75M?
	{ 0,  75000  },
	{ 1,  100000 },
	{ 2,  300000 },
	{ 3,  400000 },
	{ 4,  500000 },
	{ 5,  600000 },
	{ 6,  700000 },
	{ 7,  750000 },
	{ 8,  800000 },
	{ 9,  820000 },
	{ 10, 900000 },
	{ 11,1000000 },
	{ 12,1100000 },
	{ 13,1200000 },
	{ 14,1300000 },
	{ 15,1400000 },
	{ 100, CPUFREQ_TABLE_END },
};


static void calibrate_freqtbl(unsigned int min_freq, unsigned int max_freq)
{
	struct cpufreq_frequency_table *item = trix_freq_table;
	for(; item->frequency != CPUFREQ_TABLE_END; item++)
	{
		if (item->frequency == CPUFREQ_ENTRY_INVALID)
			continue;
		if (item->frequency > max_freq)
			item->frequency = CPUFREQ_ENTRY_INVALID;
		else if (item->frequency < min_freq)
			item->frequency = CPUFREQ_ENTRY_INVALID;
	}
}

static struct pll_config pll_cfg = {0, 0};
static struct pll_operations *pll_ops = NULL;

static unsigned int trix_pll_getclock(void)
{
	BUG_ON(pll_ops == NULL);
	return pll_ops->getclock();
}

static int trix_pll_setclock(unsigned int target)
{
	BUG_ON(pll_ops == NULL);
	return pll_ops->setclock(target);
}

static int trix_pll_getconfig(struct pll_config *cfg)
{
	BUG_ON(pll_ops == NULL);
	return pll_ops->getconfig(cfg);
}

static const char* trix_pll_getname(void)
{
	BUG_ON(pll_ops == NULL);
	return pll_ops->getname();
}

static void pll_ops_dummy(void)
{
	pr_debug("not supported function call\n");
}

static int trix_init_pll_ops(void)
{
#define PLL_SAINT_CHECK(func)	do{					\
		if (NULL == (func)) {					\
			pr_warn("'%s' is NULL, set to default\n", #func);\
			func = (void*)pll_ops_dummy;			\
		}							\
	}while(0)


	pll_ops = trix_get_pll_ops();
	if (NULL == pll_ops)
		return 1;

	//saint_check
	PLL_SAINT_CHECK(pll_ops->getclock);
	PLL_SAINT_CHECK(pll_ops->setclock);
	PLL_SAINT_CHECK(pll_ops->getconfig);
	PLL_SAINT_CHECK(pll_ops->getname);

	pr_info("Select pll source '%s'\n", trix_pll_getname());
	return 0;
#undef PLL_SAINT_CHECK
}

static int trix_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, trix_freq_table);
}

static unsigned int trix_cpufreq_get_speed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;

	return trix_pll_getclock();
}

static int trix_cpufreq_set_target(struct cpufreq_policy *policy,
				      unsigned int target_freq,
				      unsigned int relation)
{
	int ret;
	unsigned int i , cpu;
	struct cpufreq_freqs freqs;

	ret = cpufreq_frequency_table_target(policy, trix_freq_table,
					     target_freq, relation, &i);
	if (ret != 0)
		return ret;

	freqs.cpu = policy->cpu;
	freqs.old = trix_cpufreq_get_speed(policy->cpu);
	freqs.new = trix_freq_table[i].frequency;
	freqs.flags = 0;

	if (freqs.old == freqs.new)
		return 0;
	

	/* notifiers */
	for_each_cpu(cpu, policy->cpus) {
		freqs.cpu = cpu;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
#else
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
#endif
	}
	
	
	pr_info("Freq scaling %d -> %dkHz ...\n", freqs.old, freqs.new);
	ret = trix_pll_setclock(freqs.new);
	if (ret != 0)
	{
		freqs.new = freqs.old;
		pr_warn("[FAIL]\n");
	}

#ifdef CONFIG_SIGMA_SMC
	/* notify secure world */
	if (freqs.new != freqs.old)
		secure_scale_cpufreq(freqs.new);
#endif

	/* notifiers */
	for_each_cpu(cpu, policy->cpus) {
		freqs.cpu = cpu;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
#else
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
#endif
	}

	return ret;
}


static int trix_cpufreq_driver_init(struct cpufreq_policy *policy)
{
	int ret, cur_freq;

	if (policy->cpu != 0)
		return -EINVAL;

	if (trix_init_pll_ops())
	{
		pr_err("Init pll ops failed, disable freq scaling\n");
		return -ENODEV;
	}

	if (trix_pll_getconfig(&pll_cfg))
	{
		pr_err("Get pll config failed, disable freq scaling\n");
		return -ENODEV;
	}
	pll_cfg.min = pll_cfg.min >  MIN_CPUFREQ_KHZ ? pll_cfg.min : MIN_CPUFREQ_KHZ;
	if (pll_cfg.max < pll_cfg.min)
	{
		pr_err("Invalid pll config, disable freq scaling\n");
		return -ENODEV;
	}

	if (trix_freq_table == NULL) {
		pr_err("No frequency information for this CPU\n");
		return -ENODEV;
	}

	/*calibrate freqtbl according pll and user settings*/
	calibrate_freqtbl(pll_cfg.min, pll_cfg.max);

	ret = cpufreq_frequency_table_cpuinfo(policy, trix_freq_table);
	if (ret != 0) {
		pr_err("Failed to configure frequency table: %d\n",
		       ret);
	}
	cpufreq_frequency_table_get_attr(trix_freq_table, policy->cpu);
	
	/* honor first_freq as max_freq of current policy*/
	cur_freq = trix_cpufreq_get_speed(policy->cpu);
	policy->max = policy->cur = cur_freq;
	if (policy->min > policy->max)
		policy->min = policy->max;
	cpufreq_frequency_table_verify(policy, trix_freq_table);

	if (is_smp()) {
                policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
                cpumask_setall(policy->cpus);  
        }

	/* FIXME: what's the actual transition time? in ns*/ 
	policy->cpuinfo.transition_latency = (80 * 1000);

	return ret;
}

static struct freq_attr *trix_cpufreq_attr[] = {
        &cpufreq_freq_attr_scaling_available_freqs,
        NULL,                 
};

static struct cpufreq_driver trix_cpufreq_driver = {
	.owner		= THIS_MODULE,
	.flags          = 0,
	.verify		= trix_cpufreq_verify_speed,
	.target		= trix_cpufreq_set_target,
	.get		= trix_cpufreq_get_speed,
	.init		= trix_cpufreq_driver_init,
	.name		= CONFIG_SIGMA_SOC_NAME,
	.attr           = trix_cpufreq_attr,
};

static int __init trix_cpufreq_init(void)
{
	return cpufreq_register_driver(&trix_cpufreq_driver);
}

static void __exit trix_cpufreq_exit(void)
{
        cpufreq_unregister_driver(&trix_cpufreq_driver);
}

MODULE_DESCRIPTION("cpufreq driver for "CONFIG_SIGMA_SOC_NAME" SoCs");
MODULE_LICENSE("GPL");
module_init(trix_cpufreq_init);
module_exit(trix_cpufreq_exit);
