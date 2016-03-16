/*
 * arch/arm/mach-trix/pm.c
 * TRIX Power Management
 *
 * Copyright (C) 2005 David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define pr_fmt(fmt) "pm: " fmt

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
#include "s2ramctrl.h"
#include "common.h"

/* Macro to map iram*/
#define TRIX_IRAM_REMAP()	do{					\
	trix_iram_base = __arm_ioremap_exec(IRAM_BASE,IRAM_SIZE,1);	\
}while(0)
/* Macro to push a function to the internal SRAM, using the fncpy API */
#define TRIX_IRAM_PUSH(funcp, size) ({                                   \
	typeof(&(funcp)) _res = NULL;                                   \
	if (trix_iram_base) {                                            \
		if ((size) > IRAM_TEXT_SIZE)				\
			pr_err("SRAM push failed! Ask %x\n", size);	\
		else							\
			_res = fncpy((void*)trix_iram_base, &(funcp), size);\
	}								\
        _res;                                                           \
})

/* Macro to get sp base in iram*/
#define TRIX_IRAM_SP()	({             \
	void* _ret = (trix_iram_base) ? \
	(trix_iram_base + IRAM_SIZE - 4)\
	: NULL;                        \
	_ret;                          \
})

void (*trix_do_wfi_sram)(void);
void *trix_sp_base_sram = NULL;

static suspend_state_t target_state = PM_SUSPEND_ON;
static void __iomem * trix_iram_base = NULL;
static struct s2ram_resume_frame trix_resume_frame =
{
	{0x0badc0de,}
};

/*
 * This is a workaround patch as assembly can't reference
 * PHYS_OFFSET under arch/arm due to CONFIG_ARM_PATCH_PHYS_VIRT
 */
void trix_fixup_v2p_offset(void)
{
	trix_v2p_offset = PHYS_OFFSET - PAGE_OFFSET;
	return;
}


/*
 * switch back to the correct page tables
 * (when finisher failed to enter OFF/DORMANT mode)
 */
void trix_recovery_mm(void)
{
	struct mm_struct *mm = current->active_mm;
	cpu_switch_mm(mm->pgd, mm);
	local_flush_bp_all();
	local_flush_tlb_all();
}

/*defined in sleep.S*/
extern void s2ramctl_set_resume_entry(void* entry);
static void set_trix_wakeup_addr(unsigned int phys_addr)
{
	pr_info("Wakeup addr = 0x%08x\n",phys_addr);
	trix_resume_frame.S2RAM_ENTRY = phys_addr;
	flush_cache_all();
	outer_clean_range(virt_to_phys(&trix_resume_frame.S2RAM_ENTRY),
		virt_to_phys(&trix_resume_frame.S2RAM_ENTRY) + sizeof(trix_resume_frame.S2RAM_ENTRY));

	s2ramctl_set_resume_entry((void *)virt_to_phys(&trix_resume_frame));
}

/*
 * push do_wfi to SRAM
 * Note that SRAM content shall be kept during power on time
 */
static void trix_sram_restore_context(void)
{
	set_trix_wakeup_addr(virt_to_phys(trix_cpu_resume));
	trix_do_wfi_sram = TRIX_IRAM_PUSH(trix_do_wfi,trix_do_wfi_sz);
	trix_sp_base_sram = TRIX_IRAM_SP();
}

static int trix_pm_valid(suspend_state_t state)
{
	switch(state) {
		case PM_SUSPEND_STANDBY:
		case PM_SUSPEND_MEM:
			return 1;
		default:
			return 0;
	}
}

/*
 * Called after processes are frozen, but before we shutdown devices.
 */
static int trix_pm_begin(suspend_state_t state)
{
	pr_debug("[%d]%s state %d\n",__LINE__,__func__,state);
	target_state = state;
	return 0;
}

static void trix_do_suspend(void)
{
	int ret = 0;
	pr_info("[%d]%s relocate size=%x\n",__LINE__,__func__,trix_do_wfi_sz);

	trix_pm_check_store();

	/*flush L1 L2 cache back to RAM*/
	flush_cache_all();
	outer_disable();

	/*Zzzzzz*/
	ret = cpu_suspend(CPU_PWSTS_OFF,trix_finish_suspend);

	/*restore L2X0*/
	outer_resume();

	trix_pm_check_restore();
	/*
 	 * In case resume from OFF mode
 	 */
	if (0 == ret)
	{
		trix_sram_restore_context();
#ifdef CONFIG_SMP
		platform_smp_resume_cpus();
#endif
	}

	return;
}


static void trix_do_idle(void)
{
	pr_info("=====>system enter low power mode\n");
	/*Zzzzzz*/
	cpu_suspend(CPU_PWSTS_INACTIVE,trix_finish_suspend);
	pr_info("<=====system leave low power mode\n");
}

static int trix_pm_enter(suspend_state_t state)
{
	pr_info("[%d]%s\n",__LINE__,__func__);

	switch (state) {
		case PM_SUSPEND_MEM:
			/*
			 * NOTE: the Wait-for-Interrupt instruction needs to be
			 * in icache so no SDRAM accesses are needed until the
			 * wakeup IRQ occurs and self-refresh is terminated.
			 */
			trix_do_suspend();
			break;
		case PM_SUSPEND_STANDBY:
			trix_do_idle();
			break;
		default:
			pr_debug("%s: PM - bogus suspend state %d\n", trix_board_name(), state);
			goto out;
	}

out:
	target_state = PM_SUSPEND_ON;
	return 0;
}

/*
 * Called right prior to thawing processes.
 */
static void trix_pm_end(void)
{
	pr_debug("[%d]%s\n",__LINE__,__func__);
	target_state = PM_SUSPEND_ON;
}

static int trix_pm_prepare(void)
{
	/* prepare check area if configured */
	trix_pm_check_prepare();
	return 0;
}

static void trix_pm_finish(void)
{
	trix_pm_check_cleanup();
}
static const struct platform_suspend_ops trix_pm_ops = {
	.valid	= trix_pm_valid,
	.prepare= trix_pm_prepare,
	.finish = trix_pm_finish,
	.begin	= trix_pm_begin,
	.enter	= trix_pm_enter,
	.end	= trix_pm_end,
};

static int __init trix_pm_init(void)
{

	pr_info("%s: Power Management\n", trix_board_name());

	TRIX_IRAM_REMAP();
	trix_sram_restore_context();
	suspend_set_ops(&trix_pm_ops);

	return 0;
}
arch_initcall(trix_pm_init);

suspend_state_t trix_get_suspend_state(void)
{
	return target_state;
}
EXPORT_SYMBOL(trix_get_suspend_state);
