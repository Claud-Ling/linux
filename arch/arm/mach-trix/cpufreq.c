/*
 *  linux/arch/arm/mach-trix/cpufreq.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * cpufreq notifier implementation
 *
 * Author: Tony He, 2016.
 */

#define pr_fmt(fmt) "cpufreq: " fmt

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cpufreq.h>

#ifdef CONFIG_TRIX_SMC
#include "smc.h"
#endif

static int trix_cpufreq_on_transition(struct notifier_block *nb, unsigned long action, void *data)
{
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs*)data;

	/*
	 * cpu_freq will send notifiers by iterating over all
	 * activiated cpus in present cluster on each transition.
	 * here we only interest in one notifier and ignore the
	 * others.
	 */
	if (freqs->cpu != 0)
		return 0;

	switch (action) {
	case CPUFREQ_PRECHANGE:
		pr_debug("cpufreq prechange: %d -> %d kHz\n",
			freqs->old, freqs->new);
		break;
	case CPUFREQ_POSTCHANGE:
		pr_debug("cpufreq postchange: %d -> %d KHz\n",
			freqs->old, freqs->new);
#ifdef CONFIG_TRIX_CPUFREQ_FORWARD
		/* notify secure world */
		if (freqs->new != freqs->old)
			secure_scale_cpufreq(freqs->new);
#endif
		break;
	}
	return 0;
}

static struct notifier_block trix_cpufreq_nb =
{
	.notifier_call = trix_cpufreq_on_transition,
};

static int __init trix_cpufreq_notifier_init(void)
{
	cpufreq_register_notifier(&trix_cpufreq_nb, CPUFREQ_TRANSITION_NOTIFIER);
	return 0;
}

late_initcall(trix_cpufreq_notifier_init);
