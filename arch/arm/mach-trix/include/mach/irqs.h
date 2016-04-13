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

#ifndef __ASM_ARCH_TRIX_MACH_IRQS_H__
#define __ASM_ARCH_TRIX_MACH_IRQS_H__


#include <generated/autoconf.h>

/*
 * SGI and PPI IRQ numbers for interrupt handler 0-31
*/
/*global timer*/
#define TRIHIDTV_GLOBAL_TIMER_IRQ			27
/*private timer*/
#define TRIHIDTV_LOCAL_TIMER_IRQ			29
/*watchdog timer*/
#define TRIHIDTV_WATCHDOG_TIMER_IRQ			30

#define TRIHIDTV_GIC_IRQ				29

/*
 * SPI IRQ numbers for interrupt handler 32
 */
#define TRIHIDTV_EIC_IRQ_BASE 				32

/* SPIs */
#if defined(CONFIG_SIGMA_SOC_SX6)
#include "spi-trix-sx6.h"
#elif defined(CONFIG_SIGMA_SOC_UXLB)
#include "spi-trix-uxlb.h"
#elif defined(CONFIG_SIGMA_SOC_SX7)
#include "spi-trix-sx7.h"
#elif defined(CONFIG_SIGMA_SOC_SX8)
#include "spi-trix-sx8.h"
#else
#error "unknown chip when search for SPIs"
#endif

/*maxium interrupt number of gic*/
#define TRIHIDTV_MAX_IRQ 	      	(TRIHIDTV_EIC_IRQ_BASE + 160)
#define NR_IRQS				TRIHIDTV_MAX_IRQ
#if TRIHIDTV_MAX_IRQ < TRIHIDTV_RESERVED_IRQ
# error "TRIHIDTV_RESERVED_IRQ exceeds IRQ range specified by TRIHIDTV_MAX_IRQ"
#endif

#include <mach/hardware.h>

#ifdef CONFIG_FIQ
#define FIQ_START		1024
#endif

#endif /*__ASM_ARCH_TRIX_MACH_IRQS_H__*/
