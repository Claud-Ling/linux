// vim: foldmarker=<([{,}])> foldmethod=marker
/*I HCD (Host Controller Driver) for USB.
 *
 *Ported this for HiDTV chip
 * 2013-3-19
 *
 * This file is licenced under the GPL.
 *
 */

#include <linux/platform_device.h>
#include <asm/io.h>

extern int usb_disabled(void);

/*-------------------------------------------------------------------------*/

static void trihidtv_start_ehc(struct platform_device *dev)
{
        pr_debug(__FILE__ ": starting TriHidtv EHCI USB Controller\n");
	if(dev->id == 0)
        {
		pr_debug("USB EHCI%d %s____%d\n", dev->id,__func__, __LINE__);

#if defined(CONFIG_SIGMA_SOC_SX6) || defined (CONFIG_SIGMA_SOC_SX7)
		
		/* Set GPIO16 functionally as GPIO */
		MWriteRegByte((void *)0xf500ee23, 0x00, 0x70);
		MWriteRegHWord((void *)0xfb005540, 0x0001, 0x0003);
	
		/* Set GPIO16 output high */
		MWriteRegHWord((void *)0xfb005542, 0x0001, 0x0001);

#elif defined(CONFIG_SIGMA_SOC_UXLB)
		/* Set GPIO7 output high */
		MWriteRegHWord((void *)0x1b0055a0, 0x4000, 0x4000);
		MWriteRegHWord((void *)0x1b0055a2, 0x0080, 0x0080);
#endif	
	
	}
	if(dev->id == 1){
		pr_debug("USB EHCI%d %s____%d\n", dev->id,__func__, __LINE__);
#if defined(CONFIG_SIGMA_SOC_SX6) || defined (CONFIG_SIGMA_SOC_SX7)

		/* Set GPIO6 functionally as GPIO */
		MWriteRegByte((void *)0xf500ee1e, 0x00, 0x70);
		MWriteRegHWord((void *)0xfb005500, 0x0100, 0x0300);
		
		/* Set GPIO6 output high */
		MWriteRegHWord((void *)0xfb005502, 0x0040, 0x0040);

		/*usb2 clock control*/
		WriteRegByte((void*)0xf500e84d, 0x08);
#elif defined(CONFIG_SIGMA_SOC_UXLB)
		/* Set GPIO6 output high */
		MWriteRegHWord((void *)0x1b0055a0, 0x1000, 0x1000);
		MWriteRegHWord((void *)0x1b0055a2, 0x0040, 0x0040);
#endif	
	}

}
static void trihidtv_stop_ehc(struct platform_device *dev)
{
        pr_debug(__FILE__ ": stopping TriHiDTV EHCI USB Controller\n");
}
/*-------------------------------------------------------------------------*/

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * usb_ehci_trihidtv_probe - initialize TriHiDTV-based HCDs
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */
int usb_ehci_trihidtv_probe(const struct hc_driver *driver,
			  struct usb_hcd **hcd_out, struct platform_device *dev)
{
	int retval;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;

	trihidtv_start_ehc(dev);

	if (dev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
	}
	hcd = usb_create_hcd(driver, &dev->dev, "trihidtv");
	if (!hcd)
		return -ENOMEM;
	hcd->rsrc_start = dev->resource[0].start;
	hcd->rsrc_len = dev->resource[0].end - dev->resource[0].start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		pr_debug("request_mem_region failed");
		retval = -EBUSY;
		goto err1;
	}

//	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	hcd->regs = (void __iomem *)hcd->rsrc_start; //when kenrel bootup i/o space from 0xf5000000-0xfd000000 is already remaped
	if (!hcd->regs) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err2;
	}
	
	hcd->has_tt = 1;

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(ehci, readl(&ehci->caps->hc_capbase));
	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);


	/* ehci_hcd_init(hcd_to_ehci(hcd)); */

	retval =
	    usb_add_hcd(hcd, dev->resource[1].start, IRQF_DISABLED | IRQF_SHARED);
	platform_set_drvdata(dev, hcd);
	if (retval == 0)
		return retval;

	trihidtv_stop_ehc(dev);
	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_ehci_hcd_trihidtv_remove - shutdown processing for trihidtv-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_ehci_hcd_trihidtv_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
void usb_ehci_trihidtv_remove(struct usb_hcd *hcd, struct platform_device *dev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	trihidtv_stop_ehc(dev);
}

/*-------------------------------------------------------------------------*/

static const struct hc_driver ehci_trihidtv_hc_driver = {
	.description = hcd_name,
	.product_desc = "SigmaHiDTV EHCI",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_MEMORY | HCD_USB2,
	.reset			= ehci_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.get_frame_number	= ehci_get_frame,
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,
};

/*-------------------------------------------------------------------------*/

static int ehci_hcd_trihidtv_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	int ret;

	pr_debug("In ehci_hcd_trihidtv_drv_probe\n");

	if (usb_disabled())
		return -ENODEV;

	ret = usb_ehci_trihidtv_probe(&ehci_trihidtv_hc_driver, &hcd, pdev);
	return ret;
}

static int ehci_hcd_trihidtv_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_ehci_trihidtv_remove(hcd, pdev);
	return 0;
}

 /*TBD*/
/*static int ehci_hcd_trihidtv_drv_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	return 0;
}
static int ehci_hcd_trihidtv_drv_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	return 0;
}
*/
#ifdef CONFIG_PM

static int ehci_platform_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	bool wakeup = device_may_wakeup(dev);

	ehci_prepare_ports_for_controller_suspend(hcd_to_ehci(hcd), wakeup);
	return 0;
}

static int ehci_platform_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	u32 __iomem *status_reg = &ehci->regs->port_status[0];
        unsigned int temp;

	trihidtv_start_ehc(pdev);


	ehci_prepare_ports_for_controller_resume(hcd_to_ehci(hcd));

	/* Enable roothub port power */
	temp = ehci_readl(ehci, status_reg);
	ehci_writel(ehci, temp | PORT_POWER, status_reg);

	return 0;
}

#else /* !CONFIG_PM */
#define ehci_platform_suspend	NULL
#define ehci_platform_resume	NULL
#endif /* CONFIG_PM */

static const struct dev_pm_ops ehci_platform_pm_ops = {
	.suspend	= ehci_platform_suspend,
	.resume		= ehci_platform_resume,
};
MODULE_ALIAS("trihidtv-ehci");
static struct platform_driver ehci_hcd_trihidtv_driver = {
	.probe = ehci_hcd_trihidtv_drv_probe,
	.remove = ehci_hcd_trihidtv_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	/*.suspend      = ehci_hcd_trihidtv_drv_suspend, */
	/*.resume       = ehci_hcd_trihidtv_drv_resume, */
	.driver = {
		.name = "sigma-ehci",
		.bus = &platform_bus_type,
		.pm = &ehci_platform_pm_ops,
	}
};
