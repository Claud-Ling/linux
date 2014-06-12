/*
 *
 *  (C)Copyright 2011 Trident Microsystem ,Inc. All Rights Reserved.
 *     Jason mei  jason.mei@tridentmicro.com
 *  Version 1.0 2011/07/28
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
 * All Rights Reserved.
 *
 *---------------------------------------------------------------------------
 * %filename:     hwTRIDENTGMACEth_Cfg.c %
 * %pid_version:          1.1                         %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  This file holds the code for configuration of ethernet GMAC HwApi
 *
 *
 * DOCUMENT REF: 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/

/*----------------------------------------------------------------------------*/
/* Standard include files:                                                    */
/*----------------------------------------------------------------------------*/

#include "../../../intfs/IPhy/inc/tmNxTypes.h"

/*----------------------------------------------------------------------------*/
/* Project include files:                                                     */
/*----------------------------------------------------------------------------*/

#include "../../../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h"

/*----------------------------------------------------------------------------*/
/* Types and defines:                                                         */
/*----------------------------------------------------------------------------*/


/* Base addresses of GMAC0*/

#ifndef HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0
#define HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0     (0x0U)
#endif


/*----------------------------------------------------------------------------*/
/* Global data:                                                               */
/*----------------------------------------------------------------------------*/
/* This gets assigned in remapBaseAdrs fn */
#if (HWTRIDENTGMACETH_NUM_UNITS == 1)
hwTRIDENTGMACEth_Cfg_t ghwTRIDENTGMACEth_Base[HWTRIDENTGMACETH_NUM_UNITS] = 
{
    {HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0},
};
#elif (HWTRIDENTGMACETH_NUM_UNITS > 1)
hwTRIDENTGMACEth_Cfg_t ghwTRIDENTGMACEth_Base[HWTRIDENTGMACETH_NUM_UNITS] =
{
    {HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0}, {HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0},
};
#else
hwTRIDENTGMACEth_Cfg_t ghwTRIDENTGMACEth_Base[HWTRIDENTGMACETH_NUM_UNITS] =
{
    {HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0}, {HW_TRIDENTGMACETH_PHY_MMIO_ADDRESS0},
};
#endif
