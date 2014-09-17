/*
 * OMAP2/3 Power Management Routines
 *
 * Copyright (C) 2008 Nokia Corporation
 * Jouni Hogander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */    
#ifndef __ARCH_ARM_MACH_SX6_PM_H
#define __ARCH_ARM_MACH_SX6_PM_H

#include <asm-generic/sizes.h>

/* standard place to map on-chip SRAMs; they *may* support DMA */
#define IRAM_SIZE       SZ_16K
#define IRAM_SP_SIZE	SZ_4K
#define IRAM_TEXT_SIZE	(IRAM_SIZE - IRAM_SP_SIZE)
#define IRAM_BASE       0xFFFE0000
#define IRAM_SP_BASE	(IRAM_BASE + IRAM_SIZE - 4)

#ifndef __ASSEMBLY__ 
#if 0
/* Macro to push a function to the internal SRAM, using the fncpy API */
#define sx6_iram_push(funcp, size) ({                          \
        typeof(&(funcp)) _res = NULL;                           \
        void *_iram_virt_addr =  __arm_ioremap_exec(IRAM_BASE,IRAM_TEXT_SIZE,1);     \
        if (_iram_virt_addr)                                      \
                _res = fncpy(_iram_virt_addr, &(funcp), size);    \
        _res;                                                   \
})
#endif

extern int sx6_finish_suspend(long unsigned int);

/* sx6_do_wfi function pointer and size, for copy to SRAM */
extern void sx6_do_wfi(void);
extern unsigned int sx6_do_wfi_sz;
/* ... and its pointer from SRAM after copy */
extern void (*sx6_do_wfi_sram)(void);
extern void *sx6_sp_base_sram;

extern void sx6_cpu_resume(void);

#ifdef CONFIG_SIGMA_PM_CRC
#define SX6_PMDBG(fmt...) printk(fmt)
extern void sx6_pm_check_prepare(void);
extern void sx6_pm_check_restore(void);
extern void sx6_pm_check_cleanup(void);
extern void sx6_pm_check_store(void);
#else
#define sx6_pm_check_prepare() do { } while(0)
#define sx6_pm_check_restore() do { } while(0)
#define sx6_pm_check_cleanup() do { } while(0)
#define sx6_pm_check_store()   do { } while(0)
#endif
#endif /* __ASSEMBLY__ */ 
#endif
