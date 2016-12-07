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
#define SRC_A9_PLL	0x1
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
 * XTAL-------inhouse pll--------\        ____
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
struct inhouse_pll_freq_table {
	unsigned long ftarget;
	unsigned long fout;

	unsigned int postdiv;
	unsigned int prediv;
	unsigned int fbrange;
	unsigned int fbdiv;
	unsigned int offset;

	unsigned int selr; 
	unsigned int seli; 
	unsigned int selp; 
} sx8_revA_inhouse_pll_freq_table[] = {
	{400000000,  399000000,  3, 1, 1, 0x85, 0, 1, 0xc, 0x29},
	{500000000,  500000000,  2, 2, 1, 0x7d, 0, 1, 0xf, 0x3b},
	{600000000,  600000000,  2, 2, 1, 0x96, 0, 2, 0xc, 0x34},
	{700000000,  700000000,  2, 2, 1, 0xaf, 0, 2, 0xd, 0x35},
	{800000000,  798000000,  2, 1, 1, 0x85, 0, 1, 0xc, 0x29},
	{900000000,  896000000,  1, 2, 1, 0x70, 0, 1, 0xf, 0x3b},
	{1000000000, 1000000000, 1, 2, 1, 0x7d, 0, 1, 0xf, 0x3b},
	{1100000000, 1096000000, 1, 2, 1, 0x89, 0, 2, 0xc, 0x34},
	{1200000000, 1200000000, 1, 2, 1, 0x96, 0, 2, 0xc, 0x34},
	{1300000000, 1296000000, 1, 2, 1, 0xa2, 0, 2, 0xd, 0x34},
	{1400000000, 1400000000, 1, 2, 1, 0xaf, 0, 2, 0xd, 0x35},
	{1500000000, 1496000000, 1, 2, 1, 0xbb, 0, 2, 0xd, 0x35},
	{1600000000, 1600000000, 1, 2, 1, 0xc8, 0, 2, 0xe, 0x36},
	{1700000000, 1696000000, 1, 2, 1, 0xd4, 0, 2, 0xe, 0x36},
	{/* sentinel */},
};

struct inhouse_pll_data {
	struct pll_field pll_reset;
	struct pll_field pll_bypass;

	struct pll_field data_plus;
	struct pll_field clk_en;
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

	struct inhouse_pll_freq_table *freq_table;
	const struct clk_ops *ops;

	unsigned int flags;
};
#define PLL_ON_THE_FLY_EN	BIT(0)

static struct clk_ops sx8_revA_inhouse_pll_ops;

static const struct inhouse_pll_data sx8_revA_inhouse_pll_data = {
				         /* offset shift  mask*/
	.pll_reset		= PLL_FIELD(0x0,   0x0,  0x1), /*0x1500ef00[0] pll sw reset*/
	.pll_bypass		= PLL_FIELD(0x0,   0x2,  0x1), /*0x1500ef00[2] pll bypass*/
	.postdiv		= PLL_FIELD(0x1,   0x0,  0x7), /*0x1500ef01[2:0] pll_postdiv*/
	.prediv			= PLL_FIELD(0x1,   0x3,  0x7), /*0x1500ef01[5:3] pre divider*/
	.fbrange		= PLL_FIELD(0x1,   0x6,  0x3), /*0x1500ef01[7:6] prefeedback div*/
	.fbdiv			= PLL_FIELD(0x2,   0x0, 0xff), /*0x1500ef02[7:0] feedback div*/
	.data_plus		= PLL_FIELD(0X4,   0X0,  0x1), /*0x1500ef04[0] data plus*/
	.clk_en			= PLL_FIELD(0X4,   0X1,  0x1), /*0x1500ef04[1] clk output en*/
	.clk_sel		= PLL_FIELD(0x4,   0x2,  0x3), /*0x1500ef04[3:2] source select*/
	.selr			= PLL_FIELD(0x14,  0x0, 0x1f), /*0x1500ef14[4:0] PLL loop RF-part*/
	.seli			= PLL_FIELD(0x15,  0x0,  0xf), /*0x1500ef15[3:0] PLL loop Int-part*/
	.selp			= PLL_FIELD(0x16,  0x0, 0x3f), /*0x1500ef16[5:0] PLL loop Prop-part*/
	.min_rate		= 400000000,
	.max_rate		= 1700000000,
	.freq_table		= sx8_revA_inhouse_pll_freq_table,
	.ops			= &sx8_revA_inhouse_pll_ops,
};

static int find_rate_in_table(struct clk_hw *hw,
				struct inhouse_pll_freq_table *cfg,
				unsigned long rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct inhouse_pll_data *data;
	struct inhouse_pll_freq_table *sel;

	data = (struct inhouse_pll_data *)pll->priv;

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
				struct inhouse_pll_freq_table *cfg,
				unsigned long rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct inhouse_pll_data *data;

	data = (struct inhouse_pll_data *)pll->priv;

	if (rate < data->min_rate && rate > data->max_rate) {
		pr_err("%s[%d]FAILED freq:%ld beyond [%ld MHz, %ld MHz]\n", __func__, __LINE__, rate, data->min_rate/MHZ, data->max_rate/MHZ);
		return -EINVAL;
	}

	cfg->ftarget = rate;
	cfg->postdiv = (rate <= 700000)?2:1;	//FIXME
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

	pr_debug("Calculating inhouse pll setting %ld-%ld: %d %d %d %d %d\n", rate, cfg->fout,
			cfg->postdiv, cfg->prediv, cfg->fbrange, cfg->fbdiv, cfg->offset);

	return 0;
}

static int find_best_rate(struct clk_hw *hw,
				struct inhouse_pll_freq_table *cfg,
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
static unsigned long inhouse_pll_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct inhouse_pll_data *data;
	u8 postdiv, prediv, fbrange, fbdiv, offset;
	struct inhouse_pll_freq_table cfg;
	unsigned long rate;
	u8 val;

	data = (struct inhouse_pll_data *)pll->priv;

	val = pll_readb(pll->reg, &data->clk_sel);
	if (val == SRC_XTAL_CLK)
		return XTAL_CLK_RATE;

	postdiv	= pll_readb(pll->reg, &data->postdiv);
	prediv	= pll_readb(pll->reg, &data->prediv);
	fbrange	= pll_readb(pll->reg, &data->fbrange);
	fbdiv	= pll_readb(pll->reg, &data->fbdiv);
	offset	= 0;

	rate = XTAL_CLK_RATE_KHZ * (1 << fbrange) * (fbdiv + offset) / (prediv + 1) / (1 << postdiv);
	rate *= KHZ;

	if (find_rate_in_table(hw, &cfg, rate) == 0) {
		/* return the nominal rate if we find it in freq table */
		return cfg.ftarget;
	}

	return rate;
}

static long inhouse_pll_round_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long *parent_rate)
{
	struct inhouse_pll_freq_table cfg;
	int ret;

	if (rate == XTAL_CLK_RATE) {
		return rate;
	}

	ret = find_best_rate(hw, &cfg, rate);
	if (ret)
		return ret;

	return cfg.ftarget;
}

static int inhouse_pll_set_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long parent_rate)
{
	struct trix_pll *pll = to_trix_pll(hw);
	struct inhouse_pll_data *data;
	struct inhouse_pll_freq_table cfg;
	unsigned long flags = 0;
	int ret;

	data = (struct inhouse_pll_data *)pll->priv;

	if (rate == XTAL_CLK_RATE) {
		pll_writeb(pll->reg, &data->clk_sel, SRC_XTAL_CLK);  //A9 clock select -> XTAL_CLK
		pll_writeb(pll->reg, &data->data_plus, 1); //switch logic
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
		pll_writeb(pll->reg, &data->clk_sel, SRC_XTAL_CLK);  //A9 clock select -> XTAL_CLK
		pll_writeb(pll->reg, &data->data_plus, 1); //switch logic
		pll_writeb(pll->reg, &data->pll_reset, 0); //reset pll, active low

		pr_debug("setclock (inhouse)-rate:%ld prediv:%d, postdiv:%d, fbrange:%d, fbdiv:%d, \
			offset:%d selr:%d seli:%d selp:%d\n", rate, cfg.prediv, cfg.postdiv,
			cfg.fbrange, cfg.fbdiv, cfg.offset, cfg.selr, cfg.seli, cfg.selp);

		/* Aplly the new factors */
		pll_writeb(pll->reg, &data->prediv, cfg.prediv);
		pll_writeb(pll->reg, &data->postdiv, cfg.postdiv);
		pll_writeb(pll->reg, &data->fbrange, cfg.fbrange);
		pll_writeb(pll->reg, &data->fbdiv, cfg.fbdiv);
		pll_writeb(pll->reg, &data->selr, cfg.selr);
		pll_writeb(pll->reg, &data->seli, cfg.seli);
		pll_writeb(pll->reg, &data->selp, cfg.selp);

		pll_writeb(pll->reg, &data->pll_reset, 1); //release pll

		/* after update the setting, toggle the bypass bit */
		mdelay(10);
		pll_writeb(pll->reg, &data->pll_bypass, 1);
		mdelay(10);
		pll_writeb(pll->reg, &data->pll_bypass, 0);
		mdelay(10);

		/* now pll is stabilized , A9 CLK switch back to PLL */
		pll_writeb(pll->reg, &data->clk_sel, SRC_A9_PLL);  //A9 clock select -> XTAL_CLK
		pll_writeb(pll->reg, &data->data_plus, 1); //switch logic
	}

	if(pll->lock)
		spin_unlock_irqrestore(pll->lock, flags);

	return 0;
}

static struct clk_ops sx8_revA_inhouse_pll_ops = {
	.recalc_rate	= inhouse_pll_recalc_rate,
	.round_rate	= inhouse_pll_round_rate,
	.set_rate	= inhouse_pll_set_rate,
};


static const struct of_device_id inhouse_pll_of_match[] = {
	{
		.compatible = "trix,sx8_revA_inhouse_pll",
		.data = &sx8_revA_inhouse_pll_data,
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

static void __init of_sx8_inhouse_pll_clk_setup(struct device_node *np)
{
	const struct of_device_id *match;
	struct clk *clk;
	const char *parent_name;
	const char *clk_name = np->name;
	void __iomem *reg;
	struct inhouse_pll_data *pll_data;

	match = of_match_node(inhouse_pll_of_match, np);
	if (!match) {
		pr_err("%s: No matching data\n", __func__);
		return;
	}

	pll_data = (struct inhouse_pll_data *)match->data;

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
				pll_data->ops,
				reg, NULL, (void *)pll_data);
	if (!IS_ERR(clk)) {
		of_clk_add_provider(np, of_clk_src_simple_get, clk);
		clk_register_clkdev(clk, clk_name, NULL);
	}

	return;
}
CLK_OF_DECLARE(sx8_inhouse_pll, "trix,sx8-inhouse-pll", of_sx8_inhouse_pll_clk_setup);

