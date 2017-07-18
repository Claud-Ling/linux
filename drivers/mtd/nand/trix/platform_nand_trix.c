#include <linux/platform_device.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <linux/module.h>

#include "trix_nand.h"

#define TRIX_NAND_HOST_BASE  0xFB002000
#define TRIX_NAND_IO_SPACE   0xFC000000

static struct trix_nand_platform_data trix_nand_platdata ={
	0,
	(void __iomem *)TRIX_NAND_HOST_BASE,
	(void __iomem *)TRIX_NAND_IO_SPACE,
	TRIX_NAND_PIO,
};

static struct resource trix_nand_resources[] = {
        {
                .start          = TRIX_NAND_HOST_BASE,
                .end            = TRIX_NAND_HOST_BASE + 0x400 - 1,
                .flags          = IORESOURCE_MEM,
        },
};

static struct platform_device trix_nand_device = {
        .name                = "trix-nand",
        .id                  = 0,
        .dev.platform_data   = &trix_nand_platdata,
        .num_resources       = ARRAY_SIZE(trix_nand_resources),
        .resource            = trix_nand_resources,
};

static int __init trix_nand_init(void)
{
#ifdef CONFIG_MTD_NAND_DMA_TRIX
	trix_nand_platdata.xfer_type = TRIX_NAND_DMA; 
#endif
	return platform_device_register(&trix_nand_device);
}

arch_initcall(trix_nand_init);

