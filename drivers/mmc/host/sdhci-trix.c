#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <asm/io.h>

#include "sdhci.h"

struct sdhci_trihidtv_chip {
	struct platform_device *pdev;
	struct sdhci_host *host;
	unsigned int quirks;
};

void sx6_sdhci_set_clock(struct sdhci_host *host, unsigned int clock);

static struct sdhci_ops sdhci_platform_ops = {
	.set_clock = sx6_sdhci_set_clock,
	.set_bus_width = sdhci_set_bus_width,
	.reset = sdhci_reset,
	.set_uhs_signaling = sdhci_set_uhs_signaling,
};

#define TAP_REG		(0x400)

/* Output delay EN*/
#define OP_DLY_EN	(1<<12)

/* Input delay EN */
#define IP_DLY_EN	(1<<5)

#define MK_TAP_DELAY(indelay, outdelay)	 ({			\
	uint16_t val = 0x0;					\
	val = ((((outdelay) & 0xf) << 8) | 			\
		((indelay) & 0x1f) | OP_DLY_EN |		\
		IP_DLY_EN);					\
	val;							\
})

void sx6_sdhci_set_clock(struct sdhci_host *host, unsigned int clock)
{
	unsigned char timing = host->mmc->ios.timing;

	switch(timing) {
		case MMC_TIMING_SD_HS:
			/* Set value 0x1d20 */
			MWriteRegWord((void*)(host->ioaddr+TAP_REG), MK_TAP_DELAY(0x0, 0xd), 0x1fff);
			break;
		case MMC_TIMING_MMC_HS:
		case MMC_TIMING_UHS_SDR50:
#if defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX6)
			/* Set value 0x1c20*/
			MWriteRegWord((void*)(host->ioaddr+TAP_REG), MK_TAP_DELAY(0x0, 0xc), 0x1fff);
#elif defined(CONFIG_SIGMA_SOC_SX8)
			/* Set value 0x1620*/
			MWriteRegWord((void*)(host->ioaddr+TAP_REG), MK_TAP_DELAY(0x0, 0x6), 0x1fff);
#endif
			break;
		case MMC_TIMING_UHS_DDR50:	
		case MMC_TIMING_MMC_DDR52:
#if defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX6)
			/* Set value 0x1620*/
			MWriteRegWord((void*)(host->ioaddr+TAP_REG), MK_TAP_DELAY(0x0, 0x6), 0x1fff);
#elif defined(CONFIG_SIGMA_SOC_SX8)
			/* Set value 0x1520*/
			MWriteRegWord((void*)(host->ioaddr+TAP_REG), MK_TAP_DELAY(0x0, 0x5), 0x1fff);
#endif
			break;
		case MMC_TIMING_MMC_HS200:
			/* Set value 0x1424 */
			MWriteRegWord((void*)(host->ioaddr+TAP_REG), MK_TAP_DELAY(0x4, 0x4), 0x1fff);
			break;
		default:
			pr_debug("Sdhci clock=%d, no tap delay\n", clock);
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
	sdhci_set_clock(host, clock);
#endif
}

static void sdhci_sx6_pinshare_init(int dev_id)
{
	
	if (dev_id == 1) {
#ifdef CONFIG_SIGMA_SOC_SX6
		/*
 		* bit[6:4] = 001	SDIO data 1
 		* bit[2:0] = 001	SDIO data 0
 		*/
		MWriteRegByte((void*)0x1500ee30, 0x11, 0x77);

		/*
 		* bit[6:4] = 001	SDIO data 3
 		* bit[2:0] = 001	SDIO data 2
 		*/
		MWriteRegByte((void*)0x1500ee31, 0x11, 0x77);

		/*
 		* bit[6:4] = 011	SDIO CD
 		*/
		MWriteRegByte((void*)0x1500ee20, 0x30, 0x70);

		/*
 		* bit[2:0] = 011	SDIO WP
 		*/
		MWriteRegByte((void*)0x1500ee21, 0x03, 0x07);

		/*
 		* bit[2:0] = 010	SDIO CLK
 		* bit[6:4] = 010	SDIO CMD
 		*/
		MWriteRegByte((void*)0x1500ee3b, 0x22, 0x77);
	
		/*
 		* bit[1] = 1	Select IO for function sdin of SDIO
 		*/
		MWriteRegByte((void*)0x1500e868, 0x02, 0x02);

		/*
 		* bit[4] = 1	Select IO for function sdin of SDIO
 		*/
		MWriteRegByte((void*)0x1500ee41, 0x10, 0x10);

#elif defined (CONFIG_SIGMA_SOC_SX7)
		WriteRegByte((void*)0x1500ee23, 0x22);
		WriteRegByte((void*)0x1500ee34, 0x11);
		WriteRegByte((void*)0x1500ee35, 0x11);
		WriteRegByte((void*)0x1500ee3f, 0x22);
#elif defined (CONFIG_SIGMA_SOC_UXLB)
		/*TODO put UMAC setting to a better place.*/
		MWriteRegHWord((void *)0x1b000072, 0x8000, 0x8000); //set bit 15, what does this mean?

		//UMAC setting
		MWriteRegWord((void *)0x1502207c, 0x0000ffff, 0x00000021);

		//UMAC setting
		MWriteRegWord((void *)0x1502206c, 0xffff0000, 0x00210000);
		
		//SDIO pin share
		
		/* bit[14:15] 2b'11 SDIO_CLK */
		/* bit[6:7] 2b'11 SDIO_CD */
		/* bit[4:5] 2b'11 SDIO_CMD & SDIO_D0 */
		MWriteRegHWord((void *)0x1b000094, 0xc0f0, 0xc0f0); 
	
		/* bit[2:1] 2b'11 SDIO_D1 */
		MWriteRegHWord((void *)0x1b000024, 0x0006, 0x0006); 

		/* bit[15:14] 2b'11 SDIO_D2 */
		MWriteRegHWord((void *)0x1b00002a, 0xc000, 0xc000);
 
		/* bit[4:3] 2b'11 SDIO_D3 */
		MWriteRegHWord((void *)0x1b000026, 0x0018, 0x0018);
		
		/* SDIO IN sel */
		MWriteRegHWord((void *)0x1b000026, 0x0000, 0x5000); //clear bit 12 & 14

		/* Enable IO PAD */
		WriteRegByte((volatile void *)0x1b005798,0x77); //cmd
		WriteRegByte((volatile void *)0x1b00579b,0x77); //data0
		WriteRegByte((volatile void *)0x1b00579a,0x77); //data1
		WriteRegByte((volatile void *)0x1b005795,0x77); //data2
		WriteRegByte((volatile void *)0x1b005799,0x77); //data2
		WriteRegByte((volatile void *)0x1b005796,0x77); //clock
#elif defined (CONFIG_SIGMA_SOC_SX8)
		/* REG_MIO setting, disable default pull-down */
		MWriteRegByte((void *)0x1500ea76, 0x7b, 0x7f);	//FAD0 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea77, 0x7b, 0x7f);	//FAD1 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea78, 0x7b, 0x7f);	//FAD2 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea79, 0x7b, 0x7f);	//FAD3 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea7a, 0x7b, 0x7f);	//FAD4 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea7b, 0x7b, 0x7f);	//FAD5 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea7c, 0x7b, 0x7f);	//FAD6 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea7d, 0x7b, 0x7f);	//FAD7 [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea7e, 0x7b, 0x7f);	//CLE [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2 
		MWriteRegByte((void *)0x1500ea7f, 0x7b, 0x7f);	//ALE [0]:IE [1]:ST [2]:PU [3]:PD [4]:DS0 [5]:DS1 [6]:DS2

		MWriteRegByte((void*)0x1500ee23, 0x22, 0x33);	//bit[6:4]wp, bit[2:0]cd
		MWriteRegByte((void*)0x1500ee39, 0x10, 0x30);	//bit[6:4]data0
		MWriteRegByte((void*)0x1500ee3a, 0x11, 0x33);	//bit[6:4]data2,bit[2:0]data1
		MWriteRegByte((void*)0x1500ee3b, 0x01, 0x03);	//bit[2:0]data3
		MWriteRegByte((void*)0x1500ee44, 0x20, 0x30);	//bit[6:4]clk
		MWriteRegByte((void*)0x1500ee45, 0x02, 0x03);	//bit[2:0]cmd

		/* Select 200M clock source */
		MWriteRegByte((void*)0x1500e928, 0x40, 0xc0);	//bit[7:6] = 01
#else
	#error "unknown SoC type!"
#endif
	}

	if (dev_id == 0) {
#ifdef CONFIG_SIGMA_SOC_SX6
		/*
 		* bit[6:4] = 001	SDIO_2 data 0
 		*/
		MWriteRegByte((void*)0x1500ee24, 0x10, 0x70);
	
		/*
 		* bit[6:4] = 001	SDIO data 2
 		* bit[2:0] = 001	SDIO data 1
 		*/
		MWriteRegByte((void*)0x1500ee25, 0x11, 0x77);

		/*
 		* bit[6:4] = 001	SDIO data 4
 		* bit[2:0] = 001	SDIO data 3
 		*/
		MWriteRegByte((void*)0x1500ee26, 0x11, 0x77);

		/*
 		* bit[6:4] = 001	SDIO data 6
 		* bit[2:0] = 001	SDIO data 5
 		*/
		MWriteRegByte((void*)0x1500ee27, 0x11, 0x77);

		/*
 		* bit[6:4] = 001	SDIO CLK
 		* bit[2:0] = 001	SDIO data 7
 		*/
		MWriteRegByte((void*)0x1500ee28, 0x11, 0x77);

		/*
 		* bit[2:0] = 001	SDIO CMD
 		*/
		MWriteRegByte((void*)0x1500ee29, 0x01, 0x07);

		/*
 		* bit[6:4] = 011	SDIO CD
 		*/
		MWriteRegByte((void*)0x1500ee20, 0x30, 0x70);

		/*
 		* bit[3:2] = 10		Select IO for function SDCDN in of SDIO
 		*/
		MWriteRegByte((void*)0x1500ee41, 0x08, 0x0c);
	
		/*
 		* bit[1] = 0		Select IO for function sdin of SDIO
 		*/
		MWriteRegByte((void*)0x1500e868, 0x00, 0x02);

#elif defined(CONFIG_SIGMA_SOC_SX7)
		MWriteRegByte((void*)0x1500ee27, 0x10, 0x70);
		WriteRegByte((void*)0x1500ee29, 0x11);
		WriteRegByte((void*)0x1500ee2a, 0x11);
		WriteRegByte((void*)0x1500ee2b, 0x11);
		WriteRegByte((void*)0x1500ee2c, 0x11);
		MWriteRegByte((void*)0x1500ee2d, 0x01, 0x03);
#elif defined (CONFIG_SIGMA_SOC_UXLB)

		/*TODO put UMAC setting to a better place.*/
		MWriteRegHWord((void *)0x1b000072, 0x8000, 0x8000); //set bit 15, what does this mean?

		//UMAC setting
		MWriteRegWord((void *)0x1502207c, 0x0000ffff, 0x00000021);

		//UMAC setting
		MWriteRegWord((void *)0x1502206c, 0xffff0000, 0x00210000);

		MWriteRegHWord((void *)0x1b000026, 0x0000, 0x8000); //clear bit 15
		MWriteRegHWord((void *)0x1b000026, 0x0020, 0x0020); //set bit 5, Enable SDIO2(CD, WP, CMD, CLK, D0, D1, D2, D3)
#elif defined (CONFIG_SIGMA_SOC_SX8)
		MWriteRegByte((void*)0x1500ee2e, 0x11, 0x33);	//bit[6:4]data1,bit[2:0]data0
		MWriteRegByte((void*)0x1500ee2f, 0x11, 0x33);	//bit[6:4]data3,bit[2:0]data2
		MWriteRegByte((void*)0x1500ee30, 0x11, 0x33);	//bit[6:4]data5,bit[2:0]data4
		MWriteRegByte((void*)0x1500ee31, 0x11, 0x33);	//bit[6:4]data7,bit[2:0]data6
		MWriteRegByte((void*)0x1500ee32, 0x11, 0x33);	//bit[6:4]cmd,bit[2:0]clk

		/* Select 200M clock source */
		MWriteRegByte((void*)0x1500e928, 0x10, 0x30);	//bit[5:4] = 01
#else
	#error "unknown SoC type!"
#endif
	}

	return;
}

static int sdhci_trihidtv_drv_probe(struct platform_device *pdev)
{
	int retval = 0;
	struct sdhci_host *host;
	struct sdhci_trihidtv_chip *chip;
	resource_size_t addr;

	sdhci_sx6_pinshare_init(pdev->id);
	if(pdev->resource[1].flags != IORESOURCE_IRQ){
		printk(KERN_ERR "resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
		goto err;
	}

	chip = kzalloc(sizeof(struct sdhci_trihidtv_chip), GFP_KERNEL);
	if(!chip){
		retval = -ENOMEM;
		goto err;
	}
	
	chip->pdev = pdev;
	chip->quirks = 0; /*The "quirk" at present only contains NO null operation ADMA descriptor line.*/
	
	platform_set_drvdata(pdev, chip);

	host = sdhci_alloc_host(&pdev->dev, 0);
	if(IS_ERR(host)){
		retval = PTR_ERR(host);
		goto free1;
	}

	chip->host = host;
	host->hw_name = "SDIO";
	host->ops = &sdhci_platform_ops;
	host->quirks = chip->quirks;

	/*FIXME*/
#ifdef CONFIG_SIGMA_SOC_SX8
	host->quirks |= SDHCI_QUIRK_BROKEN_DMA;
	host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;
#else
	host->quirks |= SDHCI_QUIRK_FORCE_DMA;/*add by jenny enable DMA*/
#endif
	host->quirks2 |= SDHCI_QUIRK2_BROKEN_HS200;
	if (pdev->id == 1) {
	/* For SD card, currently, no regulator to adjust IO voltage */
		host->quirks2 |= SDHCI_QUIRK2_NO_1_8_V; 
	}

	host->irq = pdev->resource[1].start;
	addr = pdev->resource[0].start;
	host->ioaddr = (void __iomem *)addr;
	if(!host->ioaddr){
		dev_err(&pdev->dev, "failed to remap registers\n");
		goto free2;
	}
	
	if (pdev->id == 0) {
		host->mmc->caps |=MMC_CAP_8_BIT_DATA;
	} else {
		host->mmc->caps |=MMC_CAP_4_BIT_DATA;
	}
	retval = sdhci_add_host(host);
	if(!retval){
		if (pdev->id == 0) {
			/* Enable HS200 mode */
		//	host->mmc->caps2 |= MMC_CAP2_HS200;
			
			/* Enable poweroff notify*/
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
			host->mmc->caps2 |= MMC_CAP2_POWEROFF_NOTIFY;
#endif
		
			/* Enable cache control */
		//	host->mmc->caps2 |= MMC_CAP2_CACHE_CTRL;

			/* HS200 mode, not support yet on SX8, need Hardware rework?  */
			//host->mmc->caps2 |= MMC_CAP2_HS200_1_8V_SDR;

			/* Enable 1.8V DDR mode*/
			host->mmc->caps |= MMC_CAP_1_8V_DDR;
		}
		return 0;
	}
free2:
	iounmap((void *)host->ioaddr);
	sdhci_free_host(host);
free1:
	platform_set_drvdata(pdev, NULL);
	kfree(chip);
err:
	dev_warn(&pdev->dev, "sdhci probe failed, retval %d\n", retval);
	return retval;
}


static int sdhci_trihidtv_drv_remove(struct platform_device *pdev)
{
	struct sdhci_trihidtv_chip *chip;
	chip = platform_get_drvdata(pdev);

	if(chip){
		sdhci_remove_host(chip->host, 0);
		sdhci_free_host(chip->host);
	
		platform_set_drvdata(pdev, NULL);
		kfree(chip);
	}

	return 0;	
}
#ifdef CONFIG_PM
static int sdhci_sx6_suspend(struct device *dev)
{
	struct sdhci_trihidtv_chip *chip = dev_get_drvdata(dev);
	struct sdhci_host *host = chip->host;

	return sdhci_suspend_host(host);
}

static int sdhci_sx6_resume(struct device *dev)
{
#define CARD_DETECT_TIMEOUT_US  1000000
#define DELAY_TIME_SLICE_US     1000
	int retry = CARD_DETECT_TIMEOUT_US / DELAY_TIME_SLICE_US;
	u32 state = 0;
	struct sdhci_trihidtv_chip *chip = dev_get_drvdata(dev);
	struct sdhci_host *host = chip->host;
	struct platform_device *pdev = to_platform_device(dev);

	sdhci_sx6_pinshare_init(pdev->id);
	while (retry--) {
		state = sdhci_readl(host, SDHCI_PRESENT_STATE);
		if (state & SDHCI_CARD_PRESENT) break;
		udelay(DELAY_TIME_SLICE_US);
	}
	if (retry < 0)
		printk("failed to detect mmc card in %d ms\n", CARD_DETECT_TIMEOUT_US / 1000);

	return sdhci_resume_host(host);
}

const struct dev_pm_ops sdhci_sx6_pmops = {
	.suspend	= sdhci_sx6_suspend,
	.resume		= sdhci_sx6_resume,
};

static void sdhci_sx6_shutdown(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sdhci_trihidtv_chip *chip = platform_get_drvdata(pdev);
	struct sdhci_host *host = chip->host;

	pr_debug("In %s \n",__func__);
	sdhci_suspend_host(host);

	return;
}

#endif


static struct platform_driver sdhci_trihidtv_driver = {
	.probe = sdhci_trihidtv_drv_probe,
	.remove = sdhci_trihidtv_drv_remove,
	.driver = {
		.name = "trix-sdhci",
		.bus = &platform_bus_type,
#ifdef CONFIG_PM
		.shutdown = sdhci_sx6_shutdown,
		.pm = &sdhci_sx6_pmops,
#endif
	}
};


static int __init sdhci_drv_init(void)
{
	return platform_driver_register(&sdhci_trihidtv_driver);
}

static void __exit sdhci_drv_exit(void)
{
	platform_driver_unregister(&sdhci_trihidtv_driver);
}

module_init(sdhci_drv_init);
module_exit(sdhci_drv_exit);

MODULE_DESCRIPTION("Sigma Designs SDHCI");
MODULE_LICENSE("GPL");

