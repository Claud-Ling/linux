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

#include "clock.h"

/*
 * DOC: basic adjustable multiplexer clock that cannot gate
 *
 * Traits of this clock:
 * prepare - clk_prepare only ensures that parents are prepared
 * enable - clk_enable only ensures that parents are enabled
 * rate - rate is only affected by parent switching.  No clk_set_rate support
 * parent - parent is adjustable through clk_set_parent
 */

struct trix_clk_mux {
	struct clk_hw	hw;
	void __iomem	*reg;
	u32		*table;
	u32		mask;
	u8		shift;
	u8		flags;
	spinlock_t	*lock;

	unsigned (*read)(void __iomem *reg);
	void (*write)(unsigned val, void __iomem *reg);
};

#define to_trix_clk_mux(_hw) container_of(_hw, struct trix_clk_mux, hw)

static u8 _clk_mux_get_parent(struct clk_hw *hw)
{
	struct trix_clk_mux *mux = to_trix_clk_mux(hw);
	int num_parents = clk_hw_get_num_parents(hw);
	u32 val;

	val = mux->read(mux->reg) >> mux->shift;
	val &= mux->mask;

	if (mux->table) {
		int i;

		for (i = 0; i < num_parents; i++)
			if (mux->table[i] == val)
				return i;
		return -EINVAL;
	}

	if (val && (mux->flags & CLK_MUX_INDEX_BIT))
		val = ffs(val) - 1;

	if (val && (mux->flags & CLK_MUX_INDEX_ONE))
		val--;

	if (val >= num_parents)
		return -EINVAL;

	return val;
}

static int _clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct trix_clk_mux *mux = to_trix_clk_mux(hw);
	u32 val;
	unsigned long flags = 0;

	if (mux->table)
		index = mux->table[index];

	else {
		if (mux->flags & CLK_MUX_INDEX_BIT)
			index = 1 << index;

		if (mux->flags & CLK_MUX_INDEX_ONE)
			index++;
	}

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);

	if (mux->flags & CLK_MUX_HIWORD_MASK) {
		val = mux->mask << (mux->shift + 16);
	} else {
		val = mux->read(mux->reg);
		val &= ~(mux->mask << mux->shift);
	}
	val |= index << mux->shift;
	mux->write(val, mux->reg);

	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);

	return 0;
}

static const struct clk_ops trix_clk_mux_ops = {
	.get_parent = _clk_mux_get_parent,
	.set_parent = _clk_mux_set_parent,
	.determine_rate = __clk_mux_determine_rate,
};

static struct clk *_clk_register_mux_table(struct device *dev, const char *name,
		const char **parent_names, u8 num_parents, unsigned long flags,
		void __iomem *reg, u8 reg_width, u8 shift, u32 mask,
		u8 clk_mux_flags, u32 *table, spinlock_t *lock)
{
	struct trix_clk_mux *mux;
	struct clk *clk;
	struct clk_init_data init;
	u8 width = 0;

	if (clk_mux_flags & CLK_MUX_HIWORD_MASK) {
		width = fls(mask) - ffs(mask) + 1;
		if (width + shift > 16) {
			pr_err("mux value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the mux */
	mux = kzalloc(sizeof(struct trix_clk_mux), GFP_KERNEL);
	if (!mux) {
		pr_err("%s: could not allocate mux clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &trix_clk_mux_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = parent_names;
	init.num_parents = num_parents;

	/* struct trix_clk_mux assignments */
	mux->reg = reg;
	mux->shift = shift;
	mux->mask = mask;
	mux->flags = clk_mux_flags;
	mux->lock = lock;
	mux->table = table;
	mux->hw.init = &init;

	switch (reg_width) {
	case 1:
		mux->read = trix_clk_readb;
		mux->write = trix_clk_writeb;
		break;
	case 2:
		mux->read = trix_clk_readw;
		mux->write = trix_clk_writew;
		break;
	case 4:
		mux->read = trix_clk_readl;
		mux->write = trix_clk_writel;
		break;
	default:
		break;
	}

	clk = clk_register(dev, &mux->hw);

	if (IS_ERR(clk))
		kfree(mux);

	return clk;
}

static struct clk *_clk_register_mux(struct device *dev, const char *name,
		const char **parent_names, u8 num_parents, unsigned long flags,
		void __iomem *reg, u8 reg_width, u8 shift, u8 width,
		u8 clk_mux_flags, spinlock_t *lock)
{
	u32 mask = BIT(width) - 1;

	return _clk_register_mux_table(dev, name, parent_names, num_parents,
				      flags, reg, reg_width, shift, mask, clk_mux_flags,
				      NULL, lock);
}

/******************************************************************************
 *              Setup function for muxes
 ******************************************************************************/
static DEFINE_SPINLOCK(clk_lock);

struct trix_mux_data {
	unsigned long clk_flags;
	u8 mux_flags;
	spinlock_t *lock;
};

static struct trix_mux_data generic_mux_clock = {
	.lock = &clk_lock,
};

static const struct of_device_id clk_mux_of_match[] __initconst = {
	{
		.compatible = "trix,generic-mux-clock",
		.data = &generic_mux_clock,
	},
	{}
};

struct mux_reg_info {
	void __iomem *reg;
	u8 reg_width;
	u8 shift;
	u8 width;
};

/**
 *  mux reg info binding:
 *
 *  example:
 *  sdio0_clk_mux {
 *      compatible = "trix,generic-mux-clock", "trix,mux-clock";
 *      reg = <&cgu 0x28>;
 *      bit-shift = <4>;
 *      bit-width = <2>;
 *  };
 */
static int __init trix_mux_clk_get_reg_addr(struct device_node *np,
				             struct mux_reg_info *info)
{
	u32 offset, shift, width, reg_io_width;

	memset(info, 0, sizeof(struct mux_reg_info));

	/* get base address */
	info->reg = syscon_iomap_lookup_by_phandle(np, "reg");
	if (IS_ERR(info->reg)) {
		return PTR_ERR(info->reg);
	}

	if (of_property_read_u32_index(np, "reg", 1, &offset)) {
		pr_err("%s must have reg[1]!\n", np->name);
		return -EINVAL;
	}

	/*
	 * search for reg-io-width property in DT. If it is not provided
	 * default to 1 byte.
	 */
	if (of_property_read_u32(np, "reg-io-width", &reg_io_width)) {
		reg_io_width = 1;
	} else if ((reg_io_width != 1) && (reg_io_width != 2)  && (reg_io_width != 4)) {
		pr_err("%s reg-io-width must be 1 ,2 or 4!\n", np->name);
		return -EINVAL;
	}

	if (of_property_read_u32(np, "bit-shift", &shift)) {
		pr_err("%s must have bit-shift!\n", np->name);
		return -EINVAL;
	}

	if (of_property_read_u32(np, "bit-width", &width)) {
		pr_err("%s must have bit-width!\n", np->name);
		return -EINVAL;
	}

	info->reg += offset;
	info->reg_width = reg_io_width;
	info->shift = shift;
	info->width = width;

	return 0;
}

static const char ** __init trix_mux_get_parents(struct device_node *np,
						       int *num_parents)
{
	const char **parents;
	int nparents, i;

	nparents = of_clk_get_parent_count(np);
	if (nparents < 2) {
		pr_err("mux-clock %s must have parents\n", np->name);
		return ERR_PTR(-EINVAL);
	}

	parents = kzalloc(nparents * sizeof(const char *), GFP_KERNEL);
	if (!parents)
		return ERR_PTR(-ENOMEM);

	/* Fill parents with names of @np's parents*/
	for (i = 0; i < nparents; i++)
		parents[i] = of_clk_get_parent_name(np, i);

	*num_parents = nparents;
	return parents;
}

static void __init of_trix_mux_clk_setup(struct device_node *np)
{
	const struct of_device_id *match;
	struct clk *clk;
	const char *clk_name = np->name;
	const char **parent_names;
	int num_parents = 0;
	struct mux_reg_info info;
	struct trix_mux_data *data;

	match = of_match_node(clk_mux_of_match, np);
	if (!match) {
		pr_err("%s: No matching data\n", __func__);
		return;
	}
	data = (struct trix_mux_data *)match->data;

	parent_names = trix_mux_get_parents(np, &num_parents);
	if (IS_ERR(parent_names)) {
		pr_err("%s: Failed to get parents (%ld)\n",
				__func__, PTR_ERR(parent_names));
		return;
	}

	if (trix_mux_clk_get_reg_addr(np, &info)) {
		goto cleanup;
	}

	of_property_read_string(np, "clock-output-names", &clk_name);

	clk = _clk_register_mux(NULL, clk_name, parent_names, num_parents,
			       data->clk_flags | CLK_SET_RATE_PARENT,
			       info.reg, info.reg_width, info.shift, info.width,
			       data->mux_flags,
			       data->lock);

	if(!IS_ERR(clk)) {
		of_clk_add_provider(np, of_clk_src_simple_get, clk);
		clk_register_clkdev(clk, clk_name, NULL);
	}

cleanup:
	/* Data in parent_names[] has been copied by clk_register(), free it now! */
	kfree(parent_names);

	return;
}
CLK_OF_DECLARE(trix_mux, "trix,mux-clock", of_trix_mux_clk_setup);
