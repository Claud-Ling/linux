/*
 *  Sigmadesigns DTV Clock support
 *  - Simple multiplexer clock implementation
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
#include <linux/delay.h>

#include "clock.h"

#define XTAL_CLK_RATE (24000000UL)
#define XTAL_CLK_RATE_KHZ (24000UL)

#define SRC_XTAL_CLK	0x0
#define SRC_A53_PLL	0x1
/*
 * DOC: adjustable factor-based clock
 *
 * Traits of this clock:
 * rate - rate is adjustable.
 *        clk->rate = parent->rate * (1 << fbrange) * (fbdiv + offset) / (prediv + 1) / (1 << postdiv)
 * parent - fixed parent.  No clk_set_parent support
 *
 * the CPU clock look like this:
 *
 * XTAL-------a53 pll------------\        ____
 *  |                             \------|    |mux
 *  |                                    |\   |
 *  |                                    | ---|-----> to CPU
 *  |                                    |    |
 *  |------------------------------------|____|
 *
 */

/*
 * @ftarget:	nominal output rate(Hz)
 * @fout:	actual output rate(Hz) from PLL corresponding to fref
 * @postdiv:	post divider
 * @prediv:	pre divider
 * @fbrange:	pre feedback divider
 * @fbdiv:	feedback divider
 * @offset:	feedback divider offset
 * @selr:	PLL loop RF-part
 * @seli:	PLL loop Int-part
 * @selp:	PLL loop Prop-part
 */
struct cpu_pll_freq_table {
	unsigned long ftarget;
	unsigned long fout;

	unsigned int prediv;
	unsigned int fbrange;
	unsigned int fbdiv;
	unsigned int offset;
	unsigned int postdiv;

	unsigned int seli; 
	unsigned int selp; 
	unsigned int selr; 
} union1_pll4g0_freq_table[] = {
	{600000000,  600000000,  2, 1, 0x96, 0, 2, 0x3, 0x18, 4},
	{700000000,  700000000,  2, 1, 0xAF, 0, 2, 0x3, 0x1B, 4},
	{800000000,  800000000,  2, 1, 0xC8, 0, 2, 0x7, 0x1E, 4},
	{900000000,  900000000,  2, 1, 0xE1, 0, 2, 0xB, 0x38, 4},
	{1000000000, 1000000000, 2, 1, 0x7D, 0, 1, 0xE, 0x2E, 4},
	{1100000000, 1096000000, 2, 1, 0x89, 0, 1, 0xE, 0x2F, 4},
	{1200000000, 1200000000, 2, 1, 0X96, 0, 1, 0x3, 0x18, 4},
	{/* sentinel */},
};

struct cpu_pll_data {
	struct pll_field pll_reset;
	struct pll_field pll_standby;

	struct pll_field clk_sel;

	struct pll_field postdiv;
	struct pll_field prediv;
	struct pll_field fbrange;
	struct pll_field fbdiv;
	struct pll_field selr;
	struct pll_field seli;
	struct pll_field selp;

	unsigned long min_rate;
	unsigned long max_rate;

	struct cpu_pll_freq_table *freq_table;
	const struct clk_ops *ops;

	unsigned int flags;
};
#define PLL_ON_THE_FLY_EN	BIT(0)

static struct clk_ops cpu_pll_clk_ops;

static const struct cpu_pll_data union1_revA1_pll4g0_data = {
				         /* offset shift  mask*/
	.pll_reset		= PLL_FIELD(0x00,  0x00,  0x01), /*0xfb040000[0] pll sw reset, active LOW*/
	.pll_standby		= PLL_FIELD(0x00,  0x01,  0x01), /*0xfb040000[1] pll standby*/
	.clk_sel		= PLL_FIELD(0x10,  0x00,  0x01), /*0xfb040010[0] source select*/

	.selp			= PLL_FIELD(0x00,  0x08,  0x3f), /*0xfb040000[13:8] PLL loop Prop-part*/
	.selr			= PLL_FIELD(0x00,  0x10,  0x1f), /*0xfb040000[20:16] PLL loop RF-part*/
	.seli			= PLL_FIELD(0x00,  0x18,  0x0f), /*0xfb040000[27:24] PLL loop Int-part*/

	.postdiv		= PLL_FIELD(0x04,  0x00,  0x07), /*0xfb040004[2:0] pll_postdiv*/
	.prediv			= PLL_FIELD(0x04,  0x03,  0x07), /*0xfb040004[5:3] pre divider*/
	.fbrange		= PLL_FIELD(0x04,  0x06,  0x03), /*0xfb040004[7:6] prefeedback div*/
	.fbdiv			= PLL_FIELD(0x04,  0x08,  0xff), /*0xfb040004[15:8] feedback div*/

	.min_rate		= 400000000,
	.max_rate		= 1700000000,
	.freq_table		= union1_pll4g0_freq_table,
	.ops			= &cpu_pll_clk_ops,
};

static int find_rate_in_table(struct clk_hw *hw,
				struct cpu_pll_freq_table *cfg,
				unsigned long rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct cpu_pll_data *data;
	struct cpu_pll_freq_table *sel;

	data = (struct cpu_pll_data *)pll->priv;

	for (sel = data->freq_table; sel->fout != 0 ; sel++)
		if (sel->fout == rate ||
		    sel->ftarget == rate)
			break;

	if (sel->fout == 0) {
		return -EINVAL;
	}

	cfg->ftarget = sel->ftarget;
	cfg->fout = sel->fout;
	cfg->postdiv = sel->postdiv;
	cfg->prediv = sel->prediv;
	cfg->fbrange = sel->fbrange;
	cfg->fbdiv = sel->fbdiv;
	cfg->offset = sel->offset;

	cfg->selr = sel->selr;
	cfg->seli = sel->seli;
	cfg->selp = sel->selp;

	return 0;
}

static int calc_rate_factor(struct clk_hw *hw,
				struct cpu_pll_freq_table *cfg,
				unsigned long rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct cpu_pll_data *data;

	data = (struct cpu_pll_data *)pll->priv;

	if (rate < data->min_rate && rate > data->max_rate) {
		pr_err("%s[%d]FAILED freq:%ld beyond [%ld MHz, %ld MHz]\n", __func__, __LINE__, rate, data->min_rate/MHZ, data->max_rate/MHZ);
		return -EINVAL;
	}

	cfg->ftarget = rate;
	cfg->postdiv = (rate <= 1000000000)?2:1;	//FIXME
	cfg->prediv = 2;
	cfg->fbrange = 1;
	cfg->offset = 0;

	/*
	 * TODO:How to calculat bellowing value?
	 */
	cfg->selr = 0;
	cfg->seli = 0;
	cfg->selp = 0;

	cfg->fbdiv = rate * (cfg->prediv + 1) * (1 << cfg->postdiv) /		\
				(1 << cfg->fbrange) / XTAL_CLK_RATE - cfg->offset;

	cfg->fout = XTAL_CLK_RATE * (1 << cfg->fbrange) * (cfg->fbdiv + cfg->offset) /	\
				(cfg->prediv + 1) / (1 << cfg->postdiv);

	pr_debug("Calculating a53 pll setting %ld-%ld: %d %d %d %d %d\n", rate, cfg->fout,
			cfg->postdiv, cfg->prediv, cfg->fbrange, cfg->fbdiv, cfg->offset);

	return 0;
}

static int find_best_rate(struct clk_hw *hw,
				struct cpu_pll_freq_table *cfg,
				unsigned long rate)
{
	if (find_rate_in_table(hw, cfg, rate))
		return calc_rate_factor(hw, cfg, rate);

	return 0;
}

/*
 * Fout = Fin * PreFeedback_divider[P] * (Feedback_divider[F] + Feedback_divider_offset[O]) /
 *              Input_divider[N] / Post_divider[M]
 * NOTE:
 * PreFeedback_divider[P] = 2 ^ fbrange
 * Feedback_divider[F] = fbdiv
 * Feedback_divider_offset[O] = offset
 * Input_divider[N] = prediv + 1
 * Post_divider[M] = 2 ^ postdiv
 *
 * (parent_rate is always 24Mhz)
 * clk->rate = parent->rate * (1 << fbrange) * (fbdiv + offset) / (prediv + 1) / (1 << postdiv)
 */
static unsigned long cpu_pll_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct cpu_pll_data *data;
	u8 postdiv, prediv, fbrange, fbdiv, offset;
	struct cpu_pll_freq_table cfg;
	unsigned long rate;
	u8 val;

	data = (struct cpu_pll_data *)pll->priv;

	val = pll_readl(pll->reg, &data->clk_sel);
	if (val == SRC_XTAL_CLK)
		return XTAL_CLK_RATE;

	postdiv	= pll_readl(pll->reg, &data->postdiv);
	prediv	= pll_readl(pll->reg, &data->prediv);
	fbrange	= pll_readl(pll->reg, &data->fbrange);
	fbdiv	= pll_readl(pll->reg, &data->fbdiv);
	offset	= 0;

	rate = XTAL_CLK_RATE_KHZ * (1 << fbrange) * (fbdiv + offset) / (prediv + 1) / (1 << postdiv);
	rate *= KHZ;

	if (find_rate_in_table(hw, &cfg, rate) == 0) {
		/* return the nominal rate if we find it in freq table */
		return cfg.ftarget;
	}

	return rate;
}

static long cpu_pll_round_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long *parent_rate)
{
	struct cpu_pll_freq_table cfg;
	int ret;

	if (rate == XTAL_CLK_RATE) {
		return rate;
	}

	ret = find_best_rate(hw, &cfg, rate);
	if (ret)
		return ret;

	return cfg.ftarget;
}

static int cpu_pll_set_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long parent_rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct cpu_pll_data *data;
	struct cpu_pll_freq_table cfg;
	unsigned long flags = 0;
	int ret;

	data = (struct cpu_pll_data *)pll->priv;

	if (rate == XTAL_CLK_RATE) {
		pll_writel(pll->reg, &data->clk_sel, SRC_XTAL_CLK);  //A53 clock select -> XTAL_CLK
		return 0;
	}

	ret = find_best_rate(hw, &cfg, rate);
	if (ret)
		return ret;

	if(pll->lock)
		spin_lock_irqsave(pll->lock, flags);

	if (data->flags & PLL_ON_THE_FLY_EN) {
		//TODO: does the pll support on-the-fly?
	} else {
		pll_writel(pll->reg, &data->clk_sel, SRC_XTAL_CLK);  //A9 clock select -> XTAL_CLK
		pll_writel(pll->reg, &data->pll_reset, 0); //reset pll, active low

		pr_debug("setclock (p4g0)-rate:%ld prediv:%d, postdiv:%d, fbrange:%d, fbdiv:%d, \
			offset:%d selr:%d seli:%d selp:%d\n", rate, cfg.prediv, cfg.postdiv,
			cfg.fbrange, cfg.fbdiv, cfg.offset, cfg.selr, cfg.seli, cfg.selp);

		/* Aplly the new factors */
		pll_writel(pll->reg, &data->prediv, cfg.prediv);
		pll_writel(pll->reg, &data->postdiv, cfg.postdiv);
		pll_writel(pll->reg, &data->fbdiv, cfg.fbdiv);
		pll_writel(pll->reg, &data->fbrange, cfg.fbrange);
		pll_writel(pll->reg, &data->selr, cfg.selr);
		pll_writel(pll->reg, &data->seli, cfg.seli);
		pll_writel(pll->reg, &data->selp, cfg.selp);

		pll_writel(pll->reg, &data->pll_reset, 1); //release pll

		/* after update the setting, toggle the standby bit */
		mdelay(10);
		pll_writel(pll->reg, &data->pll_standby, 1);
		mdelay(10);
		pll_writel(pll->reg, &data->pll_standby, 0);
		mdelay(10);

		/* now pll is stabilized , A53 CLK switch back to PLL */
		pll_writel(pll->reg, &data->clk_sel, SRC_A53_PLL);  //A53 clock select -> XTAL_CLK
	}

	if(pll->lock)
		spin_unlock_irqrestore(pll->lock, flags);

	return 0;
}

static struct clk_ops cpu_pll_clk_ops = {
	.recalc_rate	= cpu_pll_recalc_rate,
	.round_rate	= cpu_pll_round_rate,
	.set_rate	= cpu_pll_set_rate,
};


static const struct of_device_id cpu_pll_of_match[] = {
	{
		.compatible = "trix,union1_revA1_pll4g0",
		.data = &union1_revA1_pll4g0_data,
	},
	{},
};

static void __iomem * __init pll_get_register_base(
				struct device_node *np)
{
	void __iomem *reg = NULL;

	reg = syscon_iomap_lookup_by_phandle(np, "reg");
	if (IS_ERR(reg)) {
		return NULL;
	}

	return reg;
}

static void __init of_union_pll4g0_setup(struct device_node *np)
{
	const struct of_device_id *match;
	struct clk *clk;
	const char *parent_name;
	const char *clk_name = np->name;
	void __iomem *reg;
	struct cpu_pll_data *private;

	match = of_match_node(cpu_pll_of_match, np);
	if (!match) {
		pr_err("%s: No matching data\n", __func__);
		return;
	}

	private = (struct cpu_pll_data *)match->data;

	parent_name = of_clk_get_parent_name(np, 0);
	if (IS_ERR(parent_name)) {
		pr_err("%s: Failed to get parents (%ld)\n",
				__func__, PTR_ERR(parent_name));
		return;
	}

	reg = pll_get_register_base(np);
	if (!reg)
		return;

	of_property_read_string(np, "clock-output-names", &clk_name);

	clk = trix_pll_register(clk_name, parent_name,
				private->ops,
				reg, NULL, (void *)private);
	if (!IS_ERR(clk)) {
		of_clk_add_provider(np, of_clk_src_simple_get, clk);
		clk_register_clkdev(clk, clk_name, NULL);
	}

	return;
}
CLK_OF_DECLARE(union1_a53_pll, "trix,pll-4g0", of_union_pll4g0_setup);

