#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif
#include <asm/memory.h>
#include <mach/smp.h>

#include "pm.h"

.align 5
.pushsection	.idmap.text,"ax"
/*
 * void trix_call_with_stack_nommu(void(*fn)(void* arg), void* arg, void* stack);
 * fn and stack shall be given in physical addr form
 */
ENTRY(trix_call_with_stack_nommu)
	mrc	p15, 0, r3, c1, c0, 0		@ ctrl register
	bic	r3, r3, #0x1			@ clear M bit
	mcr	p15, 0, r3, c1, c0, 0		@ disable MMU
	isb

	str	sp, [r2, #-4]!
	str	lr, [r2, #-4]!
	mov	sp, r2
	mov	r2, r0
	mov	r0, r1
	badr	lr, 1f
	mov	pc, r2				@ fn(arg)

1:	ldr	lr, [sp]
	ldr	sp, [sp, #4]

	mrc	p15, 0, r2, c1, c0, 0		@ ctrl register
	orr	r2, r2, #0x1			@ set M bit
	mcr	p15, 0, r2, c1, c0, 0		@ enable MMU
	isb
	nop

	mov	pc, lr
ENDPROC(trix_call_with_stack_nommu)

.popsection

/*
 * =============================
 * == CPU suspend finisher ==
 * =============================
 *
 * void trix_finish_suspend(unsigned long cpu_state)
 *
 * @cpu_state :  (reserv)
 *	contains context save state (r0)
 *	CPU_PWSTS_NONE     - none
 *	CPU_PWSTS_INACTIVE - L1 and logic lost only
 *	CPU_PWSTS_OFF      - power lost
 * @return: This function never returns for CPU OFF and DORMANT power states. 
 */
ENTRY(trix_finish_suspend)
        stmfd   sp!, {r4 - r11,lr}	@save register on stack
	mov	r4, r0			@backup power state
	mrc	p15, 0, r5, c0, c0, 5
	and	r5, r5, #0xf		@cpu id
	cmp	r0, #CPU_PWSTS_NONE
	beq	Do_idle

	bl	cpu_enter_lowpower	@turn off dcache and take cpu out of coherency

	/*
	 * Only boot core is allowed to execute L2 cleanup and power-off code
	 */
	cmp	r5, #0
	bne	Do_idle

#ifdef CONFIG_CACHE_L2X0
	tst	r4, #CPU_PM_L2_LOST
	beq	skip_cleanup_l2
	/* TODO: Clean and invalidate the L2 cache */

skip_cleanup_l2:
#endif
	tst	r4, #CPU_PM_OFF
	beq	Do_idle

	/*
	 * prepare to run code in SRAM
	 */

	/*
	 * fixup v2p offset
	 */
	bl	trix_fixup_v2p_offset

	/*
	 * switch to identity mapping, for mmu off (on) code
	 */
	bl	setup_mm_for_reboot

	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00003000	@ clear bits 13,12 (--V,I-)
	bic	r0, r0, #0x00000002	@ clear bits 1 (--A-)
	mcr	p15, 0, r0, c1, c0, 0	@ turn icache off
	isb

	adr	r6, __trix_sram_loc
	ldmia	r6, {r0 - r3, r6}
	add	r3, r3, r6		@ phys trix_call_with_stack_nommu
	blx	r3

	/*
	 * CPU is here when it failed to enter OFF/DORMANT or
	 * no low power state was attempted.
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x00003000	@ set bits 13,12 (--V,I-)
	orr	r0, r0, #0x00000002	@ set bits 1 (--A-)
	mcr	p15, 0, r0, c1, c0, 0	@ turn icache on
	isb

	/*
	 * recovery page tables
	 */
	badr	lr, Do_exit
	b	trix_recovery_mm

Do_idle:
	badr	lr, Do_exit
	tst	r4, #CPU_PM_OFF
	beq	trix_do_idle

	/* have to execute do_idle in SRAM */
	badr	r3, trix_do_wfi_sram_addr
	ldmia	r3, {r6, r7}
	ldr	r0, [r6]
	add	r0, r0, r7		@ trix_do_idle"_sram"

	/* null param */
	mov	r1, #0

	/* fixup stack for processor in SRAM */
	ldr	r6, sram_sp_base_addr
	ldr	r6, [r6]
	sub	r6, r6, #IRAM_MAIN_SP_SZ
	lsl	r7, r5, #IRAM_STACK_BITS_PER_CPU
	sub	r2, r6, r7

	/*
	 * Continue execution in SRAM with replaced stack.
	 * Note that we only have a small stack on SRAM for do_idle,
	 * so should only do as little thing as possible in there
	 */
	ldr	r6, aux_boot_id
	ldr	r6, [r6]		@ fake load just to fill TLB if possible

	b	call_with_stack
/*
 * ===================================
 * == Exit point from non-OFF modes ==
 * ===================================
 */
Do_exit:
	bl	cpu_leave_lowpower	@ turn on dcache and bring cpu back in coherency if needed
	ldmfd   sp!, {r4 - r11, pc}     @ restore regs and return
ENDPROC(trix_finish_suspend)

/*
 * Local variables
 */
trix_do_wfi_sram_addr:
	.word trix_do_wfi_sram
	.word trix_do_idle - trix_do_wfi
kernel_flush:
	.word v7_flush_dcache_all
sram_sp_base_addr:
	.word trix_sp_base_sram

__trix_sram_loc:
	.word IRAM_BASE			@ phy fn addr
	.word 0				@ arg
	.word (IRAM_SP_BASE & ~(0x7))	@ phy stack
	.word trix_call_with_stack_nommu
trix_v2p_offset:
	.global trix_v2p_offset
	.word (0 - PAGE_OFFSET) 	@ virt->phys offset.
					@ Note: can't put PHYS_OFFSET here. Just give 0
					@ instead as it will be fixup any way

/* ===================================
 * == WFI instruction => Enter idle ==
 * ===================================
 */

/*
 * Do WFI instruction
 *
 * This code gets copied to internal SRAM and is accessible
 * from both SDRAM and SRAM:
 * OFF Mode     - executed from SRAM  (trix_do_wfi_sram),DDR self-refresh mode
 * Non-OFF Mode - executed from SDRAM (trix_do_wfi),continue with wfi.
 */
        .align  3
ENTRY(trix_do_wfi)
	stmfd   sp!, {lr}
	isb
	dsb
	dmb
	bl	s2ramctl_sdram_in_selfrefresh
	cmp	r0, #0
	bne	2f		@ failed, abandon
	bl	s2ramctl_system_poweroff
1:	bl	trix_do_idle
	b	1b		@ just trap here as we have no choice other than wait for OFF
2:	ldmfd   sp!, {pc}	@ return from non-OFF mode
ENDPROC(trix_do_wfi)

ENTRY(trix_do_idle)
	isb
	/* Data memory barrier and Data sync barrier */
	dsb
	dmb

	/*
	 * CPU can specualatively prefetch the instructions so add NOPs
	 * after WFI. Sixteen NOPs as per Cortex-A9 pipeline.
	 */
1:	wfi			@ wait for interrupt

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
	nop
	nop
	nop
	nop
	nop
	nop

	mrc	p15, 0, r0, c0, c0, 5
	ands	r0, r0, #0xf
	beq	2f

	/*
	 * For secondary cores: check if we are
	 * released from the holding pen.
	 */
	ldr	r1, aux_boot_id
	ldr	r1, [r1]
	and	r1, r1, #0xf
	cmp	r0, r1
	bne	1b

2:	mov	pc, lr		@ return from non-OFF mode
ENDPROC(trix_do_idle)

/*
 * just deference to the duplicated aux boot id register,
 * as the cpu is now out of coherency and thus can't
 * access to aux_boot_addr secure register via smc call.
 */
aux_boot_id:
	.word	AUX_BOOT_ID_REG

#include "s2ramctrl.S"
ENTRY(trix_do_wfi_sz)
        .word   . - trix_do_wfi


/*
 * ============================
 * == CPU resume entry point ==
 * ============================
 *
 * void trix_cpu_resume(void)
 *
 * preboot code jumps to this function while waking up from CPU
 * OFF or DORMANT state. Physical address of the function is
 * stored in the MCU(?)  while entering to OFF or DORMANT mode.
 *
 * Note that resume_entry should locate at 16-byte aligned addr
 *
 */
/*
 *	 The following code is located into the .cpuint.data section. This is
 *	 to allow pen_release_loc to be accessed with a relative load while we
 *	 can't rely on any MMU translation. We could have put pen_release_loc
 *	 in the .text section as well, but some setups might insist on it to
 *	 be truly read-only. (Reference from: arch/arm/kernel/sleep.S)
 */
	.pushsection .cpuinit.data, "ax"
	.align 4

	/*
	 * resume magic, to allow the bootloader to check for an valid
	 * image to resume to. Must be the first word before the
	 * trix_cpu_resume entry.
	 */

	.word	0x0bedf00d
	.word	0x1bedf00d
	.word	0x2bedf00d
	.word	0x8bdef50d
ENTRY(trix_cpu_resume)
	mrc	p15, 0, r0, c0, c0, 5
	ands	r0, r0, #0xf		@ get CPUID
	beq	2f
	badr	r4, pen_release_loc
	ldmia	r4, {r5, r6}
	sub	r4, r4, r5
	add	r6, r6, r4
1:
	ldr	r7, [r6]
	cmp	r7, r0
	bne	1b
2:
	b	cpu_resume		@ Jump to generic resume
ENDPROC(trix_cpu_resume)

pen_release_loc:
	.long	.
	.long	pen_release
	.popsection

