/*
 * SoC information
 *
 * DOC:
 * This driver present SoC information & attributes in /sys/device/soc0/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/init.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/sys_soc.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <linux/soc/sigma-dtv/syscon.h>

static const struct of_device_id trix_soc_of_match[] = {
	{ .compatible = "sdesigns,sx8",},
	{ .compatible = "sdesigns,union",},
	{ .compatible = "sdesigns,union2",},
	{ }
};


static int __init trix_soc_init(void)
{
	struct soc_device *soc_dev;
	struct soc_device_attribute *soc_dev_attr;
	struct device_node *np;
	struct device *dev;
	int ret;

	np = of_find_matching_node(NULL, trix_soc_of_match);
	if (!np)
		return -ENODEV;

	soc_dev_attr = kzalloc(sizeof(*soc_dev_attr), GFP_KERNEL);
	if (!soc_dev_attr)
		return -ENOMEM;

	ret = of_property_read_string(np, "model",
				      &soc_dev_attr->machine);
	if (ret)
		return -EINVAL;

	soc_dev_attr->family = "TRIX";
	soc_dev = soc_device_register(soc_dev_attr);
	if (IS_ERR(soc_dev)) {
		kfree(soc_dev_attr);
		return -ENODEV;
	}
	dev = soc_device_to_device(soc_dev);

	/*
	 * TODO: add more extra attributes of SoC to sysfs
	 *        via device_create_file(dev, &xxx_attr)
	 */

	return 0;
}
device_initcall(trix_soc_init);
