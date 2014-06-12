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
 * %filename:     hwTRIDENTGMACEth_Cfg.h%
 * %pid_version:          1.5                         %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  Configuration header file for Ethernet HwApi Driver
 *
 *
 * DOCUMENT REF: 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/

#ifndef HWTRIDENTGMACETH_CFG_H
#define HWTRIDENTGMACETH_CFG_H

/*----------------------------------------------------------------------------*/
/* Standard include files: */
/*----------------------------------------------------------------------------*/


#include "../../../intfs/IPhy/inc/tmNxTypes.h"

/*----------------------------------------------------------------------------*/
/* Project include files: */
/*----------------------------------------------------------------------------*/

/*! 
 * \def TMFL_SD_ALL
 * Select all the APIs for final image. If this flag is disabled then scalability settings as 
 * defined below are used by preprocessor.
 * \n NOTE: The flags need to be modified by the user as per system requirements. Also, care 
 * should be taken in case multiple units of this IP are available in Hw sybsystem. 
 * In case of multiple units with different features User will need to enable superset of
 * features of individual unit. In such case it is Applications responsibility to keep track of which 
 * features are supported by which IP.
 */

/*  Scalability Settings Start  */
#define TMFL_SD_ALL 0
#if (TMFL_SD_ALL == 0)
#define TMFL_TRIDENTGMACETHSD_MMC 1
#define TMFL_TRIDENTGMACETHSD_POWER 1 
#define TMFL_TRIDENTGMACETHSD_HASH 1
#define TMFL_TRIDENTGMACETHSD_VLAN 1
#define TMFL_TRIDENTGMACETHSD_IEEE1588_TIMESTAMP 0
#define TMFL_TRIDENTGMACETHSD_OTHERS 0
#endif
/* Scalability Settings End */

/* Defining this macro to 1, incorporates work around for IPC counters bug in 3.41a */
#define IPVERSION_34_1_A  0

#define HWTRIDENTGMACETH_NUM_UNITS (1)
/*! 
 * \def HWTRIDENTGMACETH_NUM_UNITS
 *  Indicates number of ethernet units supported.
 *  The definition of ghwTRIDENTGMACEth_Base[] is to be modified.
 *  with new base addresses, if there are multiple units.
 */

/* Defined as part of ethernet driver makefile */	
//#define  TMFL_TRIDENTGMACETH_GMII_PHY_ADDRESS_VAL (0x16U)
/*! \def TMFL_TRIDENTGMACETH_GMII_PHY_ADDRESS_VAL
*   Default PHY address value used
*/
/* Defined as part of ethernet driver makefile */	
//#define TMFL_TRIDENTGMACETH_GMII_ADDRESS_CSR_VAL (0x1U)
/*! \def TMFL_TRIDENTGMACETH_GMII_ADDRESS_CSR_VAL
*     CSR clock value, which decides the frequency of MDC clock
*/

typedef struct hwTRIDENTGMACEth_Cfg
/*! \brief This structure stores the base Address of Ethernet unit*/
{
    UInt32 baseAddress; /*!< baseAddress of GMAC unit */

} hwTRIDENTGMACEth_Cfg_t, *phwTRIDENTGMACEth_Cfg_t;

extern hwTRIDENTGMACEth_Cfg_t ghwTRIDENTGMACEth_Base[HWTRIDENTGMACETH_NUM_UNITS]; /* ETH context */

/*!
 * \def GET_BASE(x)
 * Computes the base address of GMAC unit.
 */
#define GET_BASE(x)     (ghwTRIDENTGMACEth_Base[x].baseAddress)

#define TRIDENT_PHY_TIMEOUT 0x000fffff
Int32 Read_Phy_Reg(UInt32 PhyAddr,UInt32 Reg,pUInt16 pVal);
Int32 Write_Phy_Reg(UInt32 PhyAddr,UInt32 Reg, UInt16 data);

#endif

