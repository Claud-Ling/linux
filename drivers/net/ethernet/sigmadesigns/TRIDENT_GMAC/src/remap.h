/*
 *
 *  (c)copyright 2011 trident microsystem ,inc. all rights reserved.
 *     jason mei  jason.mei@tridentmicro.com
 *  version 1.0 2011/07/28
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (c) 2006-2007, LIPP Alliance
 *
 * All Rights Reserved.
 *
 *---------------------------------------------------------------------------
 * %filename:      remap.h       %
 * %pid_version:     1.2           %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  Header file for exporting remap functions
 *
 * DOCUMENT REF: 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/
#ifndef _REMAP_H_
#define _REMAP_H_

#include <linux/types.h>
#include <linux/platform_device.h>

#define REMAP_MEM_SIZE (8*1024)

extern __s32 remapBaseAdrs(struct platform_device *pdev);
extern void unmapBaseAdrs(struct platform_device * pdev);

extern __s32 registerInterrupts(struct platform_device *pdev);
extern void unregisterInterrupts(struct platform_device *pdev);


#endif

