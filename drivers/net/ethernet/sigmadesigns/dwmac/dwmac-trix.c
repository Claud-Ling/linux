/*
 * dwmac-sigma.c - Sigma DTV SoCs DWMAC Specific Glue layer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/stmmac.h>
#include <linux/phy.h>        
#include <linux/mfd/syscon.h> 
#include <linux/module.h>     
#include <linux/regmap.h>     
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_device.h>  
#include <linux/of_net.h>
#include <linux/io.h>
#include <asm/io.h>

#include "dwmac.h"

#ifdef CONFIG_MICREL_PHY
#include <linux/micrel_phy.h>
#endif

#define GMAC_PHY_IF_CTRL		0x00001f00
#define GMAC_PHY_IF_SEL_MASK		0x00000007
#define GMAC_PHY_IF_SEL_RGMII		(1UL)
#define GMAC_PHY_IF_SEL_RMII		(4UL)

#define GMAC_MON_CTR_REG		(0x00001f04)
#define GMAC_MON_CTRL_INT_MASK		(0x00000040)
#define GMAC_MON_CTRL_INT_ENABLE	(0x40)

#define IS_RMII(iface)     (iface == PHY_INTERFACE_MODE_RMII)
#define IS_RGMII(iface)     (iface == PHY_INTERFACE_MODE_RGMII)

#define mask_writel(v, m, a) do{   \
		typeof(v) tmp;                 \
		tmp = readl(a);                \
		tmp &= ~(m);                   \
		tmp |= ((v) & (m));            \
		writel(tmp, a);                \
	}while(0)

struct sigma_dwmac {
	int interface;		/* phy interface */
	void __iomem *ctrl_reg;		/* GMAC glue-logic control register */
	u32 speed;

	struct device *device;
};

static unsigned char mac_addr[6] = {0};
static int phy_mode = -1;

#ifndef MODULE
static int __init setup_mac_addr(char *str)
{
	unsigned int mac[6];
	int i;

	if (!str || !*str)
		return -EINVAL;

	sscanf (str, "%x:%x:%x:%x:%x:%x",
		&mac[0], &mac[1], &mac[2],
		&mac[3], &mac[4], &mac[5]);
	for (i = 0; i < 6; i++) {
		mac_addr[i] = mac[i];
	}

	return 0;
}

__setup("ethaddr=",setup_mac_addr);

static int __init setup_phy_mode(char *str)
{
	if (!str || !*str)
		return -EINVAL;
	if (!strncmp(str, "rmii", 4)) {
		phy_mode = PHY_INTERFACE_MODE_RMII;
	}else if (!strncmp(str, "rgmii", 5)) {
		phy_mode = PHY_INTERFACE_MODE_RGMII;
	}

	return 0;
}

__setup("phymode=",setup_phy_mode);
#endif

#ifdef CONFIG_OF
static int sigma_dwmac_probe_config_dt(struct platform_device *pdev,
			        struct plat_dwmacenet_data *plat,
			        const char **mac)
{
	struct device_node *np = pdev->dev.of_node;

	*mac = of_get_mac_address(np);
	plat->interface = of_get_phy_mode(np);

	return 0;
}
#else
static int sigma_dwmac_probe_config_dt(struct platform_device *pdev,
			        struct plat_dwmacenet_data *plat,
			        const char **mac)
{
	return -ENOSYS;
}
#endif /* CONFIG_OF */

int sigma_dwmac_get_platform_resources(struct platform_device *pdev,
				 struct dwmac_resources *dwmac_res)
{
	struct resource *res;

	memset(dwmac_res, 0, sizeof(struct dwmac_resources));

	dwmac_res->irq = platform_get_irq_byname(pdev, "mac_irq");
	if(dwmac_res->irq < 0){
		if(dwmac_res->irq != -EPROBE_DEFER){
			dev_err(&pdev->dev,
				"MAC IRQ configuration information not found\n");
		}
		return dwmac_res->irq;
	}

	/*
 	 * In case the wake up interrupt is not passed from platform,
 	 * the driver will use the the mac irq
	 */
	dwmac_res->wol_irq = platform_get_irq_byname(pdev, "wol_irq");
	if(dwmac_res->wol_irq < 0){
		if(dwmac_res->irq != -EPROBE_DEFER){
			dev_err(&pdev->dev,
				"MAC IRQ configuration information not found\n");
		}
		dwmac_res->wol_irq = dwmac_res->irq;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	dwmac_res->addr = devm_ioremap_resource(&pdev->dev, res);

	return IS_ERR_OR_NULL(dwmac_res->addr);
}

static void sigma_dwmac_fix_mac_speed(void *bsp_priv, unsigned int speed)
{
	struct sigma_dwmac *dwmac = (struct sigma_dwmac *)bsp_priv;
	struct device *device = dwmac->device;
	struct net_device *ndev = dev_get_drvdata(device);
	struct dwmac_priv *priv = netdev_priv(ndev);
#ifdef CONFIG_MICREL_PHY
	u32 phy_id = priv->phydev->phy_id;

	/*
 	 * a way to decide which PHY interface we are using:
 	 * 1. identify PHY ID
 	 * 2. choice the PHY interface according to PHY ID
 	 */
	if ( (phy_id & MICREL_PHY_ID_MASK) == PHY_ID_KSZ9031) {
		priv->phydev->interface	= PHY_INTERFACE_MODE_RGMII;
		dwmac->interface	= PHY_INTERFACE_MODE_RGMII;
	} else {
		/* default: RMII mode */
		dwmac->interface = priv->phydev->interface = PHY_INTERFACE_MODE_RMII;
	}
#else
	/* default: RMII mode */
	dwmac->interface = priv->phydev->interface = PHY_INTERFACE_MODE_RMII;

#endif
	if(IS_RGMII(priv->phydev->interface)) {
		/* RGMII - 125Mhz clock */
		mask_writel(GMAC_PHY_IF_SEL_RGMII, GMAC_PHY_IF_SEL_MASK, dwmac->ctrl_reg + GMAC_PHY_IF_CTRL);
	}
	else {
		/* RMII - 25Mhz clock */
		mask_writel(GMAC_PHY_IF_SEL_RMII, GMAC_PHY_IF_SEL_MASK, dwmac->ctrl_reg + GMAC_PHY_IF_CTRL);
	}

	return;
}

static int sigma_dwmac_init(struct platform_device *pdev, void *priv)
{
	struct sigma_dwmac *dwmac = (struct sigma_dwmac *)priv;
	int iface = dwmac->interface;

#if defined(CONFIG_SIGMA_SOC_SX7)
	WriteRegByte((volatile void *)0xf500ea2c, 0x7f); //GBE_TXEN
	WriteRegByte((volatile void *)0xf500ea2d, 0x7f); //GBE_TXC
	WriteRegByte((volatile void *)0xf500ea2e, 0x7f); //GBE_TXD0
	WriteRegByte((volatile void *)0xf500ea2f, 0x7f); //GBE_TXD1
	WriteRegByte((volatile void *)0xf500ea30, 0x7f); //GBE_TXD2
	WriteRegByte((volatile void *)0xf500ea31, 0x7f); //GBE_TXD3
	WriteRegByte((volatile void *)0xf500ea38, 0x7f); //GBE_MDC
#elif defined(CONFIG_SIGMA_SOC_SX8)
	WriteRegHWord((volatile void*)0xf500e030, 0x103f);	//[0]GBE_TXEN_OE
						//[1]GBE_TXC_OE
						//[2]GBE_TXD0_OE
						//[3]GBE_TXD1_OE
						//[4]GBE_TXD2_OE
						//[5]GBE_TXD3_OE
						//[12]GBE_MDC_OE
#endif

	/* enable interrupt logic to CPU */
	mask_writel(GMAC_MON_CTRL_INT_ENABLE, GMAC_MON_CTRL_INT_MASK, dwmac->ctrl_reg + GMAC_MON_CTR_REG);

	if(IS_RGMII(iface)) {
		/* RGMII - 125Mhz clock */
		mask_writel(GMAC_PHY_IF_SEL_RGMII, GMAC_PHY_IF_SEL_MASK, dwmac->ctrl_reg + GMAC_PHY_IF_CTRL);
	}
	else {
		/* RMII - 25Mhz clock */
		mask_writel(GMAC_PHY_IF_SEL_RMII, GMAC_PHY_IF_SEL_MASK, dwmac->ctrl_reg + GMAC_PHY_IF_CTRL);
	}

	return 0;
}

static void sigma_dwmac_exit(struct platform_device *dev, void *priv)
{
	//platform level deinit
}

static int sigma_dwmac_parse_data(struct sigma_dwmac *dwmac,
				  struct platform_device *pdev)
{
	struct resource *res;
	struct plat_dwmacenet_data *plat_dat;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dwmac->ctrl_reg = (void __iomem *)SIGMA_IO_ADDRESS(res->start);
	plat_dat = pdev->dev.platform_data;
	dwmac->interface = plat_dat->interface;	/* RMII or RGMII */

	return 0;
}

static int sigma_dwmac_probe(struct platform_device *pdev)
{
	struct plat_dwmacenet_data *plat_dat;
	struct dwmac_resources dwmac_res;
	struct sigma_dwmac *dwmac;
	int ret;

	ret = sigma_dwmac_get_platform_resources(pdev, &dwmac_res);
	if (ret)
		return ret;

	if(pdev->dev.of_node) {
		plat_dat = devm_kzalloc(&pdev->dev,
					 sizeof(struct plat_dwmacenet_data),
					 GFP_KERNEL);
		if(!plat_dat){
			pr_err("%s: ERROR:no memory",__func__);
		}

		ret = sigma_dwmac_probe_config_dt(pdev, plat_dat, &dwmac_res.mac);
		if(ret) {
			pr_err("%s: main dt probe failed",__func__);
		}
	} else {
		plat_dat		= pdev->dev.platform_data;

		/* Initialize dma_mask and coherent_dma_mask to 32-bits */
		ret = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (ret)
			return ret;
		if (!pdev->dev.dma_mask)
			pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
		else
			dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
	}

	dwmac = devm_kzalloc(&pdev->dev, sizeof(struct sigma_dwmac), GFP_KERNEL);
	if (!dwmac)
		 return -ENOMEM;

	dwmac->device = &pdev->dev;

	if(pdev->dev.of_node) {
		dwmac->interface = plat_dat->interface;
		dwmac->ctrl_reg = dwmac_res.addr;
	}else{
		ret = sigma_dwmac_parse_data(dwmac, pdev);
		if (ret) {
			dev_err(&pdev->dev, "Unable to parse OF data\n");
			return ret;
		}
	}

	plat_dat->mdio_bus_data = devm_kzalloc(&pdev->dev,
					       sizeof(struct dwmac_mdio_bus_data),
					       GFP_KERNEL);
	/*
 	 * Override with kernel runtime parameters
 	 * - phy mode
 	 * - mac address
 	 */
	if(IS_RMII(phy_mode) ||
	   IS_RGMII(phy_mode) ) {
		dwmac->interface = phy_mode;
	}

	if (is_valid_ether_addr(mac_addr)) {
		dwmac_res.mac = mac_addr;
	}

	plat_dat->has_gmac	= true;
	plat_dat->phy_addr	= -1;
	plat_dat->bsp_priv	= dwmac;
	plat_dat->interface	= dwmac->interface;
	plat_dat->init		= sigma_dwmac_init;
	plat_dat->exit		= sigma_dwmac_exit;
	plat_dat->fix_mac_speed	= sigma_dwmac_fix_mac_speed;
	plat_dat->multicast_filter_bins = HASH_TABLE_SIZE;

	ret = sigma_dwmac_init(pdev, plat_dat->bsp_priv);

	return dwmac_dvr_probe(&pdev->dev, plat_dat, &dwmac_res);
}

static int sigma_dwmac_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct dwmac_priv *priv = netdev_priv(ndev);
	int ret;

	if (priv->plat->exit)
		priv->plat->exit(pdev, priv->plat->bsp_priv);

	ret = dwmac_dvr_remove(ndev);

	return ret;
}

#if CONFIG_PM
/**
 * sigma_dwmac_suspend
 * @dev: device pointer
 * Description: this function is invoked when suspend the driver and it direcly
 * call the main suspend function and then, call platform level exit func.
 */
static int sigma_dwmac_suspend(struct device *dev)
{
	int ret;
	struct net_device *ndev = dev_get_drvdata(dev);
	struct dwmac_priv *priv = netdev_priv(ndev);
	struct platform_device *pdev = to_platform_device(dev);

	ret = dwmac_suspend(ndev);

	if (priv->plat->exit)
		priv->plat->exit(pdev, priv->plat->bsp_priv);

	return ret;
}

/**
 * sigma_dwmac_resume
 * @dev: device pointer
 * Description: this function is invoked when resume the driver before calling
 * the main resume function 
 */
static int sigma_dwmac_resume(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct dwmac_priv *priv = netdev_priv(ndev);
	struct platform_device *pdev = to_platform_device(dev);

	if (priv->plat->init)
		priv->plat->init(pdev, priv->plat->bsp_priv);

	return dwmac_resume(ndev);
}
#endif /* CONFIG_PM */

SIMPLE_DEV_PM_OPS(sigma_dwmac_pm_ops, sigma_dwmac_suspend,
				      sigma_dwmac_resume);

static const struct of_device_id sigma_dwmac_match[] = {
	{ .compatible = "sigma,trix-dwmac"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sigma_dwmac_match);

static struct platform_driver sigma_dwmac_driver = {
	.probe  = sigma_dwmac_probe,
	.remove = sigma_dwmac_remove,
	.driver = {
		.name		= "trix-dwmac",
		.owner		= THIS_MODULE,
		.pm		= &sigma_dwmac_pm_ops,
		.of_match_table	= of_match_ptr(sigma_dwmac_match),
	},
};
module_platform_driver(sigma_dwmac_driver);

MODULE_AUTHOR("Claud Ling <Claud_Ling@sigmadesigns.com>");
MODULE_DESCRIPTION("Sigmadesigns DWMAC Specific Glue layer");
MODULE_LICENSE("GPL");
