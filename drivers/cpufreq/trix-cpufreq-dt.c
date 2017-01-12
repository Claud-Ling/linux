/*
 *  Copyright (C) 2016 SigmaDesigns Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/pm_opp.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/platform_device.h>

struct private_data {
	struct device *cpu_dev;
	unsigned int max_freq;
};

static int allocate_resources(int cpu, struct device **cdev,
					struct clk **cclk)
{
	struct device *cpu_dev;
	struct clk *cpu_clk;
	int ret = 0;

	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev) {
		pr_err("failed to get cpu%d device\n", cpu);
		return -ENODEV;
	}

	cpu_clk = clk_get(cpu_dev, NULL);
	if (IS_ERR(cpu_clk)) {
		dev_err(cpu_dev, "failed to get cpu%d clock: %d\n", cpu,
			ret);

		ret = PTR_ERR(cpu_clk);

		/*
		 * If cpu's clk node is present, but clock is not yet
		 * egistered, we should try defering probe.
		 */
		if (ret == -EPROBE_DEFER)
			dev_dbg(cpu_dev, "cpu%d clock not ready, retry\n", cpu);
		else
			dev_err(cpu_dev, "failed to get cpu%d clock: %d\n", cpu,
				ret);
	} else {
		*cdev = cpu_dev;
		*cclk = cpu_clk;
	}

	return ret;
}

static int trix_cpufreq_set_target(struct cpufreq_policy *policy, unsigned int index)
{
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	struct dev_pm_opp *opp;
	struct private_data *priv = policy->driver_data;
	struct device *cpu_dev = priv->cpu_dev;
	unsigned long freq;
	unsigned int old_freq, new_freq;
	int ret;

	old_freq = clk_get_rate(policy->clk) / 1000;
	new_freq = freq_table[index].frequency;

	freq = new_freq * 1000;
	ret = clk_round_rate(policy->clk, freq);
	if (IS_ERR_VALUE(ret)) {
		dev_warn(cpu_dev,
			 "CPUfreq: Cannot find matching frequency for %lu\n",
			 freq);
		return ret;
	}
	freq = ret;

	rcu_read_lock();
	opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq);
	if (IS_ERR(opp)) {
		rcu_read_unlock();
		dev_err(cpu_dev, "%s: unable to find OPP for %d\n",
			__func__, new_freq);
		return -EINVAL;
	}
	rcu_read_unlock();

	dev_dbg(cpu_dev, "cpufreq-trix: %u MHz  --> %u MHz\n", 
		old_freq / 1000, new_freq / 1000);

	ret = clk_set_rate(policy->clk, new_freq * 1000);

	return ret;
}

static int trix_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_frequency_table *freq_table, *pos;
	struct clk *cpu_clk;
	struct device *cpu_dev;
	struct device_node *np;
	struct private_data *priv;
	unsigned int cur_freq;
	unsigned int transition_latency;
	int ret;

	ret = allocate_resources(policy->cpu, &cpu_dev,  &cpu_clk);
	if (ret) {
		pr_err("%s: Failed to allocate resources: %d\n", __func__, ret);
		return ret;
	}

	np = of_node_get(cpu_dev->of_node);
	if (!np) {
		dev_err(cpu_dev, "failed to find cpu%d node\n", policy->cpu);
		ret = -ENOENT;
		return ret;
	}

	of_init_opp_table(cpu_dev);

	ret = dev_pm_opp_get_opp_count(cpu_dev);
	if (ret < 0) {
		dev_err(cpu_dev, "no OPP table is found: %d\n", ret);
		goto out_free_opp;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto out_free_opp;
	}

	ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &freq_table);
	if (ret) {
		dev_err(cpu_dev, "failed to init cpufreq table: %d\n", ret);
		goto out_free_priv;
	}

	if (of_property_read_u32(np, "clock-latency", &transition_latency))
		transition_latency = CPUFREQ_ETERNAL;

	cur_freq = clk_get_rate(cpu_clk) / 1000;

	priv->cpu_dev = cpu_dev;
	priv->max_freq = cur_freq;

	/*
	 * use the frequency set by bootloader as the maxium freq.
	 * iterate over the cpufreq_frequency_table excluding the
	 * frequency larger than max_freq
	 */
	cpufreq_for_each_entry(pos, freq_table)
		if (pos->frequency > priv->max_freq)
			pos->frequency = CPUFREQ_ENTRY_INVALID;

	policy->driver_data = priv;
	policy->clk = cpu_clk;

	ret = cpufreq_generic_init(policy, freq_table, transition_latency);
	if (ret) {
		dev_err(cpu_dev, "%s: invalid frequency table: %d\n", __func__,
			ret);
		goto out_free_cpufreq_table;
	}

	of_node_put(np);

	return 0;

out_free_cpufreq_table:
	dev_pm_opp_free_cpufreq_table(cpu_dev, &freq_table);
out_free_priv:
	kfree(priv);
out_free_opp:
	of_free_opp_table(cpu_dev);
	of_node_put(np);

	return ret;
}

static int trix_cpufreq_exit(struct cpufreq_policy *policy)
{
	struct private_data *priv = policy->driver_data;

	dev_pm_opp_free_cpufreq_table(priv->cpu_dev, &policy->freq_table);
	of_free_opp_table(priv->cpu_dev);
	clk_put(policy->clk);

	kfree(priv);

	return 0;
}

static struct cpufreq_driver trix_cpufreq_driver = {
	.verify		= cpufreq_generic_frequency_table_verify,
	.target_index	= trix_cpufreq_set_target,
	.get		= cpufreq_generic_get,
	.init		= trix_cpufreq_init,
	.exit		= trix_cpufreq_exit,
	.name		= "trix",
	.attr		= cpufreq_generic_attr,
};

static int trix_cpufreq_probe(struct platform_device *pdev)
{
	struct clk *cpu_clk;
	struct device *cpu_dev;
	int ret;

	/* it's sufficient to only check CPU0 */
	ret = allocate_resources(0, &cpu_dev, &cpu_clk);
	if (ret)
		return ret;

	clk_put(cpu_clk);

	trix_cpufreq_driver.driver_data = dev_get_platdata(&pdev->dev);

	return cpufreq_register_driver(&trix_cpufreq_driver);
}

static int trix_cpufreq_remove(struct platform_device *pdev)
{
	return cpufreq_unregister_driver(&trix_cpufreq_driver);
}

static struct platform_driver trix_cpufreq_platdrv = {
	.driver = {
		.name	= "trix-cpufreq-dt",
	},
	.probe		= trix_cpufreq_probe,
	.remove		= trix_cpufreq_remove,
};
module_platform_driver(trix_cpufreq_platdrv);

static const struct of_device_id machines[] __initconst = {
	{ .compatible = "sdesigns,sx8", },
	{ .compatible = "sdesigns,union", },
	{ }
};

static int __init cpufreq_dt_platdev_init(void)
{
	struct device_node *np = of_find_node_by_path("/");
	const struct of_device_id *match;

	if (!np)
		return -ENODEV;

	match = of_match_node(machines, np);
	of_node_put(np);
	if (!match)
		return -ENODEV;

	return PTR_ERR_OR_ZERO(platform_device_register_simple("trix-cpufreq-dt", -1,
							       NULL, 0));
}
device_initcall(cpufreq_dt_platdev_init);

MODULE_DESCRIPTION("cpufreq driver for Sigma DTV SoCs");
MODULE_LICENSE("GPL");
