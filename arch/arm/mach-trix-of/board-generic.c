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
#include <asm/mach/arch.h>

static char const *const trix_of_compat[] __initconst = {
	"sdesigns,sx6",
	"sdesigns,sx7",
	"sdesigns,uxlb",
	"sdesigns,sx8",
	"sdesigns,union",
	NULL,
};

DT_MACHINE_START(TRIX_SXX_OF, "Sigma dtv platform")
	/* Maintainer: Sigma Designs, Inc. */
	.l2c_aux_val    = 0x00000000,
	.l2c_aux_mask   = 0xffffffff,
	.dt_compat	= trix_of_compat,
MACHINE_END


int trix_set_dbg_swen(unsigned int cpu, bool state)
{
	/* TODO:
	 * enable software debug if we have hw breakpoint
	 * facility, but it's better to initialise them in
	 * bootloader at early stage.
	 */
	return 0;
}

