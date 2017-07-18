/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */

#ifndef __ASM_ARCH_TRIX_PLAT_HARDWARE_H__
#define __ASM_ARCH_TRIX_PLAT_HARDWARE_H__

#include <asm/sizes.h>
#ifndef __ASSEMBLY__
#include <asm/types.h>
#include <plat/cpu.h>
#endif

#include <plat/space.h>
#include <plat/serial.h>

/*
 * ---------------------------------------------------------------------------
 * Common definitions for all processors
 * ---------------------------------------------------------------------------
 */

#if 0
/*
 * ----------------------------------------------------------------------------
 * Timers
 * ----------------------------------------------------------------------------
 */
#define SIGMA_MPU_TIMER1_BASE	(0xfffec500)
#define SIGMA_MPU_TIMER2_BASE	(0xfffec600)
#define SIGMA_MPU_TIMER3_BASE	(0xfffec700)
#define MPU_TIMER_FREE		(1 << 6)
#define MPU_TIMER_CLOCK_ENABLE	(1 << 5)
#define MPU_TIMER_AR		(1 << 1)
#define MPU_TIMER_ST		(1 << 0)

/*
 * ----------------------------------------------------------------------------
 * Clocks
 * ----------------------------------------------------------------------------
 */
#define CLKGEN_REG_BASE		(0xfffece00)
#define ARM_CKCTL		(CLKGEN_REG_BASE + 0x0)
#define ARM_IDLECT1		(CLKGEN_REG_BASE + 0x4)
#define ARM_IDLECT2		(CLKGEN_REG_BASE + 0x8)
#define ARM_EWUPCT		(CLKGEN_REG_BASE + 0xC)
#define ARM_RSTCT1		(CLKGEN_REG_BASE + 0x10)
#define ARM_RSTCT2		(CLKGEN_REG_BASE + 0x14)
#define ARM_SYSST		(CLKGEN_REG_BASE + 0x18)
#define ARM_IDLECT3		(CLKGEN_REG_BASE + 0x24)

#define CK_RATEF		1
#define CK_IDLEF		2
#define CK_ENABLEF		4
#define CK_SELECTF		8
#define SETARM_IDLE_SHIFT

/* DPLL control registers */
#define DPLL_CTL		(0xfffecf00)

/*
 * ---------------------------------------------------------------------------
 * Watchdog timer
 * ---------------------------------------------------------------------------
 */

/* Watchdog timer */
#define SIGMA_MPU_WATCHDOG_BASE	(0xfffec800)
#define SIGMA_WDT_TIMER		(SIGMA_MPU_WATCHDOG_BASE + 0x0)
#define SIGMA_WDT_LOAD_TIM	(SIGMA_MPU_WATCHDOG_BASE + 0x4)
#define SIGMA_WDT_READ_TIM	(SIGMA_MPU_WATCHDOG_BASE + 0x4)
#define SIGMA_WDT_TIMER_MODE	(SIGMA_MPU_WATCHDOG_BASE + 0x8)

#endif

#endif	/*__ASM_ARCH_TRIX_PLAT_HARDWARE_H__*/
