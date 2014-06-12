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
 * %filename:     PhySMSC8710.c %
 * %pid_version:           1.2                %
 *---------------------------------------------------------------------------
 * DESCRIPTION: Macros and function prototypes for SMSC8710 PHY
 *
 * DOCUMENT REF: Datasheet SMSC LAN8700/LAN8700i  
 *               Revision 2.1 (03-06-09)
 *
 *
 *-----------------------------------------------------------------------------
 *
*/

#include "../../../intfs/IPhy/inc/tmNxTypes.h"
#include "../../../intfs/IPhy/inc/tmNxCompId.h"
#include <linux/mii.h>
/*  Project include files */

#include "../../../intfs/IPhy/inc/Phy.h"
#include "../../../comps/PhySMSC8710/inc/PhySMSC8710.h"
#include "../../../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h"

/* Timeout in case of linux */
#ifdef __TRIDENT_GMAC__
#include <linux/delay.h>
#include "../../../src/remap.h"
#include <asm/io.h>
#if defined(CONFIG_MIPS_TRIDENT_FUSION)

#include <asm/mach-trifusion/trihidtv.h>
#include <asm/mach-trifusion/trihidtv_int.h>

#endif 

#define AUTO_NEG_DELAY_MS (3000)
#endif /* __LINUX_GMAC_DRV__*/

/* Defines */
#define PHY_UNIT_ID_COUNT  			1

#define TMBSLPHYSMSC8710_PHY_MMIO_ADDRESS0 (0xC0000000) 

#define ANAR_DEFAULT_VAL (0x1E1)

/* Global Data */
 /* Initialize with base address of GMAC. This will be used to access MAC Address & MAC data register */

typedef enum _PhySMSC8710Reg_t
{
    /* Basic mode control */
    PhySMSC8710Bmcr		      = 0,  

    /* Basic mode status */    
    PhySMSC8710Bmsr		      =1,  

    /* PHY ID1 register */        
    PhySMSC8710PhyIdr1		=2,

    /* PHY ID2 register */            
    PhySMSC8710PhyIdr2		=3,

    /* Auto negotiation advertisement register */                
    PhySMSC8710Anar			=4,

    /* Auto negotiation link partner ability register */                    
    PhySMSC8710Anlpar		=5,

    /* Auto negotiation expansion register */                        
    PhySMSC8710Aner			=6,

    /* 0x7 to 0xF reserved-*/

    /* Silicon Revision register */
    PhySMSC8710Srr=16,

    /* Mode control & status register */
    PhySMSC8710Mcsr=17,

    /* Special modes register */
    PhySMSC8710Smr=18,

    /* 19-25 reserved */

    /* Symbol error counter register */    
    PhySMSC8710Secr	= 26,
    
    /* Control/Status indication register */        
    PhySMSC8710Csir = 27,    

    /* Special internal testability controls register*/            
    PhySMSC8710Sitcr = 28,        
    
    /* Interrupt source register */
    PhySMSC8710Isr = 29,

    /* Interrupt mask register */
    PhySMSC8710Imr = 30,    

    /* PHY special control & status register */    
    PhySMSC8710Pscsr = 31,    

} PhySMSC8710Reg_t, *pPhySMSC8710Reg_t;


/* Static functions definition */

static tmErrorCode_t 
PhySMSC8710Read (
	tmUnitSelect_t				PhyAddr,
    PhySMSC8710Reg_t				reg,   
    pUInt16						pVal
	);
	
	
static tmErrorCode_t	
PhySMSC8710Write (
    tmUnitSelect_t				PhyAddr,
    PhySMSC8710Reg_t				reg,   
    UInt16						val
	);

#ifdef __TRIDENT_GMAC__
//static void get_phy_out_of_rst( tmUnitSelect_t	PhyAddr);
#endif
    /* Read the data from the data register */

/* Exported functions */

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710GetSWVersion:
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
	ptmSWVersion_t					pPhyVersion
	)
{
	
	pPhyVersion->compatibilityNr = PHY_COMPATIBILITY_NR;
	pPhyVersion->majorVersionNr = PHY_MAJOR_VERSION_NR;
	pPhyVersion->minorVersionNr = PHY_MINOR_VERSION_NR;
	
	return TM_OK;

}

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710GetCapabilities:
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
// NOTES:       This API can be called anytime i.e. before initializing the PHY. 
//				 
//-----------------------------------------------------------------------------


tmErrorCode_t
PhySMSC8710GetCapabilities (
    tmUnitSelect_t                			PhyAddr,  
    pPhyCapabilities_t  				pPhyCaps    
    )

{

    UInt16 bmsr;

    /*  Read the PHY capabilites from the BMSR register */
    PhySMSC8710Read(PhyAddr, PhySMSC8710Bmsr, &bmsr);
    //printk("bmsr = %d\n",bmsr);
    pPhyCaps->T4Support100Base = 
        (((bmsr & PHYSMSC8710_BMSR_T4100BASE) > 0) ? True : False);

    pPhyCaps->Tx_FullDuplexSupport100Base = 
        (((bmsr &PHYSMSC8710_BMSR_X100BASEFD) > 0) ? True : False);

    pPhyCaps->Tx_HalfDuplexSupport100Base = 
        (((bmsr &PHYSMSC8710_BMSR_X100BASEHD) > 0) ? True : False);

    pPhyCaps->Tx_FullDuplexSupport10Base = 
        (((bmsr &PHYSMSC8710_BMSR_10MBPSFD) > 0) ? True : False);        

    pPhyCaps->Tx_HalfDuplexSupport10Base = 
        (((bmsr &PHYSMSC8710_BMSR_10MBPSHD) > 0) ? True : False);


    pPhyCaps->autoNegotiationAbility = 
                (((bmsr &PHYSMSC8710_BMSR_AN_ABLE) > 0) ? True : False);

    pPhyCaps->ledStatusSupport = True ;		

    /* Other capabilites set to False */   
    pPhyCaps->nextPageIndication = False;

    pPhyCaps->force100MbpsTxOff = False;

    pPhyCaps->bypassSymbolAlignment =False;

    pPhyCaps->badSSDDetectionConfig = False;
	
    return TM_OK;

}   
    
//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710Init:
//
// DESCRIPTION: This function initializes the PHY device. It should be called
//				before any access to the device is made. 
//
// RETURN:      TM_OK 
//
// NOTES:       This function initializes the PHY device with the following 
//				default initial configuration. No Autonegotiation is done in the 
//				initialization function
//-----------------------------------------------------------------------------

tmErrorCode_t
PhySMSC8710Init(
    tmUnitSelect_t  						PhyAddr   
    )
{

    //get_phy_out_of_rst(PhyAddr);
    return TM_OK;

}   

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710Deinit:
//
// DESCRIPTION: This function de-initializes the PHY device. Once the device is
//				deinitilized device will no more be available to access 
//
// RETURN:      TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710Deinit(
    tmUnitSelect_t  						PhyAddr   
    )

{

    tmErrorCode_t       		ethStatus=TM_OK;     		
    UInt16 bmcr =0;

    bmcr |= PHYSMSC8710_BMCR_RST_VAL;

    /* All the registers will be reset */
    ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Bmcr,bmcr);
    if(ethStatus != TM_OK)
    {
        return ethStatus;
    }
    else	
    {
        return TM_OK;
    }


}   

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710SetPowerState:
//
// DESCRIPTION: This function will set the Power State of the PHY device to specified 
//				power state 
//
// RETURN:      TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------

tmErrorCode_t
PhySMSC8710SetPowerState(
    tmUnitSelect_t           				PhyAddr ,  
    tmPowerState_t          				phyPowerState
    )

{
    tmErrorCode_t ethStatus=TM_OK;     		
    UInt16 bmcr = 0;

    ethStatus = PhySMSC8710Read(PhyAddr, PhySMSC8710Bmcr, &bmcr);

    if(ethStatus != TM_OK)
    {
        return ethStatus;
    }

    if( (phyPowerState == tmPowerOn) || (phyPowerState == tmPowerOff) )
    {
        if(phyPowerState == tmPowerOff)
        {
            bmcr |= PHYSMSC8710_BMCR_PWRDN_EN;
        }
        else
        {
            bmcr &=PHYSMSC8710_BMCR_PWRDN_CLR;
        }

        ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Bmcr,bmcr);

        return ethStatus;

    }
    else
    {
        return ERR_PHY_NOT_SUPPORTED;
    }

}   

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710GetPowerState:
//
// DESCRIPTION: This function will get the preset power state of the PHY device
//
// RETURN:      TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------

tmErrorCode_t
PhySMSC8710GetPowerState(
    tmUnitSelect_t                  		PhyAddr ,  
    ptmPowerState_t				phyPowerState
    )

{

    tmErrorCode_t       		ethStatus=TM_OK;     		
    UInt16 regVal=0;

    ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmcr,&regVal);

    if(ethStatus != TM_OK)
    {
        return ethStatus ;
    }

    *phyPowerState = (((regVal & PHYSMSC8710_BMCR_PWRDN_EN) > 0) ? tmPowerOff : tmPowerOn);

    return TM_OK;

}   


//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710GetBasicModeControl:
//
// DESCRIPTION: This function will get the basic configuration of the PHY device. 
//
// RETURN:      TM_OK
//
// NOTES:       See #define for the Basic Mode Control 
//-----------------------------------------------------------------------------

tmErrorCode_t 
PhySMSC8710GetBasicModeControl (
    tmUnitSelect_t                   		PhyAddr,   
    pPhyBasicModeControl_t       pPhyBasicModeControl
    )
{
    tmErrorCode_t ethStatus=TM_OK;     		
    UInt16 bmcr;

    /* Read the present settings of the BMCR register */
    ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmcr,&bmcr);

    if(ethStatus != TM_OK)
    {
        return ethStatus;
    }

    pPhyBasicModeControl->enableCollisionTest = 
                (((bmcr & PHYSMSC8710_BMCR_COLTEST ) > 0) ? True : False);

    pPhyBasicModeControl->duplexMode = 
                (((bmcr & PHYSMSC8710_BMCR_FD_EN ) > 0) ? PhyFullDuplex : PhyHalfDuplex);

    if(bmcr & PHYSMSC8710_BMCR_SPEED_MSK)
    {
        pPhyBasicModeControl->speed = PhySpeed100Mbps;            
    }
    else
    {
        pPhyBasicModeControl->speed = PhySpeed10Mbps;                            
    }

    return TM_OK;

}   

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710SetBasicModeControl:
//
// DESCRIPTION: This function will configure the PHY device for the Basic Mode. 
//
// RETURN:      TM_OK
//
// NOTES:      
//-----------------------------------------------------------------------------

tmErrorCode_t 
PhySMSC8710SetBasicModeControl (
    tmUnitSelect_t                           PhyAddr,   
    pPhyBasicModeControl_t        pPhyBasicModeControl
    )
{

    tmErrorCode_t       		ethStatus=TM_OK;     		
    UInt16 bmcr =0;
    UInt16 regval;

    if(pPhyBasicModeControl->enableCollisionTest == True)
    {
        bmcr |= PHYSMSC8710_BMCR_COLTEST;
    }

    switch(pPhyBasicModeControl->speed)
    {

        case PhySpeed100Mbps :       
//            bmcr |= PHYSMSC8710_BMCR_SPEED_100;                
            /* Set the mode register to default value */
            ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Smr,&regval);

            /* Clear the mode bits */
            regval &= 0xFF1F;

            if(pPhyBasicModeControl->duplexMode == True)
            {
                regval |= 0x60; /*100Base T full duplex, autoneg disabled */                            
            }
            else
            {
                regval |= 0x40; /* 100Base T half duplex, autoneg disabled */
            }

            ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Smr,regval);        

            ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Smr,&regval);
//            printk(KERN_ERR"\nSMR val: %08x\n",regval);           
            break;

        case PhySpeed10Mbps :       
//            bmcr |= PHYSMSC8710_BMCR_SPEED_10;                
            ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Smr,&regval);

            /* Clear the mode bits */
            regval &= 0xFF1F;

            if(pPhyBasicModeControl->duplexMode == True)
            {
                regval |= 0x20; /*10 Base T full duplex, autoneg disabled */                            
            }
            else
            {
                regval |= 0x00; /* 10 Base T half duplex, autoneg disabled */
            }

            ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Smr,regval);        
            ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Smr,&regval);
//            printk(KERN_ERR"\nSMR val: %08x\n",regval);           
                
            break;

        default:
            break;

    }

    ethStatus = PhySMSC8710SoftReset(PhyAddr);

    mdelay(1);

//    printk(KERN_ERR"\nSPEED/MODE updated from MODE pins\n");               

    ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Pscsr,&regval);
//    printk(KERN_ERR"\nPHY STS reg:%04x\n",regval);               

    if(ethStatus != TM_OK)
    {
        //printk(KERN_ERR"\nRESET failed after mode change\n");           
    }

    /* Write the result to the BMC register */
//    ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Bmcr,bmcr);

    return ethStatus;

}   


//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710GetBasicModeStatus:
//
// DESCRIPTION: This function will get the Basic Mode Status of the PHY device 
//				such as the speed, duplex mode and other statuses
//
// RETURN:      TM_OK
//
// NOTES:      
//-----------------------------------------------------------------------------

tmErrorCode_t
PhySMSC8710GetBasicModeStatus (
    tmUnitSelect_t                  		PhyAddr,   
    pPhyBasicModeStatus_t    	pPhyBasicModeStatus     
    )

{
    tmErrorCode_t ethStatus=TM_OK;     		
    UInt16           bmsr,bmcr,phySts;

    /* Read the PHY status from the BMSR register */
    ethStatus = PhySMSC8710Read(PhyAddr, PhySMSC8710Bmsr, &bmsr);

    if(ethStatus != TM_OK)
    {
        return ethStatus ;
    }

    /* Read the PHY control register from the BMCR register */
    ethStatus = PhySMSC8710Read(PhyAddr, PhySMSC8710Bmcr, &bmcr);

    if(ethStatus != TM_OK)
    {
        return ethStatus ;
    }

    /* Read the autonegotiation status from PHY status register */
    ethStatus = PhySMSC8710Read(PhyAddr, PhySMSC8710Pscsr, &phySts);

    if(ethStatus != TM_OK)
    {
        return ethStatus ;
    }

    pPhyBasicModeStatus->jabberDetect = (((bmsr & PHYSMSC8710_BMSR_JAB_VAL) > 0) ? True : False);

    pPhyBasicModeStatus->remoteFaultDetected = (((bmsr & PHYSMSC8710_BMSR_RF_VAL) > 0) ? True : False);

    pPhyBasicModeStatus->autoNegotiationComplete = (((bmsr & PHYSMSC8710_BMSR_AN_VAL) > 0) ? True : False);

    pPhyBasicModeStatus->loopBackEnabled = (((bmcr & PHYSMSC8710_BMCR_LPBK_VAL) > 0) ? True : False);

    if((bmcr & PHYSMSC8710_BMCR_AN_EN) == 0) 
    {
        /* Not an auto negotiation. So read the values from BMCR */
        pPhyBasicModeStatus->duplexMode = 
                (((bmcr & PHYSMSC8710_BMCR_FD_EN ) > 0) ? PhyFullDuplex : PhyHalfDuplex);

        if(bmcr & PHYSMSC8710_BMCR_SPEED_MSK)
        {
            pPhyBasicModeStatus->speed = PhySpeed100Mbps;            
        }
        else
        {
            pPhyBasicModeStatus->speed = PhySpeed10Mbps;                        
        }

    }
    else 
    {
        /* If autonegotiation is enabled, read from PHYSTS register */
        pPhyBasicModeStatus->duplexMode = 
                (((phySts & PHYSMSC8710_PHYSTS_DUP_MODE) > 0) ? PhyFullDuplex : PhyHalfDuplex);

        pPhyBasicModeStatus->speed= 
                (((phySts & PHYSMSC8710_PHYSTS_SPEED_MSK) == 0x4) ? PhySpeed10Mbps : PhySpeed100Mbps);

    }

    return TM_OK;

}   


//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710AutoNegotiate:
//
// DESCRIPTION: This function will enable the Auto negotiation of the PHY device 
//				with Link Partner. Best possible performance configuration is 
//				selected automatically during this process
//
// RETURN:      TM_OK
//
// NOTES:      
//-----------------------------------------------------------------------------
 
tmErrorCode_t
PhySMSC8710AutoNegotiate (
    tmUnitSelect_t                       	      PhyAddr,   
    pUInt32     	pAutoNegotiationMask)
{
#if 0
	tmErrorCode_t   ethStatus=TM_OK;
	UInt16 advertise, lpa;
	ethStatus = Read_Phy_Reg (PhyAddr,PhySMSC8710Anar,&advertise);
	ethStatus = Read_Phy_Reg (PhyAddr,PhySMSC8710Anlpar,&lpa);
	*pAutoNegotiationMask = ( advertise & 0x0000ffff );
	*pAutoNegotiationMask |=((lpa << 16) & 0xffff0000 );
#else
	UInt16 advertise;
	Read_Phy_Reg(PhyAddr,PhySMSC8710Mcsr,&advertise);
	if( (advertise & 0x400 ) && (advertise & 0x800 )) {// link status 
		*pAutoNegotiationMask  = advertise;
	}
#endif 
	return 0;
}   

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710LoopBack:
//
// DESCRIPTION: Function will enable or disable the PHY device in the Loopback 
//				mode.
//
// RETURN:      TM_OK
// NOTES:      
//-----------------------------------------------------------------------------


tmErrorCode_t
PhySMSC8710LoopBack (
    tmUnitSelect_t                  PhyAddr,   
    PhyEnableDisable_t     loopbackMode
    )
{

    tmErrorCode_t ethStatus=TM_OK;     		
    UInt16 bmcr;

    /* Read the existing settings of the BMCR register */
    ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmcr,&bmcr);

    if(ethStatus != TM_OK) {
        return ethStatus ;
    }

    if(PhyEnable == loopbackMode) {
        bmcr |= PHYSMSC8710_BMCR_LPBK_VAL;
    } else if(PhyDisable == loopbackMode) {
        bmcr &= PHYSMSC8710_BMCR_LPBK_CLR;
    } else {
        ethStatus = ERR_PHY_NOT_SUPPORTED;
    }

    /* Write the Loopback setting to the BMCR register */
    ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Bmcr,bmcr);

    return ethStatus;

}   


//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710SoftReset:
//
// DESCRIPTION: Function will do the soft reset of the PHY device

// RETURN:      TM_OK
//
// NOTES:      
//-----------------------------------------------------------------------------
//

tmErrorCode_t
PhySMSC8710SoftReset (
    tmUnitSelect_t                     		PhyAddr      
	)

{
    tmErrorCode_t       		ethStatus=TM_OK;     		

    /* All the registers will be reset */
    ethStatus = PhySMSC8710Write(PhyAddr,PhySMSC8710Bmcr,PHYSMSC8710_BMCR_RST_VAL);

    return ethStatus;

}   

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710GetLinkStatus:
//
// DESCRIPTION: Function will get the link status

// RETURN:      TM_OK
//
// NOTES:      
//-----------------------------------------------------------------------------

tmErrorCode_t
PhySMSC8710GetLinkStatus (
    tmUnitSelect_t                          PhyAddr,   
    pPhyEnableDisable_t            pLinkStatus
    )
{
    tmErrorCode_t ethStatus=TM_OK;     		
    UInt16 physts;

    /* Read the BMSR register twice, as per datasheet */
    ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmsr,&physts);    

    if(ethStatus != TM_OK) {
        return ethStatus ;
    }

    ethStatus = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmsr,&physts);    

    if(ethStatus != TM_OK) {
        return ethStatus ;
    }

    *pLinkStatus =
        (((physts & BMSR_LSTATUS ) > 0) ? PhyEnable : PhyDisable);

    return(TM_OK);

}   

//-----------------------------------------------------------------------------
//	Local static functions	
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710Read:
//
// DESCRIPTION: Function will read from the Specified PHY register

// RETURN:      
//
// NOTES:      
//-----------------------------------------------------------------------------
//

static tmErrorCode_t
PhySMSC8710Read (
    tmUnitSelect_t				PhyAddr,
    PhySMSC8710Reg_t		reg,   
    pUInt16						pVal
	)
{
	return  Read_Phy_Reg( PhyAddr,reg,pVal);

}   


//-----------------------------------------------------------------------------
// FUNCTION:    PhySMSC8710Write:
//
// DESCRIPTION: Function will Write to the Specified PHY register

// RETURN:      
//
// NOTES:      
//-----------------------------------------------------------------------------
//

static tmErrorCode_t
PhySMSC8710Write (
    tmUnitSelect_t				PhyAddr,
    PhySMSC8710Reg_t		reg,   
    UInt16						val
	)

{
#if 0	
	Write_Phy_Reg(PhyAddr,reg, val);
#endif 	
	return TM_OK;
    
}   

#if 0
static void get_phy_out_of_rst( tmUnitSelect_t	PhyAddr)
{
	Int32 status;
	Int16 data;
	Int32 rcount;
 /*software reset */

	status = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmcr, &data);

	status = Write_Phy_Reg(PhyAddr,PhySMSC8710Bmcr, data|PHYSMSC8710_BMCR_RST_VAL);

	rcount = 100;
	while((--rcount) > 0) {
		status = PhySMSC8710Read(PhyAddr,PhySMSC8710Bmcr, &data);
		if(PHYSMSC8710_BMCR_RST_VAL& data) {  
			/* PHY not ready */
			mdelay(10);
			continue;
		} else {
			/* PHY is  ready */
			break;
		} 
	}
	if(rcount <= 0) {
		printk("Error rcount = %d\n", (int)rcount);
	}
    return;

}
#endif