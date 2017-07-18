/*
 *  umac.c
 *
 *  Copyright (c) 2016 Sigma Designs Limited
 *  All Rights Reserved
 *
 *  This file describes the top level table for UMACs on
 *  system. The code is shared by preboot, uboot and linux.
 *  It supports up to 3 umacs at most by now.
 *
 * Author: Tony He <tony_he@sigmadesigns.com>
 * Date:   05/17/2016
 */

#include <linux/kernel.h>
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
#include <umac.h>

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif


//extern struct soc_umac_ops union1_umac_ops;
static const struct of_device_id umac_of_match[];

/*
 * stub trix_udev_tbl
 */
__attribute__((weak)) \
struct umac_device *trix_udev_tbl[MAX_NR_UMACS] = { 0 };

static atomic_t nr_umacs =  ATOMIC_INIT(0);

int get_nr_umacs(void)
{
	return atomic_read(&nr_umacs);
}

struct umac_device* umac_get_dev_by_id(int uid)
{
	if (uid >= 0 && uid < ARRAY_SIZE(trix_udev_tbl))
		return trix_udev_tbl[uid];
	else
		return NULL;
}

/*
 * stub implemenation of umac_is_activated
 */
__attribute__((weak)) int umac_is_activated(int uid)
{
	struct umac_device* udev = umac_get_dev_by_id(uid);
	if (!udev || !udev->ctl) {
		return 0;
	}
	return !!umac_is_active(udev->ctl);
}

#ifndef FN_UMAC0_START_ADDR
static inline uint32_t umac0_start_addr(struct pman_con *con)
{
	pr_warn("%s stub, please implement it in MD file\n",__func__);
	return 0;
}
#endif
#ifndef FN_UMAC1_START_ADDR
static inline uint32_t umac1_start_addr(struct pman_con *con)
{
	pr_warn("%s stub, please implement it in MD file\n",__func__);
	return 0;
}
#endif
#ifndef FN_UMAC2_START_ADDR
static inline uint32_t umac2_start_addr(struct pman_con *con)
{
	pr_warn("%s stub, please implement it in MD file\n",__func__);
	return 0;
}
#endif

const u32 umac_get_addr(int uid)
{
	struct umac_device *udev = umac_get_dev_by_id(uid);
	uint32_t addr = 0;

	if (!udev) {
	/*return zero though it's not good*/
		pr_warning("Can't find UMAC dev by id(uid)\n");
		return 0;
	}
	if (!udev->pman) {
		return 0;
	}

	switch (udev->id) {
	case 0:
		addr = umac0_start_addr(udev->pman);
		break;
	case 1:
		addr = umac1_start_addr(udev->pman);
		break;
	case 2:
		addr = umac2_start_addr(udev->pman);
		break;

	default:
		return 0;

	}
	return (addr<< 28);

}

static int umac_add_dev(struct umac_device *udev)
{
	trix_udev_tbl[udev->id] = udev;
	atomic_inc(&nr_umacs);
	return 0;
}

static int umac_del_dev(struct umac_device *udev)
{
	trix_udev_tbl[udev->id] = NULL;
	atomic_dec(&nr_umacs);
	return 0;
}


static int umac_dev_of_init(struct umac_device *udev)
{
	struct device_node *np = udev->dev->of_node;
	struct device_node *child;
	int ret = -1;
	u32 id = 0, width = 0, granule = 0;

	child = of_get_child_by_name(np, "umac_ctl");
	if (child) {
		udev->ctl = of_iomap(child, 0);
	}

	child = of_get_child_by_name(np, "pman_con");
	if (child) {
		udev->pman = of_iomap(child, 0);
	}

	ret = of_property_read_u32(np, "umac-id", &id);
	if (ret || !(id < MAX_NR_UMACS)) {
		dev_err(udev->dev, "'umac-id' error(%d)\n", ret);
		return -ENODEV;
	}
	ret = of_property_read_u32(np, "width", &width);
	if (ret) {
		dev_err(udev->dev, "'width' error(%d)\n", ret);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "granule", &granule);
	if (ret) {
		/* Default mszie granule 256(MB) */
		granule = (1<<8);
		dev_warn(udev->dev, "Not specify 'granule' in DT\n");
	}

	udev->id = id;
	udev->width = width;
	udev->granule = granule;

	dev_err(udev->dev, "UMAC(%d) with ctl(%p), pman_con(%p),"
			"width(%d) granule(%d)", udev->id, udev->ctl, udev->pman,
			 udev->width, udev->granule);
	return 0;
}

static int umac_probe(struct platform_device *pdev)
{
	struct umac_device*	udev = NULL;
	const struct of_device_id*	match = NULL;
	struct device 		*dev = &pdev->dev;

	udev = devm_kzalloc(&pdev->dev, sizeof(struct umac_device), GFP_KERNEL);
	if (!udev) {
		dev_err(&pdev->dev, "Menory allocation failed\n");
		return -ENOMEM;
	}


	udev->dev = dev;
	match = of_match_node(umac_of_match, udev->dev->of_node);

	if (umac_dev_of_init(udev)) {
		return -ENODEV;
	}

	platform_set_drvdata(pdev, udev);

	umac_add_dev(udev);

	return 0;
}

static int umac_remove(struct platform_device *pdev)
{
	struct umac_device *udev = platform_get_drvdata(pdev);
	umac_del_dev(udev);
	return 0;
}

static __maybe_unused int umac_pm_suspend(struct device *dev)
{
	return 0;
}

static __maybe_unused int umac_pm_resume(struct device *dev)
{
	return 0;
}

static const struct of_device_id umac_of_match[] = {
	{ .compatible = "trix,umac"},
	{ }
};
MODULE_DEVICE_TABLE(of, umac_of_match);

SIMPLE_DEV_PM_OPS(umac_pm_ops, umac_pm_suspend, umac_pm_resume);
static struct platform_driver umac_driver = {
	.probe = umac_probe,
	.remove = umac_remove,
	.driver = {
		.name = "trix-umac",
		.owner	= THIS_MODULE,
		.pm	= &umac_pm_ops,
		.of_match_table = of_match_ptr(umac_of_match),
	},
};

static int __init umac_drv_init(void)
{
	return platform_driver_register(&umac_driver);
}
arch_initcall(umac_drv_init);

static void __exit umac_drv_exit(void)
{
	platform_driver_unregister(&umac_driver);
}
module_exit(umac_drv_exit);
MODULE_AUTHOR("Sigma Kernel team");
MODULE_DESCRIPTION("Sigma UMAC adapter");
MODULE_LICENSE("GPL");

