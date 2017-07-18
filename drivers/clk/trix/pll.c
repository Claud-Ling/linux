/*
 *  Sigmadesigns DTV Clock support
 *  - PLL infrastructure
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/clk-provider.h>
#include <linux/soc/sigma-dtv/syscon.h>

#include "clock.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s: " fmt, __func__



struct clk *trix_pll_register(const char *clk_name,
			      const char *parent_name,
			      const struct clk_ops *pll_ops,
			      void __iomem *reg,
			      spinlock_t *lock,
			      void *priv)
{
	struct trix_pll *pll;
	struct clk *clk;
	struct clk_init_data init;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	init.name = clk_name;
	init.ops = pll_ops;

	init.flags = CLK_IS_BASIC;
	init.parent_names = &parent_name;
	init.num_parents = 1;

	pll->lock = lock;
	pll->priv = priv;
	pll->reg = reg;
	pll->hw.init = &init;

	clk = clk_register(NULL, &pll->hw);
	if (IS_ERR(clk)) {
		kfree(pll);
		return clk;
	}

	pr_debug("%s: rate %lu  parent %s \n",
			__clk_get_name(clk),
			clk_get_rate(clk),
			__clk_get_name(clk_get_parent(clk)));

	return clk;
}





