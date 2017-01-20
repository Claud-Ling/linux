#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/usb/of.h>
#include <linux/usb/phy.h>


static u32 trix_phy_readl(struct usb_phy *x, u32 reg)
{
	return readl((const volatile void __iomem *)
			((unsigned long)x->io_priv + reg));
}

static void trix_phy_writel(struct usb_phy *x, u32 val, u32 reg)
{
	writel(val,(volatile void __iomem *)
			((unsigned long)x->io_priv + reg));

	return;
}

static void trix_phy_mwritel(struct usb_phy *x, u32 val, u32 reg, u32 msk)
{
	u32 tmp = 0;
	tmp = trix_phy_readl(x, reg);
	tmp &= (~msk);

	val |= tmp;

	trix_phy_writel(x, val, reg);
	return;
}

#define PHY_TST_CR	(0x204)
#define PHY_TST_RR	(0x208)

static int trix_phy_handshake(struct usb_phy *x, u32 reg, u32 val, u32 mask, int timeout)
{
	do {
		if ((trix_phy_readl(x, reg) & mask) == val) {
			return 0;
		}
		timeout--;
		udelay(1);
	}while(timeout > 0);

	return -ETIMEDOUT;
}

static int candence_usb2_phy_init(struct usb_phy *phy)
{
	int ret = 0;

	trix_phy_mwritel(phy, 0x00000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x40000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x00000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x40000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x00000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x20000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x60000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x20000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x60000, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x20000, PHY_TST_CR, 0xfffff);

	ret = trix_phy_handshake(phy, PHY_TST_CR, 0x20000, 0xfffff, 1000);
	if (ret)
		printk(KERN_INFO"Wait PHY_TST_CR transition to 0x2000, timeout\n");

	ret = trix_phy_handshake(phy, PHY_TST_RR, 0x00, 0xff, 1000);
	if (ret)
		printk(KERN_INFO"Wait PHY_TST_RR transition to 0x00, timeout\n");

	trix_phy_mwritel(phy, 0x34102, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x74102, PHY_TST_CR, 0xfffff);
	ret = trix_phy_handshake(phy, PHY_TST_RR, 0x02, 0xff, 1000);
	if (ret)
		printk(KERN_INFO"Wait PHY_TST_RR transition to 0x02, timeout\n");

	trix_phy_mwritel(phy, 0x34100, PHY_TST_CR, 0xfffff);
	trix_phy_mwritel(phy, 0x74100, PHY_TST_CR, 0xfffff);
	ret = trix_phy_handshake(phy, PHY_TST_RR, 0x00, 0xff, 1000);
	if (ret)
		printk(KERN_INFO"Wait PHY_TST_RR transition to 0x00, timeout\n");

	return ret;
	
}

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

static int candence_usb3_phy_init(struct usb_phy *phy)
{
	/* Disable USB3 PHY Scramble */
	trix_phy_mwritel(phy, 0x00202087, TRIX_USB3_TEST_DBG_REG, 0x00ffffff);
	trix_phy_mwritel(phy, 0x00302087, TRIX_USB3_TEST_DBG_REG, 0x00ffffff);
	trix_phy_mwritel(phy, 0x00202087, TRIX_USB3_TEST_DBG_REG, 0x00ffffff);
	return 0;
}

static const struct of_device_id trix_usb_phy_id_table[] = {
	{ .compatible = "sigma,candence-usb2-phy", .data = candence_usb2_phy_init},
	{ .compatible = "sigma,candence-usb3-phy", .data = candence_usb3_phy_init},
	{ },
};
MODULE_DEVICE_TABLE(of, trix_usb_phy_id_table);

static int trix_usb_phy_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct resource *res;
	struct usb_phy *phy = NULL;
	int err;

	phy = devm_kzalloc(&pdev->dev, sizeof(struct usb_phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	match = of_match_device(trix_usb_phy_id_table, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "Error: No device match found\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get I/O memory\n");
		return  -ENXIO;
	}

	phy->io_priv = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!phy->io_priv) {
		dev_err(&pdev->dev, "Failed to remap I/O memory\n");
		return -ENOMEM;
	}


	platform_set_drvdata(pdev, phy);
	phy->dev = &pdev->dev;

	dev_dbg(&pdev->dev,"Probe usb-phy@0x%08x %s\n", (unsigned int)res->start, match->compatible);
	phy->init = match->data;

	dev_dbg(&pdev->dev,"Add usb-phy@0x%p\n", (void *)phy);
	err = usb_add_phy_dev(phy);
	if (err < 0) {
		dev_err(&pdev->dev, "Add usb-phy@0x%08x failed\n", (unsigned int)res->start);
		return err;
	}

	return 0;
}

static int trix_usb_phy_remove(struct platform_device *pdev)
{
	struct usb_phy *phy = platform_get_drvdata(pdev);
	usb_remove_phy(phy);
	return 0;
}

static struct platform_driver trix_usb_phy_driver = {
	.probe		= trix_usb_phy_probe,
	.remove		= trix_usb_phy_remove,
	.driver		= {
		.name	= "trix-usb-phy",
		.of_match_table = of_match_ptr(trix_usb_phy_id_table),
	},
};
module_platform_driver(trix_usb_phy_driver);

MODULE_DESCRIPTION("Sigma DTV USB PHY driver");
MODULE_LICENSE("GPL v2");
