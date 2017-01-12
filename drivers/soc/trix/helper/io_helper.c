/*
 * System IO access helper
 *
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
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/soc/sigma-dtv/io.h>

/**
 * struct system_io_region - IO memory region
 * @phys_base:	base address for the memory region
 * @virt_base:	virtual base address of memory with this phys_base
 * @size:	size of the memory region
 * @secure:	the memory region is secure accessible
 * @node:	list node
 *
 */
struct system_io_region {
	phys_addr_t	phys_base;
	void __iomem *	virt_base;
	size_t		size;
	bool		secure;
	struct list_head node;
};

/**
 * struct io_accessor - IO accessor instance
 * @dev:	device entry
 * @lock:	spinlock protecting the lists
 * @nsec:	list of non-secure io nodes
 * @sec:	list of secure io nodes
 */
struct io_accessor {
	struct device *dev;
	spinlock_t lock;
	struct list_head nsec;
	struct list_head sec;
};

/* Pointer to the only one IO accessor instance */
static struct io_accessor *__accessor = NULL;

/*
 * iterate over the list, find the proper node.
 */
static struct system_io_region *find_io_region(phys_addr_t pa)
{
	struct system_io_region *entry, *region = NULL;

	if (!__accessor)
		return NULL;

	/* iterate over the secure io list */
	list_for_each_entry(entry, &__accessor->sec, node) {
		if (SIGMA_IO_BETWEEN(pa, entry->phys_base, entry->size)) {
			region = entry;
			goto found;
		}
	}

	/* iterate over the non-secure io list */
	list_for_each_entry(entry, &__accessor->nsec, node) {
		if (SIGMA_IO_BETWEEN(pa, entry->phys_base, entry->size)) {
			region = entry;
			goto found;
		}
	}

found:
	return region;
}

static inline int check_alignment(uint32_t mode, phys_addr_t reg)
{
	switch (mode) {
	case BYTE:
		return 0;
	case HALFWORD:
		return (reg & 0x1)? -EINVAL : 0;
	case WORD:
		return (reg & 0x3)? -EINVAL : 0;
	default:
		return -EINVAL;
	}
}

int io_accessor_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval)
{
	struct system_io_region *region;
	phys_addr_t reg = SIGMA_IO_ADDRESS(pa);
	void __iomem *va;

	BUG_ON(pval == NULL);

	if (!__accessor)
		return -EPROBE_DEFER;

	region = find_io_region(reg);
	if (!region)
		return -EINVAL;

	if (check_alignment(mode, reg))
		return -EINVAL;

	if (region->secure) {
		//TODO add support for smc call
		return -EACCES;
	} else {
		va = region->virt_base + (reg - region->phys_base);

		if (mode == BYTE)
			*pval = __raw_readb(va);
		else if (mode == HALFWORD)
			*pval = __raw_readw(va);
		else if (mode == WORD)
			*pval = __raw_readl(va);
		else
			*pval = 0;
	}

	return 0;
}
EXPORT_SYMBOL(io_accessor_read_reg);

int io_accessor_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask)
{
	struct system_io_region *region;
	phys_addr_t reg = SIGMA_IO_ADDRESS(pa);
	void __iomem *va;

	if (!__accessor)
		return -EPROBE_DEFER;

	region = find_io_region(reg);
	if (!region)
		return -EINVAL;

	if (check_alignment(mode, reg))
		return -EINVAL;

	if (region->secure) {
		//TODO add support for smc call
		return -EACCES;
	} else {
		va = region->virt_base + (reg - region->phys_base);

		if (mode == BYTE) {
			if (mask != U8_MAX)
				MaskWriteReg(b, va, val, mask);
			else
				__raw_writeb(val, va);
		} else if (mode == HALFWORD) {
			if (mask != U16_MAX)
				MaskWriteReg(w, va, val, mask);
			else
				__raw_writew(val, va);
		} else if (mode == WORD) {
			if (mask != U32_MAX)
				MaskWriteReg(l, va, val, mask);
			else
				__raw_writel(val, va);
		} else
			pr_warning("unknown access mode value (%d)\n", mode);
	}

	return 0;

}
EXPORT_SYMBOL(io_accessor_write_reg);

static int add_system_io_region(struct device_node *np,
			struct io_accessor *acc, bool is_secure)
{
	struct system_io_region *region;
	struct resource r;
	int ret;

	region = kzalloc(sizeof(*region), GFP_KERNEL);
	if (!region)
		return -ENOMEM;

	ret = of_address_to_resource(np, 0, &r);
	of_node_put(np);
	if(ret)
		goto free;

	region->phys_base = (phys_addr_t)r.start;
	region->size = resource_size(&r);
	region->secure = is_secure;

	if (is_secure) {
		list_add_tail(&region->node, &acc->sec);
	} else {
		region->virt_base = ioremap_nocache(r.start, resource_size(&r));
		if (!region->virt_base) {
			ret = -ENOMEM;
			goto free;
		}

		list_add_tail(&region->node, &acc->nsec);
	}

	return 0;

free:
	kfree(region);
	return ret;
}

static void io_accessor_dt_child(struct io_accessor *acc,
				 struct device_node *np)
{
	struct device_node *child;

	for_each_child_of_node(np, child) {
		if (of_property_read_bool(child, "secure-io")) {
			/* secure io node */
			add_system_io_region(child, acc, true);
		} else {
			/* non-secure io node */
			add_system_io_region(child, acc, false);
		}
	}
}

static int io_accessor_of_parse(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct io_accessor *accessor = platform_get_drvdata(pdev);

	io_accessor_dt_child(accessor, np);

	return 0;
}

static int io_accessor_probe(struct platform_device *pdev)
{
	struct io_accessor *accessor;

	accessor = devm_kzalloc(&pdev->dev, sizeof(*accessor), GFP_KERNEL);
	if (!accessor) {
		dev_err(&pdev->dev, "could not allocate memory\n");
		return -ENOMEM;
	}

	accessor->dev = &pdev->dev;
	spin_lock_init(&accessor->lock);
	INIT_LIST_HEAD(&accessor->nsec);
	INIT_LIST_HEAD(&accessor->sec);

	platform_set_drvdata(pdev, accessor);

	io_accessor_of_parse(pdev);

	__accessor = accessor;

	return 0;
}

static const struct of_device_id io_accessor_match[] = {
	{ .compatible = "trix,io-accessor", },
	{ }
};

static struct platform_driver io_accessor_driver = {
	.probe = io_accessor_probe,
	.driver = {
		.name = "io_accessor",
		.of_match_table	= io_accessor_match,
	},
};

static int __init io_accessor_init(void)
{
	return platform_driver_register(&io_accessor_driver);
}
arch_initcall(io_accessor_init);

static void __exit io_accessor_exit(void)
{
	platform_driver_unregister(&io_accessor_driver);
}
module_exit(io_accessor_exit);

MODULE_AUTHOR("Claud Ling <Claud_Ling@sigmadesigns.com>");
MODULE_DESCRIPTION("System io accessor driver");
MODULE_LICENSE("GPL v2");
