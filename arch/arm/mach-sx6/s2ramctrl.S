/*
 *  s2ramctrl.S
 *
 *  Copyright (c) 2014 Sigma Designs Limited
 *   tony_he@sigmadesigns.com
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Suspend to RAM low level control interfaces for DTV Soc (SX6)
 */

#if defined(__PREBOOT__)
# include <config.h>
# include <assembler.h>
# define __CONFIG_SRAM_RESUME__
# define trace_str	bl	serial_puts
# define trace_int	bl	put32
# define trace_ch	bl	serial_putc
#elif defined(__UBOOT__)
# include <config.h>
# define __CONFIG_SRAM_SUSPEND__
# define trace_str
# define trace_int
# define trace_ch
#elif defined(__KERNEL__)
# include <linux/linkage.h>
# include <asm/assembler.h>
# define __CONFIG_SRAM_SUSPEND__
# define trace_str
# define trace_int
# define trace_ch
#endif


/*
 * common compiler options
 */
#define MCUCOMM_SUPPORT_TIMEOUT
//#define HOST_CTRL_STB_GPIO

/*
 * command from host to mcu
 */
#define HOST_CMD_RMODE_ENABLE	0x75
#define HOST_CMD_POWEROFF	0x76
#define HOST_CMD_RMODE_DISABLE	0x77

/*
 * command from mcu to host
 */
#define MCU_CMD_POWERMODE	0x90
#define MCU_CMD_ACK_RMODE_ON	0x91
#define MCU_CMD_ACK_RMODE_OFF	0x92

/*
 * MCU registers
 */
#define MCU_REG_BASE		0xf5000000
#define MCU_REG_GPIO_OFS	0xa

/*
 * share reg M0~M3
 *  HOST R/W, MCU RO
 */
#define SHREG_MBASE_OFS 0x10
#define SHREG_M0_OFS SHREG_MBASE_OFS
#define SHREG_M1_OFS (SHREG_MBASE_OFS + 1)
#define SHREG_M2_OFS (SHREG_MBASE_OFS + 2)
#define SHREG_M3_OFS (SHREG_MBASE_OFS + 3)

/*
 * share reg E0~E5
 *  MCU R/W, HOST RO
 */
#define SHREG_EBASE_OFS 0x30
#define SHREG_E0_OFS SHREG_EBASE_OFS
#define SHREG_E1_OFS (SHREG_EBASE_OFS + 1)
#define SHREG_E2_OFS (SHREG_EBASE_OFS + 2)
#define SHREG_E3_OFS (SHREG_EBASE_OFS + 3)
#define SHREG_E4_OFS (SHREG_EBASE_OFS + 4)
#define SHREG_E5_OFS (SHREG_EBASE_OFS + 5)

/*timer 0*/
#define TIMER0_REG_BASE		0xf5027000
#define TIMER0_OFS_TCVRN	0x00
#define TIMER0_OFS_TRVRN	0x04
#define TIMER0_OFS_TCRN		0x08

/*
 * PCTL registers
 */
#define PCTL_REG_UMAC0_BASE	0xf5021000
#define PCTL_REG_UMAC1_BASE	0xf5023000
#define	PCTL_OFS_SCTL		0x004
#define PCTL_OFS_STAT		0x008
#define PCTL_OFS_TREFI		0x0D0
#define PCTL_OFS_PHYPVTUPDI	0x318
#define PCTL_OFS_PVTUPDI	0x328

/*
 * PCTL CMD/STATE
 */
#define PCTL_CMD_INIT	0x0
#define PCTL_CMD_CFG	0x1
#define PCTL_CMD_GO	0x2
#define PCTL_CMD_SLEEP	0x3
#define PCTL_CMD_WAKEUP	0x4

#define PCTL_STAT_INIT		0x0
#define PCTL_STAT_CONFIG	0x1
#define PCTL_STAT_CONFIG_REQ	0x2
#define PCTL_STAT_ACCESS	0x3
#define PCTL_STAT_ACCESS_REQ	0x4
#define PCTL_STAT_LOWPOWER	0x5
#define PCTL_STAT_LOWPOWER_ENTRY_REQ	0x6
#define PCTL_STAT_LOWPOWER_EXIT_REQ	0x7

/*
 * DDR training register addr
 */
#define PUB_TR_ADDR_BASE	0xf5021494
#define PUB_TR_ADDR0_OFS	0x00
#define PUB_TR_ADDR1_OFS	0x04
#define PUB_TR_ADDR2_OFS	0x08
#define PUB_TR_ADDR3_OFS	0x0c

#define MCU_TIMEOUT_US	2000000	/*2s*/

/*
 *void __send_mcu_cmd(unsigned char cmd, void* data, int len);
 */
ENTRY(__send_mcu_cmd)
	stmfd   sp!, {r4 - r11, lr}
	ldr	r4, mcu_reg_base
	add	r5, r4, #SHREG_MBASE_OFS
	mov	r6, #0x11
	strb	r6, [r5], #1	@M0
	mov	r6, #0x8
	strb	r6, [r5], #1	@M1
	mov	r6, #0xff
	strb	r6, [r5], #1	@M2
	strb	r0, [r5], #1	@M3
	cmp	r2, #0
	beq	2f
	add	r7, r1, r2
1:
	ldrb	r8, [r1], #1	@copy data
	strb	r8, [r5], #1
	cmp	r1, r7
	blo	1b
2:
	ldrb	r6, [r4]
	eor	r6, #0x8	@reverse intr
	strb	r6, [r4]
	ldmfd   sp!, {r4 - r11, pc}
ENDPROC(__send_mcu_cmd)


/*
 *int __query_mcu_cmd(int cmd);
 * input: cmd - cmd code
 * return: 
 *	r0  - 0 on success, otherwise non-zero
 */
ENTRY(__query_mcu_cmd)
	stmfd   sp!, {r4 - r5, lr}
	ldr	r4, mcu_reg_base
	ldrb	r5, [r4, #SHREG_E3_OFS]
	subs	r0, r5
	ldmfd   sp!, {r4 - r5, pc}
ENDPROC(__query_mcu_cmd)

/*
 *int __ack_mcu_cmd(void);
 * input: none
 * return: 
 *	r0  - 0 on success, otherwise non-zero
 */
ENTRY(__ack_mcu_cmd)
	stmfd   sp!, {r4 - r5, lr}
	ldr	r4, mcu_reg_base
	ldrb	r5, [r4]
	eor	r5, #0x10
	strb	r5, [r4]		@reverse resp
	ldmfd   sp!, {r4 - r5, pc}
ENDPROC(__query_mcu_cmd)

#ifdef MCUCOMM_SUPPORT_TIMEOUT
/*
 *int __wait_event_timeout(int arg, unsigned timeout, int (*chk_evt)(int arg)); 
 * inputs: 
 *        arg      - arg for check function
 *        timeout  - timeout(us, give -1 for infinite)
 *        chk_evt  - check function
 * return: 0 on success, otherwise non-zero
 */
ENTRY(__wait_event_timeout)
	stmfd   sp!, {r4 - r11, lr}
	mov	r4, r0
	mov	r10, r1
	mov	r11, #1
	mov	r7, #0
	cmp	r1, #-1		@infinite loop?
	beq	1f		@looping
	ldr	r5, timer0_reg_base
	ldr	r6, [r5, #TIMER0_OFS_TCRN]	@TIMER0_TCRN
	cmp	r6, #0
	bne	1f		@checking
	mov	r6, #-1
	str	r6, [r5, #TIMER0_OFS_TCVRN]	@TIMER0_TCVRN
	mov	r6, #0x3	@start timer
	str	r6, [r5, #TIMER0_OFS_TCRN]	@TIMER0_TCRN
	mov	r7, #1		@set flag
1:
	mov	r0, r4
	blx	r2		@chk_evt(arg)
	cmp	r0, #0
	bne	2f
	mov	r11, #0		@ok
	b	4f
2:
	cmp	r1, #-1		@infinite loop?
	beq	1b		@looping
	cmp	r10, #0		@timeout?
	beq	4f
	sub	r10, #1
	ldr	r8, [r5, #TIMER0_OFS_TRVRN]	@TIMER0_TRVRN
3:
	ldr	r9, [r5, #TIMER0_OFS_TRVRN]	@TIMER0_TRVRN
	sub	r9, r8
	cmp	r9, #200	@wait 1us
	blo	3b		@waiting
	b	1b		@try again
4:
	cmp	r7, #1
	bne	5f		@out
	mov	r6, #0		@stop timer
	str	r6, [r5, #TIMER0_OFS_TCRN]	@TIMER0_TCRN
5:
	mov	r0, r11
	ldmfd   sp!, {r4 - r11, pc}
ENDPROC(__wait_event_timeout)
#endif  //MCUCOMM_SUPPORT_TIMEOUT

/*
 *int __get_mcu_cmd(int cmd, unsigned timeout);
 * inputs: 
 *        cmd      - cmd code
 *        timeout  - timeout (us)
 * return: 
 *	r0  - 0 on success, otherwise non-zero
 *	r1  - data[0] on success, otherwise undefined
 */
ENTRY(__get_mcu_cmd)
	stmfd   sp!, {r4, lr}
#ifdef MCUCOMM_SUPPORT_TIMEOUT
	adr	r2, __query_mcu_cmd
	bl	__wait_event_timeout
#else
	mov	r4, r0
1:
	mov	r0, r4
	bl	__query_mcu_cmd
	bne	1b		@tryagain
#endif
	ldmfd   sp!, {r4, pc}
ENDPROC(__get_mcu_cmd)

/*
 *int __pctl_set_mode(unsigned base, unsigned cmd, unsigned stat);
 * inputs: 
 * 	base - controller base addr
 *	cmd  - cmd code
 *	stat - state code
 * return: 0 on success, otherwise non-zero
  */
ENTRY(__pctl_set_mode)
	stmfd   sp!, {r3, lr}
	str	r1, [r0, #PCTL_OFS_SCTL]
1:
	ldr	r3, [r0, #PCTL_OFS_STAT]
	cmp	r3, r2
	bne	1b		@looping
	mov	r0, #0
	ldmfd   sp!, {r3, pc}
ENDPROC(__pctl_set_mode)

/*
 *int __pctl_in_lowpower(unsigned base, unsigned flag);
 * put one sdram controller in lowpower mode
 * inputs:
 *	base  - sdram controller base addr
 *	flag  - recovery flag
 * return: 0 on success, otherwise non-zero
 */
ENTRY(__pctl_in_lowpower)
	stmfd   sp!, {r1 - r3, lr}
	cmp	r1, #0
	beq	1f
	mov	r3, r0
	mov	r1, #PCTL_CMD_CFG
	mov	r2, #PCTL_STAT_CONFIG
	bl	__pctl_set_mode
	mov	r0, r3
	mov	r1, #PCTL_CMD_GO
	mov	r2, #PCTL_STAT_ACCESS
	bl	__pctl_set_mode
	mov	r0, r3
1:
	mov	r1, #PCTL_CMD_SLEEP
	mov	r2, #PCTL_STAT_LOWPOWER
	bl	__pctl_set_mode
	ldmfd   sp!, {r1 - r3, pc}
ENDPROC(__pctl_in_lowpower)

/*
 *int __pctl_in_access(unsigned base, unsigned flag);
 * put one sdram controller in access mode
 * inputs: 
 * 	base  - sdram controller base addr
 *	flag  - recovery flag
 * return: 0 on success, otherwise non-zero
 */
ENTRY(__pctl_in_access)
	stmfd   sp!, {r1 - r4, lr}
	mov	r3, r0
	mov	r4, r1
	mov	r1, #PCTL_CMD_WAKEUP
	mov	r2, #PCTL_STAT_ACCESS
	bl	__pctl_set_mode
	cmp	r4, #0
	beq	1f
	mov	r0, r3
	mov	r1, #PCTL_CMD_CFG
	mov	r2, #PCTL_STAT_CONFIG
	bl	__pctl_set_mode
1:
	ldmfd   sp!, {r1 - r4, pc}
ENDPROC(__pctl_in_access)


#ifdef __CONFIG_SRAM_RESUME__

# define BOOT_MODE_NORMAL	0
# define BOOT_MODE_STANDBY	1
# define BOOT_MODE_UNKNOWN	-1

/*
 *int s2ramctl_get_boot_mode(void);
 * inputs: 
 * return:
 *	0  - NORMAL
 *	1  - STANDBY
 *	-1 - UNKNOWN
 */
ENTRY(s2ramctl_get_boot_mode)
	stmfd	sp!, {lr}
	ldr	r0, boot_mode
	cmp	r0, #-1
	bne	4f
	stmfd   sp!, {r4 - r8}
	mov	r0, #MCU_CMD_POWERMODE
	mov	r1, #0			@try only once
	bl	__get_mcu_cmd
	cmp	r0, #0			@error?
	bne	2f
	ldr	r4, mcu_reg_base
	add	r5, r4, #SHREG_E4_OFS
	ldrb	r1, [r5], #1		@bootmode
	add	r6, r5, #4
	adr	r7, resume_entry_ptr
1:
	ldrb	r8, [r5], #1
	strb	r8, [r7], #1		@get entry point
	cmp	r5, r6
	blt	1b

	bl	__ack_mcu_cmd
	cmp	r1, #0			@normal?
	beq	2f
	cmp	r1, #1			@standby?
	bne	2f			@otherwise, error
	mov	r0, #BOOT_MODE_STANDBY	@standby boot
	b	3f
2:
	mov	r0, #BOOT_MODE_NORMAL	@normal boot
3:
	str	r0, boot_mode
	ldmfd   sp!, {r4 - r8}
4:
	ldmfd	sp!, {pc}
ENDPROC(s2ramctl_get_boot_mode)

/*
 *int s2ramctl_sdram_in_access(void);
 * put sdram (controller) in access mode
 * inputs: 
 * return: 0 on success, otherwise non-zero
 */
ENTRY(s2ramctl_sdram_in_access)
	stmfd   sp!, {r4, lr}
	mov	r1, #1
	ldr	r0, pctl_reg_umac0_base
	bl	__pctl_in_lowpower
	ldr	r0, pctl_reg_umac1_base
	bl	__pctl_in_lowpower
	mov	r0, #HOST_CMD_RMODE_DISABLE
	mov	r1, #0
	mov	r2, #0
	bl	__send_mcu_cmd
	mov	r0, #MCU_CMD_ACK_RMODE_OFF
	ldr	r1, mcu_timeout_us
	bl	__get_mcu_cmd
	cmp	r0, #0			@error?
	bne	1f			@out
	bl	__ack_mcu_cmd
#ifdef HOST_CTRL_STB_GPIO
	ldr	r1, mcu_reg_base
	ldrb	r2, [r1, #MCU_REG_GPIO_OFS]
	orr	r2, #0xA0		@set bit[5] and bit[7]
	strb	r2, [r1]
#endif
	mov	r4, #0x27
	mov	r1, #1
	ldr	r0, pctl_reg_umac0_base
	str	r4, [r0, #PCTL_OFS_TREFI]
	bl	__pctl_in_access
	ldr	r0, pctl_reg_umac1_base
	str	r4, [r0, #PCTL_OFS_TREFI]
	bl	__pctl_in_access
	mov	r0, #0
1:
	ldmfd   sp!, {r4, pc}
ENDPROC(s2ramctl_sdram_in_access)

.macro trace_resume_entry
	stmfd   sp!, {r0, r4}
	adr	r0, resume_stage
	trace_str
	ldr	r4, resume_entry_ptr
	ldr	r0, [r4]
	trace_int
	mov	r0, #'\n'
	trace_ch
	ldmfd   sp!, {r0, r4}
.endm

/*
 *void s2ramctl_cpu_do_resume(void);
 * jump to resume entry
 * inputs: none
 * return: never return
 */
ENTRY(s2ramctl_cpu_do_resume)
	trace_resume_entry
	ldr	r0, resume_entry_ptr
	ldr	r0, [r0]		@resume entry
	blx	r0
1:	b	1b			@shall not reach here
ENDPROC(s2ramctl_cpu_do_resume)

/*
 *int s2ramctl_set_area(unsigned addr);
 * set memory area for DDR training (size=8k)
 * criteria on mem area: 32k aligned, and <2G
 * [31..30] Reserved
 * [29..13] Row Address
 *  Register_bit[29] = 0;
 *  Register_bit[28:25] = start_address[30:27];
 *  Register_bit[24:16] = start_address[23:15];
 *  Register_bit[15:13] = start_address[26:24];
 * [12..10] Bank Address, 3'b000
 * [9..0] Colomn Address, start from 0 and increase at a step of 0x8
 * inputs: addr  - area start addr
 * return: 0 on success, otherwise non-zero
 */
ENTRY(s2ramctl_set_area)
	stmfd   sp!, {r4 - r6, lr}
	mov	r4, r0, LSR #27
	and	r4, #0xf
	mov	r5, r4, LSL #25		@[28..25]
	mov	r4, r0, LSL #8
	mov	r4, r4, LSR #23
	orr	r5, r4, LSL #16		@[24..16]
	mov	r4, r0, LSR #24
	and	r4, #0x7
	orr	r5, r4, LSL #13		@[15..13]

	ldr	r4, pub_tr_addr_base
	add	r6, r4, #16
1:
	str	r5, [r4], #4
	add	r5, #0x8
	cmp	r4, r6
	blo	1b
	ldmfd   sp!, {r4 - r6, pc}
ENDPROC(s2ramctl_set_area)

resume_stage: .ascii "\r\nResume @\0"
.align 2

/*
 * BOOT mode (0-normal, 1-standby, oth-reserved)
 */
boot_mode:
	.word	0xffffffff

pub_tr_addr_base:
	.word	PUB_TR_ADDR_BASE

#elif defined(__CONFIG_SRAM_SUSPEND__) /*__CONFIG_SRAM_RESUME__*/

/*
 *int __sdram_enter_lowpower(unsigned base);
 * put one sdram (controller) in lowpower mode
 * inputs: base  - sdram controller base addr
 * return: 0 on success, otherwise non-zero
 */
ENTRY(__sdram_enter_lowpower)
	stmfd   sp!, {r4, lr}
	mov	r4, r0
	mov	r1, #0
	bl	__pctl_in_lowpower
	mov	r1, #0x0
	str	r1, [r4, #PCTL_OFS_TREFI]
	str	r1, [r4, #PCTL_OFS_PHYPVTUPDI]
	str	r1, [r4, #PCTL_OFS_PVTUPDI]
	mov	r0, #0
	ldmfd   sp!, {r4, pc}
ENDPROC(__sdram_enter_lowpower)

/*
 *int __sdram_leave_lowpower(unsigned base);
 * make one sdram (controller) leave lowpower mode
 * inputs: base  - sdram controller base addr
 * return: 0 on success, otherwise non-zero
 */
ENTRY(__sdram_leave_lowpower)
	stmfd   sp!, {r4, lr}
	mov	r4, #0x27
	mov	r1, #0
	str	r4, [r0, #PCTL_OFS_TREFI]
	bl	__pctl_in_access
	ldmfd   sp!, {r4, pc}
ENDPROC(__sdram_enter_lowpower)

/*
 *void __prepare_sram_selfrefresh(void);
 * prepare for sram in selfrefresh, i.e. fill TLBs
 * inputs: none
 * return: none
 */
ENTRY(__prepare_sram_selfrefresh)
	stmfd   sp!, {r4, lr}
	ldr	r4, pctl_reg_umac0_base
	ldr	r4, [r4, #PCTL_OFS_STAT]
	ldr	r4, pctl_reg_umac1_base
	ldr	r4, [r4, #PCTL_OFS_STAT]
#ifdef MCUCOMM_SUPPORT_TIMEOUT
	ldr	r4, timer0_reg_base
	ldr	r4, [r4, #TIMER0_OFS_TCVRN]
#endif
	ldr	r4, mcu_reg_base
	ldrb	r4, [r4, #SHREG_M0_OFS]
	ldmfd   sp!, {r4, pc}
ENDPROC(__prepare_sram_selfrefresh)

/*
 *int s2ramctl_sdram_in_selfrefresh(void);
 * put sdram (controllers) in selfrefresh mode
 * inputs: 
 * return: 0 on success, otherwise non-zero
 */
ENTRY(s2ramctl_sdram_in_selfrefresh)
	stmfd   sp!, {lr}
	bl	__prepare_sram_selfrefresh
	ldr	r0, pctl_reg_umac0_base
	bl	__sdram_enter_lowpower
	ldr	r0, pctl_reg_umac1_base
	bl	__sdram_enter_lowpower
	mov	r0, #HOST_CMD_RMODE_ENABLE
	mov	r1, #0
	mov	r2, #0
	bl	__send_mcu_cmd
	mov	r0, #MCU_CMD_ACK_RMODE_ON
	ldr	r1, mcu_timeout_us
	bl	__get_mcu_cmd
	cmp	r0, #0
	bne	1f			@error
	bl	__ack_mcu_cmd
#ifdef HOST_CTRL_STB_GPIO
	ldr	r1, mcu_reg_base
	ldrb	r2, [r1, #MCU_REG_GPIO_OFS]
	orr	r2, #0x20		@set bit[5]
	bic	r2, #0x80		@clear bit[7]
	strb	r2, [r1]
#endif
	b	2f			@out
1:
	ldr	r0, pctl_reg_umac0_base
	bl	__sdram_leave_lowpower
	ldr	r0, pctl_reg_umac1_base
	bl	__sdram_leave_lowpower
	mov	r0, #1			@failed
2:
	ldmfd   sp!, {pc}
ENDPROC(s2ramctl_sdram_in_selfrefresh)

/*
 *int s2ramctl_system_poweroff(void);
 * inputs: 
 * return: 0 on success, otherwise non-zero
 */
ENTRY(s2ramctl_system_poweroff)
	stmfd   sp!, {lr}
	ldr	r0, resume_entry_ptr
	str	r0, [sp, #-4]!
	mov	r0, #HOST_CMD_POWEROFF
	mov	r1, sp
	mov	r2, #4
	bl	__send_mcu_cmd
	add	sp, #4
	mov	r0, #0
	ldmfd   sp!, {pc}
ENDPROC(s2ramctl_system_poweroff)

/*
 *void s2ramctl_set_resume_entry(s2ram_resume_frame* entry);
 * set resume entry point (physical address)
 * inputs: entry  - pointer to resume frame struct
 * return: 0 on success, otherwise non-zero
 */
ENTRY(s2ramctl_set_resume_entry)
	str	r0, resume_entry_ptr
	mov	pc, lr
ENDPROC(s2ramctl_set_resume_entry)

#endif /*__CONFIG_SRAM_SUSPEND__*/


/*
 * resume entry point (physical addr)
 */
resume_entry_ptr:
	.word	0x0badc0de
pctl_reg_umac0_base:
	.word	PCTL_REG_UMAC0_BASE
pctl_reg_umac1_base:
	.word	PCTL_REG_UMAC1_BASE
mcu_reg_base:
	.word	MCU_REG_BASE
timer0_reg_base:
	.word	TIMER0_REG_BASE
mcu_timeout_us:
	.word	MCU_TIMEOUT_US

