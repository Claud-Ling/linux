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
 *  Author: Qipeng Zha, 2012.
 */


#include <linux/linkage.h>
#include <linux/init.h>

	__INIT

/*
 * specific entry point for secondary CPU to jump from ROM code.
 */
ENTRY(sigma_secondary_startup)
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #3		@ get CPUID
	adr	r4, 1f
	ldmia	r4, {r5, r6}
	sub	r4, r4, r5
	add	r6, r6, r4
pen:
	ldr	r7, [r6]
	cmp	r7, r0
	bne	pen

	/*
	 * we've been released from the wait loop,secondary_stack
	 * should now contain the SVC stack for this core
	 */
	b	secondary_startup

ENDPROC(sigma_secondary_startup)

   .align 2
1:  .long   .
    .long   pen_release

