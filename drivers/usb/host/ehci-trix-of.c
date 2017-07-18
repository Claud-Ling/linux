/*I HCD (Host Controller Driver) for USB.
 *
 *Ported this for HiDTV chip
 * 2013-3-19
 *
 * This file is licenced under the GPL.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/usb/ulpi.h>
#include <linux/pm_runtime.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include "ehci.h"


#if defined(CONFIG_SIGMA_SOC_UNION)
#define UTMI_CTL_REG	(0x104)		/* UTMI signal control reg */
#define UTMI_RST	(1<<4)		/* Assert UTMI reset signal */

#define SIGNAL_CTL_SEL	(0x10c)		/* Signal control select reg*/
#define SOFT_CTL_EN	(1<<7)		/* Software control enable*/
int ehci_trix_hub_control(
	struct usb_hcd	*hcd,
	u16		typeReq,
	u16		wValue,
	u16		wIndex,
	char		*buf,
	u16		wLength
) {
	struct ehci_hcd	*ehci = hcd_to_ehci (hcd);
	u32 __iomem *utmi_ctl =
			(u32 __iomem *)((unsigned long)hcd->regs + UTMI_CTL_REG);

	u32 __iomem *signal_sel = (u32 __iomem *)((unsigned long)hcd->regs + SIGNAL_CTL_SEL);
	int retval = 0;
	u32 temp;

	if (typeReq == SetPortFeature && wValue == USB_PORT_FEAT_RESET) {
		/* Preload signal control select content */
		temp = ehci_readl(ehci, signal_sel);

		/* Pre-set UTMI reset */
		ehci_writel(ehci, UTMI_RST, utmi_ctl);
	}

	retval = ehci_hub_control(hcd, typeReq, wValue, wIndex, buf, wLength);

	if (typeReq == SetPortFeature && wValue == USB_PORT_FEAT_RESET) {

		/* Switch signal select to software */
		ehci_writel(ehci, (temp | SOFT_CTL_EN), signal_sel);

		/* Switch back to MAC control UTMI signal */
		ehci_writel(ehci, (temp), signal_sel);
	}
	return retval;
}
#endif

static struct hc_driver __read_mostly ehci_trix_hc_driver;

static int ehci_trix_of_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	struct ehci_hcd *ehci = NULL;
	struct resource	*res = NULL;
	int ret = -ENODEV, irq = 0;
	void __iomem *regs = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	u32 tmp;

	/* USB not support case */
	if (usb_disabled())
		return -ENODEV;


	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "EHCI irq failed\n");
		return -ENODEV;
	}

	res =  platform_get_resource(pdev, IORESOURCE_MEM, 0);
	regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	ret = dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;

	ehci_init_driver(&ehci_trix_hc_driver, NULL);
#if defined(CONFIG_SIGMA_SOC_UNION)
	/*
	 * Due to UNION1 USB2.0 PHY(Innosilicon) have bug cause
	 * MAC can't send out SOF, after high-speed hand shake.
	 * So, we need over write 'hub_control' method here for
	 * apply work around patch.
	 */
	ehci_trix_hc_driver.hub_control = ehci_trix_hub_control;
#endif
	hcd = usb_create_hcd(&ehci_trix_hc_driver, dev, dev_name(dev));
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);
	hcd->regs = regs;
	hcd_to_ehci(hcd)->caps = regs;
	ehci = hcd_to_ehci(hcd);
	
	hcd->has_tt = 1;

	hcd->usb_phy = devm_usb_get_phy_by_phandle(&pdev->dev, "usb-phy", 0);
	if (IS_ERR(hcd->usb_phy)) {
		ret = PTR_ERR(hcd->usb_phy);
		if (ret == -EPROBE_DEFER)
			goto put_hcd;
		hcd->usb_phy = NULL;
	} else {
		ret = usb_phy_init(hcd->usb_phy);
		if (ret)
			goto put_hcd;
	}

	ret = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (ret) {
		dev_err(dev, "failed to add hcd with err %d\n", ret);
		goto put_hcd;
	}

	platform_set_drvdata(pdev, hcd);

#define MODE_SDIS (1<<4)
	if (of_get_property(np, "stream-disable-mode", NULL)) {
		/* Enable SDIS(Stream Diabale Mode) */
		tmp = ehci_readl(ehci, &ehci->regs->usbmode);
		tmp |= MODE_SDIS;
		ehci_writel(ehci, tmp, &ehci->regs->usbmode);
	}

put_hcd:
	usb_put_hcd(hcd);
	return ret;
}

static int ehci_trix_of_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	return 0;
}

#ifdef CONFIG_PM

static int ehci_trix_of_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	bool wakeup = device_may_wakeup(dev);

	ehci_prepare_ports_for_controller_suspend(hcd_to_ehci(hcd), wakeup);
	return 0;
}

static int ehci_trix_of_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	u32 __iomem *status_reg = &ehci->regs->port_status[0];
        unsigned int temp;

	if (usb_phy_init(hcd->usb_phy))
		dev_err(dev, "%s: phy resume failed\n", __func__);

	ehci_prepare_ports_for_controller_resume(hcd_to_ehci(hcd));

	/* Enable roothub port power */
	temp = ehci_readl(ehci, status_reg);
	ehci_writel(ehci, temp | PORT_POWER, status_reg);

	return 0;
}

#else /* !CONFIG_PM */
#define ehci_trix_of_suspend	NULL
#define ehci_trix_of_resume	NULL
#endif /* CONFIG_PM */
static const struct of_device_id ehci_trix_dt_match[] = {
	{ .compatible = "sigma,trix-ehci" },
	{/* sentinel */}
};

MODULE_DEVICE_TABLE(of, ehci_trix_dt_match);

SIMPLE_DEV_PM_OPS(ehci_trix_of_pm_ops, ehci_trix_of_suspend, ehci_trix_of_resume);

MODULE_ALIAS("trix:ehci-hcd");
static struct platform_driver ehci_hcd_trix_of_driver = {
	.probe = ehci_trix_of_probe,
	.remove = ehci_trix_of_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		.name = "trix-ehci",
		.of_match_table = of_match_ptr(ehci_trix_dt_match),
		.bus = &platform_bus_type,
		.pm = &ehci_trix_of_pm_ops,
	}
};
