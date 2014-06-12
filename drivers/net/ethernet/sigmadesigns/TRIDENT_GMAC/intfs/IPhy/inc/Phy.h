/*
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
 * %filename:               Phy.h %
 * %pid_version:                 1.2      %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  
 *
 * DOCUMENT REF: 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
 */

#ifndef  TMBSLPHY_H
#define  TMBSLPHY_H
//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include "tmNxTypes.h"  // DVP standard types/defines/structs
#include "tmNxCompId.h" // DVP system wide component IDs

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//


//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
//


#define PHY_COMPATIBILITY_NR       1
#define PHY_MAJOR_VERSION_NR       1
#define PHY_MINOR_VERSION_NR       0



#define ERR_PHY_BASE          	CID_BSL_PHY
#define ERR_PHY_COMP          	(CID_BSL_PHY | \
                                        	 TM_ERR_COMP_UNIQUE_START)
//PHY unit not supported                                         	 
#define ERR_PHY_NOT_SUPPORTED 	(ERR_PHY_BASE+TM_ERR_NOT_SUPPORTED)
// Invalid device unit number
#define ERR_PHY_BAD_UNIT_NUM  	(ERR_PHY_BASE+TM_ERR_BAD_UNIT_NUMBER)
// AutoNegotiation Time out occured
#define ERR_PHY_AUTONEG_TIMEOUT   (ERR_PHY_BASE+TM_ERR_TIMEOUT)
//PHY unit register read failed
#define ERR_PHY_READ_FAILED       (ERR_PHY_BASE + TM_ERR_READ)
//PHY unit register write failed
#define ERR_PHY_WRITE_FAILED      (ERR_PHY_BASE + TM_ERR_WRITE)
//PHY Initialisation failed
#define ERR_PHY_INIT_FAILED       (ERR_PHY_BASE + TM_ERR_INIT_FAILED)
//PHY unit reset failed
#define ERR_PHY_RESET_FAILED      (ERR_PHY_BASE + TM_ERR_HW_RESET_FAILED)

// PHY DUPLEX MODE
typedef enum	_PhyDuplexMode_t
{
	PhyHalfDuplex	=0,
	PhyFullDuplex	=1
} PhyDuplexMode_t, *pPhyDuplexMode_t;

// PHY SPEED
typedef enum	_PhySpeed_t
{
	PhySpeed10Mbps	=0,
	PhySpeed100Mbps	=1,
	PhySpeed1Gbps	= 2	
} PhySpeed_t, *pPhySpeed_t;

// ENABLE DISABLE MODE
typedef enum	_PhyEnableDisable_t
{
	PhyDisable		=0,
	PhyEnable		=1
} PhyEnableDisable_t, *pPhyEnableDisable_t;

// PHY BASIC MODE CONTROL
typedef struct	_PhyBasicModeControl_t
{
	Bool					enableCollisionTest; 
	PhyDuplexMode_t	duplexMode;
	PhySpeed_t			speed;
} PhyBasicModeControl_t, *pPhyBasicModeControl_t;

// PHY BASIC MODE STATUS
typedef struct	_PhyBasicModeStatus_t
{
	Bool					jabberDetect; // This applies only in 10Mbps : value 1 - jabber condition detected
	Bool					remoteFaultDetected; 
	Bool					autoNegotiationComplete;
	Bool					receiverErrorLatch;
	Bool					invertedPolarityDetected; // Detects link polarity
	Bool					falseCarrierEverntOccured;
	Bool					linkCodeWordPageReceived;
	Bool					loopBackEnabled; //loopback status
	PhyDuplexMode_t	duplexMode; // deplex mode status
	PhySpeed_t			speed;
	Bool					linkpartnerAcknolwedged ; // Link partner acknolwedged
	Bool					parallelDetectFaultDetected ; // Parallel detect fault is deteced

} PhyBasicModeStatus_t, *pPhyBasicModeStatus_t;

// PHY PROTOCOL SELECTION
typedef enum	_PhyProtocol_t
{
	PhyProtocolNone		= 0,
	PhyIEEE8023			= 1
} PhyProtocol_t, *pPhyProtocol_t;

/* PHY auto negotiation mask 
** When the below flags are true, that particular option is disabled/not advertised
*/
typedef struct	_PhyAutoNegotiationMask_t
{
    Bool				      masknextPageDesired;
    Bool				      maskRemoteFault; // advertise remote fault detection
    Bool					mask100BaseT4;  //advertise/mask 100BaseT4 capability 
    Bool					mask100BaseTxFullDuplexSupport; 
    Bool					mask100BaseTxSupport;
    Bool					mask10BaseTFullDuplexSupport; //advertise/mask 10BaseTFullDuplex support
    Bool					mask10BaseTSupport;
    PhyProtocol_t		protocolSel;			//(0x0001 IEEE802.3 CSMA/Cd)
    Bool                             maskAsymmetricPause;
    Bool                             maskPauseFrame;    
    /* 1G related enum  start */
    Bool                            maskMSConfigEn;
    Bool                            maskMasterEn;
    Bool                            maskMultiPortEn;
    Bool                            mask1000BaseTFullDuplexSupport;
    Bool                            mask1000BaseTHalfDuplexSupport;    
    /* 1G related ends*/
} PhyAutoNegotiationMask_t, *pPhyAutoNegotitationMask_t;


// PHY IDENTIFIER
typedef struct	_PhyIdentifier_t
{
	UInt32		oui; //PHY Organizationally Unique Identifier
	UInt32		vendorModelNr; 
	UInt32		modelRevNr;
	UInt32		siliconRevisionNr; 
} PhyIdentifier_t, *pPhyIdentifier_t;


// PHY CAPABILITIES
typedef struct	_PhyCapabilities_t
{
    Bool			T4Support100Base;
    Bool			Tx_FullDuplexSupport100Base;
    Bool			Tx_HalfDuplexSupport100Base;
    Bool			Tx_FullDuplexSupport10Base;
    Bool			Tx_HalfDuplexSupport10Base;
    Bool		 	preAmbleSuppresionCapability;
    Bool		 	autoNegotiationAbility;
    Bool		 	nextPageIndication;
    Bool			force100MbpsTxOff; 
    Bool			bypassSymbolAlignment; 
    Bool			badSSDDetectionConfig; 
    Bool		 	ledStatusSupport;	   

    /* 1G related capabilities */
    Bool		 	X1000BaseFDSupport;	       
    Bool		 	X1000BaseHDSupport;	           
    Bool		 	T1000BaseFDSupport;	       
    Bool		 	T1000BaseHDSupport;	               

} PhyCapabilities_t, *pPhyCapabilities_t;


// PHY LINK PARTNER CAPABILITIES

typedef struct	_PhyLinkPartnerCapabilities_t
{

	PhyProtocol_t			protocolSel;
	Bool						TSupport10Base;
	Bool						TFullDuplexSupport10Base;
	Bool						TxSupport100Base;
	Bool						TxFullDuplexSupport100Base;
	Bool						T4Support100Base;
	Bool						flowControl;
	Bool						remoteFault;
	Bool						acknoweledges;
	Bool						nextPageIndication;
	Bool						autoNegotiation;	
} PhyLinkPartnerCapabilities_t, *pPhyLinkPartnerCapabilities_t;


// PHY PCS(PHYSICAL SUBBAND CODING
typedef struct	_PhyPcsConfig_t
{
    /* 1 - force good link condition
         0 - normal operation  */
    Bool forceGoodLink100Mbps; 
    Bool force100MbpsTxOff;
    Bool bypassSymbolAlignment;
    /* 1 - Enable bad SSD detection */
    PhyEnableDisable_t badSSDDetection; 
} PhyPcsConfig_t, *pPhyPcsConfig_t;

// PHY COUNTERS
typedef struct 	_PhyCounters_t
{
	UInt32			disconnectCounter;		
	UInt32			falseCarrierEventCounter;//Gives the number of false carrier events 
	UInt32			rxErrorCounter;	//Gives the number of Receive error occured 		
} PhyCounters_t, *pPhyCounters_t;


// PSEUDO RANDOM SEQUENCES
typedef enum	_PhyPseudoRandomSeq_t
{
	PhyPseudoRandomSequenc9bit	=0,
	PhyPseudoRandomSequenc15bit	=1
} PhyPseudoRandomSeq_t, *pPhyPseudoRandomSeq_t;


// PHY BYPASS OPTIONS
typedef struct	_PhyBypass_t
{
	Bool		bypass4B5BCodec; //Bypass 4B 5B Encoding / Decoding
	Bool		bypassNrzi;      //Bypass NRZI Encoding / Decoding
	Bool		bypassScrambler; 
	Bool		bypassDescrambler;
} PhyBypass_t, *pPhyBypass_t;  






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
//

typedef tmErrorCode_t
(*PhyGetSWVersion_t) (
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

typedef tmErrorCode_t
(*PhyGetCapabilities_t) (
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

//				ERR_PHY_BAD_UNIT_NUM
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

typedef tmErrorCode_t
(*PhyInit_t)(
    tmUnitSelect_t  						phyUnitId   
    );

    
    
//-----------------------------------------------------------------------------
// FUNCTION:    PhyDeinit:
//
// DESCRIPTION: This function de-initializes the PHY device. Once the device is
//				deinitilized device will no more be available to access 
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:       
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyDeinit_t)(
    tmUnitSelect_t  						phyUnitId   
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhySetPowerState:
//
// DESCRIPTION: This function will set the Power State of the PHY device to specified 
//				power state
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:       
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhySetPowerState_t)(
    tmUnitSelect_t           				phyUnitId ,  
    tmPowerState_t          				phyPowerState
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetPowerState:
//
// DESCRIPTION: This function will get the preset power state of the PHY device
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:       
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyGetPowerState_t)(
    tmUnitSelect_t                  		phyUnitId ,  
    ptmPowerState_t							phyPowerState
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetBasicModeControl:
//
// DESCRIPTION: This function will get the basic configuration of the PHY device. 
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
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

typedef tmErrorCode_t 
(*PhyGetBasicModeControl_t) (
    tmUnitSelect_t                   		phyUnitId,   
    pPhyBasicModeControl_t      		pPhyBasicModeControl
    );

//-----------------------------------------------------------------------------
// FUNCTION:    PhySetBasicModeControl:
//
// DESCRIPTION: This function will configure the PHY device for the Basic Mode. 
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t 
(*PhySetBasicModeControl_t) (
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
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyGetBasicModeStatus_t) (
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
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyAutoNegotiate_t) (
    tmUnitSelect_t                       	phyUnitId,   
    pUInt32     	pAutoNegotiationMask
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetIdentifier:
//
// DESCRIPTION: This function gets the PHY device Identifier
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyGetIdentifier_t) (
    tmUnitSelect_t               			phyUnitId,   
    pPhyIdentifier_t     				pPhyIdentifier
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetLinkPartnerCapabilities:
//
// DESCRIPTION: This function will get the Link Partner Capabilities. 
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyGetLinkPartnerCapabilities_t) (
    tmUnitSelect_t                         phyUnitId,   
    pPhyLinkPartnerCapabilities_t     pPhyLinkPartnerCapabilities
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetCounters:
//
// DESCRIPTION: This function will get the present counter values of the PHY counters
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyGetCounters_t) (
    tmUnitSelect_t                  		phyUnitId,   
    pPhyCounters_t      		 		pPhyCounters
    );

//-----------------------------------------------------------------------------
// FUNCTION:    PhyPcsSetConfig:
//
// DESCRIPTION: This function will configure the Physical layer configurations. 
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyPcsSetConfig_t) (
    tmUnitSelect_t                 			phyUnitId,   
    pPhyPcsConfig_t    				pPhyPcsConfig
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyPcsGetConfig:
//
// DESCRIPTION: This function will get the present PCS configuration 
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyPcsGetConfig_t) (
    tmUnitSelect_t                 			phyUnitId,   
    pPhyPcsConfig_t    				pPhyPcsConfig
    );


//-----------------------------------------------------------------------------
// FUNCTION:    PhyBist:
//
// DESCRIPTION: This function will do the Built In Self Test and the result 
//				will be indicated in the pBistState
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//              pBistState  --> result of BIST (Built in self test) Test 
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyBist_t) (		
    tmUnitSelect_t  						phyUnitId,   
    PhyPseudoRandomSeq_t    			phyPsedoRandomSeq ,
    pUInt8                              	pBistState
	);

//-----------------------------------------------------------------------------
// FUNCTION:    PhyConfigBypass:
//
// DESCRIPTION: This function will bypass the functional blocks within 100Base-Tx transmitter
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyConfigBypass_t) (
    tmUnitSelect_t                        	phyUnitId,   
    pPhyBypass_t                     	pPhyBypass
	);


//-----------------------------------------------------------------------------
// FUNCTION:    PhyLoopBack:
//
// DESCRIPTION: Function will enable or disable the PHY device in the Loopback mode.
//
// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyLoopBack_t) (
    tmUnitSelect_t                      	phyUnitId,   
    PhyEnableDisable_t           		loopbackMode
	);


//-----------------------------------------------------------------------------
// FUNCTION:    PhySoftReset:
//
// DESCRIPTION: Function will do the soft reset of the PHY device

// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhySoftReset_t) (
    tmUnitSelect_t                     		phyUnitId      
	);

//-----------------------------------------------------------------------------
// FUNCTION:    PhyGetLinkStatus:
//
// DESCRIPTION: Function will get the link status

// RETURN:      TM_OK
//				ERR_PHY_BAD_UNIT_NUM
//
// NOTES:      
//-----------------------------------------------------------------------------
//

typedef tmErrorCode_t
(*PhyGetLinkStatus_t) (
    tmUnitSelect_t                          phyUnitId,   
    pPhyEnableDisable_t           		pLinkStatus
);

typedef struct _PhyConfig_t 
{
	char     phyName[HAL_DEVICE_NAME_LENGTH];
    UInt32   phyID;    
	UInt32   lanClkSrc; // To select Clock source
	UInt32   lanClkSpeed; // To select the speed
	
	PhyGetSWVersion_t            getSWVersionFunc; 
	PhyGetCapabilities_t         getCapabilitiesFunc;
	PhyInit_t                    initFunc;
	PhyDeinit_t                  deinitFunc;  
	PhySetPowerState_t           setPowerStateFunc;
	PhyGetPowerState_t           getPowerStateFunc;
	PhyGetBasicModeControl_t     getBasicModeControlFunc;
	PhySetBasicModeControl_t     setBasicModeControlFunc;
	PhyGetBasicModeStatus_t      getBasicModeStatusFunc;
	PhyAutoNegotiate_t           autoNegotiateFunc;
	PhyGetIdentifier_t           getIdentifier_tFunc;
	PhyGetLinkPartnerCapabilities_t  getLinkPartnerCapabilitiesFunc;
	PhyGetCounters_t                 getCountersFunc;
	PhyPcsSetConfig_t                   pcsConfigFunc;
	PhyPcsGetConfig_t                pcsGetConfigFunc;
	PhyBist_t                        bistFunc;
	PhyConfigBypass_t                configBypassFunc;
	PhyLoopBack_t                    loopBackFunc;
	PhySoftReset_t                   softResetFunc;
	PhyGetLinkStatus_t               getLinkStatusFunc;

} PhyConfig_t,*pPhyConfig_t;

typedef struct {
UInt32 phyID;
PhyConfig_t *interface_fns;
}phyID_interface_tbl;

void PhyGetInterface(int unitno, UInt32 phyID, UInt32 isExternal, PhyConfig_t**pPhyInterface);

#endif //#ifndef  TMBSLPHYDP83847_H

