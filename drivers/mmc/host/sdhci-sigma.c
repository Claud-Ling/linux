#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/module.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

//#include <asm/scatterlist.h>
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
};

void sx6_sdhci_set_clock(struct sdhci_host *host, unsigned int clock)
{
	int div = 0; /* Initialized for compiler warning */
	int real_div = div, clk_mul = 1;
	u16 clk = 0;
	unsigned long timeout;

	struct mmc_host *mmc = host->mmc;
	unsigned char timing = mmc->ios.timing;

	switch(timing) {
		case MMC_TIMING_MMC_HS:
		case MMC_TIMING_UHS_SDR50:
			MWriteRegWord((volatile void*)0x1b00a400, 0x1600, 0x1fff);
			break;
		case MMC_TIMING_UHS_DDR50:	
			MWriteRegWord((volatile void*)0x1b00a400, 0x1300, 0x1fff);
			break;
		case MMC_TIMING_MMC_HS200:
			MWriteRegWord((volatile void*)0x1b00a400, 0x1424, 0x1fff);
			break;
		default:
			printk("Sdhci clock=%d, no tap delay\n", clock);
	}



	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		goto out;

	if (host->version >= SDHCI_SPEC_300) {
		/*
		 * Check if the Host Controller supports Programmable Clock
		 * Mode.
		 */
		if (host->clk_mul) {
			u16 ctrl;

			/*
			 * We need to figure out whether the Host Driver needs
			 * to select Programmable Clock Mode, or the value can
			 * be set automatically by the Host Controller based on
			 * the Preset Value registers.
			 */
			ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
			if (!(ctrl & SDHCI_CTRL_PRESET_VAL_ENABLE)) {
				for (div = 1; div <= 1024; div++) {
					if (((host->max_clk * host->clk_mul) /
					      div) <= clock)
						break;
				}
				/*
				 * Set Programmable Clock Mode in the Clock
				 * Control register.
				 */
				clk = SDHCI_PROG_CLOCK_MODE;
				real_div = div;
				clk_mul = host->clk_mul;
				div--;
			}
		} else {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock)
				div = 1;
			else {
				for (div = 2; div < SDHCI_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			real_div = div;
			div >>= 1;
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		real_div = div;
		div >>= 1;
	}

	if (real_div)
		host->mmc->actual_clock = (host->max_clk * clk_mul) / real_div;
/*FIXME: SX6 SDHCI max clock is fixed at 52MHZ*/
 #ifdef CONFIG_MMC_SDHCI1_SIGMA
/* SX6 SDHCI host max clock is 12.5 =(200/16), so mini div shoud be 8*/
	if(div <= 8) 
		div =8;
 #elif defined(CONFIG_MMC_SDHCI2_SIGMA)
/* SX6 SDHCI host max clock is 50M =(200/4), so mini div shoud be 2*/
	if(div <= 2) 
		div = 2;
 #endif
	clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;
	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	timeout = 20;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			pr_err("%s: Internal clock never "
				"stabilised.\n", mmc_hostname(host->mmc));
			return;
		}
		timeout--;
		mdelay(1);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

out:
	host->clock = clock;
	return;
}
static void sdhci_sx6_pinshare_init(void)
{
	unsigned int temp = 0;
	
        /*TODO put UMAC setting to a better place.*/

        /*FIXME SXL SDIO port pin share. May conflict with the high speed UART pin share.*/
#ifdef CONFIG_MMC_SDHCI1_SIGMA
#ifdef CONFIG_SIGMA_SOC_SX6
	temp = ReadRegByte((volatile unsigned char*)0xf500ee30);
	temp |= 0x11;
	temp &= 0x99;
	WriteRegByte((volatile unsigned char *)0xf500ee30, temp);

	temp = ReadRegByte((volatile unsigned char*)0xf500ee31);
	temp |= 0x11;
	temp &= 0x99;
	WriteRegByte((volatile unsigned char *)0xf500ee31, temp);
	
	//SDIO_CD
	temp = ReadRegByte((volatile unsigned char*)0xf500ee20);
	temp |= 0x30;
	temp &= 0xbf;
	WriteRegByte((volatile unsigned char *)0xf500ee20, temp);
	//SDIO_WP
	temp = ReadRegByte((volatile unsigned char*)0xf500ee21);
	temp |= 0x03;
	temp &= 0xfb;
	WriteRegByte((volatile unsigned char *)0xf500ee21, temp);
//SDIO_CLK
	temp = ReadRegByte((volatile unsigned char*)0xf500ee3b);
	temp |= 0x2;
	temp &= 0xfa;
	WriteRegByte((volatile unsigned char *)0xf500ee3b, temp);
//SDIO_CMD
	
	temp = ReadRegByte((volatile unsigned char*)0xf500ee3b);
	temp |= 0x20;
	temp &= 0xaf;
	WriteRegByte((volatile unsigned char *)0xf500ee3b, temp);
// input pin share
	temp = ReadRegByte((volatile unsigned char*)0xf500e868);
	temp |= 0x2;
	WriteRegByte((volatile unsigned char *)0xf500e868, temp);
	temp = ReadRegByte((volatile unsigned char*)0xf500ee41);
	temp |= 0x10;
	WriteRegByte((volatile unsigned char *)0xf500ee41, temp);
#elif defined (CONFIG_SIGMA_SOC_SX7)
	MWriteRegByte(0x1500ee27, 0x10, 0x70);
	WriteRegByte(0x1500ee29, 0x11);
	WriteRegByte(0x1500ee2a, 0x11);
	WriteRegByte(0x1500ee2b, 0x11);
	WriteRegByte(0x1500ee2c, 0x11);
	MWriteRegByte(0x1500ee2d, 0x01, 0x03);
#else
	#error "unknown SoC type!"
#endif
#endif /*CONFIG_MMC_SDHCI1_SIGMA*/

#ifdef CONFIG_MMC_SDHCI2_SIGMA
#ifdef CONFIG_SIGMA_SOC_SX6
 //0xf500ee24[6:4] = 3¡¯h1;
	temp = ReadRegByte((volatile unsigned char*)0xf500ee24);
        temp &= 0x8f;
        temp |= 0x10; 
        WriteRegByte((volatile unsigned char *)0xf500ee24, temp);
	
        WriteRegByte((volatile unsigned char *)0xf500ee25, 0x11);
        WriteRegByte((volatile unsigned char *)0xf500ee26, 0x11);
        WriteRegByte((volatile unsigned char *)0xf500ee27, 0x11);
        WriteRegByte((volatile unsigned char *)0xf500ee28, 0x11);

	//0xf500ee29[2:0] = 3¡¯h1;
	temp = ReadRegByte((volatile unsigned char*)0xf500ee29);
        temp &=0xf8;
        temp |= 0x01;
        WriteRegByte((volatile unsigned char *)0xf500ee29, temp);

	 //0xf500ee20[6:4] = 3¡¯h3;
	temp = ReadRegByte((volatile unsigned char*)0xf500ee20);
        temp &=0x8f;
        temp |= 0x30;
        WriteRegByte((volatile unsigned char *)0xf500ee20, temp);

    //0xf500ee41[3:2] = 2¡¯h2;
	temp = ReadRegByte((volatile unsigned char*)0xf500ee41);
        temp &=0xf3;
        temp |= 0x08;
        WriteRegByte((volatile unsigned char *)0xf500ee41, temp);
	
	//0xf500e868[1] = 0
	temp = ReadRegByte((volatile unsigned char*)0xf500e868);
        temp &= 0xfd;
        WriteRegByte((volatile unsigned char *)0xf500e868, temp);
#elif defined(CONFIG_SIGMA_SOC_SX7)
	printk("[TODO] sdhci2 pinshare is required for SX7!!\n");
#else
	#error "unknown SoC type!"
#endif

#endif /*CONFIG_MMC_SDHCI2_SIGMA*/

}
static int sdhci_trihidtv_drv_probe(struct platform_device *pdev)
{
	int retval = 0;
	struct sdhci_host *host;
	struct sdhci_trihidtv_chip *chip;
	resource_size_t addr;

	sdhci_sx6_pinshare_init();

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
	//host->quirks |= SDHCI_QUIRK_BROKEN_DMA;
	host->quirks |= SDHCI_QUIRK_FORCE_DMA;/*add by jenny enable DMA*/
	/* Add by Shown, SDHCI cann't support max frequency */
	host->quirks |= SDHCI_QUIRK_NONSTANDARD_CLOCK;

	host->irq = pdev->resource[1].start;
	addr = pdev->resource[0].start;
	//host->ioaddr = ioremap_nocache(addr, 0x1000);
	host->ioaddr = (void __iomem *)addr;
	//printk("%s sdhci addr %x\n",__func__,addr);
	if(!host->ioaddr){
		dev_err(&pdev->dev, "failed to remap registers\n");
		goto free2;
	}
	
#ifdef CONFIG_MMC_SDHCI2_SIGMA
	host->mmc->caps |=MMC_CAP_8_BIT_DATA;
#endif
	retval = sdhci_add_host(host);
	if(!retval){
#ifdef CONFIG_MMC_SDHCI2_SIGMA
//		host->mmc->caps2 |= MMC_CAP2_HS200;
		host->mmc->caps |= MMC_CAP_1_8V_DDR;
#endif
		return 0;
	}
free2:
	iounmap((void *)host->ioaddr);
	sdhci_free_host(host);
free1:
	platform_set_drvdata(pdev, NULL);
	kfree(chip);
err:
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
	struct sdhci_trihidtv_chip *chip = dev_get_drvdata(dev);
	struct sdhci_host *host = chip->host;

	sdhci_sx6_pinshare_init();

	return sdhci_resume_host(host);
}

const struct dev_pm_ops sdhci_sx6_pmops = {
	.suspend	= sdhci_sx6_suspend,
	.resume		= sdhci_sx6_resume,
};
#endif

static struct platform_driver sdhci_trihidtv_driver = {
	.probe = sdhci_trihidtv_drv_probe,
	.remove = sdhci_trihidtv_drv_remove,
	.driver = {
		.name = "sigma-sdhci",
		.bus = &platform_bus_type,
		.pm = &sdhci_sx6_pmops,
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

