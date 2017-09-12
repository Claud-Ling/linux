/*
 *  linux/drivers/soc/trix/dcsn.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns DTV SoC DCSN Driver
 *
 */
#define pr_fmt(fmt) "dcsndrv: " fmt

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>

#include <linux/soc/sigma-dtv/io.h>

struct initiator_info {
	const char **names;
	int nr;
};

struct trix_dcsn_device {
	phys_addr_t phys_base;
	int plf_irq;

	struct initiator_info *initiators;
};

/* dcsn configure regs offset */
#define DCSC_ADDR		0x100C /*R*/
#define DCSC_STAT		0x1010 /*R*/
#define DCSC_INT_CLR_EN		0x1FD8 /*W*/
#define DCSC_INT_SET_EN		0x1FDC /*W*/
#define DCSC_INT_STAT		0x1FE0 /*R*/
#define DCSC_INT_EN		0x1FE4 /*R/W*/
#define DCSC_INT_CLR_SEL	0x1FE8 /*W*/
#define DCSC_INT_SET_SEL	0x1FEC /*W*/

struct dcsn_ctrl{
	volatile u32 pad0[0x1000>>2];		/* +0x00000000 */
	volatile u32 bc_ctrl;			/* +0x00001000 */
	volatile u32 pad1[(0x100c-0x1004)>>2];	/* +0x00001004 */
	volatile u32 bc_addr;			/* +0x0000100c */
	volatile u32 bc_stat;			/* +0x00001010 */
	volatile u32 pad2[(0x1040-0x1014)>>2];	/* +0x00001014 */
	volatile u32 dcsc_feature;		/* +0x00001040 */
	volatile u32 pad3[(0x1200-0x1044)>>2];	/* +0x00001044 */
	volatile u32 cfg_dcs7_1_high;		/* +0x00001200 */
	volatile u32 cfg_dcs7_1_low;		/* +0x00001204 */
	volatile u32 pad4[(0x1fd8-0x1208)>>2];	/* +0x00001208 */
	volatile u32 bc_int_clr_en;		/* +0x00001fd8 */
	volatile u32 bc_int_set_en;		/* +0x00001fdc */
	volatile u32 bc_int_status;		/* +0x00001fe0 */
	volatile u32 bc_int_en;			/* +0x00001fe4 */
	volatile u32 bc_int_clr_sel;		/* +0x00001fe8 */
	volatile u32 bc_int_set_sel;		/* +0x00001fec */
	volatile u32 pad5[(0x1ffc-0x1ff0)>>2];	/* +0x00001ff0 */
	volatile u32 cfg_module_id;		/* +0x00001ffc */
};

/*
 * Interrupter handler for DCSN bus error and timeout.
 * Note: it's only for PLF CPU.
 *
 * [28:24] initiator ID, shows which master causing error.
 * [16:10] shows the error from which targets.
 * For the targets list, you can find it from 2.3.1 Address Allocation in TRM
 * [8] 1b'0: w, 1b'1: r.
 */
static const char *sx6_initiators_names[] =
{
	"Coresight DAP",	/* 0 */
	"A9 SECURE READ",	/* 1 */
	"A9 SECURE WRITE",	/* 2 */
	"A9 NORMAL",		/* 3 */
	"I2C Slave",		/* 4 */
	"AV MIPS",		/* 5 */
	"MVED",			/* 6 */
	"DISP MIPS",		/* 7 */
	"Standby MCU",		/* 8 */
	"EJTAG MMIO",		/* 9 */
	NULL,
};

static const char *sx7_initiators_names[] =
{
	"Coresight DAP",	/* 0 */
	"A9 SECURE READ",	/* 1 */
	"A9 SECURE WRITE",	/* 2 */
	"A9 NORMAL",		/* 3 */
	"I2C Slave",		/* 4 */
	"AV MIPS",		/* 5 */
	"MVED/HEVC",		/* 6 */
	"DISP MIPS",		/* 7 */
	"Standby MCU",		/* 8 */
	"EJTAG MMIO",		/* 9 */
	NULL,
};

static struct initiator_info sx6_intiators_info = {
	.names = sx6_initiators_names,
	.nr = ARRAY_SIZE(sx6_initiators_names),
};

static struct initiator_info sx7_intiators_info = {
	.names = sx7_initiators_names,
	.nr = ARRAY_SIZE(sx7_initiators_names),
};

static const struct of_device_id trix_dcsn_ids[] = {
	/* SoC specific */
	{ .compatible = "trix,dcsn-sx6", .data = &sx6_intiators_info},
	{ .compatible = "trix,dcsn-sx7", .data = &sx7_intiators_info},
	{ .compatible = "trix,dcsn-sx8", .data = &sx7_intiators_info},
	{ .compatible = "trix,dcsn-union", .data = &sx7_intiators_info},
	{ /* sentinel */ }
};

/**
 * DCSN event selection
 * 1 - for bus error
 * 2 - for timeout
 * 3 - for both bus error and timeout
 */
static unsigned int dcs_sel = 0x1; /*defaultly, only check bus error.*/
static int __init do_with_dcsn_sel(char *str)
{
	dcs_sel = simple_strtoul(str, NULL, 10) & 0x3;
	pr_info("get selection %x\n", dcs_sel);
	return 0;
}
early_param("dcsn_sel", do_with_dcsn_sel);

static u32 dcsn_reg_read(struct trix_dcsn_device *dcsn, unsigned int offset)
{
	return ReadRegWord(dcsn->phys_base + offset);
}

static void dcsn_reg_write(struct trix_dcsn_device *dcsn, unsigned int offset, u32 val)
{
	WriteRegWord(dcsn->phys_base + offset, val);
}

static int dcsn_hw_init(struct trix_dcsn_device *dcsn)
{
	unsigned int timeout_sel;

	dcs_sel &= 0x3; /*mask -- bit0: error check; bit1: timeout check*/

	/*
	 * enable DCS interrupt
	 */
	timeout_sel = (dcs_sel & 0x2) ? 1 : 0; /*timeout interrupt set or unchanged*/

	dcsn_reg_write(dcsn, DCSC_INT_SET_EN, timeout_sel);
	dcsn_reg_write(dcsn, DCSC_INT_EN, dcs_sel);

	return 0;
}

/*
 * dcs_plf_interrupt - main ISR
 * @irq: interrupt number.
 * @dev_id: to pass the net device pointer.
 */
static irqreturn_t dcs_plf_interrupt(int irq, void *dev_id)
{
	struct trix_dcsn_device *dcsn = (struct trix_dcsn_device *)dev_id;
	char *dcsn_source = "PLF DCSN";
	unsigned int dcs_addr, dcs_stat;

	printk(KERN_INFO "\n===================================\n");
	printk(KERN_INFO "dcs-c: DSCN interrupt from %s\n", dcsn_source);

	//NOTE: Once clear interrupter, int_status will be lost.
	dcs_addr = dcsn_reg_read(dcsn, DCSC_ADDR);
	dcs_stat = dcsn_reg_read(dcsn, DCSC_STAT);


	/* status */
	printk(KERN_INFO "dcs-c: DCSC_ADDR 0x%x\n", dcs_addr);
	printk(KERN_INFO "dcs-c: DCSC_STAT 0x%x\n", dcs_stat);
	printk(KERN_INFO "dcs-c: DCSC_INT_STAT 0x%x\n", dcsn_reg_read(dcsn, DCSC_INT_STAT));

	{
		int indictor = (dcs_stat>>24)&0x1F; //[28:24]
		int op = (dcs_stat>>8)&0x1; //[8]

		 printk(KERN_INFO "dcs-c: (!)DSCN error/timeout for [%s] tries to [%s] address [0x%x]!\n",
			(indictor>=0 && indictor < dcsn->initiators->nr) ? dcsn->initiators->names[indictor] : "Unkown initiator",
			(op==1) ? "read" : "write",
			dcs_addr );
	}

	/*
	 * Clear interrupter
	 */
	dcsn_reg_write(dcsn, DCSC_INT_CLR_SEL, dcs_sel);

	return IRQ_HANDLED;
}

/*
 * dcsn_probe_config_dt - parse device-tree driver parameters
 * @pdev: platform_device structure
 * @dcsn: driver data platform structure
 * Description:
 * this function is to read the driver parameters from device-tree and
 * set some private fields that will be used by the main at runtime.
 */
static int dcsn_probe_config_dt(struct platform_device *pdev,
				struct trix_dcsn_device *dcsn)
{
	const struct of_device_id *device;

	device = of_match_device(trix_dcsn_ids, &pdev->dev);
	if (!device) {
		dev_err(&pdev->dev, "Error: No device match found\n");
		return -ENODEV;
	}

	if (!device->data) {
		dev_err(&pdev->dev, "Error: No device spec data found\n");
		return -ENODEV;
	}

	dcsn->initiators = (struct initiator_info *)device->data;

	return 0;
}

static int dcsn_probe(struct platform_device *pdev)
{
	struct trix_dcsn_device *dcsn;
	struct resource *res;
	int ret;

	dcsn = devm_kzalloc(&pdev->dev, sizeof(struct trix_dcsn_device), GFP_KERNEL);
	if (!dcsn) {
		dev_err(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	if (pdev->dev.of_node) {
		ret = dcsn_probe_config_dt(pdev, dcsn);
		if (ret) {
			pr_err("%s: main dt probe failed", __func__);
			return ret;
		}
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dcsn->phys_base = (phys_addr_t)res->start;

	dcsn->plf_irq = platform_get_irq(pdev, 0);
	if (dcsn->plf_irq < 0) {
		if (dcsn->plf_irq != EPROBE_DEFER)
			dev_err (&pdev->dev, "DCSN PLF IRQ configuration information not found\n");

		return dcsn->plf_irq;
	}

	/* Request the IRQ lines */
	ret = request_irq(dcsn->plf_irq, dcs_plf_interrupt,
			  IRQF_SHARED, "DCSN", dcsn);
	if (unlikely(ret < 0)) {
		pr_err("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, dcsn->plf_irq, ret);
		return ret;
	}

	platform_set_drvdata(pdev, dcsn);

	dcsn_hw_init(dcsn);

	return 0;
}

static int dcsn_remove(struct platform_device *pdev)
{
	struct trix_dcsn_device *dcsn = platform_get_drvdata(pdev);

	free_irq(dcsn->plf_irq, NULL);

	return 0;
}

static __maybe_unused int dcsn_pm_suspend(struct device *dev)
{
	return 0;
}

static __maybe_unused int dcsn_pm_resume(struct device *dev)
{
	struct trix_dcsn_device *dcsn = dev_get_drvdata(dev);

	dcsn_hw_init(dcsn);

	return 0;
}

SIMPLE_DEV_PM_OPS(dcsn_pm_ops, dcsn_pm_suspend, dcsn_pm_resume);

static struct platform_driver dcsn_driver = {
	.probe = dcsn_probe,
	.remove = dcsn_remove,
	.driver = {
		.name = "trix-dcsn",
		.owner	= THIS_MODULE,
		.pm	= &dcsn_pm_ops,
		.of_match_table = of_match_ptr(trix_dcsn_ids),
	},
};

module_platform_driver(dcsn_driver);

MODULE_AUTHOR("Sigma Kernel team");
MODULE_DESCRIPTION("Sigma DCSN driver");
MODULE_LICENSE("GPL");

