/*
 *  linux/arch/arm/mach-trix/dcsn.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SoC DCSN Driver
 *
 * Author: Tony He, 2015.
 */

#define pr_fmt(fmt) "dcsndrv: " fmt

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
#include <asm/delay.h>

#include <mach/irqs.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <plat/io.h>
/*
 * DCSN registers (Register Base + Offset)
 * NOTE: For SX6/7. Different chip MAY has different I/O offset. Need further check.
 */

/* base address */
#define TRIX_PLF_DCSC_IO_BASE	0xf5002000

#define TRIX_DCSC_REG_LENGTH	0x2000

/* dcsn configure regs offset */
#define DCSC_ADDR		0x00C /*R*/
#define DCSC_STAT		0x010 /*R*/
#define DCSC_INT_CLR_EN		0xFD8 /*W*/
#define DCSC_INT_SET_EN		0xFDC /*W*/
#define DCSC_INT_STAT		0xFE0 /*R*/
#define DCSC_INT_EN		0xFE4 /*R/W*/
#define DCSC_INT_CLR_SEL	0xFE8 /*W*/
#define DCSC_INT_SET_SEL	0xFEC /*W*/

#define DCSN_WRITE_REG(a, v) WriteRegWord((void*)(TRIX_PLF_DCSC_IO_BASE + 0x1000 + (a)), v)
#define DCSN_READ_REG(a) ReadRegWord((void*)(TRIX_PLF_DCSC_IO_BASE + 0x1000 + (a)))

struct dcsn_ctrl{
	volatile u32 pad0[0x1000>>2];		/* +0x00000000 */
	volatile u32 bc_ctrl;			/* +0x00001000 */
	volatile u32 pad1[(0x100c-0x1004)>>2];	/* +0x00001004 */
	volatile u32 bc_addr;			/* +0x0000100c */
	volatile u32 bc_stat;			/* +0x00001010 */
	volatile u32 pad2[(0x1040-0x1014)>>2];	/* +0x00001014 */
	volatile u32 dcsc_feature;		/* +0x00001040 */
	volatile u32 pad3[(0x1200-0x1044)>>2];	/* +0x00001044 */
	volatile u32 cfg_dcs7_1_high;		/* +0x00001200 */
	volatile u32 cfg_dcs7_1_low;		/* +0x00001204 */
	volatile u32 pad4[(0x1fd8-0x1208)>>2];	/* +0x00001208 */
	volatile u32 bc_int_clr_en;		/* +0x00001fd8 */
	volatile u32 bc_int_set_en;		/* +0x00001fdc */
	volatile u32 bc_int_status;		/* +0x00001fe0 */
	volatile u32 bc_int_en;			/* +0x00001fe4 */
	volatile u32 bc_int_clr_sel;		/* +0x00001fe8 */
	volatile u32 bc_int_set_sel;		/* +0x00001fec */
	volatile u32 pad5[(0x1ffc-0x1ff0)>>2];	/* +0x00001ff0 */
	volatile u32 cfg_module_id;		/* +0x00001ffc */
};

/*
 * Interrupter handler for DCSN bus error and timeout.
 * Note: it's only for PLF CPU.
 *
 * [28:24] initiator ID, shows which master causing error.
 * [16:10] shows the error from which targets.
 * For the targets list, you can find it from 2.3.1 Address Allocation in TRM
 * [8] 1b'0: w, 1b'1: r.
 */
static const char *g_initiators[] =
{
#if defined(CONFIG_SIGMA_SOC_SX6)
#define CFG_NUM_INITIATOR 10
	"Coresight DAP",	/* 0 */
	"A9 SECURE READ",	/* 1 */
	"A9 SECURE WRITE",	/* 2 */
	"A9 NORMAL",		/* 3 */
	"I2C Slave",		/* 4 */
	"AV MIPS",		/* 5 */
	"MVED",			/* 6 */
	"DISP MIPS",		/* 7 */
	"Standby MCU",		/* 8 */
	"EJTAG MMIO",		/* 9 */
	NULL,
#elif defined(CONFIG_SIGMA_SOC_SX7)
#define CFG_NUM_INITIATOR 10
	"Coresight DAP",	/* 0 */
	"A9 SECURE READ",	/* 1 */
	"A9 SECURE WRITE",	/* 2 */
	"A9 NORMAL",		/* 3 */
	"I2C Slave",		/* 4 */
	"AV MIPS",		/* 5 */
	"MVED/HEVC",		/* 6 */
	"DISP MIPS",		/* 7 */
	"Standby MCU",		/* 8 */
	"EJTAG MMIO",		/* 9 */
	NULL,
#else
#define CFG_NUM_INITIATOR 0
	NULL,
#endif
};

/**
 * DCSN event selection
 * 1 - for bus error
 * 2 - for timeout
 * 3 - for both bus error and timeout
 */
static unsigned int dcs_sel = 0x1; /*defaultly, only check bus error.*/
static int __init do_with_dcsn_sel(char *str)
{
	dcs_sel = simple_strtoul(str, NULL, 10) & 0x3;
	pr_info("get selection %x\n", dcs_sel);
	return 0;
}
early_param("dcsn_sel", do_with_dcsn_sel);

static irqreturn_t dcs_plf_irq(unsigned int irq, void *dev, struct pt_regs *reg)
{
	char *dcsn_source = "PLF DCSN";
	unsigned int dcs_addr, dcs_stat;

	printk(KERN_INFO "\n===================================\n");
	printk(KERN_INFO "dcs-c: DSCN interrupt from %s\n", dcsn_source);

	//NOTE: Once clear interrupter, int_status will be lost.
	dcs_addr = DCSN_READ_REG(DCSC_ADDR);
	dcs_stat = DCSN_READ_REG(DCSC_STAT);

	/* status */
	printk(KERN_INFO "dcs-c: DCSC_ADDR 0x%x\n", dcs_addr);
	printk(KERN_INFO "dcs-c: DCSC_STAT 0x%x\n", dcs_stat);
	printk(KERN_INFO "dcs-c: DCSC_INT_STAT 0x%x\n", DCSN_READ_REG(DCSC_INT_STAT));

	{
		int indictor = (dcs_stat>>24)&0x1F; //[28:24]
		int op = (dcs_stat>>8)&0x1; //[8]

		printk(KERN_INFO "dcs-c: (!)DSCN error/timeout for [%s] tries to [%s] address [0x%x]!\n",
			(indictor>=0&&indictor<CFG_NUM_INITIATOR) ? g_initiators[indictor] : "Unkown initiator",
			(op==1) ? "read" : "write",
			dcs_addr );
	}

	/*
	 * Clear interrupter
	 */
	DCSN_WRITE_REG(DCSC_INT_CLR_SEL, dcs_sel);

	return IRQ_HANDLED;
}

static int __init trix_dcsn_init(void)
{
	int retval = 0;
        unsigned int timeout_sel;

        dcs_sel &= 0x3; /*mask -- bit0: error check; bit1: timeout check*/

	/*
	 * register IRQs for DCS on A9
	 */
	retval = request_irq(TRIHIDTV_PLF_DCS_INTERRUPT,(void *)dcs_plf_irq, IRQF_DISABLED/*IRQF_SHARED*/, "DCSN", NULL);
	pr_debug("<1> register PLF DCS IRQ (%d).\n", retval);

	/*
	 * enable DCS interrupt
	 */
	timeout_sel = (dcs_sel & 0x2) ? 1 : 0; /*timeout interrupt set or unchanged*/

	DCSN_WRITE_REG(DCSC_INT_SET_EN, timeout_sel);
	DCSN_WRITE_REG(DCSC_INT_EN, dcs_sel );

	return 0;
}

static void __exit trix_dcsn_exit(void)
{
	DCSN_WRITE_REG(DCSC_INT_CLR_EN, 3);
	free_irq(TRIHIDTV_PLF_DCS_INTERRUPT, NULL);
}

MODULE_DESCRIPTION("DCSN driver for "CONFIG_SIGMA_SOC_NAME" SoCs");
MODULE_LICENSE("GPL");
module_init(trix_dcsn_init);
module_exit(trix_dcsn_exit);
