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
 *  Author: Tony He, 2014.
 */


#include <linux/linkage.h>

/* void sigma_trix_smc1(int arg, int code)
 * beware: this has been copied from
 * repo/hwdep_xos/xos/xboot4/armuk/boot/armuk_asm.S
 * Report any modification over there!
 */
ENTRY(sigma_trix_smc1)
	stmfd	sp!, {r2 - r12, lr}
	mov     ip, r1
	dsb
	smc	#0
	ldmfd	sp!, {r2 - r12, pc}
ENDPROC(sigma_trix_smc1)

/* void sigma_trix_smc2(int arg0, int arg1, int code)
 * beware: this has been copied from
 * repo/hwdep_xos/xos/xboot4/armuk/boot/armuk_asm.S
 * Report any modification over there!
 */
ENTRY(sigma_trix_smc2)
	stmfd	sp!, {r3 - r12, lr}
	mov     ip, r2
	dsb
	smc	#0
	ldmfd	sp!, {r3 - r12, pc}
ENDPROC(sigma_trix_smc2)

/* void sigma_trix_smc3(int arg0, int arg1, int arg2, int code)
 * beware: this has been copied from
 * repo/hwdep_xos/xos/xboot4/armuk/boot/armuk_asm.S
 * Report any modification over there!
 */
ENTRY(sigma_trix_smc3)
	stmfd	sp!, {r4 - r12, lr}
	mov     ip, r3
	dsb
	smc	#0
	ldmfd	sp!, {r4 - r12, pc}
ENDPROC(sigma_trix_smc3)

