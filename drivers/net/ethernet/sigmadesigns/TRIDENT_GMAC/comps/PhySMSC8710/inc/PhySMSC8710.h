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
 * %filename:     PhySMSC8710.h %
 * %pid_version:              1.2              %
 *---------------------------------------------------------------------------
 * DESCRIPTION: Macros and function prototypes for SMSC8710 PHY
 *
 * DOCUMENT REF: Datasheet SMSC LAN8700/LAN8700i  
 *               Revision 2.1 (03-06-09)
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/

#ifndef _PHYSMSC8710_H_
#define _PHYSMSC8710_H_

#include "../../../intfs/IPhy/inc/tmNxTypes.h"
#include "../../../intfs/IPhy/inc/Phy.h"

#define __TRIDENT_GMAC__

/* Macros */

/* MAC Macros */
#define HW_TRIDENTGMACETH_ADR_REG_OFFSET (0x010)
#define HW_TRIDENTGMACETH_DATA_REG_OFFSET (0x014)

#define HW_TRIDENTGMACETH_ADR_PHY_REG_CLR (0xFFFFF83F)
#define HW_TRIDENTGMACETH_ADR_PHY_REG_POS (6)
#define HW_TRIDENTGMACETH_ADR_PHY_WR_VAL (0x2)
#define HW_TRIDENTGMACETH_ADR_PHY_RD_CLR (0xFFFFFFFD)
#define HW_TRIDENTGMACETH_ADR_PHY_EN_VAL (0x1)

#define PHY_ADDR_OFFSET  11
#define PHY_REG_OFFSET   6
#define PHY_BUSY	0x00000001	
#define PHY_WRITE	0x00000002
#define PHY_CLK		0x00000004	

/*PHY Macros */
/*************** Basic Mode control register ************/
#define PHYSMSC8710_BMCR_RST_VAL (0x8000)

/* Enable loopback */
#define PHYSMSC8710_BMCR_LPBK_VAL (0x4000)
#define PHYSMSC8710_BMCR_LPBK_CLR (0xBFFF)

/* Set Speed to 1Gbps */
#define PHYSMSC8710_BMCR_SPEED_1G (0x40)

/* Set Speed to 100Mbps */
#define PHYSMSC8710_BMCR_SPEED_100 (0x2000)

/* Set Speed */
#define PHYSMSC8710_BMCR_SPEED_10 (0)

/* Speed mask */
#define PHYSMSC8710_BMCR_SPEED_MSK (0x2000)

/* Enable autonegotiation */
#define PHYSMSC8710_BMCR_AN_EN (0x1000)

#define PHYSMSC8710_BMCR_AN_RESTART (0x200)
#define PHYSMSC8710_BMCR_AN_RESTART_CLR (0xFDFF)

#define PHYSMSC8710_BMCR_AN_CLR (0xEFFF)

/* Set power down mode */
#define PHYSMSC8710_BMCR_PWRDN_EN (0x800)

/* Disable  power down mode */
#define PHYSMSC8710_BMCR_PWRDN_CLR (0xF7FF)

/* Isolate PHY enable */
#define PHYSMSC8710_BMCR_ISO_PHY (0x400)

/* Auto negotiation restart */
#define PHYSMSC8710_BMCR_AN_RESTART (0x200)

/* Collision Test enable */
#define PHYSMSC8710_BMCR_COLTEST (0x80)

/* Full duplex enable */
#define PHYSMSC8710_BMCR_FD_EN (0x100)

/****** Basic Mode status Register bits ******/

/* Autonegotiation complete value */
#define PHYSMSC8710_BMSR_T4100BASE (0x8000)

#define PHYSMSC8710_BMSR_X100BASEFD (0x4000)

#define PHYSMSC8710_BMSR_X100BASEHD (0x2000)

#define PHYSMSC8710_BMSR_10MBPSFD (0x1000)

#define PHYSMSC8710_BMSR_10MBPSHD (0x800)

#define PHYSMSC8710_BMSR_T2100BASEFD (0x400)

#define PHYSMSC8710_BMSR_T2100BASEHD (0x200)

/* Preamble suppression capability */
#define PHYSMSC8710_BMSR_PREAMBLE_SUP (0x40)

#define PHYSMSC8710_BMSR_AN_VAL (0x20)

/* Remote fault value */
#define PHYSMSC8710_BMSR_RF_VAL (0x10)

/* PHY is able to perform auto negotiation */
#define PHYSMSC8710_BMSR_AN_ABLE (0x8)

#define PHYSMSC8710_BMSR_LINK_STAT (0x4)

/* Jabber detected */
#define PHYSMSC8710_BMSR_JAB_VAL (0x2)



/****** Auto Negotiation Advertisement Register bits ******/

/* Advertise Next page desired */
#define PHYSMSC8710_ANAR_NP (0x8000)

/* Advertise remote fault */
#define PHYSMSC8710_ANAR_ADV_RF (0x2000)

/* Advertise asymmetric pause */
#define PHYSMSC8710_ANAR_AP (0x800)

/* Advertise pause frame support */
#define PHYSMSC8710_ANAR_PAUSE (0xc00)

/* Advertise 100Base-TX full duplex support */
#define PHYSMSC8710_ANAR_100B_TX_FD (0x100)

/* Advertise 100Base-TX half duplex support */
#define PHYSMSC8710_ANAR_100B_TX_HD (0x80)

/* Advertise 10Base-TX full duplex support */
#define PHYSMSC8710_ANAR_10B_TX_FD (0x40)

/* Advertise 10Base-TX half duplex support */
#define PHYSMSC8710_ANAR_10B_TX_HD (0x20)


/****** 1KTCR : 1000 Base-T Master-Slave Control Register ******/

/* Maser/Slave config enable */
#define PHYSMSC8710_1KTCR_MS_CONFIG (0x1000)

/* Set PHY as master */
#define PHYSMSC8710_1KTCR_MASTER_EN (0x800)

/* Advertise device as Multiport */
#define PHYSMSC8710_1KTCR_MULTIPORT_EN (0x400)

/* 1000 Base-T Full duplex capable */
#define PHYSMSC8710_1KTCR_1000BT_FD (0x200)

/* 1000 Base-T Half duplex capable */
#define PHYSMSC8710_1KTCR_1000BT_HD (0x100)

/********1KSTSR 1000 BASE-T Master-Slave Status Register *****/
#define PHYSMSC8710_1KSTSR_MAN_FAULT (0x8000)
#define PHYSMSC8710_1KSTSR_MASTER (0x4000)
#define PHYSMSC8710_1KSTSR_LOCAL_RX_STAT (0x2000)
#define PHYSMSC8710_1KSTSR_REMOTE_RX_STAT (0x1000)
#define PHYSMSC8710_1KSTSR_PART_FD_CAP (0x800)
#define PHYSMSC8710_1KSTSR_PART_HD_CAP (0x400)

/********** EXTENDED STATUS REGISTER ******************/
#define PHYSMSC8710_1KSCR_1000BASEX_FD (0x8000)
#define PHYSMSC8710_1KSCR_1000BASEX_HD (0x4000)
#define PHYSMSC8710_1KSCR_1000BASET_FD (0x2000)
#define PHYSMSC8710_1KSCR_1000BASET_HD (0x1000)

/**************** VENDOR SPECIFIC REGISTERS **************/
/****** STRAP options register ******/
#define PHYSMSC8710_STRAP_ANE (0x8000)
#define PHYSMSC8710_STRAP_DUP (0x4000)

/* Bit 13:12  similar to bits 6:13 in basic mode control register */
#define PHYSMSC8710_STRAP_SPD_MSK (0x3000)
#define PHYSMSC8710_STRAP_SPD_1G (0x2000)
#define PHYSMSC8710_STRAP_SPD_100 (0x1000)
#define PHYSMSC8710_STRAP_SPD_10 (0x0)

#define PHYSMSC8710_PHYSTS_SPEED_MSK (0xC)
#define PHYSMSC8710_PHYSTS_LINK_STAT (0x4)
#define PHYSMSC8710_PHYSTS_DUP_MODE (0x10)




//Structure Declarations
//-----------------------------------------------------------------------------
//	Typedefinition
//-----------------------------------------------------------------------------
typedef struct  _PhySMSC8710Context_t
{
    UInt32    			pRegs;         // Array ETHERNET Module regs 
} PhySMSC8710Context_t, *pPhySMSC8710Context_t;

//-----------------------------------------------------------------------------
// Exported functions:
//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetSWVersion:
//
// DESCRIPTION: This function returns the PHY device interface software version 
//				information
//
// RETURN:      TM_OK
//
// NOTES:       This API can be called anytime i.e. before initializing the PHY 
//				or in PowerOff state.
//-----------------------------------------------------------------------------

tmErrorCode_t
PhySMSC8710GetSWVersion (
    ptmSWVersion_t      					pPhyVersion    
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetCapabilities:
//
// DESCRIPTION: This function returns the PHY capabilities for the specified PHY
//				unit. The function is callable at any time to return the unit's 
//				capabilities (PHY unit initialization is not necessary). 
//				Capabilities may be different among multiple PHY units.For completeness, 
//				a PHY BSL user should call this function for each PHY unit to 
//				determine its individual capabilities. 
//
// RETURN:      TM_OK
//
// NOTES:       This API can be called anytime i.e. before initializing the PHY 
//				or in PowerOff state. 
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710GetCapabilities (
    tmUnitSelect_t                			phyUnitId,  
    pPhyCapabilities_t  				pPhyCaps    
    );

    
//-----------------------------------------------------------------------------
// FUNCTION:    PhyInit:
//
// DESCRIPTION: This function initializes the PHY device. It should be called
//				before any access to the device is made. 
//
// RETURN:      TM_OK 

//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:       This function initializes the PHY device with the following 
//				default initial configuration.
//				1. Enable the auto negotiation. In auto-negotiation mode the 
//				highest possible performance mode is selected automatically 
//				which the Link Partner also supports. (In auto negotiation mode
//				speed and duplex mode will be selected by means of auto negotiation. 
//				Writing to speed and duplex mode does not have meaning in auto 
//				negotiation mode.)
//				2. The device abilities are programmed to Auto negotiation advertise 
//				register. None of the device abilities are masked. If the PHY 
//				device does not support Auto-negotiation mode, then this function 
//				will set the PHY to basic capabilities of the device.The function 
//				PhyGetBasicModeControl can be called after tmbsPhyInit to 
//				know what are the default configurations the PhyInit function 
//				has set to
 
//-----------------------------------------------------------------------------
//
    
tmErrorCode_t
PhySMSC8710Init(
    tmUnitSelect_t  						phyUnitId   
    );


    
//-----------------------------------------------------------------------------
// FUNCTION:    PhyDeinit:
//
// DESCRIPTION: This function de-initializes the PHY device. Once the device is
//				deinitilized device will no more be available to access 
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
PhySMSC8710Deinit(
    tmUnitSelect_t  						phyUnitId   
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhySetPowerState:
//
// DESCRIPTION: This function will set the Power State of the PHY device to specified 
//				power state
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:       
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710SetPowerState(
    tmUnitSelect_t           				phyUnitId ,  
    tmPowerState_t          				phyPowerState
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetPowerState:
//
// DESCRIPTION: This function will get the preset power state of the PHY device
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:       
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710GetPowerState(
    tmUnitSelect_t                  		phyUnitId ,  
    ptmPowerState_t							phyPowerState
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetBasicModeControl:
//
// DESCRIPTION: This function will get the basic configuration of the PHY device. 
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:       PHY device is initialized using the function PhyInit.  
//				PhyInit Enable the Auto negotiation mode and will configure 
//				the PHY device for the maximum performance mode available. After 
//				the device is initialized this function can be called to know the 
//				present settings of the PHY device. If application wants to change 
//				the settings it can call PhySetBasicModeControl to change 
//				the configuration after knowing the capabilities of the PHY device 
//				and Link Partner. #define for the Basic Mode Control 
//-----------------------------------------------------------------------------
//

tmErrorCode_t 
PhySMSC8710GetBasicModeControl (
    tmUnitSelect_t                   		phyUnitId,   
    pPhyBasicModeControl_t      		pPhyBasicModeControl
    );

//-----------------------------------------------------------------------------
// FUNCTION:    PhySetBasicModeControl:
//
// DESCRIPTION: This function will configure the PHY device for the Basic Mode. 
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//


tmErrorCode_t 
PhySMSC8710SetBasicModeControl (
    tmUnitSelect_t                   		phyUnitId,   
    pPhyBasicModeControl_t      		pPhyBasicModeControl
    );

//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetBasicModeStatus:
//
// DESCRIPTION: This function will get the Basic Mode Status of the PHY device 
//				such as the speed, duplex mode 
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710GetBasicModeStatus (
    tmUnitSelect_t                  		phyUnitId,   
    pPhyBasicModeStatus_t    			pPhyBasicModeStatus     
    );

//-----------------------------------------------------------------------------
// FUNCTION:    PhyAutoNegotiate:
//
// DESCRIPTION: This function will enable the Auto negotiation of the PHY device 
//				with Link Partner. Best possible performance configuration is 
//				selected automatically during this process
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710AutoNegotiate (
    tmUnitSelect_t                       	phyUnitId,   
    pUInt32      	pAutoNegotiationMask
    );

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710LoopBack:
//
// DESCRIPTION: Function will enable or disable the PHY device in the Loopback mode.
//
// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710LoopBack (
    tmUnitSelect_t                      	phyUnitId,   
    PhyEnableDisable_t           		loopbackMode
	);


//-----------------------------------------------------------------------------
// FUNCTION:    PhySoftReset:
//
// DESCRIPTION: Function will do the soft reset of the PHY device

// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710SoftReset (
    tmUnitSelect_t                     		phyUnitId      
	);

//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetLinkStatus:
//
// DESCRIPTION: Function will get the link status

// RETURN:      TM_OK
//				TM_BSLPHY_ERR_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710GetLinkStatus (
    tmUnitSelect_t                          phyUnitId,   
    pPhyEnableDisable_t           		pLinkStatus
	);

#endif //_PHYSMSC8710_H_
