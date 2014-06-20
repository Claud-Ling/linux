#include <linux/platform_device.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <linux/module.h>

#include "monza_nand.h"

#define SX6_NAND_HOST_BASE  0xFB002000
#define SX6_NAND_IO_SPACE   0xFC000000

static struct monza_nand_platform_data sx6_nand_platdata ={
	0,
	(void __iomem *)SX6_NAND_HOST_BASE,
	(void __iomem *)SX6_NAND_IO_SPACE,
	MONZA_NAND_PIO,
};

static struct resource sx6_nand_resources[] = {
        {
                .start          = SX6_NAND_HOST_BASE,
                .end            = SX6_NAND_HOST_BASE + 0x400 - 1,
                .flags          = IORESOURCE_MEM,
        },
};

static struct platform_device sx6_nand_device = {
        .name                = "monza-nand",
        .id                  = 0,
        .dev.platform_data   = &sx6_nand_platdata,
        .num_resources       = ARRAY_SIZE(sx6_nand_resources),
        .resource            = sx6_nand_resources,
};

static int __init sx6_nand_init(void)
{
#ifdef CONFIG_MTD_NAND_DMA
	sx6_nand_platdata.xfer_type = MONZA_NAND_DMA; 
#endif
	return platform_device_register(&sx6_nand_device);
}

arch_initcall(sx6_nand_init);

