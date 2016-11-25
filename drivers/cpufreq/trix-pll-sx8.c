/*
 *  drivers/cpufreq/trix-pll-sx8.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SX8 SoC PLL control
 * support both A9_PLL and inhouse PLL
 *
 * Author: Cane Xu, Nov 2016.
 */

#define PLL_CFG0_REG			0x1500ef00//A9_PLL
							//[7] pll_req
							//[6] pll_trans_fb
							//[5..4] pll_sellockdct
							//[3] pll_select
							//[2] pll_bypass
							//[1] pll_standby
							//[0] pll_sreset_n
#define PLL_SRESET_MASK      0x1
#define PLL_SRESET_SHIFT       0
#define PLL_STANDBY_MASK     0x2
#define PLL_STANDBY_SHIFT      1
#define PLL_BYPASS_MASK      0x4
#define PLL_BYPASS_SHIFT       2
#define PLL_SELECT_MASK      0x8
#define PLL_SELECT_SHIFT       3
#define PLL_SELLOCKDCT_MASK 0x30
#define PLL_SELLOCKDCT_SHIFT   4
#define PLL_TRANS_FB_MASK   0x40
#define PLL_TRANS_FB_SHIFT     6
#define PLL_REQ_MASK        0x80
#define PLL_REQ_SHIFT          7

#define PLL_CFG1_REG      0x1500ef01//A9_PLL
							//[7..6] pll_fbrange
							//[5..3] pll_prediv
							//[2..0] pll_postdiv
#define PLL_POSTDIV_MASK     0x7
#define PLL_POSTDIV_SHIFT      0
#define PLL_PREDIV_MASK     0x38
#define PLL_PREDIV_SHIFT       3
#define PLL_FBRANGE_MASK    0xc0
#define PLL_FBRANGE_SHIFT      6
#define PLL_MK_CFG1(postdiv, prediv, fbrange) ({	\
		unsigned char __val = Freq_ReadRegByte(PLL_CFG1_REG); \
		__val = (__val & ~(PLL_POSTDIV_MASK | PLL_PREDIV_MASK | PLL_FBRANGE_MASK)) | \
		(((postdiv) << PLL_POSTDIV_SHIFT) & PLL_POSTDIV_MASK) | 	\
		(((prediv) << PLL_PREDIV_SHIFT) & PLL_PREDIV_MASK) |		\
		(((fbrange) << PLL_FBRANGE_SHIFT) & PLL_FBRANGE_MASK);  \
		})

#define PLL_CFG2_REG      0x1500ef02//A9_PLL
							//[7..0] pll_fddiv, feedback divider control
#define PLL_FBDIV_MASK      0xff
#define PLL_FBDIV_SHIFT        0

#define CLK_SEL_REG 			0x1500ef04//A9_CGU
							//[7..4] reserved
						  //[3..2] clock source select
						  //[1] clock output enable
						  //[0] data pluse enable
#define DATA_PLS_MASK			0x01  //data_pls_clk_cortexA9
#define DATA_PLS_SHIFT	     0
#define EN_CLK_MASK			  0x02  //en_clk_cortexA9
#define EN_CLK_SHIFT			   1
#define SEL_CLK_MASK			0x0c  //sel_clk_cortexA
#define SEL_CLK_SHIFT		     2
#define SRC_XTAL_CLK			 0x0
#define SRC_A9_PLL			   0x1
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
				})

#define PLL_SELR_REG    0x1500ef14
              //[4..0] selr
#define PLL_SELR_MASK     0x1f
#define PLL_SELR_SHIFT       0

#define PLL_SELI_REG    0x1500ef15
							//[3..0] seli
#define PLL_SELI_MASK     0xf
#define PLL_SELI_SHIFT      0

#define PLL_SELP_REG    0x1500ef16
							//[5..0] selp
#define PLL_SELP_MASK     0x3f
#define PLL_SELP_SHIFT       0

#define CLK_SCALING_PREV()	do{	\
					pr_debug("%s[%d]clk src chg to crystal\n", __func__, __LINE__);	\
					Freq_WriteRegByte(CLK_SEL_REG, MK_CLKSEL(1, 1, SRC_XTAL_CLK));	\
				}while(0)
#define CLK_SCALING_POST()	do{	\
					pr_debug("%s[%d]clk src back to pll\n", __func__, __LINE__);	\
					Freq_WriteRegByte(CLK_SEL_REG, MK_CLKSEL(1, 1, SRC_A9_PLL));	\
				}while(0)

#define XTAL_CLK_KHZ			24000

#define PLL_GET_SUB(val, nm) (((val) & PLL_##nm##_MASK) >> PLL_##nm##_SHIFT)

//A9 inhouse PLL
//Fomula: Fout = Fin * PreFeedback_divider[P] * (Feedback_divider[F] + Feedback_divider_offset[O]) / Input_divider[N] / Post_divider[M])
//Note: PreFeedback_divider[P] = 2 ^ fbrange
//      Feedback_divider[F] = fbdiv
//      Feedback_divider_offset[O] = offset
//      Input_divider[N] = prediv + 1
//      Post_divider[M] = 2 ^ postdiv
//      Fin = 24000000
static struct inhouse_pll{
	int ftarget;
	int fout;
	int postdiv;
	int prediv;
	int fbrange;
	int fbdiv;
	int offset;
	int selr;
	int seli;
	int selp;
} pll_tbl[] = {
	//placeholder for generated settings, keep it at tbl[0]
	{CPUFREQ_ENTRY_INVALID, CPUFREQ_ENTRY_INVALID, -1, -1, -1, -1, -1, -1, -1, -1},

	//clock source: crystal, 24M fixed
	{XTAL_CLK_KHZ, XTAL_CLK_KHZ, -1, -1, -1, -1, -1, -1, -1, -1},

	//clock source: in-house PLL
	//ftarget,fout,postdiv,prediv,fbrange,fbdiv,offset,selr,seli,selp
	{400000,  399000,  3, 1, 1, 0x85, 0, 1, 0xc, 0x29},
	{500000,  500000,  2, 2, 1, 0x7d, 0, 1, 0xf, 0x3b},
	{600000,  600000,  2, 2, 1, 0x96, 0, 2, 0xc, 0x34},
	{700000,  700000,  2, 2, 1, 0xaf, 0, 2, 0xd, 0x35},
	{800000,  798000,  2, 1, 1, 0x85, 0, 1, 0xc, 0x29},
	{900000,  896000,  1, 2, 1, 0x70, 0, 1, 0xf, 0x3b},
	{1000000, 1000000, 1, 2, 1, 0x7d, 0, 1, 0xf, 0x3b},
	{1100000, 1096000, 1, 2, 1, 0x89, 0, 2, 0xc, 0x34},
	{1200000, 1200000, 1, 2, 1, 0x96, 0, 2, 0xc, 0x34},
	{1300000, 1296000, 1, 2, 1, 0xa2, 0, 2, 0xd, 0x34},
	{1400000, 1400000, 1, 2, 1, 0xaf, 0, 2, 0xd, 0x35},
	{1500000, 1496000, 1, 2, 1, 0xbb, 0, 2, 0xd, 0x35},
	{1600000, 1600000, 1, 2, 1, 0xc8, 0, 2, 0xe, 0x36},
	{1700000, 1696000, 1, 2, 1, 0xd4, 0, 2, 0xe, 0x36},

	{CPUFREQ_TABLE_END, CPUFREQ_TABLE_END, 0, 0, 0, 0, 0, 0, 0, 0},
};

struct pll_descriptor {
	const char* name;
	unsigned int max;	/*KHz*/
	unsigned int min;	/*KHz*/
	struct pll_operations ops;
};

static void ms_delay(unsigned int n)
{
		int i = 0;
		for(i = 0; i < n; i++)
		{
			udelay(1000);
		}
}

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

static struct pll_descriptor sx8_inhouse_pll = {
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
		if(target > 400000 && target < 1700000)
		{
			//Fomula: Fout = Fin * PreFeedback_divider[P] * (Feedback_divider[F] + Feedback_divider_offset[O]) / Input_divider[N] / Post_divider[M])
			//Note: PreFeedback_divider[P] = 2 ^ fbrange
			//      Feedback_divider[F] = fbdiv
			//      Feedback_divider_offset[O] = offset
			//      Input_divider[N] = prediv + 1
			//      Post_divider[M] = 2 ^ postdiv
			//      Fin = 24000000
			pll->ftarget = target;
			pll->postdiv = (target <= 700000)?2:1; //FIXME: when to set 2 or 1?
			pll->prediv = 2;
			pll->fbrange = 1;
			pll->fbdiv = target * (pll->prediv + 1) * (1 << pll->postdiv) / (1 << pll->fbrange) / 24000 - pll->offset;
			pll->offset = 0;
			pll->fout = 24000 * (1 << pll->fbrange) * (pll->fbdiv + pll->offset) / (pll->prediv + 1) / (1 << pll->postdiv);

			pr_debug("new inhouse pll setting %d-%d: %d %d %d %d %d\n", target, pll->fout,
				pll->postdiv, pll->prediv, pll->fbrange, pll->fbdiv, pll->offset);
			memcpy(&pll_tbl[0], pll, sizeof(pll_tbl[0]));	//overwrite generated settings
			ret = 0;
		}
		else
		{
			pr_err("%s[%d]FAILED freq:%d beyond [500M, 1.5G]\n", __func__, __LINE__, target);
		}
	}
	return ret;
}

static unsigned int inhouse_pll_getclock(void)
{
	//Fomula: Fout = Fin * PreFeedback_divider[P] * (Feedback_divider[F] + Feedback_divider_offset[O]) / Input_divider[N] / Post_divider[M])
	//Note: PreFeedback_divider[P] = 2 ^ fbrange
	//      Feedback_divider[F] = fbdiv
	//      Feedback_divider_offset[O] = offset
	//      Input_divider[N] = prediv + 1
	//      Post_divider[M] = 2 ^ postdiv
	//      Fin = 24000000

#define CLK_THRESHOLD 8000
	unsigned regval;
	int i, postdiv, prediv, fbrange, fbdiv, offset, ftarget, fout;

	if (SRC_XTAL_CLK == GET_CLK_SRC())
		return XTAL_CLK_KHZ;

	regval = Freq_ReadRegByte(PLL_CFG1_REG);
	postdiv = PLL_GET_SUB(regval, POSTDIV);
	prediv = PLL_GET_SUB(regval, PREDIV);
	fbrange = PLL_GET_SUB(regval, FBRANGE);
	regval = Freq_ReadRegByte(PLL_CFG2_REG);
	fbdiv = PLL_GET_SUB(regval, FBDIV);
	offset = 0;
	ftarget = fout = 24000 * (1 << fbrange) * (fbdiv + offset) / (prediv + 1) / (1 << postdiv);

	for(i=0; pll_tbl[i].ftarget != CPUFREQ_TABLE_END; i++)
	{
		if(CPUFREQ_ENTRY_INVALID == pll_tbl[i].ftarget)
			continue;
		if(abs(pll_tbl[i].fout - fout) < CLK_THRESHOLD)  //FIXME: CLK_THRESHOLD?
		{
			ftarget = pll_tbl[i].ftarget;
			break;
		}
	}
	pr_debug("getclock (inhouse) ftarget:%d, fout:%d, delta:%d\n", ftarget, fout, (pll_tbl[i].fout - fout));
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
		Freq_WriteRegByteMask(PLL_CFG0_REG, 0x40, 0xff);
		pr_debug("setclock (inhouse) prediv:%d, postdiv:%d, fbrange:%d, fbdiv:%d, offset:%d\n", 
				pll.prediv, pll.postdiv, pll.fbrange, pll.fbdiv, pll.offset);
		Freq_WriteRegByte(PLL_CFG1_REG, PLL_MK_CFG1(pll.postdiv, pll.prediv, pll.fbrange));
		Freq_WriteRegByteMask(PLL_CFG2_REG, pll.fbdiv, PLL_FBDIV_MASK);
		Freq_WriteRegByteMask(PLL_SELR_REG, pll.selr, PLL_SELR_MASK); //0x02
		Freq_WriteRegByteMask(PLL_SELI_REG, pll.seli, PLL_SELI_MASK); //0x0e
		Freq_WriteRegByteMask(PLL_SELP_REG, pll.selp, PLL_SELP_MASK); //0x36
		Freq_WriteRegByteMask(PLL_CFG0_REG, 0x41, 0xff);
		ms_delay(10);
		//For PLL_A9 setting, after update the setting, toggle the bypass bit in 1500ef00
		Freq_WriteRegByteMask(PLL_CFG0_REG, 1, PLL_BYPASS_MASK);
		ms_delay(10);
		Freq_WriteRegByteMask(PLL_CFG0_REG, 0, PLL_BYPASS_MASK);
		ms_delay(10);
		CLK_SCALING_POST();
	}
	return 0;
}

static int inhouse_pll_getconfig(struct pll_config* cfg)
{
	struct pll_descriptor *pll_desc = &sx8_inhouse_pll;
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
		pll_inuse = &sx8_inhouse_pll;
	} else {
		pr_err("unknown clock source [%d]\n", clk_src);
		return NULL;
	}

	return &pll_inuse->ops;
}
