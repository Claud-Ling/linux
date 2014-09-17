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

#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <asm/tlb.h>


#include <mach/hardware.h>
#include <mach/io.h>
#include <asm/mach/map.h>


static struct map_desc sigma_io_desc[] __initdata = {
	{
		.virtual	= (unsigned long)SIGMA_IO_0_BASE_VIRT,
		.pfn		= __phys_to_pfn(SIGMA_IO_0_BASE_PHYS),
		.length		= SIGMA_IO_0_SIZE,
		.type		= MT_DEVICE
	},
#if 0
	{
		.virtual	= (unsigned long)SIGMA_IO_1_BASE_VIRT,
		.pfn		= __phys_to_pfn(SIGMA_IO_1_BASE_PHYS),
		.length		= SIGMA_IO_1_SIZE,
		.type		= MT_DEVICE
	},
#endif
	{
		.virtual	= (unsigned long)SIGMA_IO_ARM_BASE_VIRT,
		.pfn		= __phys_to_pfn(SIGMA_IO_ARM_BASE_PHYS),
		.length		= SIGMA_IO_ARM_SIZE,
		.type		= MT_DEVICE
	},
};


void __init trix_map_io(void)
{
	TRI_DBG("[%d] %s\n",__LINE__,__func__);

	iotable_init(sigma_io_desc, ARRAY_SIZE(sigma_io_desc));
	TRI_DBG("[%d] %s\n",__LINE__,__func__);

	local_flush_tlb_all();
	flush_cache_all();

	TRI_DBG("[%d] %s\n",__LINE__,__func__);
}

