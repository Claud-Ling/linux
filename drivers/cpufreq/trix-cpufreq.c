/*
 *  driver/cpufreq/trix-cpufreq.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SXx series SoC cpufreq driver
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

#define MIN_CPUFREQ_KHZ		CONFIG_TRIX_MIN_CPUFREQ	/*khz*/

#define Freq_ReadRegByte(addr) ReadRegByte((void*)addr)
#define Freq_WriteRegByte(addr, val) WriteRegByte((void*)addr, val)
#define Freq_WriteRegByteMask(addr, val, mask) do{		\
		unsigned int __tmp = Freq_ReadRegByte(addr);	\
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
# include "trix-pll-sx6.c"
#elif defined (CONFIG_SIGMA_SOC_SX7)
# include "trix-pll-sx7.c"
#elif defined (CONFIG_SIGMA_SOC_SX8)
# include "trix-pll-sx8.c"
#else
# include "trix-pll-stub.c"	/*stub implementation*/
#endif

#define FTAB_ENTRY(f,i,freq) {f,i,freq}
static struct cpufreq_frequency_table trix_freq_table[] = {
	//	   flag id	freq
	FTAB_ENTRY(0,	0,	24000),
	FTAB_ENTRY(0,	1,	75000),
	FTAB_ENTRY(0,	2,	100000),
	FTAB_ENTRY(0,	3,	300000),
	FTAB_ENTRY(0,	4,	400000),
	FTAB_ENTRY(0,	5,	500000),
	FTAB_ENTRY(0,	6,	600000),
	FTAB_ENTRY(0,	7,	700000),
	FTAB_ENTRY(0,	8,	800000),
	FTAB_ENTRY(0,	9,	900000),
	FTAB_ENTRY(0,	10,	1000000),
	FTAB_ENTRY(0,	11,	1100000),
	FTAB_ENTRY(0,	12,	1200000),
	FTAB_ENTRY(0,	13,	1300000),
	FTAB_ENTRY(0,	14,	1400000),
	FTAB_ENTRY(0,	15,	1500000),
	FTAB_ENTRY(0,	16,	1600000),
	FTAB_ENTRY(0,	17,	1700000),

	FTAB_ENTRY(0,	20,	CPUFREQ_TABLE_END),
};


static void calibrate_freqtbl(unsigned int min_freq, unsigned int max_freq)
{
	struct cpufreq_frequency_table *item = trix_freq_table;
	for(; item->frequency != CPUFREQ_TABLE_END; item++) {
		if (item->frequency == CPUFREQ_ENTRY_INVALID)
			continue;
		if (item->frequency > max_freq)
			item->frequency = CPUFREQ_ENTRY_INVALID;
		else if (item->frequency < min_freq)
			item->frequency = CPUFREQ_ENTRY_INVALID;
	}
}

static struct cpufreq_frequency_table* find_entry_by_index(unsigned int index)
{
	struct cpufreq_frequency_table *iret = NULL, *item = trix_freq_table;
	for(; item->frequency != CPUFREQ_TABLE_END; item++) {
		if (item->driver_data == index) {
			iret = item;
			break;
		}
	}
	return iret;
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
#define PLL_SANITY_CHECK(func)	do{					\
		if (NULL == (func)) {					\
			pr_warn("'%s' is NULL, set to default\n", #func);\
			func = (void*)pll_ops_dummy;			\
		}							\
	}while(0)

	pll_ops = trix_get_pll_ops();
	if (NULL == pll_ops)
		return 1;

	//sanity_check
	PLL_SANITY_CHECK(pll_ops->getclock);
	PLL_SANITY_CHECK(pll_ops->setclock);
	PLL_SANITY_CHECK(pll_ops->getconfig);
	PLL_SANITY_CHECK(pll_ops->getname);

	pr_info("Select pll source '%s'\n", trix_pll_getname());
	return 0;
#undef PLL_SANITY_CHECK
}

static unsigned int trix_cpufreq_get_speed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;

	return trix_pll_getclock();
}

static int trix_cpufreq_target_index(struct cpufreq_policy *policy, unsigned int index)
{
	int ret;
	struct cpufreq_frequency_table *entry = NULL;
	unsigned int freq_new, freq_old;

	if (policy->cpu != 0)
		return -EINVAL;

	if ((entry = find_entry_by_index(index)) == NULL) {
		pr_err("cant find tab entry, invalid index %d\n", index);
		return -EINVAL;
	}

	freq_old = trix_cpufreq_get_speed(policy->cpu);
	freq_new = entry->frequency;
	if (freq_new == freq_old)
		return 0;

	pr_info("Freq scaling %d -> %dkHz ...\n", freq_old, freq_new);
	ret = trix_pll_setclock(freq_new);
	if (ret != 0) {
		freq_new = freq_old;
		pr_warn("[FAIL]\n");
	}
	return ret;
}

static int trix_cpufreq_driver_init(struct cpufreq_policy *policy)
{
	int ret, cur_freq;

	if (policy->cpu != 0)
		return -EINVAL;

	if (trix_init_pll_ops()) {
		pr_err("Init pll ops failed, disable freq scaling\n");
		return -ENODEV;
	}

	if (trix_pll_getconfig(&pll_cfg)) {
		pr_err("Get pll config failed, disable freq scaling\n");
		return -ENODEV;
	}
	pll_cfg.min = pll_cfg.min >  MIN_CPUFREQ_KHZ ? pll_cfg.min : MIN_CPUFREQ_KHZ;
	if (pll_cfg.max < pll_cfg.min) {
		pr_err("Invalid pll config, disable freq scaling\n");
		return -ENODEV;
	}

	/*calibrate freqtbl according pll and user settings*/
	calibrate_freqtbl(pll_cfg.min, pll_cfg.max);
	/* FIXME: what's the actual transition time? in ns*/
	policy->cpuinfo.transition_latency = (80 * 1000);

	ret = cpufreq_table_validate_and_show(policy, trix_freq_table);
	if (ret != 0) {
		pr_err("Failed to setup frequency table: %d\n", ret);
		return ret;
	}

	/* respect first_freq as max_freq of current policy*/
	cur_freq = trix_cpufreq_get_speed(policy->cpu);
	policy->max = policy->cur = cur_freq;
	if (policy->min > policy->max)
		policy->min = policy->max;
	cpufreq_frequency_table_verify(policy, trix_freq_table);

	if (is_smp()) {
		policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
		cpumask_setall(policy->cpus);
        }

	return 0;
}

static struct cpufreq_driver trix_cpufreq_driver = {
	.verify		= cpufreq_generic_frequency_table_verify,
	.target_index	= trix_cpufreq_target_index,
	.get		= trix_cpufreq_get_speed,
	.init		= trix_cpufreq_driver_init,
	.name		= CONFIG_TRIX_SOC_NAME"-cpufreq",
	.attr           = cpufreq_generic_attr,
};

static int __init trix_cpufreq_init(void)
{
	return cpufreq_register_driver(&trix_cpufreq_driver);
}

static void __exit trix_cpufreq_exit(void)
{
	cpufreq_unregister_driver(&trix_cpufreq_driver);
}

MODULE_DESCRIPTION("cpufreq driver for "CONFIG_TRIX_SOC_NAME" SoCs");
MODULE_LICENSE("GPL");
module_init(trix_cpufreq_init);
module_exit(trix_cpufreq_exit);
