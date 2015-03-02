/*
 * arch/arm/mach-sx6/pm.c
 * SX6 Power Management
 *
 * Copyright (C) 2005 David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/gpio.h>
#include <linux/suspend.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <asm/cacheflush.h>

#include <linux/atomic.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>
#include <asm/suspend.h>
#include <asm/memory.h>
#include <asm/tlbflush.h> 
#include <asm/fncpy.h>
#include "pm.h"
#include "s2ramctrl.h"
#include "common.h"

/* Macro to map iram*/
#define SX6_IRAM_REMAP()	do{					\
	sx6_iram_base = __arm_ioremap_exec(IRAM_BASE,IRAM_SIZE,1);	\
}while(0)
/* Macro to push a function to the internal SRAM, using the fncpy API */
#define SX6_IRAM_PUSH(funcp, size) ({                                   \
        typeof(&(funcp)) _res = NULL;                                   \
        if (sx6_iram_base)                                              \
                _res = fncpy((void*)sx6_iram_base, &(funcp), size);     \
        _res;                                                           \
})

/* Macro to get sp base in iram*/
#define SX6_IRAM_SP()	({             \
	void* _ret = (sx6_iram_base) ? \
	(sx6_iram_base + IRAM_SIZE - 4)\
	: NULL;                        \
	_ret;                          \
})

void (*sx6_do_wfi_sram)(void);
void *sx6_sp_base_sram = NULL;

static suspend_state_t target_state;
static void __iomem * sx6_iram_base = NULL;
static struct s2ram_resume_frame sx6_resume_frame =
{
	{0x0badc0de,}
};

/*
 * make sure TLB contain the addr we want
 */
void sx6_flush_tlb(void)
{
	flush_cache_all();
	local_flush_tlb_all();
}

/*defined in sleep.S*/
extern void s2ramctl_set_resume_entry(void* entry);
static void set_sx6_wakeup_addr(unsigned int phys_addr)
{
	printk("Wakeup addr = 0x%08x\n",phys_addr);
	sx6_resume_frame.S2RAM_ENTRY = phys_addr;
	flush_cache_all();
	outer_clean_range(virt_to_phys(&sx6_resume_frame.S2RAM_ENTRY), 
		virt_to_phys(&sx6_resume_frame.S2RAM_ENTRY) + sizeof(sx6_resume_frame.S2RAM_ENTRY));

	s2ramctl_set_resume_entry((void *)virt_to_phys(&sx6_resume_frame));
}
/*
 * Called after processes are frozen, but before we shutdown devices.
 */
static int sx6_pm_begin(suspend_state_t state)
{
	pr_debug("[%d]%s\n",__LINE__,__func__);
	target_state = state;
	return 0;
}

static void sx6_do_suspend(void)
{
	int ret = 0;
	printk("[%d]%s relocate size=%x\n",__LINE__,__func__,sx6_do_wfi_sz);
	set_sx6_wakeup_addr(virt_to_phys(sx6_cpu_resume));
	
	sx6_do_wfi_sram = SX6_IRAM_PUSH(sx6_do_wfi,sx6_do_wfi_sz);
	sx6_sp_base_sram = SX6_IRAM_SP();
	
	sx6_pm_check_store();
	
	/*flush L1 L2 cache back to RAM*/
	flush_cache_all();
	outer_disable();

	/*Zzzzzz*/
	ret = cpu_suspend(0,sx6_finish_suspend);
	
	/*restore L2X0*/
	outer_resume();

	sx6_pm_check_restore();
	/*
 	 * In case resume from OFF mode
 	 */
	if (0 == ret)
	{
#ifdef CONFIG_SMP
		platform_smp_resume_cpus();
#endif
	}

	/* we should never get past here */
	//panic("sleep resumed to originator?");
	return;
}
static int sx6_pm_enter(suspend_state_t state)
{	

	printk("[%d]%s\n",__LINE__,__func__);

	switch (state) {
		case PM_SUSPEND_MEM:
		case PM_SUSPEND_STANDBY:
			/*
			 * NOTE: the Wait-for-Interrupt instruction needs to be
			 * in icache so no SDRAM accesses are needed until the
			 * wakeup IRQ occurs and self-refresh is terminated.
			 */
			sx6_do_suspend();
			break;

		default:
			pr_debug("SX6: PM - bogus suspend state %d\n", state);
			goto error;
	}

error:
	target_state = PM_SUSPEND_ON;
	return 0;
}

/*
 * Called right prior to thawing processes.
 */
static void sx6_pm_end(void)
{
	pr_debug("[%d]%s\n",__LINE__,__func__);
	target_state = PM_SUSPEND_ON;
}

static int sx6_pm_prepare(void)
{
	/* prepare check area if configured */
	sx6_pm_check_prepare();
	return 0;
}
 
static void sx6_pm_finish(void)
{
	sx6_pm_check_cleanup();
}
static const struct platform_suspend_ops sx6_pm_ops = {
	.valid	= suspend_valid_only_mem,
	.prepare= sx6_pm_prepare,
	.finish = sx6_pm_finish,
	.begin	= sx6_pm_begin,
	.enter	= sx6_pm_enter,
	.end	= sx6_pm_end,
};

static int __init sx6_pm_init(void)
{

	pr_info("SX6: Power Management\n");
	
	SX6_IRAM_REMAP();
	suspend_set_ops(&sx6_pm_ops);

	return 0;
}
arch_initcall(sx6_pm_init);
