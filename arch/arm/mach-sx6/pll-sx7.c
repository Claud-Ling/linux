/*
 *  linux/arch/arm/mach-trix/pll_sx7.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SX7 SoC PLL control
 * support inhouse PLL
 *
 * Author: Tony He, Oct 2014.
 */

#define PLL_CFG0_REG			0x1500ef00//A9_PLL
						  //[7] pll_req
						  //[6..4] pll_tst
						  //[3] pll_enfclkout
						  //[2] pll_enclkfdb
						  //[1] pll_bypass
						  //[0] pll_sreset_n
#define PLL_SRESET_MASK			0x1
#define PLL_SRESET_SHIFT		0
#define PLL_BYPASS_MASK			0x2
#define PLL_BYPASS_SHIFT		1
#define PLL_FCOE_MASK			0x4
#define PLL_FCOE_SHIFT			2
#define PLL_COE_MASK			0x8
#define PLL_COE_SHIFT			3
#define PLL_TST_MASK			0x70
#define PLL_TST_SHIFT			4
#define PLL_REQ_MASK			0x80
#define PLL_REQ_SHIFT			7

#define PLL_CFG1_REG			0x1500ef01//A9_PLL
						  //[7] pll_stdby
						  //[6] pll_frun
						  //[5..4] pll_sellockdct
						  //[3..2] pll_predivsel
						  //[1..0] pll_postdivsel
#define PLL_POSTDIVSEL_MASK		0x3
#define PLL_POSTDIVSEL_SHIFT		0
#define PLL_PREDIVSEL_MASK		0xc
#define PLL_PREDIVSEL_SHIFT		2
#define PLL_SELLOCKDCT_MASK		0x30
#define PLL_SELLOCKDCT_SHIFT		4
#define PLL_FRUN_MASK			0x40
#define PLL_FRUN_SHIFT			6
#define PLL_STDBY_MASK			0x80
#define PLL_STDBY_SHIFT			7

#define PLL_MK_DIVSEL(pre,post)({									\
					unsigned char __val = Freq_ReadRegByte(PLL_CFG1_REG);		\
					__val = (__val & ~(PLL_POSTDIVSEL_MASK | PLL_PREDIVSEL_MASK))|	\
					(((pre) << PLL_PREDIVSEL_SHIFT) & PLL_PREDIVSEL_MASK) |		\
					(((post) << PLL_POSTDIVSEL_SHIFT) & PLL_POSTDIVSEL_MASK);	\
				})

#define PLL_CFG2_REG			0x1500ef02//A9_PLL
						  //[7..0] pll_seldiv, feedback divider control
#define PLL_SELDIV_MASK			0xff
#define PLL_SELDIV_SHIFT		0
#define PLL_MK_SELDIV(div)	({								\
					unsigned char __val = Freq_ReadRegByte(PLL_CFG2_REG);	\
					__val = (__val & ~(PLL_SELDIV_MASK))|			\
					(((div) << PLL_SELDIV_SHIFT) & PLL_SELDIV_MASK);	\
				})

#define PLL_CFG3_REG			0x1502ef03//A9_PLL
						  //[7] pll_lock_status
						  //[6] pll_ackn 
						  //[5..3] reserved
						  //[2..0] pll_sel_obs_clk, observe clock select
#define PLL_OCS_MASK			0x7
#define PLL_OCS_SHIFT			0
#define PLL_ACKN_MASK			0x40
#define PLL_ACKN_SHIFT			6
#define PLL_LOCK_MASK			0x80
#define PLL_LOCK_SHIFT			7
#define PLL_CHK_LOCK()		({								\
					unsigned char __val = Freq_ReadRegByte(PLL_CFG3_REG);	\
					(__val & PLL_LOCK_MASK) ? 1 : 0;			\
				})

#define CGU_CFG0_REG 			0x1500ef04//A9_CGU
						  //[7..4] reserved
						  //[3..2] clock source select
						  //[1] clock output enable
						  //[0] data pluse enable
#define CLK_SEL_REG			CGU_CFG0_REG
#define CGU_DPE_MASK			0x01	//data_pls_clk_cortexA9
#define CGU_DPE_SHIFT			0
#define CGU_COE_MASK			0x02	//en_clk_cortexA9
#define CGU_COE_SHIFT			1
#define CGU_CSS_MASK			0x0c	//sel_clk_cortexA9
#define CGU_CSS_SHIFT			2
#define SRC_XTAL_CLK			0x0
#define SRC_A9_PLL			0x1
#define MK_CLKSEL(pls,en,src)	({								\
					unsigned char __val = Freq_ReadRegByte(CLK_SEL_REG);	\
					__val = (__val & ~(CGU_DPE_MASK | CGU_COE_MASK | CGU_CSS_MASK)) |\
					(((pls) << CGU_DPE_SHIFT) & CGU_DPE_MASK) |		\
					(((en) << CGU_COE_SHIFT) & CGU_COE_MASK) |		\
					(((src) << CGU_CSS_SHIFT) & CGU_CSS_MASK);		\
				})
#define GET_CLK_SRC()		({								\
					unsigned char __val = Freq_ReadRegByte(CLK_SEL_REG);	\
					__val = ((__val & CGU_CSS_MASK) >> CGU_CSS_SHIFT);	\
				})

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

#define GENERIC_GET_SUB(domain, val, nm) (((val) & domain##_##nm##_MASK) >> domain##_##nm##_SHIFT)
#define GENERIC_SET_SUB(domain, val, nm, sub) (((val) & ~(domain##_##nm##_MASK)) | \
				(((sub) << domain##_##nm##_SHIFT) & domain##_##nm##_MASK))

#define PLL_GET_SUB(val, nm)		GENERIC_GET_SUB(PLL, val, nm)
#define PLL_SET_SUB(val, nm, sub)	GENERIC_SET_SUB(PLL, val, nm, sub)
#define CGU_GET_SUB(val, nm)		GENERIC_GET_SUB(CGU, val, nm)
#define CGU_SET_SUB(val, nm, sub)	GENERIC_SET_SUB(CGU, val, nm, sub)


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
} pll_tbl[] = {
	//placeholder for generated settings, keep it at tbl[0]
	{CPUFREQ_ENTRY_INVALID, CPUFREQ_ENTRY_INVALID, -1, -1, -1},

	//clock source: crystal, 24M fixed
	{XTAL_CLK_KHZ, XTAL_CLK_KHZ, -1, -1, -1},

	// clock source: in-house PLL, 75M ~ 1.2G
	// Fvco must be bigger than 1200M and post divider is mostly 16.
	// So the lowest in-house pll output frequency is 75Mhz
	{100000,  100000,  1, 100, 3},
	{200000,  200000,  1, 100, 2},
	{300000,  300000,  1, 75,  1},
	{400000,  400000,  1, 100, 1},
	{500000,  496000,  1, 124, 1},
	{600000,  600000,  1, 75,  0},
	{700000,  696000,  1, 87,  0},
	{800000,  800000,  1, 100, 0},
	{900000,  896000,  1, 112, 0},
	{1000000, 1000000, 1, 125, 0},
	{1100000, 1096000, 1, 137, 0},
	{1200000, 1200000, 1, 150, 0},
	{1300000, 1296000, 1, 162, 0},
	{1400000, 1400000, 1, 175, 0},
	{1500000, 1496000, 1, 187, 0},
	{1600000, 1600000, 1, 200, 0},

	{CPUFREQ_TABLE_END, CPUFREQ_TABLE_END, 0, 0, 0},
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

static struct pll_descriptor sx7_inhouse_pll = {
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

	for(i=0; pll_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll_tbl[i].ftarget)
			continue;
		if(target == pll_tbl[i].ftarget)
			break;
	}

	if(pll_tbl[i].ftarget != CPUFREQ_TABLE_END)
	{
		memcpy(pll, &pll_tbl[i], sizeof(struct inhouse_pll));
		ret = 0;
	}
	else
	{
		//in case missing in pll tbl, try to generate one
		if(target > 500000 && target < 1000000)
		{
			pll->ftarget = target;
			pll->predivsel = 1;
			pll->postdivsel = 0;
			pll->seldiv = target / 8000; //feedback_div = Fout * pre_div * post_div / (2 * Fin)
			pll->fout = 2 * 24000 * pll->seldiv / ((pll->predivsel + 2) * (1 << (pll->postdivsel + 1)));
			pr_debug("new inhouse pll setting %d-%d: %d %d %d\n", target, pll->fout, 
				pll->predivsel, pll->seldiv, pll->postdivsel);
			memcpy(&pll_tbl[0], pll, sizeof(pll_tbl[0]));	//overwrite generated settings
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

	regval = Freq_ReadRegByte(PLL_CFG1_REG);
	predivsel = PLL_GET_SUB(regval, PREDIVSEL);
	postdivsel = PLL_GET_SUB(regval, POSTDIVSEL);
	regval = Freq_ReadRegByte(PLL_CFG2_REG);
	seldiv = PLL_GET_SUB(regval, SELDIV);
	ftarget = fout = 2 * 24000 * seldiv / ((predivsel + 2) * (1 << (postdivsel +1)));

	for(i=0; pll_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll_tbl[i].ftarget)
			continue;
		if(abs(pll_tbl[i].fout - fout) < CLK_THRESHOLD)
		{
			ftarget = pll_tbl[i].ftarget;
			break;
		}
	}
	pr_debug("getclock (inhouse) ftarget:%d, fout:%d, delta:%d\n", ftarget, fout, (pll_tbl[i].fout - fout));
#undef CLK_THRESHOLD
	return ftarget;
}

#define FREQ_ON_THE_FLY_EN
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
		unsigned regval, predivsel, postdivsel;
		regval = Freq_ReadRegByte(PLL_CFG1_REG);
		predivsel = PLL_GET_SUB(regval, PREDIVSEL);
		postdivsel = PLL_GET_SUB(regval, POSTDIVSEL);

#ifdef	FREQ_ON_THE_FLY_EN
		if ((SRC_A9_PLL == GET_CLK_SRC()) && 
			(predivsel == pll.predivsel) && (postdivsel == pll.postdivsel)) {
			/*
			 * on-the-fly (faster)
			 * currently only support feedback divider change case,
			 * also it's reliable only by changing feedback divider
			 * at a step of 1. [by tony.li and tony.wei]
			 *
			 * each step consumes ~2us or so per test result (PLL regs in 24M domain)
			 *
			 * it might support post divider change case also, but not
			 * implemented yet.
			 */
			int tmp, step;
			regval = Freq_ReadRegByte(PLL_CFG2_REG);
			tmp = PLL_GET_SUB(regval, SELDIV);
			pr_debug("on-the-fly (inhouse) seldiv: %d -> %d\n", tmp, pll.seldiv);
			step = (tmp > pll.seldiv) ? (-1) : 1;
			while(tmp != pll.seldiv) {
				tmp += step;
				//printk(" -> %d", tmp);
				Freq_WriteRegByte(PLL_CFG2_REG, PLL_MK_SELDIV(tmp));
				Freq_WriteRegByteMask(PLL_CFG0_REG, (1 << PLL_REQ_SHIFT), PLL_REQ_MASK);
				Freq_WriteRegByteMask(PLL_CFG0_REG, 0, PLL_REQ_MASK);
			}
			//printk("\n");
		} else
#endif
		{
			CLK_SCALING_PREV();
			pr_debug("setclock (inhouse) predivsel:%d,seldiv:%d,postdivsel:%d\n", pll.predivsel, pll.seldiv, pll.postdivsel);
			Freq_WriteRegByteMask(PLL_CFG0_REG, 0, PLL_SRESET_MASK);
			Freq_WriteRegByte(PLL_CFG1_REG, PLL_MK_DIVSEL(pll.predivsel, pll.postdivsel));
			Freq_WriteRegByte(PLL_CFG2_REG, PLL_MK_SELDIV(pll.seldiv));
			Freq_WriteRegByteMask(PLL_CFG0_REG, (1 << PLL_REQ_SHIFT) | (1 << PLL_SRESET_SHIFT), 
						PLL_REQ_MASK | PLL_SRESET_MASK);
			udelay(50);
			Freq_WriteRegByteMask(PLL_CFG0_REG, 0, PLL_REQ_MASK);
			CLK_SCALING_POST();
		}
	}

	return 0;
}

static int inhouse_pll_getconfig(struct pll_config* cfg)
{
	struct pll_descriptor *pll_desc = &sx7_inhouse_pll;
	struct inhouse_pll *tbl = pll_tbl;

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
	} else if(SRC_A9_PLL == clk_src) {
		pll_inuse = &sx7_inhouse_pll;
	} else {
		pr_err("unknown clock source [%d]\n", clk_src);
		return NULL;
	}

	return &pll_inuse->ops;
}
