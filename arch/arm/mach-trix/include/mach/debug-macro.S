/*
 * Debugging macro for DaVinci
 *
 * Author: Kevin Hilman, MontaVista Software, Inc. <source@mvista.com>
 *
 * 2007 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

/* Modifications
 * Jan 2009	Chaithrika U S	Added senduart, busyuart, waituart
 *				macros, based on debug-8250.S file
 *				but using 32-bit accesses required for
 *                              some davinci devices.
 */

#include <linux/serial_reg.h>
#include <mach/serial.h>

#ifndef __ASM_ARCH_TRIX_MACH_DEBUG_MACRO_S__
#define __ASM_ARCH_TRIX_MACH_DEBUG_MACRO_S__

.macro addruart, rp, rv, tmp
	ldr \rp, =0xFB005100
	ldr \rv, =0xFB005100
.endm

#define UART_SHIFT	0

#endif /*__ASM_ARCH_TRIX_MACH_DEBUG_MACRO_S__*/

#include <asm/hardware/debug-8250.S>

