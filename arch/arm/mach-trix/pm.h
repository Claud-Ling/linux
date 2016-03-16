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
#ifndef __ARCH_ARM_MACH_SX6_PM_H__
#define __ARCH_ARM_MACH_SX6_PM_H__

#include <asm-generic/sizes.h>

/*
 * CPU power status
 */
#define CPU_PM_L1_LOST (1 << 0)	/*L1 cache and logic lost only*/
#define CPU_PM_L2_LOST (1 << 1)	/*L2 cache lost only*/
#define CPU_PM_OFF (1 << 2)	/*power lost*/

#define CPU_PWSTS_NONE	0
#define CPU_PWSTS_INACTIVE CPU_PM_L1_LOST
#define CPU_PWSTS_OFF	(CPU_PM_L1_LOST | CPU_PM_L2_LOST | CPU_PM_OFF)

/*
 * standard place to map on-chip SRAMs; they *may* support DMA
 * +-----------------+-----------+ 0
 * |                 |           |
 * |    IRAM_TEXT    | text,data |
 * |                 |           |
 * +-----------------+-----------+
 * |    IRAM_CPU_SP  | stack for |
 * |     *NR_CPUS    | cpus idle |
 * +-----------------+-----------+ 12k
 * |  IRAM_MAIN_SP   | stack for |
 * |                 | s2ram code|
 * +-----------------+-----------+ 16k
 */
#define IRAM_SIZE		SZ_16K
#define IRAM_MAIN_SP_SZ		SZ_2K	/* stack for s2ram code */
#define IRAM_STACK_BITS_PER_CPU	8
#define IRAM_CPU_SP_SZ		(1 << IRAM_STACK_BITS_PER_CPU) /* stack for idle code, 256 byte*/
#define IRAM_SP_SIZE		(IRAM_MAIN_SP_SZ + IRAM_CPU_SP_SZ * NR_CPUS)
#define IRAM_TEXT_SIZE		(IRAM_SIZE - IRAM_SP_SIZE)
#define IRAM_BASE		0xFFFE0000
#define IRAM_SP_BASE		(IRAM_BASE + IRAM_SIZE - 4)

#ifndef __ASSEMBLY__ 
#if 0
/* Macro to push a function to the internal SRAM, using the fncpy API */
#define trix_iram_push(funcp, size) ({                          \
        typeof(&(funcp)) _res = NULL;                           \
        void *_iram_virt_addr =  __arm_ioremap_exec(IRAM_BASE,IRAM_TEXT_SIZE,1);     \
        if (_iram_virt_addr)                                      \
                _res = fncpy(_iram_virt_addr, &(funcp), size);    \
        _res;                                                   \
})
#endif

extern int trix_finish_suspend(long unsigned int);

/* trix_do_wfi function pointer and size, for copy to SRAM */
extern void trix_do_wfi(void);
extern unsigned int trix_do_wfi_sz;
extern unsigned int trix_v2p_offset;
/* ... and its pointer from SRAM after copy */
extern void (*trix_do_wfi_sram)(void);
extern void *trix_sp_base_sram;

extern void trix_cpu_resume(void);

#ifdef CONFIG_TRIX_PM_CRC
#define TRIX_PMDBG(fmt...) printk(fmt)
extern void trix_pm_check_prepare(void);
extern void trix_pm_check_restore(void);
extern void trix_pm_check_cleanup(void);
extern void trix_pm_check_store(void);
#else
#define trix_pm_check_prepare() do { } while(0)
#define trix_pm_check_restore() do { } while(0)
#define trix_pm_check_cleanup() do { } while(0)
#define trix_pm_check_store()   do { } while(0)
#endif
#endif /* __ASSEMBLY__ */ 
#endif
