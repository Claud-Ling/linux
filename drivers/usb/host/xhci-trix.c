/*
 * xhci-plat.c - xHCI host controller driver platform Bus Glue.
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com
 * Author: Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * A lot of code borrowed from the Linux xHCI driver.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/usb/phy.h>
#include <linux/slab.h>
#include <linux/usb/xhci_pdriver.h>

#include "xhci.h"

/*** Register Offset ***/
#define TRIX_USB3_USBCMD	0x0020	/*USBCMD*/
#define TRIX_USB3_GRXTHRCFG	0xc10c	/*GRXTHRCFG*/
#define TRIX_USB3_GUCTL		0xc12c	/*GUCTL*/
#define TRIX_USB3_GCTL		0xc110	/*GCTL*/
#define TRIX_USB3_GUSB2PHYCFG	0xc200	/*GUSB2PHYCFG*/
#define TRIX_USB3_GUSB3PIPECTL	0xc2c0	/*GUSB3PIPECTL*/
#define TRIX_USB3_TEST_DBG_REG	0xcd04	/*TEST_DBG_REG*/
#define TRIX_USB3_PHY_DBG_REG	0xcd08	/*PHY_DBG_REG*/
#define TRIX_USB3_PHY_TUNE_REG2	0xcd0c	/*PHY_TUNE_REG2*/
#define TRIX_USB3_CFG_REG2	0xcd14	/*CFG_REG2*/

/*** Register Settings ***/
#define TRIX_USB3_USBCMD_HCRST		BIT(1)
#define TRIX_USB3_USBCMD_HCRST_MASK	BIT(1)

#define TRIX_USB3_GUCTL_VAL	0x0a808010

#define TRIX_USB3_GCTL_VAL	0x2fa01004

#define TRIX_USB3_GUSB3PIPECTL_VAL	0x130e0002 /*set bit[28] to enable detection in P2*/

#define TRIX_USB3_GUSB2PHYCFG_VAL	0x00002540

#define TRIX_USB3_CFG_REG2_USBPHY_REF_SSP_EN		BIT(9)
#define TRIX_USB3_CFG_REG2_USBPHY_REF_SSP_EN_MASK	BIT(9)

#define TRIX_USB3_PHY_TUNE_REG2_USBPHY_PCS_TX_SWING_FULL	0x7f
#define TRIX_USB3_PHY_TUNE_REG2_USBPHY_PCS_TX_SWING_FULL_MASK	0x7f	/*bit[6:0]*/

#define TRIX_USB3_GRXTHRCFG_USBMaxRxBurstSize_VAL	(0x2 << 19)
#define TRIX_USB3_GRXTHRCFG_USBMaxRxBurstSize_MASK	(0x1f << 19)	/*bit[23:19]*/
#define TRIX_USB3_GRXTHRCFG_USBRxPktCnt_VAL		(0x3 << 24)
#define TRIX_USB3_GRXTHRCFG_USBRxPktCnt_MASK		(0xf << 24)	/*bit[27:24]*/
#define TRIX_USB3_GRXTHRCFG_USBRxPktCntSel_VAL		BIT(29)		/*bit[29]*/
#define TRIX_USB3_GRXTHRCFG_USBRxPktCntSel_MASK		BIT(29)		/*bit[29]*/

#if !defined(CONFIG_OF)
static void xhci_trix_init_quirks(struct usb_hcd *hcd)
{
#ifdef CONFIG_SIGMA_SOC_SX6
	MWriteRegByte((volatile void*)0x1500ee20, 0x0, 0x7);	//[2:0]=3'b000
	MWriteRegByte((volatile void*)0x1500ee87, 0x4, 0x4);	//[2]=1'b1
	MWriteRegHWord((volatile void*)0x1b005520, 0x4, 0xc);	//[3:2]=2'b01
	MWriteRegHWord((volatile void*)0x1b005522, 0x2, 0x2);	//[1]=1'b1

#elif defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX8)
	/* Set GPIO9 pin as GPIO */
	MWriteRegByte((volatile void*)0x1500ee22, 0x0, 0x70);

	/* Set GPIO9 output mode */
	MWriteRegHWord((volatile void*)0x1b005520, 0x4, 0xc);

	/* Set GPIO9 output val equ 1 */
	MWriteRegHWord((volatile void*)0x1b005522, 0x2, 0x2);

	/* Set GPIO8 act as USB3_OC */
	MWriteRegByte((volatile void*)0x1500ee22, 0x2, 0x7);

	/* Set GPIO7 act as USB3_OC */
	MWriteRegByte((volatile void*)0x1500ee21, 0x20, 0x70);
#else
	#error "Unknow SOC type!!"
#endif

	BUG_ON(hcd == NULL);
	BUG_ON(hcd->regs == NULL);
#if defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX6)
	/* Reset USB Host */
	//MWriteRegWord((volatile void*)0x15200020, 0x2,0x2);
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_USBCMD, TRIX_USB3_USBCMD_HCRST, TRIX_USB3_USBCMD_HCRST_MASK);

	/* Set REFCLKPER */	
	//MWriteRegWord((volatile void*)0x1520c12c, 0xa808010, 0xffffffff);
	WriteRegWord((volatile void*)hcd->regs + TRIX_USB3_GUCTL, TRIX_USB3_GUCTL_VAL);
	
	/* Set Power down scale */
	//MWriteRegWord((volatile void*)0x1520c110, 0x2fa01004, 0xffffffff);
	WriteRegWord((volatile void*)hcd->regs + TRIX_USB3_GCTL, TRIX_USB3_GCTL_VAL);

	/* Set USB3 PIPE Control */
	/* Disable receiver detection in P3, Fix synopsys PHY bug */
	//MWriteRegWord((volatile void*)0x1520c2c0, 0x030e0002, 0xffffffff);
	WriteRegWord((volatile void*)hcd->regs + TRIX_USB3_GUSB3PIPECTL, TRIX_USB3_GUSB3PIPECTL_VAL);

	/* Set USB2 PHY Configuration */
	//MWriteRegWord((volatile void*)0x1520c200, 0x00002540, 0xffffffff);
	WriteRegWord((volatile void*)hcd->regs + TRIX_USB3_GUSB2PHYCFG, TRIX_USB3_GUSB2PHYCFG_VAL);

	/* Enable SS PHY REF clock */
	//MWriteRegWord((volatile void*)0x1520cd14, 0x200,0x200);
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_CFG_REG2, 0x200, TRIX_USB3_CFG_REG2_USBPHY_REF_SSP_EN_MASK);

	/* Set USB3.0 PHY TX swing strength to full level */
	//MWriteRegWord((volatile void*)0x1520cd0c, 0x7f, 0x7f);
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_PHY_TUNE_REG2, 0x7f, TRIX_USB3_PHY_TUNE_REG2_USBPHY_PCS_TX_SWING_FULL_MASK);

	/* Enable USB3.0 PHY power */
	//MWriteRegWord((volatile void*)0x1520cd04, 0x00000000, 0xc0000000);
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_TEST_DBG_REG, 0x00000000, 0xc0000000);

	/* Disable receiver detection in P3, Fix synopsys PHY bug */
	//MWriteRegWord((volatile void*)0x1520c2c0, 
	//				(1<<28), 0x10000000);
#elif defined(CONFIG_SIGMA_SOC_SX8)
	/* Disable USB3 PHY Scramble */
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_TEST_DBG_REG, 0x00202087, 0x00ffffff);
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_TEST_DBG_REG, 0x00302087, 0x00ffffff);
	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_TEST_DBG_REG, 0x00202087, 0x00ffffff);
#endif

}

/* called after usb_add_hcd*/
static void xhci_trix_start(struct usb_hcd *hcd)
{
	BUG_ON(hcd == NULL);
	BUG_ON(hcd->regs == NULL);
#if defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX6)
	/* Adjust USB3 RX FIFO threshold */
	//MWriteRegWord((volatile void*)0x1520c10c, 
	//		((3<<24)|(2<<19)), 0x0ff80000);
	/* Enable FIFO threshold control */
	//MWriteRegWord((volatile void*)0x1520c10c, 
	//				(1<<29), 0x20000000);

	MWriteRegWord((volatile void*)hcd->regs + TRIX_USB3_GRXTHRCFG, 
			TRIX_USB3_GRXTHRCFG_USBMaxRxBurstSize_VAL | 
			TRIX_USB3_GRXTHRCFG_USBRxPktCnt_VAL | 
			TRIX_USB3_GRXTHRCFG_USBRxPktCntSel_VAL, 
			TRIX_USB3_GRXTHRCFG_USBMaxRxBurstSize_MASK | 
			TRIX_USB3_GRXTHRCFG_USBRxPktCnt_MASK | 
			TRIX_USB3_GRXTHRCFG_USBRxPktCntSel_MASK);
#endif
}
#endif /*!CONFIG_OF*/

static struct hc_driver __read_mostly xhci_plat_hc_driver;

static void xhci_plat_quirks(struct device *dev, struct xhci_hcd *xhci)
{
	/*
	 * As of now platform drivers don't provide MSI support so we ensure
	 * here that the generic code does not try to make a pci_dev from our
	 * dev struct in order to setup MSI
	 */
	xhci->quirks |= XHCI_PLAT;
}

/* called during probe() after chip reset completes */
static int xhci_plat_setup(struct usb_hcd *hcd)
{
	return xhci_gen_setup(hcd, xhci_plat_quirks);
}

static int xhci_plat_start(struct usb_hcd *hcd)
{
	return xhci_run(hcd);
}

static int xhci_plat_probe(struct platform_device *pdev)
{
	struct device_node	*node = pdev->dev.of_node;
	struct usb_xhci_pdata	*pdata = dev_get_platdata(&pdev->dev);
	const struct hc_driver	*driver;
	struct xhci_hcd		*xhci;
	struct resource         *res;
	struct usb_hcd		*hcd;
	struct clk              *clk;
	int			ret;
	int			irq;

	if (usb_disabled())
		return -ENODEV;

	driver = &xhci_plat_hc_driver;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return -ENODEV;

	/* Initialize dma_mask and coherent_dma_mask to 32-bits */
	ret = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;
	if (!pdev->dev.dma_mask)
		pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
	else
		dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	hcd->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(hcd->regs)) {
		ret = PTR_ERR(hcd->regs);
		goto put_hcd;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	/*
	 * Not all platforms have a clk so it is not an error if the
	 * clock does not exists.
	 */
	clk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(clk)) {
		ret = clk_prepare_enable(clk);
		if (ret)
			goto put_hcd;
	}
#if !defined(CONFIG_OF)
	xhci_trix_init_quirks(hcd);
#endif
	ret = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (ret)
		goto disable_clk;

	device_wakeup_enable(hcd->self.controller);

	/* USB 2.0 roothub is stored in the platform_device now. */
	hcd = platform_get_drvdata(pdev);
	xhci = hcd_to_xhci(hcd);
	xhci->clk = clk;
	xhci->shared_hcd = usb_create_shared_hcd(driver, &pdev->dev,
			dev_name(&pdev->dev), hcd);
	if (!xhci->shared_hcd) {
		ret = -ENOMEM;
		goto dealloc_usb2_hcd;
	}

	if ((node && of_property_read_bool(node, "usb3-lpm-capable")) ||
			(pdata && pdata->usb3_lpm_capable))
		xhci->quirks |= XHCI_LPM_SUPPORT;
	/*
	 * Set the xHCI pointer before xhci_plat_setup() (aka hcd_driver.reset)
	 * is called by usb_add_hcd().
	 */
	*((struct xhci_hcd **) xhci->shared_hcd->hcd_priv) = xhci;

	if (HCC_MAX_PSA(xhci->hcc_params) >= 4)
		xhci->shared_hcd->can_do_streams = 1;

	hcd->usb_phy = devm_usb_get_phy_by_phandle(&pdev->dev, "usb-phy", 0);
	if (IS_ERR(hcd->usb_phy)) {
		ret = PTR_ERR(hcd->usb_phy);
		if (ret == -EPROBE_DEFER)
			goto put_usb3_hcd;
		hcd->usb_phy = NULL;
	} else {
		ret = usb_phy_init(hcd->usb_phy);
		if (ret)
			goto put_usb3_hcd;
	}

	ret = usb_add_hcd(xhci->shared_hcd, irq, IRQF_SHARED);
	if (ret)
		goto disable_usb_phy;

#if !defined(CONFIG_OF)
	xhci_trix_start(hcd);
#endif
	return 0;

disable_usb_phy:
	usb_phy_shutdown(hcd->usb_phy);

put_usb3_hcd:
	usb_put_hcd(xhci->shared_hcd);

dealloc_usb2_hcd:
	usb_remove_hcd(hcd);

disable_clk:
	if (!IS_ERR(clk))
		clk_disable_unprepare(clk);

put_hcd:
	usb_put_hcd(hcd);

	return ret;
}

static int xhci_plat_remove(struct platform_device *dev)
{
	struct usb_hcd	*hcd = platform_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	struct clk *clk = xhci->clk;

	usb_remove_hcd(xhci->shared_hcd);
	usb_phy_shutdown(hcd->usb_phy);
	usb_put_hcd(xhci->shared_hcd);

	usb_remove_hcd(hcd);
	if (!IS_ERR(clk))
		clk_disable_unprepare(clk);
	usb_put_hcd(hcd);
	kfree(xhci);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int xhci_plat_suspend(struct device *dev)
{
	struct usb_hcd	*hcd = dev_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);

	/*
	 * xhci_suspend() needs `do_wakeup` to know whether host is allowed
	 * to do wakeup during suspend. Since xhci_plat_suspend is currently
	 * only designed for system suspend, device_may_wakeup() is enough
	 * to dertermine whether host is allowed to do wakeup. Need to
	 * reconsider this when xhci_plat_suspend enlarges its scope, e.g.,
	 * also applies to runtime suspend.
	 */
	return xhci_suspend(xhci, device_may_wakeup(dev));
}

static int xhci_plat_resume(struct device *dev)
{
	struct usb_hcd	*hcd = dev_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);

#if !defined(CONFIG_OF)
	xhci_trix_init_quirks(hcd);
	xhci_trix_start(hcd);
#endif
	if (usb_phy_init(hcd->usb_phy))
		dev_err(dev, "%s: phy resume failed\n", __func__);

	return xhci_resume(xhci, 0);
}

static const struct dev_pm_ops xhci_plat_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xhci_plat_suspend, xhci_plat_resume)
};
#define DEV_PM_OPS	(&xhci_plat_pm_ops)
#else
#define DEV_PM_OPS	NULL
#endif /* CONFIG_PM */

static const struct of_device_id xhci_trix_dt_match[] = {
	{ .compatible = "sigma,trix-xhci" },
	{/* sentinel */}
};

static struct platform_driver usb_xhci_driver = {
	.probe	= xhci_plat_probe,
	.remove	= xhci_plat_remove,
	.driver	= {
		.name = "trix-xhci",
		.of_match_table = of_match_ptr(xhci_trix_dt_match),
		.pm = DEV_PM_OPS,
	},
};
MODULE_ALIAS("trix:xhci-hcd");

static int __init xhci_plat_init(void)
{
	xhci_init_driver(&xhci_plat_hc_driver, xhci_plat_setup);
	xhci_plat_hc_driver.start = xhci_plat_start;
	return platform_driver_register(&usb_xhci_driver);
}
module_init(xhci_plat_init);

static void __exit xhci_plat_exit(void)
{
	platform_driver_unregister(&usb_xhci_driver);
}
module_exit(xhci_plat_exit);

MODULE_DESCRIPTION("xHCI Platform Host Controller Driver");
MODULE_LICENSE("GPL");
