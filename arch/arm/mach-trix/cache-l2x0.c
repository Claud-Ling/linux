/*
 * Copyright (C) 2016 Sigma Designs
 * Tony He <tony_he@sigmadesigns.com>
 *
 * This program is free software,you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/init.h>

#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>

#ifdef CONFIG_TRIX_SMC
#include "smc.h"
#endif
#include "common.h"

static void __iomem *l2cache_base = SIGMA_IO_ADDRESS(SIGMA_TRIX_L2CACHE_BASE);

#ifdef CONFIG_TRIX_SMC
static void trix_l2x0_write_sec(unsigned long val, unsigned reg)
{
	switch (reg) {
	case L2X0_CTRL:
	case L2X0_AUX_CTRL:
	case L2X0_DEBUG_CTRL:
	//case L310_TAG_LATENCY_CTRL:
	//case L310_DATA_LATENCY_CTRL:
	//case L310_ADDR_FILTER_START:
	//case L310_ADDR_FILTER_END:
	case L310_PREFETCH_CTRL:
		secure_l2x0_set_reg(l2cache_base, reg, val);
		return;
	case L310_POWER_CTRL:
		pr_info_once("TRIX L2C310: ROM does not support power control setting\n");
		return;
	default:
		WARN_ONCE(1, "%s: ignoring write to reg 0x%x\n", __func__, reg);
		return;
	}
}
#endif

#ifdef CONFIG_TRIX_L2X0_PERFORMANCE
static void __init trix_l2x0_tuning(void)
{
	u32 pctrl, backup;

	backup = pctrl = readl_relaxed(l2cache_base + L310_PREFETCH_CTRL);
	/*
	 * enable double linefill
	 * bit[30]: Double linefile enable
	 */
	pctrl |= (1 << 30);
	if (backup ^ pctrl) {
#ifdef CONFIG_TRIX_SMC
		trix_l2x0_write_sec(pctrl, L310_PREFETCH_CTRL);
#else
		writel_relaxed(pctrl, l2cache_base + L310_PREFETCH_CTRL);
#endif
	}
}
#endif

static int __init trix_l2x0_init(void)
{
	u32 aux = 0, mask = 0;

	trix_make_l2x_auxctrl(&aux, &mask);

#ifdef CONFIG_TRIX_SMC
	outer_cache.write_sec = trix_l2x0_write_sec;
#endif
	l2x0_init(l2cache_base, aux, mask);

#ifdef CONFIG_TRIX_L2X0_PERFORMANCE
	trix_l2x0_tuning();
#endif
	return 0;
}

early_initcall(trix_l2x0_init);
