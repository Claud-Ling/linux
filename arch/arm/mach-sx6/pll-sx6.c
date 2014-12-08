/*
 *  linux/arch/arm/mach-trix/pll_sx6.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SX6 SoC PLL control
 * support both A9_PLL and inhouse PLL
 *
 * Author: Tony He, 2014.
 */

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

struct pll_descriptor {
	const char* name;
	unsigned int max;	/*KHz*/
	unsigned int min;	/*KHz*/
	struct pll_operations ops;
};

static const char* pll_getname(void);
/********************************************************/
/*                     XTAL PLL                         */
/********************************************************/
static int xtal_pll_setclock(unsigned int target);
static unsigned int xtal_pll_getclock(void);
static int xtal_pll_getconfig(struct pll_config* cfg);

static struct pll_descriptor xtal_pll = {
	.name = "XTAL PLL",
	.max = XTAL_CLK_KHZ,
	.min = XTAL_CLK_KHZ,
	.ops = {
		.getclock = xtal_pll_getclock,
		.setclock = xtal_pll_setclock,
		.getconfig = xtal_pll_getconfig,
		.getname = pll_getname,
	},
};

static int xtal_pll_setclock(unsigned int target)
{
	pr_debug("XTAL doesn't support on the fly!\n");
	return 0;
}

static unsigned int xtal_pll_getclock(void)
{
	return XTAL_CLK_KHZ;
}

static int xtal_pll_getconfig(struct pll_config* cfg)
{
	BUG_ON(cfg == NULL);
	memset(cfg, 0, sizeof(struct pll_config));
	pr_debug("pll config <min - max> <%d - %d>\n", xtal_pll.min, xtal_pll.max);
	cfg->max = xtal_pll.max;
	cfg->min = xtal_pll.min;
	return 0;
}

/********************************************************/
/*                   inhouse PLL                        */
/********************************************************/
static unsigned int inhouse_pll_getclock(void);
static int inhouse_pll_setclock(unsigned int target);
static int inhouse_pll_getconfig(struct pll_config* cfg);

static struct pll_descriptor sx6_inhouse_pll = {
	.name = "inhouse PLL",
	.max = CPUFREQ_ENTRY_INVALID,
	.min = CPUFREQ_ENTRY_INVALID,
	.ops = {
		.getclock = inhouse_pll_getclock,
		.setclock = inhouse_pll_setclock,
		.getconfig = inhouse_pll_getconfig,
		.getname = pll_getname,
	},
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
	if (SRC_XTAL_CLK == GET_CLK_SRC())
		return XTAL_CLK_KHZ;

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

static int inhouse_pll_getconfig(struct pll_config* cfg)
{
	struct pll_descriptor *pll_desc = &sx6_inhouse_pll;
	struct inhouse_pll *tbl = pll1_tbl;

	BUG_ON(cfg == NULL);
	memset(cfg, 0, sizeof(struct pll_config));

	if (CPUFREQ_ENTRY_INVALID == pll_desc->max ||
		CPUFREQ_ENTRY_INVALID == pll_desc->min) {
		unsigned int i, min, max;
		min = -1u;
		max = 0;
		for(i = 0; tbl[i].ftarget != CPUFREQ_TABLE_END; i++) {
			if(CPUFREQ_ENTRY_INVALID == tbl[i].ftarget)
				continue;

			if (tbl[i].ftarget < min)
				min = tbl[i].ftarget;
			if (tbl[i].ftarget > max)
				max = tbl[i].ftarget;
		}

		if (max < min) {
			pr_err("Invalid pll settings!?\n");
			pll_desc->max = pll_desc->min = CPUFREQ_ENTRY_INVALID;
			return 1;
		}

		pll_desc->max = max;
		pll_desc->min = min;
	}

	pr_debug("pll config <min - max> <%d - %d>\n", pll_desc->min, pll_desc->max);
	cfg->max = pll_desc->max;
	cfg->min = pll_desc->min;

	return 0;
}

/********************************************************/
/*                    licensed PLL                      */
/********************************************************/
static unsigned int licensed_pll_getclock(void);
static int licensed_pll_setclock(unsigned int target);
static int licensed_pll_getconfig(struct pll_config* cfg);

static struct pll_descriptor sx6_licensed_pll = {
	.name = "licensed PLL",
	.max = CPUFREQ_ENTRY_INVALID,
	.min = CPUFREQ_ENTRY_INVALID,
	.ops = {
		.getclock = licensed_pll_getclock,
		.setclock = licensed_pll_setclock,
		.getconfig = licensed_pll_getconfig,
		.getname = pll_getname,
	},
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

	if (SRC_XTAL_CLK == GET_CLK_SRC())
		return XTAL_CLK_KHZ;

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

static int licensed_pll_getconfig(struct pll_config* cfg)
{
	struct pll_descriptor *pll_desc = &sx6_licensed_pll;
	struct licensed_pll *tbl = pll_tbl;

	BUG_ON(cfg == NULL);
	memset(cfg, 0, sizeof(struct pll_config));

	if (CPUFREQ_ENTRY_INVALID == pll_desc->max ||
		CPUFREQ_ENTRY_INVALID == pll_desc->min) {
		unsigned int i, min, max;
		min = -1u;
		max = 0;
		for(i = 0; tbl[i].ftarget != CPUFREQ_TABLE_END; i++) {
			if(CPUFREQ_ENTRY_INVALID == tbl[i].ftarget)
				continue;

			if (tbl[i].ftarget < min)
				min = tbl[i].ftarget;
			if (tbl[i].ftarget > max)
				max = tbl[i].ftarget;
		}

		if (max < min) {
			pr_err("Invalid pll settings!?\n");
			pll_desc->max = pll_desc->min = CPUFREQ_ENTRY_INVALID;
			return 1;
		}

		pll_desc->max = max;
		pll_desc->min = min;
	}

	pr_debug("pll config <min - max> <%d - %d>\n", pll_desc->min, pll_desc->max);
	cfg->max = pll_desc->max;
	cfg->min = pll_desc->min;

	return 0;
}

/********************************************************/

static struct pll_descriptor *pll_inuse = NULL;

static const char* pll_getname(void)
{
	if (SRC_XTAL_CLK == GET_CLK_SRC())
		return xtal_pll.name;

	BUG_ON(pll_inuse == NULL);
	return pll_inuse->name;
}

/*
 * external interface
 */
static struct pll_operations* trix_get_pll_ops(void)
{
	int clk_src=GET_CLK_SRC();
	if(SRC_XTAL_CLK == clk_src) {
		pll_inuse = &xtal_pll;
	} else if(SRC_A9_INHOUSE_PLL == clk_src) {
		pll_inuse = &sx6_inhouse_pll;
	} else if (SRC_A9_DEFAULT_PLL == clk_src){
		pll_inuse = &sx6_licensed_pll;
	} else {
		pr_err("unknown clock source [%d]\n", clk_src);
		return NULL;
	}

	return &pll_inuse->ops;
}
