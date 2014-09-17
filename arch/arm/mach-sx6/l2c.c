/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Qipeng Zha, 2012.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <asm/hardware/cache-l2x0.h>
#include <mach/hardware.h>

#ifdef CONFIG_SIGMA_SMC
#include "smc.h"
#endif
#include "common.h"

#define L2X_WRITEL_MASK(v, m, a) do{		\
	unsigned int __tmp = readl_relaxed(a);	\
	__tmp &= ~(m);				\
	__tmp |= ((v) & (m));			\
	writel_relaxed(__tmp, a);		\
	}while(0)

static void __iomem *l2cache_base = SIGMA_IO_ADDRESS(SIGMA_TRIX_L2CACHE_BASE);


/**************************************************************************/
/* sx6 l2x0 control wrapper						  */
/**************************************************************************/
void sigma_sx6_l2x0_enable(void)
{
	/* Disable PL310 L2 Cache controller */
#ifdef CONFIG_SIGMA_SMC
	sx6_smc_l2x0_enable();
#else
	writel_relaxed(1, l2cache_base + L2X0_CTRL);
#endif
}
void sigma_sx6_l2x0_disable(void)
{
	/* Disable PL310 L2 Cache controller */
#ifdef CONFIG_SIGMA_SMC
	sx6_smc_l2x0_disable();
#else
	writel_relaxed(0, l2cache_base + L2X0_CTRL);
#endif
}

void sigma_sx6_l2x0_set_auxctrl(unsigned long val)
{
	/* Set L2 Cache auxiliary control register */
#ifdef CONFIG_SIGMA_SMC
	sx6_smc_l2x0_set_auxctrl(val);
#else
	writel_relaxed(val, l2cache_base + L2X0_AUX_CTRL);
#endif
}

void sigma_sx6_l2x0_set_debug(unsigned long val)
{
#ifdef CONFIG_SIGMA_SMC
	/* Program PL310 L2 Cache controller debug register */
	sx6_smc_l2x0_set_debug(val);
#else
	writel_relaxed(val, l2cache_base + L2X0_DEBUG_CTRL);
#endif
}

void sigma_sx6_l2x0_set_prefetchctrl(unsigned long val)
{
#ifdef CONFIG_SIGMA_SMC
	/* Program PL310 L2 Cache controller prefetch control register */
	sx6_smc_l2x0_set_prefetchctrl(val);
#else
	writel_relaxed(val, l2cache_base + L2X0_PREFETCH_CTRL);
#endif
}


#ifdef CONFIG_SIGMA_SMC
/**************************************************************************/
/* Following code slices are copied from arm/mm/cache-l2x0.c		  */
/**************************************************************************/

#define CACHE_LINE_SIZE		32

static DEFINE_RAW_SPINLOCK(l2x0_lock);
static u32 l2x0_way_mask;	/* Bitmask of active ways */
static u32 l2x0_size;
#ifdef CONFIG_ANDROID_PLATFORM
static u32 l2x0_cache_id;
static unsigned int l2x0_sets;
static unsigned int l2x0_ways;
#endif
static unsigned long sync_reg_offset = L2X0_CACHE_SYNC;

#ifdef CONFIG_ANDROID_PLATFORM
static inline bool is_pl310_rev(int rev)
{
	return (l2x0_cache_id &
		(L2X0_CACHE_ID_PART_MASK | L2X0_CACHE_ID_REV_MASK)) ==
			(L2X0_CACHE_ID_PART_L310 | rev);
}
#endif

static struct l2x0_regs sx6_l2x0_saved_regs;
#define l2x0_saved_regs sx6_l2x0_saved_regs

static inline void cache_wait_way(void __iomem *reg, unsigned long mask)
{
	/* wait for cache operation by line or way to complete */
	while (readl_relaxed(reg) & mask)
		cpu_relax();
}

#ifdef CONFIG_CACHE_PL310
static inline void cache_wait(void __iomem *reg, unsigned long mask)
{
	/* cache operations by line are atomic on PL310 */
}
#else
#define cache_wait	cache_wait_way
#endif

static inline void cache_sync(void)
{
	void __iomem *base = l2cache_base;

	writel_relaxed(0, base + sync_reg_offset);
	cache_wait(base + L2X0_CACHE_SYNC, 1);
}

static inline void l2x0_clean_line(unsigned long addr)
{
	void __iomem *base = l2cache_base;
	cache_wait(base + L2X0_CLEAN_LINE_PA, 1);
	writel_relaxed(addr, base + L2X0_CLEAN_LINE_PA);
}

static inline void l2x0_inv_line(unsigned long addr)
{
	void __iomem *base = l2cache_base;
	cache_wait(base + L2X0_INV_LINE_PA, 1);
	writel_relaxed(addr, base + L2X0_INV_LINE_PA);
}

#if defined(CONFIG_PL310_ERRATA_588369) || defined(CONFIG_PL310_ERRATA_727915)
static inline void debug_writel(unsigned long val)
{
	if (outer_cache.set_debug)
		outer_cache.set_debug(val);
}

static void pl310_set_debug(unsigned long val)
{
	sigma_sx6_l2x0_set_debug(val);
}
#else
/* Optimised out for non-errata case */
static inline void debug_writel(unsigned long val)
{
}

#define pl310_set_debug	NULL
#endif

#ifdef CONFIG_PL310_ERRATA_588369
static inline void l2x0_flush_line(unsigned long addr)
{
	void __iomem *base = l2cache_base;

	/* Clean by PA followed by Invalidate by PA */
	cache_wait(base + L2X0_CLEAN_LINE_PA, 1);
	writel_relaxed(addr, base + L2X0_CLEAN_LINE_PA);
	cache_wait(base + L2X0_INV_LINE_PA, 1);
	writel_relaxed(addr, base + L2X0_INV_LINE_PA);
}
#else

static inline void l2x0_flush_line(unsigned long addr)
{
	void __iomem *base = l2cache_base;
	cache_wait(base + L2X0_CLEAN_INV_LINE_PA, 1);
	writel_relaxed(addr, base + L2X0_CLEAN_INV_LINE_PA);
}
#endif

static void l2x0_cache_sync(void)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&l2x0_lock, flags);
	cache_sync();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

#if defined(CONFIG_ANDROID_PLATFORM) && defined(CONFIG_PL310_ERRATA_727915)
static void l2x0_for_each_set_way(void __iomem *reg)
{
	int set;
	int way;
	unsigned long flags;

	for (way = 0; way < l2x0_ways; way++) {
		raw_spin_lock_irqsave(&l2x0_lock, flags);
		for (set = 0; set < l2x0_sets; set++)
			writel_relaxed((way << 28) | (set << 5), reg);
		cache_sync();
		raw_spin_unlock_irqrestore(&l2x0_lock, flags);
	}
}
#endif

static void __l2x0_flush_all(void)
{
	void __iomem *base = l2cache_base;
	debug_writel(0x03);
	writel_relaxed(l2x0_way_mask, base + L2X0_CLEAN_INV_WAY);
	cache_wait_way(base + L2X0_CLEAN_INV_WAY, l2x0_way_mask);
	cache_sync();
	debug_writel(0x00);
}

static void l2x0_flush_all(void)
{
	void __iomem *base = l2cache_base;
	unsigned long flags;

#if defined(CONFIG_ANDROID_PLATFORM) && defined(CONFIG_PL310_ERRATA_727915)
	if (is_pl310_rev(REV_PL310_R2P0)) {
		l2x0_for_each_set_way(base + L2X0_CLEAN_INV_LINE_IDX);
		return;
	}
#endif

	/* clean all ways */
	raw_spin_lock_irqsave(&l2x0_lock, flags);
	__l2x0_flush_all();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_clean_all(void)
{
	void __iomem *base = l2cache_base;
	unsigned long flags;

#if defined(CONFIG_ANDROID_PLATFORM) && defined(CONFIG_PL310_ERRATA_727915)
	if (is_pl310_rev(REV_PL310_R2P0)) {
		l2x0_for_each_set_way(base + L2X0_CLEAN_LINE_IDX);
		return;
	}
#endif

	/* clean all ways */
	raw_spin_lock_irqsave(&l2x0_lock, flags);
#ifdef CONFIG_ANDROID_PLATFORM
	debug_writel(0x03);
#endif
	writel_relaxed(l2x0_way_mask, base + L2X0_CLEAN_WAY);
	cache_wait_way(base + L2X0_CLEAN_WAY, l2x0_way_mask);
	cache_sync();
#ifdef CONFIG_ANDROID_PLATFORM
	debug_writel(0x00);
#endif
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_inv_all(void)
{
	void __iomem *base = l2cache_base;
	unsigned long flags;

	/* invalidate all ways */
	raw_spin_lock_irqsave(&l2x0_lock, flags);
	/* Invalidating when L2 is enabled is a nono */
	BUG_ON(readl(base + L2X0_CTRL) & 1);
	writel_relaxed(l2x0_way_mask, base + L2X0_INV_WAY);
	cache_wait_way(base + L2X0_INV_WAY, l2x0_way_mask);
	cache_sync();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_inv_range(unsigned long start, unsigned long end)
{
	void __iomem *base = l2cache_base;
	unsigned long flags;

	raw_spin_lock_irqsave(&l2x0_lock, flags);
	if (start & (CACHE_LINE_SIZE - 1)) {
		start &= ~(CACHE_LINE_SIZE - 1);
		debug_writel(0x03);
		l2x0_flush_line(start);
		debug_writel(0x00);
		start += CACHE_LINE_SIZE;
	}

	if (end & (CACHE_LINE_SIZE - 1)) {
		end &= ~(CACHE_LINE_SIZE - 1);
		debug_writel(0x03);
		l2x0_flush_line(end);
		debug_writel(0x00);
	}

	while (start < end) {
		unsigned long blk_end = start + min(end - start, 4096UL);

		while (start < blk_end) {
			l2x0_inv_line(start);
			start += CACHE_LINE_SIZE;
		}

		if (blk_end < end) {
			raw_spin_unlock_irqrestore(&l2x0_lock, flags);
			raw_spin_lock_irqsave(&l2x0_lock, flags);
		}
	}
	cache_wait(base + L2X0_INV_LINE_PA, 1);
	cache_sync();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_clean_range(unsigned long start, unsigned long end)
{
	void __iomem *base = l2cache_base;
	unsigned long flags;

	if ((end - start) >= l2x0_size) {
		l2x0_clean_all();
		return;
	}

	raw_spin_lock_irqsave(&l2x0_lock, flags);
	start &= ~(CACHE_LINE_SIZE - 1);
	while (start < end) {
		unsigned long blk_end = start + min(end - start, 4096UL);

		while (start < blk_end) {
			l2x0_clean_line(start);
			start += CACHE_LINE_SIZE;
		}

		if (blk_end < end) {
			raw_spin_unlock_irqrestore(&l2x0_lock, flags);
			raw_spin_lock_irqsave(&l2x0_lock, flags);
		}
	}
	cache_wait(base + L2X0_CLEAN_LINE_PA, 1);
	cache_sync();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_flush_range(unsigned long start, unsigned long end)
{
	void __iomem *base = l2cache_base;
	unsigned long flags;

	if ((end - start) >= l2x0_size) {
		l2x0_flush_all();
		return;
	}

	raw_spin_lock_irqsave(&l2x0_lock, flags);
	start &= ~(CACHE_LINE_SIZE - 1);
	while (start < end) {
		unsigned long blk_end = start + min(end - start, 4096UL);

		debug_writel(0x03);
		while (start < blk_end) {
			l2x0_flush_line(start);
			start += CACHE_LINE_SIZE;
		}
		debug_writel(0x00);

		if (blk_end < end) {
			raw_spin_unlock_irqrestore(&l2x0_lock, flags);
			raw_spin_lock_irqsave(&l2x0_lock, flags);
		}
	}
	cache_wait(base + L2X0_CLEAN_INV_LINE_PA, 1);
	cache_sync();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_disable(void)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&l2x0_lock, flags);
	__l2x0_flush_all();
	sigma_sx6_l2x0_disable();
	dsb();
	raw_spin_unlock_irqrestore(&l2x0_lock, flags);
}

static void l2x0_unlock(u32 cache_id)
{
	void __iomem *base = l2cache_base;
	int lockregs;
	int i;

	if (cache_id == L2X0_CACHE_ID_PART_L310)
		lockregs = 8;
	else
		/* L210 and unknown types */
		lockregs = 1;

	for (i = 0; i < lockregs; i++) {
		writel_relaxed(0x0, base + L2X0_LOCKDOWN_WAY_D_BASE +
			       i * L2X0_LOCKDOWN_STRIDE);
		writel_relaxed(0x0, base + L2X0_LOCKDOWN_WAY_I_BASE +
			       i * L2X0_LOCKDOWN_STRIDE);
	}
}

static void sx6_l2x0_init(void __iomem *base, u32 aux_val, u32 aux_mask)
{
	u32 aux;
	u32 cache_id;
	u32 way_size = 0;
	int ways;
	const char *type;

	cache_id = readl_relaxed(base + L2X0_CACHE_ID);
	aux = readl_relaxed(base + L2X0_AUX_CTRL);

	aux &= aux_mask;
	aux |= aux_val;

	/* Determine the number of ways */
	switch (cache_id & L2X0_CACHE_ID_PART_MASK) {
	case L2X0_CACHE_ID_PART_L310:
		if (aux & (1 << 16))
			ways = 16;
		else
			ways = 8;
		type = "L310";
#ifdef CONFIG_PL310_ERRATA_753970
		/* Unmapped register. */
		sync_reg_offset = L2X0_DUMMY_REG;
#endif
		outer_cache.set_debug = pl310_set_debug;
		break;
	case L2X0_CACHE_ID_PART_L210:
		ways = (aux >> 13) & 0xf;
		type = "L210";
		break;
	default:
		/* Assume unknown chips have 8 ways */
		ways = 8;
		type = "L2x0 series";
		break;
	}

	l2x0_way_mask = (1 << ways) - 1;

	/*
	 * L2 cache Size =  Way size * Number of ways
	 */
	way_size = (aux & L2X0_AUX_CTRL_WAY_SIZE_MASK) >> 17;
	way_size = 1 << (way_size + 3);
	l2x0_size = ways * way_size * SZ_1K;

#ifdef CONFIG_ANDROID_PLATFORM
	l2x0_cache_id = cache_id;
	l2x0_ways = ways;
	l2x0_sets = way_size * SZ_1K / CACHE_LINE_SIZE;
#endif
	/*
	 * Check if l2x0 controller is already enabled.
	 * If you are booting from non-secure mode
	 * accessing the below registers will fault.
	 */
	if (!(readl_relaxed(base + L2X0_CTRL) & 1)) {
		/* Make sure that I&D is not locked down when starting */
		//l2x0_unlock(cache_id); /*write only for secure access*/

		/* l2x0 controller is disabled */
		sigma_sx6_l2x0_set_auxctrl(aux);
		sx6_l2x0_saved_regs.aux_ctrl = aux;

		l2x0_inv_all();

		/* enable L2X0 */
		sigma_sx6_l2x0_enable();
	}

	outer_cache.inv_range = l2x0_inv_range;
	outer_cache.clean_range = l2x0_clean_range;
	outer_cache.flush_range = l2x0_flush_range;
	outer_cache.sync = l2x0_cache_sync;
	outer_cache.flush_all = l2x0_flush_all;
	outer_cache.inv_all = l2x0_inv_all;
	outer_cache.disable = l2x0_disable;

	printk(KERN_INFO "%s cache controller enabled\n", type);
	printk(KERN_INFO "l2x0: %d ways, CACHE_ID 0x%08x, AUX_CTRL 0x%08x, Cache size: %d B\n",
			ways, cache_id, aux, l2x0_size);

}

#endif

void __iomem *sx6_get_l2cache_base(void)
{
        return l2cache_base;
}

static void sx6_l2x0_resume(void)
{
	if (!(readl_relaxed(l2cache_base + L2X0_CTRL) & 1)) {
                /* restore aux ctrl */
		sigma_sx6_l2x0_set_auxctrl(l2x0_saved_regs.aux_ctrl);
		/* invalidate all */
		outer_inv_all();
		/* enable l2cache */
		sigma_sx6_l2x0_enable();
        }	
	return;
}

static int __init sx6_l2_cache_init(void)
{
	u32 aux_ctrl = 0;

	TRI_DBG("[%d] %s\n",__LINE__,__func__);	

	/* Static mapping, never released */
	printk("l2cc iomap '%x' -> '%p'\n", SIGMA_TRIX_L2CACHE_BASE, l2cache_base);
	BUG_ON(!l2cache_base);

	aux_ctrl = trix_make_l2x_auxctrl();
#ifdef CONFIG_SIGMA_SMC
	/* in non-secure state */
	sx6_l2x0_init(l2cache_base, aux_ctrl, L2X0_AUX_CTRL_MASK);
#else
	/* in secure state, l2cc will be turned on by this init function */
	l2x0_init(l2cache_base, aux_ctrl, L2X0_AUX_CTRL_MASK);
#endif

#ifdef CONFIG_SIGMA_L2X0_PERFORMANCE
	/*
	 * enable double linefill
	 * bit[30]: Double linefile enable
	 */
	sigma_sx6_l2x0_set_prefetchctrl(readl_relaxed(l2cache_base + L2X0_PREFETCH_CTRL) | (1 << 30));
#endif

	/*
	 * Override default outer_cache.resume with a SX6
	 * specific one
	*/
#ifdef CONFIG_PM
	outer_cache.resume = sx6_l2x0_resume;
#endif

	TRI_DBG("[%d] %s\n",__LINE__,__func__);	

	return 0;
}

early_initcall(sx6_l2_cache_init);
