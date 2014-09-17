/*
 *  linux/arch/arm/mach-sx6/hotplug.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SX6 soc  CPUfreq Support
 * support both A9_PLL and inhouse PLL
 *
 * Author: Tony He, 2014.
 */

#define pr_fmt(fmt) "cpufreq: " fmt

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>
#include <linux/cpu.h> 
#include <linux/version.h>

#include <asm/smp_plat.h>
#include <asm/io.h>
#include <asm/delay.h>
#include "chipver.h"

#define Freq_ReadRegByte(addr) ReadRegByte((void*)addr)
#define Freq_WriteRegByte(addr, val) WriteRegByte((void*)addr, val)
#define Freq_WriteRegByteMask(addr, val, mask) do{		\
		unsigned char __tmp = Freq_ReadRegByte(addr);	\
		__tmp &= ~(mask);				\
		__tmp |= ((val) & (mask));			\
		Freq_WriteRegByte(addr, __tmp);			\
	}while(0)

#define PLL_CTRL_REG			0x1500ef00//A9_PLL
						  //[7] pll_sreset_n
						  //[6..2] reserved
						  //[1] pll_fse
						  //[0] pll_bypass
#define PLL_BYPASS_MASK			0x1
#define PLL_BYPASS_SHIFT		0
#define PLL_FSE_MASK			0x2
#define PLL_FSE_SHIFT			1
#define PLL_SRESET_MASK			0x80
#define PLL_SRESET_SHIFT		7

#define PLL_CFG_P0_REG			0x1500ef01//A9_PLL
						  //[7..5] pll_divq
						  //[4..2] pll_divr
						  //[1..0] pll_range
#define PLL_RANGE_MASK			0x3
#define PLL_RANGE_SHIFT			0
#define PLL_DIVR_MASK			0x1c
#define PLL_DIVR_SHIFT			2
#define PLL_DIVQ_MASK			0xe0
#define PLL_DIVQ_SHIFT			5
#define PLL_MK_P0(range, divr, divq)	\
		((((range) << PLL_RANGE_SHIFT) & PLL_RANGE_MASK) | 	\
		(((divr) << PLL_DIVR_SHIFT) & PLL_DIVR_MASK) |		\
		(((divq) << PLL_DIVQ_SHIFT) & PLL_DIVQ_MASK))		\

#define PLL_CFG_P1_REG			0x1500ef02//A9_PLL
						  //[7] reserved
						  //[6..0] pll_divf
#define PLL_DIVF_MASK			0x7f
#define PLL_DIVF_SHIFT			0
#define PLL_MK_P1(divf)	\
		(((divf) << PLL_DIVF_SHIFT) & PLL_DIVF_MASK)

#define CLK_SEL_REG 			0x1500ef04//A9_CGU
						  //[7..4] reserved
						  //[3..2] clock source select
						  //[1] clock output enable
						  //[0] data pluse enable
#define DATA_PLS_MASK			0x01
#define DATA_PLS_SHIFT			0
#define EN_CLK_MASK			0x02
#define EN_CLK_SHIFT			1
#define SEL_CLK_MASK			0x0C
#define SEL_CLK_SHIFT			2
#define SRC_XTAL_CLK			0x0
#define SRC_A9_PLL			0x1
#define MK_CLKSEL(pls,en,src)	({	\
					unsigned char __val = Freq_ReadRegByte(CLK_SEL_REG);			\
					__val = (__val & ~(DATA_PLS_MASK | EN_CLK_MASK | SEL_CLK_MASK)) |	\
					(((pls) << DATA_PLS_SHIFT) & DATA_PLS_MASK) |		\
					(((en) << EN_CLK_SHIFT) & EN_CLK_MASK) |		\
					(((src) << SEL_CLK_SHIFT) & SEL_CLK_MASK);		\
				})
#define GET_CLK_SRC()		({	\
					unsigned char __val = Freq_ReadRegByte(CLK_SEL_REG);	\
					__val = ((__val & SEL_CLK_MASK) >> SEL_CLK_SHIFT);	\
					((SRC_A9_PLL == __val) ? GET_A9_PLL_SRC() : __val);	\
				})

#define PLL1_CTRL_REG			0x1500ef0c//INHOUSE_PLL
						  //[7..5] test
						  //[4]	enfclkout
						  //[3]	enclkfdb
						  //[2]	sreset
						  //[1]	bypass
						  //[0] output_select: 1'b0 - from A9 PLL; 1'b1 - from A9_inhouse_PLL
#define PLL1_TEST_MASK			0xE0
#define PLL1_TEST_SHIFT			5
#define PLL1_ENFCLKOUT_MASK		0x10
#define PLL1_ENFCLKOUT_SHIFT		4
#define PLL1_ENCLKFDB_MASK		0x08
#define PLL1_ENCLKFDB_SHIFT		3
#define PLL1_SRESET_MASK		0x04
#define PLL1_SRESET_SHIFT		2
#define PLL1_BYPASS_MASK		0x2
#define PLL1_BYPASS_SHIFT		1
#define PLL1_OUTSEL_MASK		0x1
#define PLL1_OUTSEL_SHIFT		0

#define SRC_SYNTAX_DEFAULT		0x0
#define SRC_SYNTAX_INHOUSE		0x1
#define SRC_A9_DEFAULT_PLL		0x10
#define SRC_A9_INHOUSE_PLL		0x11
#define GET_A9_PLL_SRC()	({	\
					unsigned char __val = Freq_ReadRegByte(PLL1_CTRL_REG);		\
					__val = ((__val & PLL1_OUTSEL_MASK) >> PLL1_OUTSEL_SHIFT);	\
					((__val == SRC_SYNTAX_DEFAULT) ? SRC_A9_DEFAULT_PLL :		\
					SRC_A9_INHOUSE_PLL);						\
				})

#define PLL1_DIVSEL_REG			0x1500ef0d//INHOUSE_PLL
						  //[7..4] other
						  //[3..2] prevdivsel
						  //[1..0] postdivsel
#define PLL1_POSTDIVSEL_MASK		0x03
#define PLL1_POSTDIVSEL_SHIFT		0
#define PLL1_PREDIVSEL_MASK		0x0C
#define PLL1_PREDIVSEL_SHIFT		2
#define PLL1_MK_DIVSEL(pre,post)({	\
					unsigned char __val = Freq_ReadRegByte(PLL1_DIVSEL_REG);	\
					__val = (__val & ~(PLL1_POSTDIVSEL_MASK | PLL1_PREDIVSEL_MASK))|\
					(((pre) << PLL1_PREDIVSEL_SHIFT) & PLL1_PREDIVSEL_MASK) |	\
					(((post) << PLL1_POSTDIVSEL_SHIFT) & PLL1_POSTDIVSEL_MASK);	\
				})

#define PLL1_FB_DIVSEL_REG		0x1500ef0e//INHOUSE_PLL feeback div

#define PLL1_REQ_CTRL_REG		0x1500ef0f//INHOUSE_PLL 
						  //[7] pll1_req, A9_INHOUSE_PLL feedback div request control
						  //[6..2] others, N/A
						  //[1] pll lock status, A9 PLL lock status
						  //[0] pll1 lock status, A9_INHOUSE_PLL lock status
#define PLL_LOCKSTATE_MASK		0x01
#define PLL_LOCKSTATE_SHIFT		0
#define PLL1_LOCKSTATE_MASK		0x02
#define PLL1_LOCKSTATE_SHIFT		1
#define PLL1_REQ_MASK			0x80
#define PLL1_REQ_SHIFT			7

#define CLK_SCALING_PREV()	do{	\
					pr_debug("%s[%d]clk src chg to crystal\n", __func__, __LINE__);	\
					Freq_WriteRegByte(CLK_SEL_REG, MK_CLKSEL(1, 1, SRC_XTAL_CLK));	\
				}while(0)
#define CLK_SCALING_POST()	do{	\
					pr_debug("%s[%d]clk src back to pll\n", __func__, __LINE__);	\
					Freq_WriteRegByte(CLK_SEL_REG, MK_CLKSEL(1, 1, SRC_A9_PLL));	\
				}while(0)

#define XTAL_CLK_KHZ			24000
#define MIN_CPUFREQ_KHZ			CONFIG_SIGMA_MIN_CPUFREQ	/*khz*/

#define PLL_GET_SUB(val, nm) (((val) & PLL_##nm##_MASK) >> PLL_##nm##_SHIFT)
#define PLL1_GET_SUB(val, nm) (((val) & PLL1_##nm##_MASK) >> PLL1_##nm##_SHIFT)

struct pll_operations {
	unsigned int (*getclock)(void);
	int (*setclock)(unsigned int target);
};

//A9 inhouse PLL
//Fomula: Fout = 2 * Fin * Feedback_divider / (pre_divider * post_divider)
//Note:	pre_divider = predivsel + 2
//	feedback_divider = seldiv
//	post_divider = 2 ^ (postdivsel + 1)
//	Fin = 24000000
static struct inhouse_pll{
	int ftarget;
	int fout;
	int predivsel;
	int seldiv;
	int postdivsel;
} pll1_tbl[] = {
	//placeholder for generated settings, keep it at tbl[0]
	{CPUFREQ_ENTRY_INVALID, CPUFREQ_ENTRY_INVALID, -1, -1, -1},

	//clock source: crystal, 24M fixed
	{XTAL_CLK_KHZ, XTAL_CLK_KHZ, -1, -1, -1},

	// clock source: in-house PLL, 75M ~ 1.2G
	// Fvco must be bigger than 1200M and post divider is mostly 16.
	// So the lowest in-house pll output frequency is 75Mhz
	{75000,   75000,   1, 75,  3},
	{100000,  100000,  1, 100, 3},
	{150000,  150000,  1, 75,  2},
	{200000,  200000,  1, 100, 2},
	{250000,  248000,  1, 124, 2},
	{300000,  300000,  1, 75,  1},
	{350000,  348000,  1, 87,  1},
	{400000,  400000,  1, 100, 1},
	{450000,  448000,  1, 112, 1},
	{500000,  496000,  1, 124, 1},
	{600000,  600000,  1, 150, 1},
	{700000,  700000,  1, 175, 1},
	{750000,  748000,  1, 187, 1},
	{800000,  800000,  1, 200, 1},
	{900000,  900000,  1, 225, 1},
	{950000,  948000,  1, 237, 1},
	{1000000, 1000000, 1, 250, 1},
	{1100000, 1096000, 1, 137, 0},
	{1200000, 1200000, 1, 150, 0},

	{CPUFREQ_TABLE_END, CPUFREQ_TABLE_END, 0, 0, 0},
};

//A9 licensed PLL
//Formula: Fout = Fin * Feedback_divider / (pre_divider * post_divider)
//Note:	 pre_divider = DIVR + 1
//	 feedback_divider = DIVF + 1
//	 post_divider = 2 ^ DIVQ
//	 Fin = 24000000
static struct licensed_pll{
	int ftarget;
	int fout;
	int part0;	/*(divq << 5) | (divr << 2) | range*/
	int part1;	/*divf*/
} pll_tbl[] = {
	//placeholder for generated settings, keep it at tbl[0]
	{CPUFREQ_ENTRY_INVALID, CPUFREQ_ENTRY_INVALID, -1, -1},
	//clock source: crystal, 24M fixed
	{XTAL_CLK_KHZ, XTAL_CLK_KHZ, -1, -1},

	// clock source: A9 PLL, 300M ~ 1.6G
	//ftarget fout		range,divr,divq		divf
	{300000,  300000,  PLL_MK_P0(1,   0,   2), PLL_MK_P1(50)},
	{500000,  492000,  PLL_MK_P0(1,   0,   1), PLL_MK_P1(40)},
	{600000,  600000,  PLL_MK_P0(1,   0,   1), PLL_MK_P1(49)},
	{700000,  696000,  PLL_MK_P0(1,   0,   1), PLL_MK_P1(57)},
	{750000,  744000,  PLL_MK_P0(1,   0,   0), PLL_MK_P1(30)},
	{800000,  792000,  PLL_MK_P0(1,   0,   0), PLL_MK_P1(32)},
	{820000,  816000,  PLL_MK_P0(1,   0,   0), PLL_MK_P1(33)},
	{850000,  840000,  PLL_MK_P0(1,   0,   0), PLL_MK_P1(34)},
	{900000,  888000,  PLL_MK_P0(1,   0,   0), PLL_MK_P1(36)},
	{1000000, 984000,  PLL_MK_P0(1,   0,   0), PLL_MK_P1(40)},
	{1100000, 1080000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(44)},
	{1200000, 1200000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(49)},
	{1300000, 1296000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(53)},
	{1350000, 1344000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(55)},
	{1400000, 1392000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(57)},
	{1450000, 1440000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(59)},
	{1500000, 1488000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(61)},
	{1550000, 1536000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(63)},
	{1600000, 1584000, PLL_MK_P0(1,   0,   0), PLL_MK_P1(65)},

	{CPUFREQ_TABLE_END, CPUFREQ_TABLE_END, 0, 0},
};

static struct cpufreq_frequency_table sx6_freq_table[] = {
	//{ 0,  24000  }, //TODO: risk due to its another clk src than others. do we realy need clk lower than 75M?
	{ 1,  75000  },
	{ 2,  100000 },
	{ 3,  300000 },
	{ 4,  500000 },
	{ 5,  600000 },
	{ 6,  700000 },
	{ 7,  750000 },
	{ 8,  800000 },
	{ 9,  820000 },
	{ 10, 900000 },
	{ 11,1000000 },
	{ 12,1200000 },
	{ 100, CPUFREQ_TABLE_END },
};

static int inhouse_pll_get(const unsigned int target, struct inhouse_pll *pll)
{
	int i, ret = 1;
	if( !pll )
	{
		pr_err("%s[%d]NULL pointer!\n", __func__, __LINE__);
		return 1;
	}

	for(i=0; pll1_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll1_tbl[i].ftarget)
			continue;
		if(target == pll1_tbl[i].ftarget)
			break;
	}

	if(pll1_tbl[i].ftarget != CPUFREQ_TABLE_END)
	{
		memcpy(pll, &pll1_tbl[i], sizeof(struct inhouse_pll));
		ret = 0;
	}
	else
	{
		//in case missing in pll tbl, try to generate one
		if(target > 500000 && target < 1000000)
		{
			pll->ftarget = target;
			pll->predivsel = 1;
			pll->postdivsel = 1;
			pll->seldiv = target / 4000; //feedback_div = Fout * pre_div * post_div / (2 * Fin)
			pll->fout = 2 * 24000 * pll->seldiv / ((pll->predivsel + 2) * (1 << (pll->postdivsel + 1)));
			pr_debug("new inhouse pll setting %d-%d: %d %d %d\n", target, pll->fout, 
				pll->predivsel, pll->seldiv, pll->postdivsel);
			memcpy(&pll1_tbl[0], pll, sizeof(pll1_tbl[0]));	//overwrite generated settings
			ret = 0;
		}
		else
		{
			pr_err("%s[%d]FAILED freq:%d beyond [600M, 1G]\n", __func__, __LINE__, target);
		}
	}
	return ret;
}

static unsigned int inhouse_pll_getclock(void)
{
	//Fout = 2 * Fin * Feedback_div / (pre_div *  post_div)
	//pre_div = predivsel + 2
	//feedback_div = seldiv
	//post_div = 2 ^ (postdivsel + 1)
	//Fin = 24000KHz
#define CLK_THRESHOLD 8000
	unsigned regval;
	int i, predivsel, seldiv, postdivsel, ftarget, fout;
	regval = Freq_ReadRegByte(PLL1_DIVSEL_REG);
	predivsel = PLL1_GET_SUB(regval, PREDIVSEL);
	postdivsel = PLL1_GET_SUB(regval, POSTDIVSEL);
	seldiv = Freq_ReadRegByte(PLL1_FB_DIVSEL_REG);
	ftarget = fout = 2 * 24000 * seldiv / ((predivsel + 2) * (1 << (postdivsel +1)));

	for(i=0; pll1_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll1_tbl[i].ftarget)
			continue;
		if(abs(pll1_tbl[i].fout - fout) < CLK_THRESHOLD)
		{
			ftarget = pll1_tbl[i].ftarget;
			break;
		}
	}
	pr_debug("getclock (inhouse) ftarget:%d, fout:%d, delta:%d\n", ftarget, fout, (pll1_tbl[i].fout - fout));
#undef CLK_THRESHOLD
	return ftarget;
}

static int inhouse_pll_setclock(unsigned int target)
{
	struct inhouse_pll pll;
	memset(&pll, 0, sizeof(struct inhouse_pll));
	if(inhouse_pll_get(target, &pll) != 0)
	{
		pr_err("cant get pll for freq:%dKHz\n", target);
		return 1;
	}

	if(XTAL_CLK_KHZ == pll.ftarget)
	{
		Freq_WriteRegByte(CLK_SEL_REG, MK_CLKSEL(1, 1, SRC_XTAL_CLK));
	}
	else
	{
		CLK_SCALING_PREV();
		pr_debug("setclock (inhouse) predivsel:%d, seldiv:%d,postdivsel:%d\n", pll.predivsel, pll.seldiv, pll.postdivsel);
		Freq_WriteRegByte(PLL1_CTRL_REG, (1 << PLL1_ENFCLKOUT_SHIFT));
		Freq_WriteRegByte(PLL1_DIVSEL_REG, PLL1_MK_DIVSEL(pll.predivsel, pll.postdivsel));
		if (CHIP_SX6_REVA == dtv_get_chip_id())
		{
			//[per tony.wei]
			//in case SX6_REVA it's risky to change feedback divider 
			//at one time, for instead to change it bit by bit
			int i, j;
			unsigned char val, delta, tmp;
			tmp = val = Freq_ReadRegByte(PLL1_FB_DIVSEL_REG);
			delta = pll.seldiv ^ val;
			for (i=0; i<8; i++)
			{
				j = (val > pll.seldiv) ? (7 - i) : i;
				if (delta & (1 << j))
				{
					tmp = tmp ^ (1 << j);
					Freq_WriteRegByte(PLL1_FB_DIVSEL_REG, tmp);
				}
			}
		}
		else
		{
			Freq_WriteRegByte(PLL1_FB_DIVSEL_REG, pll.seldiv);
		}
		Freq_WriteRegByteMask(PLL1_REQ_CTRL_REG, (1 << PLL1_REQ_SHIFT), PLL1_REQ_MASK);
		udelay(10);
		Freq_WriteRegByte(PLL1_CTRL_REG, (1 << PLL1_ENFCLKOUT_SHIFT) | 
				(1 << PLL1_SRESET_SHIFT) | (1 << PLL1_OUTSEL_SHIFT));
		udelay(40);
		Freq_WriteRegByteMask(PLL1_REQ_CTRL_REG, 0, PLL1_REQ_MASK);
		CLK_SCALING_POST();
	}
	return 0;

}

static struct pll_operations inhouse_pll_ops = 
{
	.getclock = inhouse_pll_getclock,
	.setclock = inhouse_pll_setclock,
};

static int licensed_pll_get(const unsigned int target, struct licensed_pll *pll)
{
	int i, ret = 1;
	if( !pll )
	{
		pr_err("%s[%d]NULL pointer!\n", __func__, __LINE__);
		return 1;
	}

	for(i=0; pll_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll_tbl[i].ftarget)
			continue;
		if(target == pll_tbl[i].ftarget)
			break;
	}

	if(pll_tbl[i].ftarget != CPUFREQ_TABLE_END)
	{
		memcpy(pll, &pll_tbl[i], sizeof(struct licensed_pll));
		ret = 0;
	}
	else
	{
		//in case missing in pll tbl, try to generate one
		if(target > 300000 && target < 1600000)
		{
			#define A9_PLL_FVCO_MIN	1400	/*MHz*/
			int range, divr, divq, divf, fvco;
			range = 1;
			divr = 0;
			divq = -1;
			do {
				divq += 1;
				divf = target * (divr + 1) * (1 << divq) / 24000 - 1;
				fvco = 24 * (divf + 1) * 2 / (divr + 1);
			}while(fvco < A9_PLL_FVCO_MIN);

			pll->ftarget = target;
			pll->fout = 24000 * (divf + 1) / ((divr + 1) * (1 << divq));
			pll->part0 = PLL_MK_P0(range, divr, divq);
			pll->part1 = PLL_MK_P1(divf);
			pr_debug("new pll setting %d-%d: %x %x\n", pll->ftarget, pll->fout, pll->part0, pll->part1);
			memcpy(&pll_tbl[0], pll, sizeof(pll_tbl[0]));	/*overwrite generated settings*/
			ret = 0;
			#undef A9_PLL_FVCO_MIN
		}
		else
		{
			pr_err("%s[%d]FAILED freq:%d beyond allowed range\n", __func__, __LINE__, target);
		}
	}
	return ret;
}
static unsigned int licensed_pll_getclock(void)
{
	//Formula: Fout = Fin * Feedback_divider / (pre_divider * post_divider)
	//Note:	 pre_divider = DIVR + 1
	//	 feedback_divider = DIVF + 1
	//	 post_divider = 2 ^ DIVQ
	//	 Fin = 24000KHz
	unsigned regval, delta;
	int i, range, divr, divq, divf, ftarget, fout;
	regval = Freq_ReadRegByte(PLL_CFG_P0_REG);
	range = PLL_GET_SUB(regval, RANGE);
	divr = PLL_GET_SUB(regval, DIVR);
	divq = PLL_GET_SUB(regval, DIVQ);
	regval = Freq_ReadRegByte(PLL_CFG_P1_REG);
	divf = PLL_GET_SUB(regval, DIVF);

	ftarget = fout = 24000 * (divf + 1) / ((divr + 1) * (1 << divq));
	delta = INT_MAX;
	for(i=0; pll_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll_tbl[i].ftarget)
			continue;
		if(abs(pll_tbl[i].fout - fout) < abs(delta))
		{
			ftarget = pll_tbl[i].ftarget;
			delta = pll_tbl[i].fout - fout;
			if (0 == delta)
				break; //bingo
		}
	}
	pr_debug("getclock (a9) target:%d, fout:%d, delta:%d\n", ftarget, fout, delta);
	return ftarget;
}

static int licensed_pll_setclock(unsigned int target)
{
	struct licensed_pll pll;
	memset(&pll, 0, sizeof(struct licensed_pll));
	if(licensed_pll_get(target, &pll) != 0)
	{
		pr_err("cant get pll for freq:%dKHz\n", target);
		return 1;
	}

	if(XTAL_CLK_KHZ == pll.ftarget)
	{
		Freq_WriteRegByte(CLK_SEL_REG, MK_CLKSEL(1, 1, SRC_XTAL_CLK));
	}
	else
	{
		CLK_SCALING_PREV();
		pr_debug("setclock (a9) range:%d, divr:%d, divq:%d, divf:%d\n",
			PLL_GET_SUB(pll.part0, RANGE), PLL_GET_SUB(pll.part0, DIVR), 
			PLL_GET_SUB(pll.part0, DIVQ), PLL_GET_SUB(pll.part1, DIVF));
		Freq_WriteRegByteMask(PLL_CTRL_REG, (1 << PLL_FSE_SHIFT), 
					PLL_BYPASS_MASK | PLL_FSE_MASK | PLL_SRESET_MASK);
		Freq_WriteRegByte(PLL_CFG_P0_REG, pll.part0);
		Freq_WriteRegByteMask(PLL_CFG_P1_REG, pll.part1, PLL_DIVF_MASK);
		Freq_WriteRegByteMask(PLL_CTRL_REG, (1 << PLL_FSE_SHIFT) | (1 << PLL_SRESET_SHIFT), 
					PLL_BYPASS_MASK | PLL_FSE_MASK | PLL_SRESET_MASK);
		udelay(50);
		CLK_SCALING_POST();
	}
	return 0;

}

static struct pll_operations licensed_pll_ops = 
{
	.getclock = licensed_pll_getclock,
	.setclock = licensed_pll_setclock,
};

static void calibrate_freqtbl(unsigned int cur_freq)
{
	struct cpufreq_frequency_table *item = sx6_freq_table;
	for(; item->frequency != CPUFREQ_TABLE_END; item++)
	{
		if (item->frequency == CPUFREQ_ENTRY_INVALID)
			continue;
		if (item->frequency > cur_freq)
			item->frequency = CPUFREQ_ENTRY_INVALID;	/*honor freq_cur as max, mask N/A freqs*/
		else if (item->frequency < MIN_CPUFREQ_KHZ)
			item->frequency = CPUFREQ_ENTRY_INVALID;
	}
}

static void pll_ops_dummy(void)
{
	pr_debug("not supported function call\n");
}

static struct pll_operations *pll_ops = NULL;
static int sx6_init_pll_ops(void)
{
#define PLL_SAINT_CHECK(func)	do{					\
		if (NULL == (func)) {					\
			pr_warn("'%s' is NULL, set to default\n", #func);\
			func = (void*)pll_ops_dummy;			\
		}							\
	}while(0)

	int clk_src=GET_CLK_SRC();
	if(SRC_A9_INHOUSE_PLL == clk_src) {
		pll_ops = &inhouse_pll_ops;
	} else if (SRC_A9_DEFAULT_PLL == clk_src){
		pll_ops = &licensed_pll_ops;
	} else {
		pr_err("unknown clock source [%d]\n", clk_src);
		return 1;
	}

	//saint_check
	PLL_SAINT_CHECK(pll_ops->getclock);
	PLL_SAINT_CHECK(pll_ops->setclock);
	return 0;
#undef PLL_SAINT_CHECK
}

static unsigned int sx6_pll_getclock(void)
{
	int ftarget, clk_src=GET_CLK_SRC();

	if(SRC_XTAL_CLK == clk_src)
	{
		ftarget = XTAL_CLK_KHZ;
	}
	else
	{
		ftarget = pll_ops->getclock();
	}
	return ftarget;
}

static int sx6_pll_setclock(unsigned int target)
{
	return pll_ops->setclock(target);
}

static int sx6_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, sx6_freq_table);
}

static unsigned int sx6_cpufreq_get_speed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;

	return sx6_pll_getclock();
}

static int sx6_cpufreq_set_target(struct cpufreq_policy *policy,
				      unsigned int target_freq,
				      unsigned int relation)
{
	int ret;
	unsigned int i , cpu;
	struct cpufreq_freqs freqs;

	ret = cpufreq_frequency_table_target(policy, sx6_freq_table,
					     target_freq, relation, &i);
	if (ret != 0)
		return ret;

	freqs.cpu = policy->cpu;
	freqs.old = sx6_cpufreq_get_speed(policy->cpu);
	freqs.new = sx6_freq_table[i].frequency;
	freqs.flags = 0;

	if (freqs.old == freqs.new)
		return 0;
	

	/* notifiers */
	for_each_cpu(cpu, policy->cpus) {
		freqs.cpu = cpu;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
#else
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
#endif
	}
	
	
	pr_info("Freq scaling %d -> %dkHz ...\n", freqs.old, freqs.new);
	ret = sx6_pll_setclock(freqs.new);
	if (ret != 0)
	{
		freqs.new = freqs.old;
		pr_warn("[FAIL]\n");
	}

	/* notifiers */
	for_each_cpu(cpu, policy->cpus) {
		freqs.cpu = cpu;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
#else
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
#endif
	}

	return ret;
}


static int sx6_cpufreq_driver_init(struct cpufreq_policy *policy)
{
	int ret, cur_freq;

	if (policy->cpu != 0)
		return -EINVAL;

	if (sx6_init_pll_ops())
	{
		pr_err("Init pll ops failed, disable freq scaling\n");
		return -ENODEV;
	}

	if (sx6_freq_table == NULL) {
		pr_err("No frequency information for this CPU\n");
		return -ENODEV;
	}

	ret = cpufreq_frequency_table_cpuinfo(policy, sx6_freq_table);
	if (ret != 0) {
		pr_err("Failed to configure frequency table: %d\n",
		       ret);
	}
	cpufreq_frequency_table_get_attr(sx6_freq_table, policy->cpu);
	
	/* honor first_freq as max_freq */
	cur_freq = sx6_cpufreq_get_speed(policy->cpu);
	if (policy->cpu == 0)
		calibrate_freqtbl(cur_freq);
	policy->cpuinfo.max_freq = policy->cur = cur_freq;
	if (policy->cpuinfo.min_freq > policy->cpuinfo.max_freq)
		policy->cpuinfo.min_freq = policy->cpuinfo.max_freq;
	cpufreq_frequency_table_verify(policy, sx6_freq_table);

	if (is_smp()) {
                policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
                cpumask_setall(policy->cpus);  
        }

	/* FIXME: what's the actual transition time? in ns*/ 
	policy->cpuinfo.transition_latency = (80 * 1000);

	return ret;
}

static struct freq_attr *sx6_cpufreq_attr[] = {
        &cpufreq_freq_attr_scaling_available_freqs,
        NULL,                 
};

static struct cpufreq_driver sx6_cpufreq_driver = {
	.owner		= THIS_MODULE,
	.flags          = 0,
	.verify		= sx6_cpufreq_verify_speed,
	.target		= sx6_cpufreq_set_target,
	.get		= sx6_cpufreq_get_speed,
	.init		= sx6_cpufreq_driver_init,
	.name		= "sx6",
	.attr           = sx6_cpufreq_attr,
};

static int __init sx6_cpufreq_init(void)
{
	return cpufreq_register_driver(&sx6_cpufreq_driver);
}

static void __exit sx6_cpufreq_exit(void)
{
        cpufreq_unregister_driver(&sx6_cpufreq_driver);
}

MODULE_DESCRIPTION("cpufreq driver for SX6 SoCs");
MODULE_LICENSE("GPL");
module_init(sx6_cpufreq_init);
module_exit(sx6_cpufreq_exit);
