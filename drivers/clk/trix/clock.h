/*
 * TRIX Clock driver internal definitions
 *
 * Copyright (C) 2016 SigmaDesigns
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __DRIVERS_CLK_TRIX_CLOCK__
#define __DRIVERS_CLK_TRIX_CLOCK__
#include <linux/soc/sigma-dtv/syscon.h>

#define KHZ	(1000UL)
#define MHZ	(KHZ*KHZ)

static unsigned __maybe_unused trix_clk_readb(void __iomem *reg)
{
	return readb(reg);
}

static unsigned __maybe_unused trix_clk_readw(void __iomem *reg)
{
	return readw(reg);
}

static unsigned __maybe_unused trix_clk_readl(void __iomem *reg)
{
	return readl(reg);
}

static void __maybe_unused trix_clk_writeb(unsigned val, void __iomem *reg)
{
	writeb(val, reg);
}

static void __maybe_unused trix_clk_writew(unsigned val, void __iomem *reg)
{
	writew(val, reg);
}

static void __maybe_unused trix_clk_writel(unsigned val, void __iomem *reg)
{
	writel(val, reg);
}


/***********************************************
 * PLL
 ***********************************************/
struct trix_pll {
	struct clk_hw hw;
	void __iomem *reg;
	spinlock_t *lock; 

	void *priv;
};

#define to_trix_pll(_hw) container_of(_hw, struct trix_pll, hw)

struct pll_field {
	unsigned int offset;
	unsigned int shift;
	unsigned int mask;
};

static inline u8 pll_readb(void __iomem *base,
				struct pll_field *field)
{
	return (readb(base + field->offset) >> field->shift) & field->mask;
}

static inline void pll_writeb(void __iomem *base, struct pll_field *field,
				u8 val)
{
	writeb((readb(base + field->offset) &
		~(field->mask << field->shift)) | (val << field->shift),
		base + field->offset);

	return;
}

static inline u32 pll_readl(void __iomem *base,
				struct pll_field *field)
{
	return (readl(base + field->offset) >> field->shift) & field->mask;
}

static inline void pll_writel(void __iomem *base, struct pll_field *field,
				u32 val)
{
	writel((readl(base + field->offset) &
		~(field->mask << field->shift)) | (val << field->shift),
		base + field->offset);

	return;
}

#define PLL_FIELD(_offset, _shift, _mask) {	\
				.offset	= _offset,	\
				.shift	= _shift,	\
				.mask	= _mask,	\
				}

struct clk *trix_pll_register(const char *clk_name,
				const char *parent_name,
				const struct clk_ops *pll_ops,
				void __iomem *reg,
				spinlock_t *lock,
				void *priv);

#endif /* __DRIVERS_CLK_TRIX_CLOCK__ */

