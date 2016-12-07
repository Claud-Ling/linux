/*
 * drivers/mmc/host/sdhci-of-trix.c - Sigma DTV SDHCI Platform driver
 *
 */
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/mmc/mmc.h>
#include <linux/slab.h>

#include "sdhci-pltfm.h"

struct sdhci_trix_host {
	struct platform_device *pdev;
	struct mmc_host *mmc;
	struct sdhci_pltfm_data sdhci_trix_pdata;

	u16 mmc_hs_timing;
	u16 sd_hs_timing;
	u16 sd_sdr50_timing;
	u16 sd_sdr104_timing;
	u16 sd_ddr50_timing;
	u16 mmc_ddr52_timing;
	u16 mmc_hs200_timing;
	u16 mmc_hs400_timing;

};

#define TAP_REG		(0x400)

/* Output delay EN*/
#define OP_DLY_EN	(1<<12)

/* Input delay EN */
#define IP_DLY_EN	(1<<5)

#define TAP_DLY_TIMING_MASK (0x1fff)

#define MK_TAP_DELAY(indelay, outdelay)	 ({			\
	uint16_t val = 0x0;					\
	val = ((((outdelay) & 0xf) << 8) | 			\
		((indelay) & 0x1f) | OP_DLY_EN |		\
		IP_DLY_EN);					\
	val;							\
})

/**
 * sdhci_trix_clocks_init:
 * @np: dt device node
 * @host: sdhci host
 * Description: this function is to (Re)configure the controller
 */
static void sdhci_trix_clocks_init(struct device_node *np, struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct mmc_host *mhost = host->mmc;

	/*
	 * Set clock frequency, default to 200MHz if max-frequency
	 * is not provided
	 * FIXME when necessary
	 */
	switch (mhost->f_max) {
	case 200000000:
		clk_set_rate(pltfm_host->clk, mhost->f_max);
		break;
	case 100000000:
		clk_set_rate(pltfm_host->clk, mhost->f_max);
		break;
	default:
		clk_set_rate(pltfm_host->clk, 200000000);
		break;
	}
}

/*
 *  parse the device-tree for trix sdhci quirks
 */
static void sdhci_trix_get_of_property(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_trix_host *trix_host = pltfm_host->priv;
	u32 timing[2];

	if (of_get_property(np, "no-dma", NULL)) {
		host->quirks |= SDHCI_QUIRK_BROKEN_DMA;
		host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;
	} else
		host->quirks |= SDHCI_QUIRK_FORCE_DMA;

	/* For SD card, currently, no regulator to adjust IO voltage */
	if (of_get_property(np, "no-1-8-v", NULL))
		host->quirks2 |= SDHCI_QUIRK2_NO_1_8_V;

	if (of_get_property(np, "no-hs200", NULL))
		host->quirks2 |= SDHCI_QUIRK2_BROKEN_HS200;

	if(!of_property_read_u32_array(np,"sigma,mmc_hs_timing", timing, 2))
		trix_host->mmc_hs_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,sd_hs_timing", timing, 2))
		trix_host->sd_hs_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,sd_sdr50_timing", timing, 2))
		trix_host->sd_sdr50_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,sd_sdr104_timing", timing, 2))
		trix_host->sd_sdr104_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,sd_ddr50_timing", timing, 2))
		trix_host->sd_ddr50_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,mmc_ddr52_timing", timing, 2))
		trix_host->mmc_ddr52_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,mmc_hs200_timing", timing, 2))
		trix_host->mmc_hs200_timing = MK_TAP_DELAY(timing[0], timing[1]);

	if(!of_property_read_u32_array(np,"sigma,mmc_hs400_timing", timing, 2))
		trix_host->mmc_hs400_timing = MK_TAP_DELAY(timing[0], timing[1]);
}

static void sdhci_trix_set_clk_delay(struct sdhci_host *host, u16 timing)
{
	u32 clksel;

	clksel = sdhci_readl(host, TAP_REG);
	clksel &= ~TAP_DLY_TIMING_MASK;
	clksel |= timing & TAP_DLY_TIMING_MASK;
	sdhci_writel(host, clksel, TAP_REG);
}

static void sdhci_trix_set_clock(struct sdhci_host *host,
					 unsigned int clock)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_trix_host *trix_host = pltfm_host->priv;
	struct mmc_ios ios = host->mmc->ios;

	switch(ios.timing) {
		case MMC_TIMING_SD_HS:
			sdhci_trix_set_clk_delay(host, trix_host->sd_hs_timing);
			break;
		case MMC_TIMING_MMC_HS:
			sdhci_trix_set_clk_delay(host, trix_host->mmc_hs_timing);
			break;
		case MMC_TIMING_UHS_SDR50:
			sdhci_trix_set_clk_delay(host, trix_host->sd_sdr50_timing);
			break;
		case MMC_TIMING_UHS_DDR50:	
			sdhci_trix_set_clk_delay(host, trix_host->sd_ddr50_timing);
			break;
		case MMC_TIMING_MMC_DDR52:
			sdhci_trix_set_clk_delay(host, trix_host->mmc_ddr52_timing);
			break;
		case MMC_TIMING_MMC_HS200:
			sdhci_trix_set_clk_delay(host, trix_host->mmc_hs200_timing);
			break;
		default:
			pr_debug("Sdhci clock=%d, no tap delay\n", clock);
	}

	sdhci_set_clock(host, clock);
}

#if CONFIG_PM_SLEEP
/**
 * sdhci_trix_suspend - Suspend method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Put the device in a low power state.
 */
static int sdhci_trix_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	int ret = sdhci_suspend_host(host);

	clk_disable_unprepare(pltfm_host->clk);

	return ret;
}

/**
 * sdhci_trix_resume - Resume method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Resume operation after suspend
 */
static int sdhci_trix_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct device_node *np = dev->of_node;

	clk_prepare_enable(pltfm_host->clk);

	sdhci_trix_clocks_init(np, host);

	return sdhci_resume_host(host);
}
#endif
static SIMPLE_DEV_PM_OPS(sdhci_trix_dev_pm_ops, sdhci_trix_suspend,sdhci_trix_resume);

static struct sdhci_ops sdhci_trix_ops = {
	.reset = sdhci_reset,
	.set_clock = sdhci_trix_set_clock,
	.set_bus_width = sdhci_set_bus_width,
	.set_uhs_signaling = sdhci_set_uhs_signaling,
};

static int sdhci_trix_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_trix_host *trix_host;
	struct clk *clk;
	int ret;

	trix_host = devm_kzalloc(&pdev->dev, sizeof(*trix_host), GFP_KERNEL);
	if (!trix_host)
		return -ENOMEM;

	clk = devm_clk_get(&pdev->dev, "sdioclk");
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "Peripheral clk not found\n");
		return PTR_ERR(clk);
	}

	trix_host->sdhci_trix_pdata.ops = &sdhci_trix_ops;
	host = sdhci_pltfm_init(pdev, &trix_host->sdhci_trix_pdata, 0);
	if (IS_ERR(host))
		return PTR_ERR(host);

	clk_prepare_enable(clk);

	pltfm_host = sdhci_priv(host);
	pltfm_host->priv = trix_host;
	pltfm_host->clk = clk;

	trix_host->pdev = pdev;

	/* call to generic mmc_of_parse to support additional capabilities */
	ret = mmc_of_parse(host->mmc);
	if (ret)
		goto pltfm_free;

	sdhci_trix_get_of_property(pdev);

	ret = sdhci_add_host(host);
	if (ret)
		goto pltfm_free;

	sdhci_trix_clocks_init(np, host);

	return 0;

pltfm_free:
	sdhci_pltfm_free(pdev);
	return ret;
}

static int sdhci_trix_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	int dead = (sdhci_readl(host, SDHCI_INT_STATUS) == 0xffffffff);

	sdhci_remove_host(host, dead);
	sdhci_pltfm_free(pdev);

	return 0;
}

static const struct of_device_id sdhci_trix_dt_match[] = {
	{ .compatible = "sigma,trix-sdhci" },
	{/* sentinel */}
};

MODULE_DEVICE_TABLE(of, sdhci_trix_dt_match);

static struct platform_driver sdhci_trix_driver = {
	.probe = sdhci_trix_probe,
	.remove = sdhci_trix_remove,
	.driver = {
		.name = "sdhci_trix",
		.of_match_table = sdhci_trix_dt_match,
		.pm = &sdhci_trix_dev_pm_ops,
	},
};

module_platform_driver(sdhci_trix_driver);

MODULE_DESCRIPTION("SigmaDesigns Secure Digital Host Controller Interface driver");
MODULE_LICENSE("GPL v2");
