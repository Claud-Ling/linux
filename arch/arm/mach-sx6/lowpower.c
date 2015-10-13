/*
 *  linux/arch/arm/mach-sx6/lowpower.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>

#include <asm/smp_scu.h>
#include <asm/cacheflush.h>
#include <asm/cp15.h>

#include "common.h"

#define SCU_BASE platform_get_scu_base()

/*
 * make call-in cpu enter lowpower mode so as to prepare for upcoming suspend (i.e. do_wfi)
 * what are supposed to be done inside this function
 * 1. dcache is flushed and SCTLR.C=0 (both secure and NS worlds)
 * 2. ACTLR.SMP=0
 * 3. set CPU power state to POWER-OFF
 */
void cpu_enter_lowpower(void)
{
	/*
	 * Flush all data from the L1 data cache before disabling
	 * SCTLR.C bit.
	 */
	flush_cache_all();

	/*
	 * Clear the SCTLR.C bit to prevent further data cache
	 * allocation. Clearing SCTLR.C would make all the data accesses
	 * strongly ordered and would not hit the cache.
	 */
	asm volatile(
	"	mcr	p15, 0, %0, c7, c5, 0\n"	/*A9 only, invalidate all icache to PoU*/
	"	dsb\n"
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	/* Disable the C bit */
	"	bic	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	"	isb\n"
	:
	: "r" (0), "Ir" (CR_C)
	: "cc");

	/*
	 * Invalidate L1 data cache. Even though only invalidate is
	 * necessary exported flush API is used here. Doing clean
	 * on already clean cache would be almost NOP.
	 */
	flush_cache_all();

	/*
	 * Take core out of coherency
	 */
	asm volatile(
	/*
	 * Disable ACTLR.SMP bit
	 */
	"	mrc	p15, 0, r0, c1, c0, 1\n"
	"	bic	r0, r0, #(1 << 6)		@ Disable SMP bit\n"

	/*
	 * Update ACTLR
	 * It's RO in Non-secure state if NSACR.NS_SMP=0, RW if NSACR.NS_SMP=1
	 */
#ifdef CONFIG_SIGMA_SMC
	"	bl	secure_set_actlr\n"
#else
	"	mcr	p15, 0, r0, c1, c0, 1\n"
#endif
	"	isb\n"
	:
	:
	: "cc");

	/*
	 * This CPU is about to enter power-off mode
	 *
	 * Note: don't touch SCU power status register for sx6/7
	 * sine they don't have PMU on chip so can't control
	 * power from core to core. Otherwise some weird behavior
	 * might be seen at some low rate when hotplug cpus.
	 *
	 * One typical issue when resume certain core (rate ~1/200):
	 *
	 * BUG: scheduling while atomic: swapper/2/0/0xffff0000
	 *
	 */
	//scu_power_mode(SCU_BASE, SCU_PM_POWEROFF);

	dsb();
}

/*
 * make call-in cpu leave lowpower mode so as to prepare for upcoming wake-up
 * what are supposed to be done inside this function
 * 1. SCTLR.C=1 (both secure and NS worlds)
 * 2. ACTLR.SMP=1
 * 3. set CPU power state to NORMAL
 */
void cpu_leave_lowpower(void)
{
	asm volatile(
	/*
	 * Check if need to enable the C bit
	 */
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	tst	%0, %1\n"
	"	orreq	%0, %0, %1\n"
	"	mcreq	p15, 0, %0, c1, c0, 0\n"
	"	isb\n"
	/*
	 * Check if need to enable SMP bit
	 */
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	tst	%0, #(1 << 6)\n"
	"	orreq	r0, %0, #(1 << 6)\n"
#ifdef CONFIG_SIGMA_SMC
	"	bleq	secure_set_actlr\n"

#else
	"	mcreq	p15, 0, r0, c1, c0, 1\n"
#endif
	"	isb\n"
	:
	: "r" (0), "Ir" (CR_C)
	: "cc");

	/*
	 * This CPU is about to enter normal mode
	 */
	//scu_power_mode(SCU_BASE, SCU_PM_NORMAL);

	dsb();
}
