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

#ifndef __ASM_ARCH_TRIX_MACH_ENTRY_MACRO_H__
#define __ASM_ARCH_TRIX_MACH_ENTRY_MACRO_H__

#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/irqs.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#include <linux/irqchip/arm-gic.h>
#else
#include <asm/hardware/gic.h>
#endif

#include <asm/hardware/entry-macro-gic.S>

	.macro  disable_fiq
        .endm

        .macro  arch_ret_to_user, tmp1, tmp2
        .endm

#endif
