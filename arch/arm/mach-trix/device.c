/*
 * Platform device support for Au1x00 SoCs.
 *
 * Copyright 2004, Matt Porter <mporter@kernel.crashing.org>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>

#include <asm/io.h>
#include <asm/irq.h>
#if defined(CONFIG_I2C_SIGMA_TRIX)
#include <linux/soc/sigma-dtv/mi2c.h>
#endif

#if defined(CONFIG_GMAC_ETH)
#include <linux/phy.h>
#include <linux/gmac.h>
#endif

#if defined(CONFIG_USB_EHCI_SIGMA_DTV)
/* EHCI (USB high speed host controller) */
static struct resource sigma_usb_ehci_resources[] = {
	[0] = {
		.start		= SIGMA_EHCI_BASE,
		.end		= SIGMA_EHCI_BASE + SIGMA_EHCI_LEN - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= TRIHIDTV_USB_0_INTERRUPT,
		.end		= TRIHIDTV_USB_0_INTERRUPT,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct resource sigma_usb_ehci2_resources[] = {
       [0] = {
               .start          = SIGMA_EHCI2_BASE,
               .end            = SIGMA_EHCI2_BASE + SIGMA_EHCI2_LEN - 1,
               .flags          = IORESOURCE_MEM,
       },
       [1] = {
               .start          = TRIHIDTV_USB_1_INTERRUPT,
               .end            = TRIHIDTV_USB_1_INTERRUPT,
               .flags          = IORESOURCE_IRQ,
       },
};
#endif

#if defined(CONFIG_USB_XHCI_SIGMA_DTV)
static struct resource sigma_usb_xhci_resources[] = {
       [0] = {
               .start          = SIGMA_XHCI_BASE,
               .end            = SIGMA_XHCI_BASE + SIGMA_XHCI_LEN - 1,
               .flags          = IORESOURCE_MEM,
       },
       [1] = {
               .start          = TRIHIDTV_USB_3_INTERRUPT,
               .end            = TRIHIDTV_USB_3_INTERRUPT,
               .flags          = IORESOURCE_IRQ,
       },
};
#endif

#if defined(CONFIG_MMC_SDHCI1_TRIX)
static struct resource sigma_sdhci_1_resources[] = {
        [0] = { 
                .start          = SIGMA_SDHCI_1_BASE,
                .end            = SIGMA_SDHCI_1_BASE + SIGMA_SDHCI_1_LEN - 1,
                .flags          = IORESOURCE_MEM,
        },
        [1] = {
                .start          = TRIHIDTV_SDIO_1_INTERRUPT,
                .end            = TRIHIDTV_SDIO_1_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
};
#endif

#if defined(CONFIG_MMC_SDHCI2_TRIX)
static struct resource sigma_sdhci_2_resources[] = {
        [0] = { 
                .start          = SIGMA_SDHCI_2_BASE,
                .end            = SIGMA_SDHCI_2_BASE + SIGMA_SDHCI_2_LEN - 1,
                .flags          = IORESOURCE_MEM,
        },
        [1] = {
                .start          = TRIHIDTV_SDIO_0_INTERRUPT,
                .end            = TRIHIDTV_SDIO_0_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
};
#endif

#if defined(CONFIG_I2C_SIGMA_TRIX)
static struct resource sigma_mi2c_0_resources[] = {
        [0] = {
                .start          = SIGMA_MI2C0_BASE,
                .end            = SIGMA_MI2C0_BASE + SIGMA_MI2C_SIZE - 1,
                .flags          = IORESOURCE_MEM,
        },
        [1] = {
                .start          = TRIHIDTV_MASTER_I2C_0_INTERRUPT,
                .end            = TRIHIDTV_MASTER_I2C_0_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
};

static struct resource sigma_mi2c_1_resources[] = {
        [0] = {
                .start          = SIGMA_MI2C1_BASE,
                .end            = SIGMA_MI2C1_BASE + SIGMA_MI2C_SIZE - 1,
                .flags          = IORESOURCE_MEM,
        },
        [1] = {
                .start          = TRIHIDTV_MASTER_I2C_1_INTERRUPT,
                .end            = TRIHIDTV_MASTER_I2C_1_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
};

static struct resource sigma_mi2c_2_resources[] = {
        [0] = {
                .start          = SIGMA_MI2C2_BASE,
                .end            = SIGMA_MI2C2_BASE + SIGMA_MI2C_SIZE - 1,
                .flags          = IORESOURCE_MEM,
        },
        [1] = {
                .start          = TRIHIDTV_MASTER_I2C_2_INTERRUPT,
                .end            = TRIHIDTV_MASTER_I2C_2_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
};

static struct resource sigma_mi2c_3_resources[] = {
        [0] = {
                .start          = SIGMA_MI2C3_BASE,
                .end            = SIGMA_MI2C3_BASE + SIGMA_MI2C_SIZE - 1,
                .flags          = IORESOURCE_MEM,
        },
        [1] = {
                .start          = TRIHIDTV_MASTER_I2C_3_INTERRUPT,
                .end            = TRIHIDTV_MASTER_I2C_3_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
};
#endif

#if defined(CONFIG_GMAC_ETH)
static struct resource sigma_gmac_0_resources[] = {
	[0] = {
		.start	= SIGMA_GMAC0_BASE,
		.end	= SIGMA_GMAC0_BASE + SIGMA_GMAC0_SIZE -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= TRIHIDTV_ETHERNET_INTERRUPT,
		.end	= TRIHIDTV_ETHERNET_INTERRUPT,
		.flags	= IORESOURCE_IRQ,
		.name	= "mac_irq",
	},
	[2] = {
		.start	= TRIHIDTV_ETHERNET_PMT_INTERRUPT,
		.end	= TRIHIDTV_ETHERNET_PMT_INTERRUPT,
		.flags	= IORESOURCE_IRQ,
		.name	= "wol_irq",
	},
};
#endif

static u64 ehci_dmamask = ~(u32)0;

#if defined(CONFIG_USB_XHCI_SIGMA_DTV)
static struct platform_device sigma_usb_xhci_device = {
	.name		= "trix-xhci",
	.id		= 0,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sigma_usb_xhci_resources),
	.resource	= sigma_usb_xhci_resources,
};
#endif

#if defined(CONFIG_USB_EHCI_SIGMA_DTV)
static struct platform_device sigma_usb_ehci_device = {
	.name		= "trix-ehci",
	.id		= 0,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sigma_usb_ehci_resources),
	.resource	= sigma_usb_ehci_resources,
};

static struct platform_device sigma_usb_ehci2_device = { 
       .name           = "trix-ehci",
       .id             = 1,    
       .dev = {
               .dma_mask               = &ehci_dmamask,
               .coherent_dma_mask      = 0xffffffff,
       },
       .num_resources  = ARRAY_SIZE(sigma_usb_ehci2_resources),
       .resource       = sigma_usb_ehci2_resources,
};
#endif

#if defined(CONFIG_MMC_SDHCI1_TRIX) 
static struct platform_device sigma_sdhci_1_device = { 
       .name           = "trix-sdhci",
       .id             = 1,
       .dev = {
               .dma_mask               = &ehci_dmamask,
               .coherent_dma_mask      = 0xffffffff,
       },
       .num_resources  = ARRAY_SIZE(sigma_sdhci_1_resources),
       .resource       = sigma_sdhci_1_resources,
};
#endif

#if defined(CONFIG_MMC_SDHCI2_TRIX)
static struct platform_device sigma_sdhci_2_device = { 
       .name           = "trix-sdhci",
       .id             = 0,
       .dev = {
               .dma_mask               = &ehci_dmamask,
               .coherent_dma_mask      = 0xffffffff,
       },
       .num_resources  = ARRAY_SIZE(sigma_sdhci_2_resources),
       .resource       = sigma_sdhci_2_resources,
};
#endif

#if defined(CONFIG_I2C_SIGMA_TRIX)
static struct sigma_mi2c_platform_data mi2c_pdata = {
		.clkbase = 200000000,		//200 MHz
		.default_speed = 100000,	//100 kHz
		.capacity = MI2C_CAP_BULK,
		.fifo_size = 32,
};

static struct platform_device sigma_mi2c0_device = {
       .name           = "trix-mi2c",
       .id             = 0,
       .dev = {
		.platform_data = &mi2c_pdata,
       },
       .num_resources  = ARRAY_SIZE(sigma_mi2c_0_resources),
       .resource       = sigma_mi2c_0_resources,
};

static struct platform_device sigma_mi2c1_device = {
       .name           = "trix-mi2c",
       .id             = 1,
       .dev = {
		.platform_data = &mi2c_pdata,
       },
       .num_resources  = ARRAY_SIZE(sigma_mi2c_1_resources),
       .resource       = sigma_mi2c_1_resources,
};

static struct platform_device sigma_mi2c2_device = {
       .name           = "trix-mi2c",
       .id             = 2,
       .dev = {
		.platform_data = &mi2c_pdata,
       },
       .num_resources  = ARRAY_SIZE(sigma_mi2c_2_resources),
       .resource       = sigma_mi2c_2_resources,
};

static struct platform_device sigma_mi2c3_device = {
       .name           = "trix-mi2c",
       .id             = 3,
       .dev = {
		.platform_data = &mi2c_pdata,
       },
       .num_resources  = ARRAY_SIZE(sigma_mi2c_3_resources),
       .resource       = sigma_mi2c_3_resources,
};
#endif

static struct resource sigma_pmu_resources[] = {
        {
                .start          = TRIHIDTV_A9_CPU0_PMU_INTERRUPT,
                .end            = TRIHIDTV_A9_CPU0_PMU_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
#if CONFIG_NR_CPUS > 1
        {
                .start          = TRIHIDTV_A9_CPU1_PMU_INTERRUPT,
                .end            = TRIHIDTV_A9_CPU1_PMU_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
#endif
#if CONFIG_NR_CPUS > 2
        {
                .start          = TRIHIDTV_A9_CPU2_PMU_INTERRUPT,
                .end            = TRIHIDTV_A9_CPU2_PMU_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
        {
                .start          = TRIHIDTV_A9_CPU3_PMU_INTERRUPT,
                .end            = TRIHIDTV_A9_CPU3_PMU_INTERRUPT,
                .flags          = IORESOURCE_IRQ,
        },
#endif
#if CONFIG_NR_CPUS > 4
# error "Not support NR_CPUS > 4 yet!"
#endif
};

#if defined(CONFIG_GMAC_ETH)

static u64 gmac_dmamask = ~(u32)0;

static struct plat_dwmacenet_data gmac_pdata = {
	.interface	= PHY_INTERFACE_MODE_RMII,
};

static struct platform_device sigma_gmac_device = {
       .name           = "trix-dwmac",
       .id             = 0,
       .dev = {
		.platform_data		= &gmac_pdata,
                .dma_mask		= &gmac_dmamask,
                .coherent_dma_mask	= 0xffffffff,
       },
       .num_resources  = ARRAY_SIZE(sigma_gmac_0_resources),
       .resource       = sigma_gmac_0_resources,
};
#endif

#if defined(CONFIG_MTD_TRIX_SPI_NOR)
static struct resource sigma_spi_nor_resource[] = {
	[0] = {
		.start		= SIGMA_SPI_NOR_BASE,
		.end		= SIGMA_SPI_NOR_BASE + SIGMA_SPI_NOR_SIZE -1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= SIGMA_SPI_NOR_IO,
		.end		= SIGMA_SPI_NOR_IO + SIGMA_SPI_IO_SIZE -1,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device sigma_spi_nor_device = {
       .name           = "spi-nor",
       .id             = 0,
       .num_resources  = ARRAY_SIZE(sigma_spi_nor_resource),
       .resource       = sigma_spi_nor_resource,
};
#endif

static struct platform_device sigma_pmu_device = {
        .name                   = "arm-pmu",
        .id                     = -1,
        .num_resources          = ARRAY_SIZE(sigma_pmu_resources),
        .resource               = sigma_pmu_resources,
};

static struct platform_device *sigma_platform_devices[] __initdata = {
#if defined(CONFIG_USB_EHCI_SIGMA_DTV)
	&sigma_usb_ehci_device,
	&sigma_usb_ehci2_device,
#endif
#if defined(CONFIG_USB_XHCI_SIGMA_DTV)
	&sigma_usb_xhci_device,
#endif
#if defined(CONFIG_MMC_SDHCI2_TRIX)
	&sigma_sdhci_2_device,
#endif
#if defined(CONFIG_MMC_SDHCI1_TRIX)
	&sigma_sdhci_1_device,
#endif
	&sigma_pmu_device,
#if defined(CONFIG_I2C_SIGMA_TRIX)
	&sigma_mi2c0_device,
	&sigma_mi2c1_device,
	&sigma_mi2c2_device,
	&sigma_mi2c3_device,
#endif
#if defined(CONFIG_GMAC_ETH)
	&sigma_gmac_device,
#endif
#if defined(CONFIG_MTD_TRIX_SPI_NOR)
	&sigma_spi_nor_device,
#endif
};

int __init sigma_platform_init(void)
{
	return platform_add_devices(sigma_platform_devices, ARRAY_SIZE(sigma_platform_devices));
}

arch_initcall(sigma_platform_init);
