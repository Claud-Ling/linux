#include <linux/linkage.h>    
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/hardware/cache-l2x0.h>

#include "pm.h"

/*
 * void sx6_flush_cache(void);
 * Flush all data from L1 and L2 data cache
 */
ENTRY(sx6_flush_cache)
	stmfd	sp!, {r4 - r11, lr}
	/*
	 * Flush all data from L1 data cache.
	 */
	bl	v7_flush_dcache_all

	ldmfd	sp!, {r4 - r11, pc}
ENDPROC(sx6_flush_cache)

/*
 * =============================
 * == CPU suspend finisher == 
 * =============================
 *
 * void sx6_finish_suspend(unsigned long cpu_state)                                                                               
 *
 *
 * @cpu_state :  (reserv)
 *	 contains context save state (r0)
 * @return: This function never returns for CPU OFF and DORMANT power states. 
 */
ENTRY(sx6_finish_suspend)
        stmfd   sp!, {r4 - r11,lr}	@save register on stack
	
/*
 * prepare to run code in SRAM
 */
	bl	sx6_flush_cache
	bl	sx6_flush_tlb

	mov	r4, sp
	ldr	r5, sram_sp_base_addr
	ldr	sp, [r5]
	bic     sp, sp, #7      @8-byte alignment for ABI compliance
	stmfd	sp!, {r4}

	ldr     r4, sx6_do_wfi_sram_addr
	ldr	r5, [r4]
	cmp	r5, #0

/***********************************/
	ldr	r5, =IRAM_BASE		@save physical entry addr
	
	ldr	sp, =IRAM_SP_BASE
	bic	sp, sp, #7		@8-byte alignment for ABI compliance
        /*
         * Invalidate L1 I/D  
         */
        mov     r0, #0                  @ set up for MCR
        mcr     p15, 0, r0, c8, c7, 0   @ invalidate TLBs
        mcr     p15, 0, r0, c7, c5, 0   @ invalidate icache
        mcr     p15, 0, r0, c7, c5, 6   @ invalidate BP array
        mcr     p15, 0, r0, c7, c10, 4  @ DSB  
        mcr     p15, 0, r0, c7, c5, 4   @ ISB                                                                                       

        /*
         * disable MMU stuff and caches 
         */
        mrc     p15, 0, r0, c1, c0, 0  
        bic     r0, r0, #0x00002000     @ clear bits 13 (--V-)
        bic     r0, r0, #0x00000007     @ clear bits 2:0 (-CAM)
        orr     r0, r0, #0x00000002     @ set bit 1 (--A-) Align
        @orr     r0, r0, #0x00000800     @ set bit 11 (Z---) BTB 
	bic     r0, r0, #0x00001000     @ clear bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0	@ turn MMU and cache off
	
	mov	pc, r5			@  jump to the WFI code in SRAM
		
/***********************************/
/*
 * CPU is here when it failed to enter OFF/DORMANT or
 * no low power state was attempted.
 */  
	ldmfd	sp!, {r4}
	mov	sp, r4

/*
 * ===================================
 * == Exit point from non-OFF modes ==
 * ===================================
 */
       ldmfd   sp!, {r4 - r11, pc}     @ restore regs and return
ENDPROC(sx6_finish_suspend)

/*
 * Local variables
 */
sx6_do_wfi_sram_addr:
        .word sx6_do_wfi_sram
kernel_flush:
        .word v7_flush_dcache_all
sram_sp_base_addr:
	.word sx6_sp_base_sram


/* ===================================
 * == WFI instruction => Enter idle ==
 * ===================================
 */

/*
 * Do WFI instruction
 *
 * This code gets copied to internal SRAM and is accessible
 * from both SDRAM and SRAM:
 * - executed from SRAM  (sx6_do_wfi_sram),DDR self-refresh mode
 * - executed from SDRAM (sx6_do_wfi),continue with wfi.
 */
        .align  3
ENTRY(sx6_do_wfi)
	stmfd   sp!, {lr}
	isb
	dsb
	dmb
	bl	s2ramctl_sdram_in_selfrefresh
	cmp	r0, #0
	bne	1f			@ failed, abandon
	bl	s2ramctl_system_poweroff	
	/*****************3*******/
	isb
        /* Data memory barrier and Data sync barrier */
        dsb
        dmb

        wfi                             @ wait for interrupt

        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
1:
       ldmfd   sp!, {pc}     @ return from non-OFF mode
ENDPROC(sx6_do_wfi)

#include "s2ramctrl.S"
ENTRY(sx6_do_wfi_sz)
        .word   . - sx6_do_wfi




/*
 * ============================
 * == CPU resume entry point ==
 * ============================
 *
 * void sx6_cpu_resume(void)
 *
 * preboot code jumps to this function while waking up from CPU
 * OFF or DORMANT state. Physical address of the function is
 * stored in the MCU(?)  while entering to OFF or DORMANT mode.
 *
 */
/*
 *	 The following code is located into the .data section. This is to
 *	 allow l2x0_regs_phys to be accessed with a relative load while we
 *	 can't rely on any MMU translation. We could have put l2x0_regs_phys
 *	 in the .text section as well, but some setups might insist on it to
 *	 be truly read-only. (Reference from: arch/arm/kernel/sleep.S)
 */
	.data
	.align

	/*
	 * resume magic, to allow the bootloader to check for an valid
	 * image to resume to. Must be the first word before the
	 * sx6_cpu_resume entry.
	 */

	.word	0x8bdef50d
ENTRY(sx6_cpu_resume)
	b	cpu_resume	@ Jump to generic resume
ENDPROC(sx6_cpu_resume)
