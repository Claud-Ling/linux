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
 * %filename:             PhyAccess.c %
 * %pid_version:             1.2                  %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  PHY interface file for Linux Ethernet driver
 *
 * DOCUMENT REF:
 *
 * NOTES:        This is to be updated if there is change in PHY hardware
 *               Appropriate APIs are to be repopulated in the array.
 *
 *-----------------------------------------------------------------------------
 *
*/
#include <linux/kernel.h>
#include "../intfs/IPhy/inc/Phy.h"
#include "../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h"

#define LAN_CLK_SEL        0
#define LAN_CLK_SPEED_SEL  0

#include "../comps/PhySMSC8710/inc/PhySMSC8710.h"

PhyConfig_t gSMSC8710PhyInterface =
{
       "SMSC-8720",
	0x7c0f1,        
        LAN_CLK_SEL,
        LAN_CLK_SPEED_SEL,
        PhySMSC8710GetSWVersion,
        PhySMSC8710GetCapabilities,
        PhySMSC8710Init,
        PhySMSC8710Deinit,
        PhySMSC8710SetPowerState,
        PhySMSC8710GetPowerState,
        PhySMSC8710GetBasicModeControl,
        PhySMSC8710SetBasicModeControl,
        PhySMSC8710GetBasicModeStatus,
        PhySMSC8710AutoNegotiate,
        Null,
        Null,
        Null,
        Null,
        Null,
        Null,
        Null,
        PhySMSC8710LoopBack,
        PhySMSC8710SoftReset,
        PhySMSC8710GetLinkStatus
};
#if 0
phyID_interface_tbl phy_tbl[]={
{ 0x7c0f1, &gSMSC8710PhyInterface },
};
#endif 
void PhyGetInterface(int unitnumber, UInt32 phyID, UInt32 isExternal, PhyConfig_t ** pPhyInterface)
{
        *(pPhyInterface) =&gSMSC8710PhyInterface ;
#if 0	
    UInt32 i;
    {
        for(i = 0 ; i< sizeof(phy_tbl)/sizeof(phyID_interface_tbl); i++)
            if(phy_tbl[i].phyID == phyID)
                *(pPhyInterface) = phy_tbl[i].interface_fns;
    }
#endif     
}

