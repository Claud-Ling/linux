/*
 * System IRQ manager Driver
 *
 * DOC:
 * Linux IRQ numbers, as used by request_irq() and similar functions,
 * are arbitrary software tokens rather than hardware IRQ.
 * To obtain such a token (called a virtual IRQ number, or virq), you need
 * to create a mapping.The most common way to do that is to use irq_of_parse_and_map(),
 *
 * This driver makes life easier for platforms that want to request a few
 * hard coded interrupt numbers that aren't properly represented in the
 * device-tree.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/string.h>

#include <linux/soc/sigma-dtv/irqs.h>

/*
 * set the GIC(gic-v2 or gic-v3)as the default system
 * IRQ domain
 * if we intend to request an IRQ from an extension
 * interrupt-controller in the future, fix here.
 */
static struct device_node *default_irq_device;

/*
 * Create mapping for GIC
 * @irq:   the interrupt number.SPI interrupts are in the range [0-987]
 *         PPI interrupts are in the range [0-15]
 * @type:  interrupt type; 0 for SPI interrupts, 1 for PPI
 *         interrupts.
 * @flags: trigger type and level flags
 *
 * see Documentation/devicetree/bindings/arm/gic(-v3).txt for the details
 * of gic bindings
 */
unsigned int trix_gic_create_mapping(unsigned int irq, unsigned int type, unsigned int flags)
{
	struct of_phandle_args irq_data;
	int virq;

	if (!default_irq_device)
		return 0;

	irq_data.np = default_irq_device;
	irq_data.args_count = 3;
	irq_data.args[0] = type;
	irq_data.args[1] = irq;
	irq_data.args[2] = flags;

	virq = irq_create_of_mapping(&irq_data);

	return virq;
}
EXPORT_SYMBOL(trix_gic_create_mapping);

/*
 * trix_request_irq - allocate an interrupt line
 * @type: 0 - SPI
 *        1 - PPI
 */
int trix_request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev, unsigned int type)
{
	int virq;

	virq = trix_gic_create_mapping(irq, type, flags);
	if (virq <= 0)
		return -EINVAL;

	return request_irq(virq, handler, flags, name, dev);
}
EXPORT_SYMBOL(trix_request_irq);

static int __init trix_irq_accessor_init(void)
{
	char *gic_name = IS_ENABLED(CONFIG_ARM64) ? "arm,gic-v3":
						    "arm,cortex-a9-gic";

	default_irq_device = of_find_compatible_node(NULL, NULL, gic_name);
	if (!default_irq_device) {
		pr_err("%s: GIC node not found\n", __func__);
	}

	of_node_put(default_irq_device);

	return 0;
}
arch_initcall(trix_irq_accessor_init);

static void __exit trix_irq_accessor_exit(void)
{
}
module_exit(trix_irq_accessor_exit);

MODULE_AUTHOR("Claud Ling <Claud_Ling@sigmadesigns.com>");
MODULE_DESCRIPTION("System IRQ manager driver");
MODULE_LICENSE("GPL v2");
