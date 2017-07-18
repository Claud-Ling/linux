/*
 * System Control Driver
 *
 * DOC:
 * System controller node represents a register region containing a set
 * of miscellaneous registers. The registers are not cohesive enough to
 * represent as any specific type of device. The typical use-case is for
 * some other node's driver, or platform-specific code, to acquire a
 * reference to the syscon node (e.g. by phandle, node path, or search
 * using a specific compatible value), interrogate the node (or associated
 * OS driver) to determine the location of the registers, and access the
 * registers directly.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_data/syscon.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/soc/sigma-dtv/syscon.h>

static struct platform_driver syscon_driver;

static DEFINE_SPINLOCK(syscon_list_slock);
static LIST_HEAD(syscon_list);

struct syscon {
	struct device_node *np;
	void __iomem *reg;
	struct list_head list;
};

static struct syscon *of_syscon_register(struct device_node *np)
{
	struct syscon *syscon;
	void __iomem *base;
	int ret;
	struct resource res;

	if (!of_device_is_compatible(np, "trix,syscon"))
		return ERR_PTR(-EINVAL);

	syscon = kzalloc(sizeof(*syscon), GFP_KERNEL);
	if (!syscon)
		return ERR_PTR(-ENOMEM);

	if (of_address_to_resource(np, 0, &res)) {
		ret = -ENOMEM;
		goto err_map;
	}

	base = ioremap(res.start, resource_size(&res));
	if (!base) {
		ret = -ENOMEM;
		goto err_map;
	}

	syscon->reg = base;
	syscon->np = np;

	spin_lock(&syscon_list_slock);
	list_add_tail(&syscon->list, &syscon_list);
	spin_unlock(&syscon_list_slock);

	return syscon;

err_map:
	kfree(syscon);
	return ERR_PTR(ret);
}

void __iomem *syscon_node_to_iomap(struct device_node *np)
{
	struct syscon *entry, *syscon = NULL;

	spin_lock(&syscon_list_slock);

	list_for_each_entry(entry, &syscon_list, list)
		if (entry->np == np) {
			syscon = entry;
			break;
		}

	spin_unlock(&syscon_list_slock);

	if (!syscon)
		syscon = of_syscon_register(np);

	if (IS_ERR(syscon))
		return ERR_CAST(syscon);

	return syscon->reg;
}
EXPORT_SYMBOL_GPL(syscon_node_to_iomap);

void __iomem *syscon_iomap_lookup_by_compatible(const char *s)
{
	struct device_node *syscon_np;
	void __iomem *reg;

	syscon_np = of_find_compatible_node(NULL, NULL, s);
	if (!syscon_np)
		return ERR_PTR(-ENODEV);

	reg = syscon_node_to_iomap(syscon_np);
	of_node_put(syscon_np);

	return reg;
}
EXPORT_SYMBOL_GPL(syscon_iomap_lookup_by_compatible);

static int syscon_match_pdevname(struct device *dev, void *data)
{
	return !strcmp(dev_name(dev), (const char *)data);
}

void __iomem *syscon_iomap_lookup_by_pdevname(const char *s)
{
	struct device *dev;
	struct syscon *syscon;

	dev = driver_find_device(&syscon_driver.driver, NULL, (void *)s,
				 syscon_match_pdevname);
	if (!dev)
		return ERR_PTR(-EPROBE_DEFER);

	syscon = dev_get_drvdata(dev);

	return syscon->reg;
}
EXPORT_SYMBOL_GPL(syscon_iomap_lookup_by_pdevname);

void __iomem *syscon_iomap_lookup_by_phandle(struct device_node *np,
					const char *property)
{
	struct device_node *syscon_np;
	void __iomem *reg;

	if (property)
		syscon_np = of_parse_phandle(np, property, 0);
	else
		syscon_np = np;

	if (!syscon_np)
		return ERR_PTR(-ENODEV);

	reg = syscon_node_to_iomap(syscon_np);
	of_node_put(syscon_np);

	return reg;
}
EXPORT_SYMBOL_GPL(syscon_iomap_lookup_by_phandle);

static int syscon_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct syscon *syscon;
	struct resource *res;
	void __iomem *base;

	syscon = devm_kzalloc(dev, sizeof(*syscon), GFP_KERNEL);
	if (!syscon)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOENT;

	base = devm_ioremap(dev, res->start, resource_size(res));
	if (!base)
		return -ENOMEM;

	platform_set_drvdata(pdev, syscon);

	dev_dbg(dev, "syscon %pR registered\n", res);

	return 0;
}

static const struct platform_device_id syscon_ids[] = {
	{ "trix,syscon", },
	{ }
};

static struct platform_driver syscon_driver = {
	.driver = {
		.name = "trix,syscon",
	},
	.probe		= syscon_probe,
	.id_table	= syscon_ids,
};

static int __init syscon_init(void)
{
	return platform_driver_register(&syscon_driver);
}
postcore_initcall(syscon_init);

static void __exit syscon_exit(void)
{
	platform_driver_unregister(&syscon_driver);
}
module_exit(syscon_exit);

MODULE_AUTHOR("Claud Ling <Claud_Ling@sigmadesigns.com>");
MODULE_DESCRIPTION("System Control driver");
MODULE_LICENSE("GPL v2");
