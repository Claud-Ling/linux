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
 * %filename:     hwTRIDENTGMACEth.c %
 * %pid_version:          1.7                  %
 *---------------------------------------------------------------------------
 * DESCRIPTION:   Scalable HwApi Driver for Ethernet GMAC
 *
 *  DOCUMENT REF: Synopsys DesignWare Ethernet Universal Databook 
 *                         Version 3.41a, February 7, 2008 
  *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
 */

/*-----------------------------------------------------------------------------
/ Standard include files:
/-----------------------------------------------------------------------------
*/

#include "../../../intfs/IPhy/inc/tmNxTypes.h"

/*-----------------------------------------------------------------------------
/ Project include files:
/-----------------------------------------------------------------------------
*/
#include "../../../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h"
#include "../../../comps/hwTRIDENTGMACEth/inc/hwTRIDENTGMACEth.h"
#include "../../../comps/hwTRIDENTGMACEth/src/hwTRIDENTGMACEth_Vhip.h"

#include <linux/kernel.h>
#include <linux/mii.h>

#ifdef CONFIG_ARCH_SIGMA_TRIX
#include <plat/gmac_eth_drv.h>
#endif

/*-----------------------------------------------------------------------------
/ Types and defines:
/-----------------------------------------------------------------------------
*/
#if (  TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC) 
    
    #if ( IPVERSION_34_1_A)
    #define MMC_RST_ON_RD_SAVE(_base_, _val_) \
    {\
        UInt32 _tmp_;\
        TMVH_GEN_READ(_base_+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,_val_);\
        _tmp_ = _val_;\
        _tmp_ &= ~TMVH_TRIDENTGMACETH_MMC_CTRL_RESET_ON_RD_VAL;\
        TMVH_GEN_WRITE(_base_+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,_tmp_);\
    }

    #define MMC_RST_ON_RD_RESTORE(_base_,_val_) \
        TMVH_GEN_WRITE(_base_+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,_val_)
    #else
    #define MMC_RST_ON_RD_SAVE(_base_, _val_) 
    #define MMC_RST_ON_RD_RESTORE(_base_,_val_) 
    #endif

#endif

/*-----------------------------------------------------------------------------
/ Global data:
/-----------------------------------------------------------------------------
*/
#if (  TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC) 

    #if ( IPVERSION_34_1_A)
    static UInt32 ghwTRIDENTGMACEth_ActRegVal[14];
    #endif

#endif

#define GMAC_PRINT_DBG(args...) //printk(KERN_INFO"GMAC:" args)
/*-----------------------------------------------------------------------------
/ Internal Prototypes:
/-----------------------------------------------------------------------------
*/

#if (  TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC) 

    #if ( IPVERSION_34_1_A)
    UInt32  compute_ipc_value (UInt32 pRegs,UInt32 regOffset,UInt32 index);
    #endif

#endif


/*-----------------------------------------------------------------------------
/ Exported functions:
/-----------------------------------------------------------------------------
*/

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_GetSWVersion:
**
** DESCRIPTION: This function returns the Ethernet HWAPI device interface software 
** version information.Higher-level software layers to ensure that the 
** Ethernet HWAPI exports the expected function interface version typically
** call it.
**
** RETURN:      TM_OK
**
** NOTES:   This API can be called anytime i.e. before initializing the Ethernet 
**          or in PowerOff state.
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetSWVersion (
    ptmSWVersion_t  pEthGmacVersion    
    )
{
    pEthGmacVersion->compatibilityNr = HW_TRIDENTGMACETH_COMPATIBILITY_NR;
    pEthGmacVersion->majorVersionNr  = HW_TRIDENTGMACETH_MAJOR_VERSION_NR;
    pEthGmacVersion->minorVersionNr  = HW_TRIDENTGMACETH_MINOR_VERSION_NR;
    return (TM_OK);
}    

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_GetHWVersion:
**
** DESCRIPTION: This function returns the hardware version number of the
**                      Ethernet unit passed as parameter to this function.
**
** RETURN:      TM_OK
**
** NOTES:       This API can be called anytime i.e. before initializing the Ethernet
**                   or in PowerOff state
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetHWVersion(
    tmUnitSelect_t                    ethUnitId,  
    pUInt32                             pHWVersion
    ) 
{
    UInt32 pRegs; 
    
    pRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_VERSION_REG_OFFSET,*pHWVersion);

    return (TM_OK);

}        

#endif
Int32 Read_Phy_Reg(UInt32 PhyAddr,UInt32 Reg,pUInt16 pVal)
{
	UInt32 addr = 0;
	UInt32 pEthRegs,timeout = 0;
	volatile UInt32  *pAdrReg;
	volatile UInt32  *pDataReg;
	/* get Ethernet Module Reg Pointer  */
	pEthRegs = GET_BASE( 0 );
	/* Get the Address register */
	pAdrReg = (UInt32*)(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET);
	pDataReg = (UInt32*)(pEthRegs+TMVH_TRIDENTGMACETH_GMII_DATA_REG_OFFSET);
	/* Program the Register address in the MII */
	addr |= PhyAddr << TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_POS &\
		TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_MSK;
	addr |= Reg << TMVH_TRIDENTGMACETH_GMII_GMII_REGISTER_POS &\
		TMVH_TRIDENTGMACETH_GMII_PHY_REGISTER_MSK;
	addr |= 0x00000004;   //clk
	addr |= HW_TRIDENTGMACETH_GMII_BUSY_VAL;

	WriteRegWord((volatile void *)pAdrReg, addr);
	do {
		timeout++;
	}while ((ReadRegWord((volatile void *)pAdrReg) & \
		HW_TRIDENTGMACETH_GMII_BUSY_VAL) && (timeout < TRIDENT_PHY_TIMEOUT) );
	if(timeout < TRIDENT_PHY_TIMEOUT){
		timeout = 0;
		do{
			timeout++;
		}while ( (ReadRegWord((volatile void *)pDataReg) == \
		0xFFFF) && (timeout < TRIDENT_PHY_TIMEOUT) );

		*pVal = (ReadRegWord((volatile void *)pDataReg) & 0xFFFF);
		return TM_OK;
	} else{
		printk("Can't read PHY register !\n");
		return TM_ERR_READ;
	}
}

Int32 Write_Phy_Reg(UInt32 PhyAddr,UInt32 Reg, UInt16 data)
{
	UInt32 addr = 0;
	UInt32 pEthRegs,timeout = 0;
	volatile UInt32  *pAdrReg;
	volatile UInt32  *pDataReg;
	addr |= PhyAddr << TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_POS &\
		TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_MSK;
	addr |= Reg << TMVH_TRIDENTGMACETH_GMII_GMII_REGISTER_POS &\
		TMVH_TRIDENTGMACETH_GMII_PHY_REGISTER_MSK;

	addr |= 0x00000004;   //clk
	addr |= 0x00000002;   //write
	addr |= HW_TRIDENTGMACETH_GMII_BUSY_VAL;

	/* get Ethernet Module Reg Pointer  */
	pEthRegs = GET_BASE( 0 );
	/* Get the Address register */
	pAdrReg = (UInt32*)(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET);
	pDataReg = (UInt32*)(pEthRegs+TMVH_TRIDENTGMACETH_GMII_DATA_REG_OFFSET);
	/* Clear the bit GMII write for read operation */
	WriteRegWord((volatile void *)pDataReg, data);
	WriteRegWord((volatile void *)pAdrReg, addr);
	do {
		timeout++;

	}while ((ReadRegWord((volatile void *)pAdrReg) & \
		HW_TRIDENTGMACETH_GMII_BUSY_VAL) && (timeout < TRIDENT_PHY_TIMEOUT) );
	if(timeout < TRIDENT_PHY_TIMEOUT)
		return TM_OK;
	else 
		return TM_ERR_WRITE;
}



tmErrorCode_t hwTRIDENTGMACEth_get_phyInfo(tmUnitSelect_t  ethUnitId, UInt32* phyID, UInt32* phyAddr)
{
    UInt32 ijk, ret_val;
    UInt16 ID0Reg=0, ID1Reg=0;

    *phyAddr = 0xFFFF;
    *phyID = 0xFFFF;

    /* Auto Detect for first available PHY */
    for(ijk= 0; ijk < 32; ijk++) {

            /* Read the PHY ID0 */
            ret_val = Read_Phy_Reg( ijk, MII_PHYSID1, &ID0Reg);
	    if( ret_val == TM_ERR_READ ){
		    if( ijk < 32 )
			continue;
		    else
			return TM_ERR_INIT_FAILED;
	    }
            if( ID0Reg > 0 && ID0Reg < 0xFFFF ){            /* Read the PHY ID1 */
		    ret_val =  Read_Phy_Reg( ijk, MII_PHYSID2, &ID1Reg); 
		    if( ret_val == TM_ERR_READ ){
			    if( ijk < 32 )
				continue;
			    else
				return TM_ERR_INIT_FAILED;
		    }
		    if( ID1Reg > 0 && ID1Reg < 0xFFFF ) {
			    *phyAddr = ijk;
			    *phyID = (ID0Reg <<16) | ID1Reg;
			    printk("Found phyAddr=%lx\tphyID=0x%lx\n",*phyAddr,*phyID);
			    /* PHY found marker */
			return TM_OK;
		    }
	    }
    }
    /* We are OK if PHY is found */
    return TM_ERR_INIT_FAILED;
}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_Init:
**
** DESCRIPTION: This function initializes the Ethernet device hardware. 
**                      It should be called before any access to the device is made. 
**                      This function resets the MAC
**
** RETURN:      TM_OK
**
** NOTES:       
**-----------------------------------------------------------------------------
*/


tmErrorCode_t
hwTRIDENTGMACEth_Init(
    tmUnitSelect_t  ethUnitId,
    hwTRIDENTGMACEth_PhyInfo_Kpk_t pPhyinfo,
    Bool miiSelect
    )
{
    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);

    /* Reset the GMAC core */
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_BUS_MODE_REG_OFFSET,
                                TMVH_TRIDENTGMACETH_BUS_MODE_RESET_VAL);

    /* Select the GMII or MII interface */
    if(miiSelect == True)
        regVal = TMVH_TRIDENTGMACETH_CONFIG_MII_VAL;
    else
        regVal = TMVH_TRIDENTGMACETH_CONFIG_GMII_VAL;

    /* Program the MAC configuration register. Untouched values are in default state 
    ** PS =1 : MII 10/100 Mbps 
    ** FES = : 10 Mbps 
    ** Half duplex
    ** Automatic CRC padding/stripping
    */
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,
                                regVal |
                                TMVH_TRIDENTGMACETH_CONFIG_ACS_VAL);

    /* Default values for PHY at address 2, MDIO clock is selected in 
    ** GMII address register 
    */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET,
                             regVal);

    regVal = ((regVal & TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_CLR) & 
                    TMVH_TRIDENTGMACETH_GMII_ADDRESS_CSR_CLR);


    regVal |= (pPhyinfo->phyAddr << TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_POS) |
                  (pPhyinfo->clkCsrVal << TMVH_TRIDENTGMACETH_GMII_ADDRESS_CSR_POS);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET,
                                regVal);    

    /* Flow control settings 
    ** Enable transmit and receive flow control in half duplex mode
    */
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_FLOWCTRL_REG_OFFSET,
                                TMVH_TRIDENTGMACETH_FLOWCTRL_TFE_VAL |
                                TMVH_TRIDENTGMACETH_FLOWCTRL_RFE_VAL);

    /* Address filtering: Accept broadcast & Unicast packets. 
    ** No special settings required 
    */
    /* All interrupts are disabled after reset */

    return (TM_OK);
   
}

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_Deinit:
** 
**  DESCRIPTION: This function deinitializes the Ethernet device. 
**                      This function will perform following actions
**                      Disables DMA tx & rx
**                      Disables GMAC tx & rx state machines
**                      Resets the GMAC
** 
**  RETURN:        TM_OK
**  
**  NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t
hwTRIDENTGMACEth_Deinit(
    tmUnitSelect_t  ethUnitId   
    )
{

    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);        

    /* Disable DMA transmission & reception */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,
                               regVal);

    regVal = ((regVal & TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_CLR) &
                    TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_CLR);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,
                                regVal);

    /* Disable GMAC transmission & reception state machines */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

    regVal = ((regVal & TMVH_TRIDENTGMACETH_CONFIG_TX_EN_CLR) & 
                    TMVH_TRIDENTGMACETH_CONFIG_RX_EN_CLR);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

    /* Reset the GMAC */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_BUS_MODE_REG_OFFSET,regVal);

    regVal |=TMVH_TRIDENTGMACETH_BUS_MODE_RESET_VAL;

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_BUS_MODE_REG_OFFSET,regVal);

    return (TM_OK);

}    

#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_POWER)
/*-----------------------------------------------------------------------------
** FUNCTION    : hwTRIDENTGMACEth_GetPowerState
**
** DESCRIPTION: Function will get the existing power state of the device
**
** RETURN       : TM_OK
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetPowerState(
    tmUnitSelect_t    ethUnitId ,
    ptmPowerState_t pEthPowerState
    )
{
    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);  
    
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,regVal);

    regVal &=TMVH_TRIDENTGMACETH_PWRMGMT_POWERDN_EN_MSK;

    *pEthPowerState = ( (regVal == TMVH_TRIDENTGMACETH_POWER_OFF ) ? tmPowerOff : tmPowerOn);

    return TM_OK;
    
}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetPowerState
**
** DESCRIPTION: Function will set the power state to the device to the specified value
**
** RETURN:      TM_OK
**
** NOTES:       Set the Power state to power down will do the following things
**                  1. Disable the both the transmitt and receive DMA Manager
**                  2. set the Power down bit by writing to the power down register
**                  Set the power state of the device to the power up will do the following things
**                  1. Reset the power down bit in the power down register
**                  2. Reenable the both the transmitt and receive  channels
**-----------------------------------------------------------------------------
*/

tmErrorCode_t	
hwTRIDENTGMACEth_SetPowerState(
    tmUnitSelect_t   ethUnitId  , 
    tmPowerState_t  ethPowerState
    )
{
    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);
    
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,regVal);

    regVal &=TMVH_TRIDENTGMACETH_PWRMGMT_POWERDN_EN_MSK;

    if( ((ethPowerState == tmPowerOff) && (regVal == TMVH_TRIDENTGMACETH_POWER_ON)) || 
        ((ethPowerState == tmPowerOn) && (regVal == TMVH_TRIDENTGMACETH_POWER_OFF)) )
    {

        if(ethPowerState == tmPowerOff)
        {
            /* Disable DMA Transmission and reception */
            TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,
                                       regVal);

            regVal = ((regVal & TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_CLR) &
                            TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_CLR);

            TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,
                                        regVal);

            /* Disable GMAC transmission & reception state machines */
            TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

            regVal = ((regVal & TMVH_TRIDENTGMACETH_CONFIG_TX_EN_CLR) & 
                            TMVH_TRIDENTGMACETH_CONFIG_RX_EN_CLR);

            TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

            /* Turn off the port power */
            TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,regVal);

            regVal |=TMVH_TRIDENTGMACETH_PWRMGMT_POWERDN_EN_VAL;

        }
        else
        {

            /* Enable GMAC transmission & reception state machines */
            TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

            regVal |=(TMVH_TRIDENTGMACETH_CONFIG_TX_EN_VAL|
                            TMVH_TRIDENTGMACETH_CONFIG_RX_EN_VAL);

            TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

            /* Enable DMA Transmission and reception */
            TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,
                                       regVal);
            regVal |= (TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_VAL|
                            TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_VAL);

            TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,
                                        regVal);

            /* Clear the port power bit */
            TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,regVal);

            regVal &=TMVH_TRIDENTGMACETH_PWRMGMT_POWERDN_EN_CLR;
           
        }

        TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,regVal);          

    }

    return (TM_OK);

}   

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_WoLConfig
**
** DESCRIPTION: This function will set the wakeup procedure, required before going down 
**                     to power down mode.
**
** RETURN:         TM_OK 
**
** NOTES:       See hwTRIDENTGMACEth_WkupCfg_t structure.
**-----------------------------------------------------------------------------
*/
tmErrorCode_t	
hwTRIDENTGMACEth_WoLConfig(
    tmUnitSelect_t   ethUnitId  , 
    hwTRIDENTGMACEth_WkupCfg_Kpk_t  pWolConfig
    )
{

    UInt32 pEthRegs;
    UInt32 regVal = 0;

    pEthRegs = GET_BASE(ethUnitId);

    if(pWolConfig->rstRegptr == True)
    {
        TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,TMVH_TRIDENTGMACETH_WKUP_FMFILTER_RSTREG_VAL);
    }

    if(pWolConfig->globalUnicastEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_PWRMGMT_GU_VAL;        
    }

    if(pWolConfig->magicPktEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_PWRMGMT_MAGIC_PKT_EN_VAL;
    }

    if(pWolConfig->wkupFrameEn == True)
    {
         regVal |= TMVH_TRIDENTGMACETH_PWRMGMT_WKUP_FM_EN_VAL;        
    }

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,regVal);    

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterMask[0]);
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterMask[1]);   
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterMask[2]);    
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterMask[3]);    
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterCommand);    
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterOffset);    
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterCrcVal6);    
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_WKUP_FMFILTER_REG_OFFSET,pWolConfig->filterCrcVal7);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_GetWakeupStatus
**
** DESCRIPTION: This function is used to find the cause of wakeup event when in power down
**                     mode.
**                     
** RETURN:         TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetWakeupStatus(
    tmUnitSelect_t       ethUnitId ,    
    pUInt32                  pRegVal
    )
{
    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_PWRMGMT_CTRLSTAT_REG_OFFSET,*pRegVal);

    return(TM_OK);    

}
#endif

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetConfig
**
** DESCRIPTION: This function will configure the general Ethernet configuration 
**                     related Ethernet and PHY device with the parameters passed
**
** RETURN:         TM_OK 
**
** NOTES:       See hwTRIDENTGMACEth_DevConfig_t structure.
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetConfig(
    tmUnitSelect_t                  ethUnitId ,    
    hwTRIDENTGMACEth_DevConfig_Kpk_t pEthConfig
    )
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);  

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->txConfiginfo == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_TXCONFIG_VAL ;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->wdTimer == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_WD_VAL ;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->jabberTimer == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_JD_VAL;
    }

    if((pEthConfig->miiSelect == False ) && (pEthConfig->frameBurst == True)) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |=TMVH_TRIDENTGMACETH_CONFIG_FBE_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if (pEthConfig->jumboFrame == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |=TMVH_TRIDENTGMACETH_CONFIG_JFE_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if (pEthConfig->disableCS == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_DISCS_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    regVal |= (UInt32)(pEthConfig->ifg << TMVH_TRIDENTGMACETH_CONFIG_IFG_POS);    

    
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->miiSelect == True) {
        /* Select the GMII or MII interface */
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_MII_VAL;

        if(pEthConfig->speed100Mbps == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
            regVal |= TMVH_TRIDENTGMACETH_CONFIG_FES_VAL;
        }
    } else
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_GMII_VAL;
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);

    if(pEthConfig->disableReceiveOwn == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_DRXOWN_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->duplexMode == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_DUPLEX_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->ipChecksumOffload == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_IPCCHK_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->disableRetry == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
      regVal |= TMVH_TRIDENTGMACETH_CONFIG_DRETRY_VAL;
    }
    
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->autoPadCRC == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_ACS_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    regVal |= (UInt32)(pEthConfig->backOffLim << TMVH_TRIDENTGMACETH_CONFIG_BACKOFF_POS);

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    if(pEthConfig->deferralCheck == True) {
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_DEFCHK_VAL;
    }

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    /* Write regVal to MAC config register */
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    /* Write the PHY adress & Select MDC clock frequency in GMII address register */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET,regVal);

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    regVal = ((regVal & TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_CLR) &    
                    TMVH_TRIDENTGMACETH_GMII_ADDRESS_CSR_CLR);

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    regVal |= pEthConfig->phyAddress << TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_POS;
    regVal |= (UInt32)(pEthConfig->clockSelect << TMVH_TRIDENTGMACETH_GMII_ADDRESS_CSR_POS);
        
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET,regVal);

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    /* Set the station Address */
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_ADDR0_HIGH_REG_OFFSET,
                                pEthConfig->station.adrHigh); 
    
    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_ADDR0_LOW_REG_OFFSET,
                                pEthConfig->station.adrLow); 

    GMAC_PRINT_DBG(" [%s] [%d]\n", __func__,__LINE__);
    return (TM_OK);
    
}    

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION    : hwTRIDENTGMACEth_GetConfig
** 
** DESCRIPTION: This function will get the present configure of the Ethernet and PHY device
** RETURN:         TM_OK 
** 
** NOTES:       See phwTRIDENTGMACEth_DevConfig_t structure.
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetConfig(
    tmUnitSelect_t                      ethUnitId ,    
    phwTRIDENTGMACEth_DevConfig_t    pEthConfig
    )
{
    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);  

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

    pEthConfig->txConfiginfo = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_TXCONFIG_VAL) > 0) ? True : False);

    pEthConfig->wdTimer = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_WD_VAL) > 0) ? True : False);

    pEthConfig->txConfiginfo = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_TXCONFIG_VAL) > 0) ? True : False);    

    pEthConfig->jabberTimer = (Bool) (((regVal & TMVH_TRIDENTGMACETH_CONFIG_JD_VAL) > 0) ? True : False);
    
    pEthConfig->frameBurst = (Bool) (((regVal & TMVH_TRIDENTGMACETH_CONFIG_FBE_VAL) > 0) ? True : False);
    pEthConfig->jumboFrame = (Bool) (((regVal & TMVH_TRIDENTGMACETH_CONFIG_JFE_VAL) > 0) ? True : False);

    pEthConfig->ifg = (hwTRIDENTGMACEth_IFG_t) ((regVal &TMVH_TRIDENTGMACETH_CONFIG_IFG_MSK) >>
                                TMVH_TRIDENTGMACETH_CONFIG_IFG_POS);

    pEthConfig->disableCS = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_DISCS_VAL) > 0) ? True : False);        

    pEthConfig->miiSelect = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_MII_VAL) > 0) ? True : False);
    pEthConfig->speed100Mbps = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_FES_VAL) > 0) ? True : False);
    
    pEthConfig->duplexMode = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_DUPLEX_VAL) > 0) ? True : False);
    pEthConfig->disableReceiveOwn = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_DRXOWN_VAL) > 0) ? True : False);
    pEthConfig->ipChecksumOffload = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_IPCCHK_VAL) > 0) ? True : False);
    pEthConfig->disableRetry = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_DRETRY_VAL) > 0) ? True : False);
    pEthConfig->linkup = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_LNKUP_VAL) > 0) ? True : False);
    pEthConfig->autoPadCRC = (Bool)(((regVal & TMVH_TRIDENTGMACETH_CONFIG_ACS_VAL) > 0) ? True : False);
    pEthConfig->backOffLim = (hwTRIDENTGMACEth_BackOffLimit_t) ((regVal & TMVH_TRIDENTGMACETH_CONFIG_BACKOFF_MSK) >> 
                                            TMVH_TRIDENTGMACETH_CONFIG_BACKOFF_POS);

    pEthConfig->deferralCheck = (Bool)(regVal & TMVH_TRIDENTGMACETH_CONFIG_DEFCHK_VAL);

    /* Get the MDC clock value */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_GMII_ADDRESS_REG_OFFSET,regVal);

    pEthConfig->clockSelect = (hwTRIDENTGMACEth_ClkDiv_t) ((regVal & TMVH_TRIDENTGMACETH_GMII_ADDRESS_CSR_MSK) >> 
                                            TMVH_TRIDENTGMACETH_GMII_ADDRESS_CSR_POS);

    pEthConfig->phyAddress = (regVal & TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_MSK) >>TMVH_TRIDENTGMACETH_GMII_PHY_ADDRESS_POS;

    /* Read the station address into the structure member */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_ADDR0_HIGH_REG_OFFSET,regVal);
    
    pEthConfig->station.adrHigh = regVal & TMVH_TRIDENTGMACETH_ADDR0_HIGH_REG_MSK;
    
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_ADDR0_LOW_REG_OFFSET,regVal);
    
    pEthConfig->station.adrLow = regVal;

    return (TM_OK);
    
}    
#endif

/*-----------------------------------------------------------------------------
** FUNCTION    : hwTRIDENTGMACEth_IntGetStatus
**
** DESCRIPTION: The function returns the DMA interrupt status register value
**
** RETURN       :  TM_OK 
** NOTES        :  
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_IntGetStatus (
    tmUnitSelect_t   ethUnitId ,    
    phwTRIDENTGMACEth_Int_t pIntStat
    )
{
    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    /* Read the DMA interrupt status register */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_DMA_STATUS_REG_OFFSET,pIntStat->dmaIntVal);

    /* Read the MMC, PMT, PCS, RGMII interrupt status */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_INTR_STATUS_REG_OFFSET,pIntStat->gmacIntVal);    

    return (TM_OK);

}    

/*-----------------------------------------------------------------------------
** FUNCTION    : hwTRIDENTGMACEth_IntEnable
**
** DESCRIPTION: Read the current interrupt register values & OR the value passed
**
** RETURN       : TM_OK 
** NOTES        :       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_IntEnable (
    tmUnitSelect_t              ethUnitId ,    
    hwTRIDENTGMACEth_Int_Kpk_t pIntEn
    )
{
    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);

    /* Writing a 1 in the DMA interrupt enable register enables the interrupt */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_DMA_INT_ENABLE_REG_OFFSET,regVal);
    
    regVal |=pIntEn->dmaIntVal;

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_DMA_INT_ENABLE_REG_OFFSET,regVal);    

    /* Clearing the corresponding bit in the Interrupt mask register enables the interrupt, 
    ** if previously masked 
    */    
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_INTR_MASK_REG_OFFSET,regVal);

    regVal &= ~(pIntEn->gmacIntVal);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_INTR_MASK_REG_OFFSET,regVal);     

    return (TM_OK);
    
}    

/*-----------------------------------------------------------------------------
** FUNCTION:     hwEthGmacIntDisable
**
** DESCRIPTION: This function will Disable the sources of interrupt(s) for DMA
**
** RETURN:         TM_OK 
**
** NOTES:           
**
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_IntDisable (
    tmUnitSelect_t                  ethUnitId ,    
    hwTRIDENTGMACEth_Int_Kpk_t pIntDis
    )
{
    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);

    /* Writing a 0, disables the interrupt */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_DMA_INT_ENABLE_REG_OFFSET,regVal);

    regVal &=~(pIntDis->dmaIntVal);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_DMA_INT_ENABLE_REG_OFFSET,regVal);    

    /* Writing a 1 masks the corresponding interrupt */
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_INTR_MASK_REG_OFFSET,regVal);

    regVal |=pIntDis->gmacIntVal;

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_INTR_MASK_REG_OFFSET,regVal);        

    return (TM_OK);

}    

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_IntClear
**
** DESCRIPTION: This function will clear the sources of interrupt(s) for DMA
**
** RETURN:         TM_OK 
**
** NOTES:           
**
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_IntClear (
    tmUnitSelect_t              ethUnitId ,    
    UInt32                         ethIntstatus
    )
{
    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_DMA_STATUS_REG_OFFSET,ethIntstatus);

    return (TM_OK);
}    


/*-----------------------------------------------------------------------------
** FUNCTION:     thwTRIDENTGMACEth_PerfectAdrSetConfig
**
** DESCRIPTION: This function Configures the perfect address filtering register 
**
** RETURN:         TM_OK 
**
** NOTES:           There are 31 perfect address filter registers available. The register number (1-31)
**                       is passed as the paramter to this function.
**                       Station address register is not part of these 1-31 registers
**
**-----------------------------------------------------------------------------
*/
tmErrorCode_t 
hwTRIDENTGMACEth_PerfectAdrSetConfig(
    tmUnitSelect_t                      ethUnitId , 
    UInt32                                 regNum,
    hwTRIDENTGMACEth_Kpk_PerAdrCfg_t pAdrConfig
)
{
    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    if(pAdrConfig->addressEnable == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_ADDR_ENABLE_VAL;    
    }

    if(pAdrConfig->srcAddrCmp == True)
    {
        regVal |=TMVH_TRIDENTGMACETH_ADDR_SA_EN_VAL;    
    }

    /* Set the mask value */
    regVal |= pAdrConfig->addrMask << TMVH_TRIDENTGMACETH_ADDR_MBC_POS;

    regVal |= (pAdrConfig->macAddrHigh & TMVH_TRIDENTGMACETH_ADDR_HIGH_REG_MSK);

    if (regNum < 16 )
    {
        /* Write into the perfect address High Register */        
        TMVH_GEN_WRITE((pEthRegs+TMVH_TRIDENTGMACETH_ADDR1_HIGH_REG_OFFSET)+
                                    (TMVH_TRIDENTGMACETH_PERADRBLK1_OFFSET(regNum)),regVal);        

        /* Write into the perfect address low Register */            
        TMVH_GEN_WRITE((pEthRegs+TMVH_TRIDENTGMACETH_ADDR1_LOW_REG_OFFSET)+        
                                    (TMVH_TRIDENTGMACETH_PERADRBLK1_OFFSET(regNum)),pAdrConfig->macAddrlow);                
    }
    else
    {
        /* Write into the perfect address High Register */        
        TMVH_GEN_WRITE((pEthRegs+TMVH_TRIDENTGMACETH_ADDR16_HIGH_REG_OFFSET)+
                                    (TMVH_TRIDENTGMACETH_PERADRBLK2_OFFSET(regNum)),regVal);        

        /* Write into the perfect address low Register */            
        TMVH_GEN_WRITE((pEthRegs+TMVH_TRIDENTGMACETH_ADDR16_LOW_REG_OFFSET)+        
                                    (TMVH_TRIDENTGMACETH_PERADRBLK2_OFFSET(regNum)),pAdrConfig->macAddrlow);                    
    }

    return (TM_OK);

}

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_PerfectAdrGetConfig
**
** DESCRIPTION: This function gets the current values of perfect address filtering register 
**
** RETURN:         TM_OK 
**
** NOTES:           There are 32 perfect filter registers available. The register number (0-31)
**                       is passed as the paramter to this function
**
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_PerfectAdrGetConfig(
    tmUnitSelect_t                      ethUnitId , 
    UInt32                                 regNum,    
    phwTRIDENTGMACEth_PerfectAdrConfig_t pAdrConfig
)
{

    UInt32 pEthRegs;
    UInt32 regHigh=0;

    pEthRegs = GET_BASE(ethUnitId);

    if (regNum < 16 ) {
        /* Read from the High Register */        
        TMVH_GEN_READ((pEthRegs+TMVH_TRIDENTGMACETH_ADDR1_HIGH_REG_OFFSET)+
                                    (TMVH_TRIDENTGMACETH_PERADRBLK1_OFFSET(regNum)),regHigh);        

        /*Read from the perfect address low Register */            
        TMVH_GEN_READ((pEthRegs+TMVH_TRIDENTGMACETH_ADDR1_LOW_REG_OFFSET)+        
                                    (TMVH_TRIDENTGMACETH_PERADRBLK1_OFFSET(regNum)),pAdrConfig->macAddrlow);                
    } else {
        /* Read from the perfect address High Register */        
        TMVH_GEN_READ((pEthRegs+TMVH_TRIDENTGMACETH_ADDR16_HIGH_REG_OFFSET)+
                                    (TMVH_TRIDENTGMACETH_PERADRBLK2_OFFSET(regNum)),regHigh);        

        /* Read from the perfect address low Register */            
        TMVH_GEN_WRITE((pEthRegs+TMVH_TRIDENTGMACETH_ADDR16_LOW_REG_OFFSET)+        
                                    (TMVH_TRIDENTGMACETH_PERADRBLK2_OFFSET(regNum)),pAdrConfig->macAddrlow);                    
    }

    pAdrConfig->addressEnable = (Bool)(((regHigh & TMVH_TRIDENTGMACETH_ADDR_ENABLE_VAL ) > 0) ? True: False); 

    pAdrConfig->addrMask = (regHigh & TMVH_TRIDENTGMACETH_ADDR_MBC_MSK) >> 
                                        TMVH_TRIDENTGMACETH_ADDR_MBC_POS;
    
    pAdrConfig->macAddrHigh = regHigh & TMVH_TRIDENTGMACETH_ADDR_HIGH_REG_MSK;

    pAdrConfig->srcAddrCmp = (Bool) ( ((regHigh & TMVH_TRIDENTGMACETH_ADDR_SA_EN_VAL ) > 0) ? True: False); 

    return (TM_OK);

}
#endif

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_FilterSetConfig
**
** DESCRIPTION: This function configures the filters for incoming frames.
**
** RETURN:         TM_OK 
**
** NOTES:           
**
**-----------------------------------------------------------------------------
*/
tmErrorCode_t 
hwTRIDENTGMACEth_FilterSetConfig(
    tmUnitSelect_t                          ethUnitId , 
    hwTRIDENTGMACEth_FilterConfig_Kpk_t  pFilterConfig
) 
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);
    
    if(pFilterConfig->receiveAllEnable == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_RX_ALL_VAL;    
    }

    if(pFilterConfig->passAllFrames == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_PR_VAL;
    }                

    if(pFilterConfig->srcAdrFilterEnable == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_SAF_VAL;     
    }    

    if(pFilterConfig->srcAdrInvFilterEnable == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_SAIF_VAL;     
    }    

    if(pFilterConfig->destAdrInvFiltering == True) {
         regVal |= TMVH_TRIDENTGMACETH_FMFLTR_DAIF_VAL;   
    }        

    if(pFilterConfig->filterBroadCastFrames == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_DBF_VAL;     
    }        

    if(pFilterConfig->recvAllMulticast == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_PM_VAL;    
    }        

    if(pFilterConfig->hashMulticastEnable == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_HMC_VAL;     
    }        
    
    if(pFilterConfig->hashUnicastEnable == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_HUC_VAL;    
    }        

     /* Enable hash filtering as well as perfect address filtering simultaneously on a given frame.
     i.e. Frame is dropped only when it fails the hash filtering as well as perfect address filtering.
     To enable this bit 10 of Filter Frame Register has to be set. Also Hash Unicast & Hash Multicast
     bits in Filter Frame Register should be set. The bit 10 when set ensures that perfect address
     filtering is enabled for both Unicast as well as Multicast frames (irrespective of value of Hash
     Unicast & Hash Multicast bits).
     */
    if(pFilterConfig->hashNPerfectFilterEn == True) {
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_HASH_N_PER_EN_VAL;
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_HMC_VAL;             
        regVal |= TMVH_TRIDENTGMACETH_FMFLTR_HUC_VAL;            
    }

    regVal |= (UInt32)(pFilterConfig->pauseSetting << TMVH_TRIDENTGMACETH_FMFLTR_PCF_POS);
	regVal |= TMVH_TRIDENTGMACETH_FMFLTR_RX_ALL_VAL; 
    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_FMFLTR_REG_OFFSET,regVal);
    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_FMFLTR_REG_OFFSET,regVal);

    return (TM_OK);    

}


#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_FilterGetConfig
**
** DESCRIPTION: This function gets the current configuration of filters
**
** RETURN:         TM_OK 
**
** NOTES:           
**
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_FilterGetConfig(
    tmUnitSelect_t                      ethUnitId , 
    phwTRIDENTGMACEth_FilterConfig_t        pFilterConfig
) 
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_FMFLTR_REG_OFFSET,regVal);

    pFilterConfig->receiveAllEnable = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_RX_ALL_VAL) > 0 )? True:False);
    pFilterConfig->srcAdrFilterEnable = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_SAF_VAL) > 0) ? True:False);
    pFilterConfig->srcAdrInvFilterEnable = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_SAIF_VAL) > 0) ? True:False);
    pFilterConfig->hashNPerfectFilterEn = (Bool) (((regVal & TMVH_TRIDENTGMACETH_FMFLTR_HASH_N_PER_EN_VAL) > 0) ? True:False);
    pFilterConfig->filterBroadCastFrames = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_DBF_VAL) > 0) ? True:False);    
    pFilterConfig->recvAllMulticast = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_PM_VAL) > 0) ? True:False);    
    pFilterConfig->destAdrInvFiltering = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_DAIF_VAL) > 0) ? True:False);    
    pFilterConfig->hashMulticastEnable = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_HMC_VAL) > 0) ? True:False);    
    pFilterConfig->hashUnicastEnable = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_HUC_VAL) > 0) ? True:False);    
    pFilterConfig->passAllFrames = (Bool) (((regVal &TMVH_TRIDENTGMACETH_FMFLTR_PR_VAL) > 0) ? True:False);    
    pFilterConfig->pauseSetting = (hwTRIDENTGMACEth_PCF_t) ((regVal & TMVH_TRIDENTGMACETH_FMFLTR_PCF_MSK) >>
                                                TMVH_TRIDENTGMACETH_FMFLTR_PCF_POS);
    return (TM_OK);    
    
}
#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_HASH)
/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_HashFilterSetConfig
**
** DESCRIPTION: This function will configure the Hash Filter Table.
**
** RETURN:      TM_OK 
**
** NOTES:        Hash filtering (Multicast/Unicast) must be enabled by calling the function 
**                   hwTRIDENTGMACEth_FilterSetConfig()
** 
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_HashFilterSetConfig(
    tmUnitSelect_t                          ethUnitId ,
    hwTRIDENTGMACEth_HashFilter_Kpk_t    pHashConfig
    )
{

    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_HASHTBL_HIGH_REG_OFFSET,
                                pHashConfig->hashFilterH);

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_HASHTBL_LOW_REG_OFFSET,
                                pHashConfig->hashFilterL);
    
    return (TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_HashFilterGetConfig
**
** DESCRIPTION: Function will get the present Hash Filter Table configuration.
**
** RETURN:      TM_OK 
**                  
** NOTES:        
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_HashFilterGetConfig(
    tmUnitSelect_t                                ethUnitId ,
    phwTRIDENTGMACEth_HashFilter_t          pHashConfig
    )
{

    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_HASHTBL_HIGH_REG_OFFSET,
                                pHashConfig->hashFilterH);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_HASHTBL_LOW_REG_OFFSET,
                                pHashConfig->hashFilterL);
    
    return (TM_OK);

}
#endif

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_GmacEnableDisable
**
** DESCRIPTION: Enables/Disables the GMAC state machine. 
**
** RETURN:      TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t 
hwTRIDENTGMACEth_GmacEnableDisable (
    tmUnitSelect_t                              ethUnitId,
    hwTRIDENTGMACEth_EnTxfr_Kpk_t            pEndir    
    )
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);
        

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

    if(pEndir->dirFlag == hwTRIDENTGMACEth_Dir_TxRx)
    {
        if(pEndir->enFlag == hwTRIDENTGMACEth_Enable)
        {
            regVal |= TMVH_TRIDENTGMACETH_CONFIG_RX_EN_VAL | 
                          TMVH_TRIDENTGMACETH_CONFIG_TX_EN_VAL;
        }
        else
        {
            regVal = ((regVal & TMVH_TRIDENTGMACETH_CONFIG_TX_EN_CLR) & 
                            TMVH_TRIDENTGMACETH_CONFIG_RX_EN_CLR);        
        }

    }
    else if(pEndir->dirFlag == hwTRIDENTGMACEth_Dir_Tx)
    {
        if(pEndir->enFlag == hwTRIDENTGMACEth_Enable)
        {
             regVal |= TMVH_TRIDENTGMACETH_CONFIG_TX_EN_VAL;
        }
        else
        {
            regVal &= TMVH_TRIDENTGMACETH_CONFIG_TX_EN_CLR;        
        }
        
    }
    else
    {
        if(pEndir->enFlag == hwTRIDENTGMACEth_Enable)
        {
             regVal |= TMVH_TRIDENTGMACETH_CONFIG_RX_EN_VAL;
        }
        else
        {
            regVal &= TMVH_TRIDENTGMACETH_CONFIG_RX_EN_CLR;        
        }

    }

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_LpbkEnableDisable
**
** DESCRIPTION: Function will set the hardware in the loop back mode at the MAC interface
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_LpbkEnableDisable (
    tmUnitSelect_t                              ethUnitId ,
    hwTRIDENTGMACEth_EnableDisable_t     enableDisable
    ) 
{
    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);

    if(enableDisable == hwTRIDENTGMACEth_Enable)
    {
        regVal |= TMVH_TRIDENTGMACETH_CONFIG_LPBK_VAL ;
    }
    else
    {
        regVal &=TMVH_TRIDENTGMACETH_CONFIG_LPBK_CLR;
    }

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_CONFIG_REG_OFFSET,regVal);    

    return (TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_FlowCtrlSetConfig
**
** DESCRIPTION: Configures the flow control register.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlSetConfig(
    tmUnitSelect_t                                    ethUnitId ,
    hwTRIDENTGMACEth_Kpk_FlowCtrlCfg_t        pFlowCtrlCfg
) 
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    regVal |= (UInt32)pFlowCtrlCfg->pauseTime << TMVH_TRIDENTGMACETH_FLOWCTRL_PAUSET_POS;

    if (pFlowCtrlCfg->zeroQuanta == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_FLOWCTRL_ZEROQ_VAL;
    }

    regVal |= (UInt32)pFlowCtrlCfg->pauseLowThreshold << TMVH_TRIDENTGMACETH_FLOWCTRL_PLT_POS;

    if (pFlowCtrlCfg->unicastPsDetect == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_FLOWCTRL_UP_VAL;
    }

    if (pFlowCtrlCfg->rxFlowCtrlEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_FLOWCTRL_RFE_VAL;
    }

    if (pFlowCtrlCfg->txFlowCtrlEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_FLOWCTRL_TFE_VAL;
    }    

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_FLOWCTRL_REG_OFFSET,regVal);        

    return(TM_OK);    

}

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_FlowCtrlGetConfig
**
** DESCRIPTION: Gets the configured values in the the flow control register.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlGetConfig(
    tmUnitSelect_t                                    ethUnitId ,
    phwTRIDENTGMACEth_FlowCtrlConfig_t        pFlowCtrlCfg
) 
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_FLOWCTRL_REG_OFFSET,regVal);

    pFlowCtrlCfg->pauseLowThreshold = (hwTRIDENTGMACEth_PLT_t) ((regVal & TMVH_TRIDENTGMACETH_FLOWCTRL_PLT_MSK) >>
                                                    TMVH_TRIDENTGMACETH_FLOWCTRL_PLT_POS);

    pFlowCtrlCfg->zeroQuanta = (Bool)(((regVal & TMVH_TRIDENTGMACETH_FLOWCTRL_ZEROQ_VAL) >0) ? True: False);
    
    pFlowCtrlCfg->pauseTime = (regVal & TMVH_TRIDENTGMACETH_FLOWCTRL_PAUSET_MSK) >> 
                                                    TMVH_TRIDENTGMACETH_FLOWCTRL_PAUSET_POS;

    pFlowCtrlCfg->zeroQuanta = (Bool)(((regVal & TMVH_TRIDENTGMACETH_FLOWCTRL_ZEROQ_VAL) >0) ? True: False);

    pFlowCtrlCfg->rxFlowCtrlEn = (Bool)(((regVal & TMVH_TRIDENTGMACETH_FLOWCTRL_RFE_VAL) >0) ? True: False);
    
    pFlowCtrlCfg->txFlowCtrlEn = (Bool)(((regVal & TMVH_TRIDENTGMACETH_FLOWCTRL_TFE_VAL) > 0) ? True:False); 
    
    pFlowCtrlCfg->unicastPsDetect = (Bool)(((regVal &TMVH_TRIDENTGMACETH_FLOWCTRL_UP_VAL) > 0) ? True:False);

    return(TM_OK);    

}
#endif

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_FlowCtrlEnableDisable
**
** DESCRIPTION: Enables flow control in Full Duplex and Half duplex modes
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlEnableDisable(
    tmUnitSelect_t                          ethUnitId ,
    hwTRIDENTGMACEth_EnableDisable_t enableDisable
) 
{
    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_FLOWCTRL_REG_OFFSET,regVal);    

    if(enableDisable == hwTRIDENTGMACEth_Disable) 
    {
        regVal &=TMVH_TRIDENTGMACETH_FLOWCTRL_FCBBPA_CLR;
    }
    else
    {
        regVal |=TMVH_TRIDENTGMACETH_FLOWCTRL_FCBBPA_VAL;
    }

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_FLOWCTRL_REG_OFFSET,regVal);

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_FlowCtrlStatus
**
** DESCRIPTION: Gets the staus of FCA/BP bit 
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlStatus(
    tmUnitSelect_t                          ethUnitId,
    pUInt32                                    pRegVal
) 
{
    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs+TMVH_TRIDENTGMACETH_FLOWCTRL_REG_OFFSET,*pRegVal);    

    return(TM_OK);

}


/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaConfig
**
** DESCRIPTION: This function does the general configuration of DMA
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_DmaConfig(
    tmUnitSelect_t                               ethUnitId ,
    hwTRIDENTGMACEth_DmaCfg_Kpk_t        pDmaConfig
) 
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    /* General DMA configuration */
    if(pDmaConfig->fixedBurstEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_BUS_MODE_FB_VAL; 
    }

    if(pDmaConfig->pBL4xmode == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_BUS_MODE_4XPBL_VAL; 
    }

    if(pDmaConfig->differentPBL == True)
    {
        /* Separate PBL for reception */
        regVal |= TMVH_TRIDENTGMACETH_BUS_MODE_SEPPBL_VAL; 
        regVal |= (UInt32)pDmaConfig->rxPBL << TMVH_TRIDENTGMACETH_BUS_MODE_RX_PBL_POS;        
    }

    regVal |= (UInt32)pDmaConfig->burstLen << TMVH_TRIDENTGMACETH_BUS_MODE_PBL_POS;
    
    regVal |= pDmaConfig->descSkipLen << TMVH_TRIDENTGMACETH_BUS_MODE_DSL_POS;

    if(pDmaConfig->dmaArbitration == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_BUS_MODE_DA_VAL;
    }
    else
    {
        regVal |= (UInt32)pDmaConfig->priority << TMVH_TRIDENTGMACETH_BUS_MODE_PR_POS; 
    }

    if(pDmaConfig->addrAlignedBtsEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_BUS_MODE_AAL_VAL;
    }

    if(pDmaConfig->enableAltDescSize == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_BUS_MODE_EN_ALTDESC;
    }    

    /* Write to Bus mode register */
     TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_BUS_MODE_REG_OFFSET,regVal);

    regVal =0;

    if(pDmaConfig->disableFrameFlush == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_FRM_FLUSH_DIS_VAL; 
    }

    if(pDmaConfig->storeNforwardEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_SF_VAL;
    }

    if(pDmaConfig->rxStoreNforwardEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_RSF_VAL;
    }

    if(pDmaConfig->recvTcpIpErrFrms == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_DT_VAL;
    }

    if(pDmaConfig->txSecondFrameEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_OSF_VAL;
    }

    regVal |= (UInt32) (pDmaConfig->txThreshold << TMVH_TRIDENTGMACETH_OPERN_MODE_TTC_POS);

    regVal |= (UInt32) (pDmaConfig->rxThreshold << TMVH_TRIDENTGMACETH_OPERN_MODE_RTC_POS);

    /* Receive side configuration */
    if(pDmaConfig->hwFlowCtrlEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_EFC_VAL;
    }

    if(pDmaConfig->errFramesEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_FEF_VAL;
    }
    
    if(pDmaConfig->underSizedGdFramesEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_FUF_VAL;
    }

    if(pDmaConfig->actRxThreshold > hwTRIDENTGMACEth_RFA_4K)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_RFA2_VAL;
    }

    regVal |= (UInt32)((((UInt32)pDmaConfig->actRxThreshold) & TMVH_TRIDENTGMACETH_OPERN_MODE_MSB_CLR) << 
                    TMVH_TRIDENTGMACETH_OPERN_MODE_RFA_POS);

    if(pDmaConfig->deactRxThreshold > hwTRIDENTGMACEth_RFD_4K)
    {
        regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_RFD2_VAL;
    }

    regVal |= (UInt32) ((((UInt32)pDmaConfig->deactRxThreshold) & TMVH_TRIDENTGMACETH_OPERN_MODE_MSB_CLR) << 
                    TMVH_TRIDENTGMACETH_OPERN_MODE_RFD_POS);

    /* Write to Operation mode register */
    TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,regVal );    
    /* Write the Transmit descriptor base address */
    TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_TXDESC_LISTADDR_REG_OFFSET,
                                pDmaConfig->txDescListBaseAdr);

    /* Write the receive descriptor base address */    
    TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_RXDESC_LISTADDR_REG_OFFSET,
                                pDmaConfig->rxDescListBaseAdr);

    return(TM_OK);

}

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaFlushTxFifo
**
** DESCRIPTION: This function flushes the transmit fifo
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_DmaFlushTxFifo(
    tmUnitSelect_t  ethUnitId 
) 
{
    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,regVal);        

    regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_FTF_VAL;

    TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,regVal);            

    return(TM_OK);

}
#endif

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaEnableDisable
**
** DESCRIPTION: This function enables/disables the DMA in Tx & Rx directions
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_DmaEnableDisable(
    tmUnitSelect_t                      ethUnitId, 
    hwTRIDENTGMACEth_EnTxfr_Kpk_t    pTxfr
) 
{

    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,regVal);        

    if(pTxfr->dirFlag == hwTRIDENTGMACEth_Dir_TxRx) 
    {
    
        if(pTxfr->enFlag == hwTRIDENTGMACEth_Enable)
        {
            regVal |= (TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_VAL |
                           TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_VAL);
        }
        else
        {
            /* If it is disable, clear the tx enable & rx enable bits */
            regVal = ((regVal & TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_CLR) & 
                            TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_CLR);

        }

    }
    else if(pTxfr->dirFlag == hwTRIDENTGMACEth_Dir_Rx)
    {
        if(pTxfr->enFlag == hwTRIDENTGMACEth_Enable)
        {
            regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_VAL;                    
        }
        else
        {
            regVal &= TMVH_TRIDENTGMACETH_OPERN_MODE_RX_EN_CLR;
        }

    }
    else
    {
        if(pTxfr->enFlag == hwTRIDENTGMACEth_Enable)
        {
            regVal |= TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_VAL;                    
        }
        else
        {
            regVal &= TMVH_TRIDENTGMACETH_OPERN_MODE_TX_EN_CLR;
        }

    }

    TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_OPERN_MODE_REG_OFFSET,regVal);            

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaPollDesc
**
** DESCRIPTION: This function writes a dummy value into either Tx or Rx or both the registers
**                     depending on the 'dir' value passed. This reenables DMA if it is suspended. 
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t  
hwTRIDENTGMACEth_DmaPollDesc   (
    tmUnitSelect_t          ethUnitId,
    hwTRIDENTGMACEth_Dir_t       dir
    )
{

    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);

    if(dir == hwTRIDENTGMACEth_Dir_Tx)
    {
        TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_TXMT_DMD_REG_OFFSET,0xFF);        
    }
    else if (dir == hwTRIDENTGMACEth_Dir_Rx)
    {
        TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_RECV_DMD_REG_OFFSET,0xFF);            
    }
    else
    {
        TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_TXMT_DMD_REG_OFFSET,0xFF);            
        TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_RECV_DMD_REG_OFFSET,0xFF);            
    }

    return(TM_OK);

}

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaGetCurrentHostRegs
**
** DESCRIPTION: This function returns all the host register values.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_DmaGetCurrentHostRegs(
    tmUnitSelect_t                                 ethUnitId,
    phwTRIDENTGMACEth_DmaHostRegs_t     pDmaHostReg       
    )
{

    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);
     
    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_CUR_HOST_TX_DESC_REG_OFFSET,
                                pDmaHostReg->curHostTxDesc);       
    
    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_CUR_HOST_RX_DESC_REG_OFFSET,
                                pDmaHostReg->curHostRxDesc);       

    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_CUR_HOST_TXBUFADR_REG_OFFSET,
                                pDmaHostReg->curHostTxBufferAdr);       

    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_CUR_HOST_RXBUFADR_REG_OFFSET,
                                pDmaHostReg->curHostRxBufferAdr);       

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaGetMissedFrameCount
**
** DESCRIPTION: This function returns the number of missed frames during reception 
**                     due to Receive FIFO overflow or due to host buffer being unavailable
**                     This API can be used for debug purposes
**
** RETURN:        TM_OK 
**
**-----------------------------------------------------------------------------
*/

tmErrorCode_t 
hwTRIDENTGMACEth_DmaGetMissedFrameCount(
    tmUnitSelect_t                                 ethUnitId,
    phwTRIDENTGMACEth_DmaMissedFrmCnt_t pMissedFrmCnt
    )
{

    UInt32 pEthRegs;
    UInt32 regVal;

    pEthRegs = GET_BASE(ethUnitId);
     
    TMVH_GEN_READ(pEthRegs +TMVH_TRIDENTGMACETH_MISSFR_BUFOVRFLOW_CNT_REG_OFFSET,
                                regVal);       

    pMissedFrmCnt->fifoOverflowCnt = (regVal & TMVH_TRIDENTGMACETH_MISSFR_FIFO_CNT_MSK) >>
                                                       TMVH_TRIDENTGMACETH_MISSFR_FIFO_CNT_POS;

    pMissedFrmCnt->buffNotAvlCnt = (regVal & TMVH_TRIDENTGMACETH_MISSFR_BUF_CNT_MSK);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_DmaMissedFrameCounterClear
**
** DESCRIPTION: This function clears the missed frame counter values
**                      This API can be used for debug purposes
**
** RETURN:        TM_OK 
**
**-----------------------------------------------------------------------------
*/
tmErrorCode_t 
hwTRIDENTGMACEth_DmaMissedFrameCounterClear(
    tmUnitSelect_t                                 ethUnitId
    )
{

    UInt32 pEthRegs;

    pEthRegs = GET_BASE(ethUnitId);
     
    TMVH_GEN_WRITE(pEthRegs +TMVH_TRIDENTGMACETH_MISSFR_BUFOVRFLOW_CNT_REG_OFFSET,0x1fffffff);       

    return(TM_OK);
}

#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC)
/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_MMCConfig
**
** DESCRIPTION: Configures the behaviour of MAC statistics counters
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t 
hwTRIDENTGMACEth_MMCConfig(
    tmUnitSelect_t                      ethUnitId,
    hwTRIDENTGMACEth_MmcCtrl_Kpk_t           pCtrlConfig
    
)
{
    UInt32 pEthRegs;
    UInt32 regVal=0;

    pEthRegs = GET_BASE(ethUnitId);

    if(pCtrlConfig->freezeCntrs == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_MMC_CTRL_CNTR_FREZ_VAL;
    }

    if(pCtrlConfig->resetOnRdEn == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_MMC_CTRL_RESET_ON_RD_VAL;
    }

    if(pCtrlConfig->rollOverDisable == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_MMC_CTRL_STOP_ROLLOVER_VAL;
    }

    if(pCtrlConfig->resetCounters == True)
    {
        regVal |= TMVH_TRIDENTGMACETH_MMC_CTRL_CNTR_RESET_VAL;
    }

#if (  TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC)     
    #if ( IPVERSION_34_1_A)
    if (pCtrlConfig->resetCounters == True )
    {
        UInt8 i;
        for(i=0;i < 28;i++)
        {
            ghwTRIDENTGMACEth_ActRegVal[i]=0;
        }
    }
    #endif    
#endif

    TMVH_GEN_WRITE(pEthRegs+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET, regVal);

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_MMCIntStatus
**
** DESCRIPTION: This function gets the current interrupt status on the MMC counters in Tx 
** and Rx direction
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t
hwTRIDENTGMACEth_MMCIntStatus(
    tmUnitSelect_t                            ethUnitId ,    
    phwTRIDENTGMACEth_MmcIntStat_t   pMmcStat
    )
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    if (pMmcStat->dir == hwTRIDENTGMACEth_Dir_Rx)
    {
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_RX_REG_OFFSET,pMmcStat->intStat);
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_IPC_INT_REG_OFFSET,pMmcStat->rxCsumStatus);
    }
    else
    {
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_TX_REG_OFFSET,pMmcStat->intStat);    
    }

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_MMCIntEnable
**
** DESCRIPTION: This function enables MMC interrupts. The value passed is ORed with the 
**                      already configured interrupts
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_MMCIntEnable(
    tmUnitSelect_t                           ethUnitId ,    
    hwTRIDENTGMACEth_MmcIntr_Kpk_t  pMmcIntEn
    )
{

    UInt32 pRegs;
    UInt32 regVal;

    pRegs = GET_BASE(ethUnitId);

    if (pMmcIntEn->dir == hwTRIDENTGMACEth_Dir_Rx)
    {
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_RXMASK_REG_OFFSET,regVal);

        regVal &=~(pMmcIntEn->intrVal);

        TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_RXMASK_REG_OFFSET,regVal);

        /* Checksum offload interrupt mask register */        
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_IPC_INTMASK_REG_OFFSET,regVal);

        regVal &= ~(pMmcIntEn->rxCsumIntVal);

        TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_MMC_IPC_INTMASK_REG_OFFSET,regVal);

    }
    else
    {
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_TXMASK_REG_OFFSET,regVal);

        regVal &=~(pMmcIntEn->intrVal);

        TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_TXMASK_REG_OFFSET,regVal);
    }

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_MMCIntDisable
**
** DESCRIPTION: This function disables the MMC interrupts. Only the interrupts which 
**                      needs to be disabled are cleared.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_MMCIntDisable(
    tmUnitSelect_t                            ethUnitId ,    
    hwTRIDENTGMACEth_MmcIntr_Kpk_t   pMmcIntDis
    )
{

    UInt32 pRegs;
    UInt32 regValue;

    pRegs = GET_BASE(ethUnitId);

    if (pMmcIntDis->dir == hwTRIDENTGMACEth_Dir_Rx)
    {
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_RXMASK_REG_OFFSET,regValue);

        regValue |= pMmcIntDis->intrVal;

        TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_RXMASK_REG_OFFSET,regValue);

        /* Checksum offload interrupt mask register */        
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_IPC_INTMASK_REG_OFFSET,regValue);

        regValue |= pMmcIntDis->rxCsumIntVal;

        TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_MMC_IPC_INTMASK_REG_OFFSET,regValue);

    }
    else
    {
        TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_TXMASK_REG_OFFSET,regValue);

        regValue |= pMmcIntDis->intrVal;

        TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_MMC_INTR_TXMASK_REG_OFFSET,regValue);
    }

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_MMCCountersRead
**
** DESCRIPTION: This function reads the MMC counter and returns the value. 
**               The counter to be read is passed as a parameter to this function
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
  
tmErrorCode_t
hwTRIDENTGMACEth_MMCCountersRead(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_MmcRegVal_t pMmcReg
    )
{
    UInt32 pRegs;

#if ( IPVERSION_34_1_A)
    UInt32 regVal;
#endif

    pRegs = GET_BASE(ethUnitId);

    switch(pMmcReg->regToRd)
    {
        case HW_TRIDENTGMACETH_TX_OCTET_CNT_GB:

                //MMC_RST_ON_RD_SAVE(pRegs,regVal);
                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TXOCTET_CNT_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                //MMC_RST_ON_RD_RESTORE(pRegs,regVal);
                break;
            
        case HW_TRIDENTGMACETH_TX_FRAME_CNT_GB :
                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TXFRAME_CNT_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;
            
        case HW_TRIDENTGMACETH_TX_BRDCST_CNT_G:
                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TXBRDCST_CNT_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;

        case HW_TRIDENTGMACETH_TX_MULTCST_CNT_G:

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TXMULTCST_CNT_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;

        case HW_TRIDENTGMACETH_TX_64_CNT_GB:

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX64_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                

        case HW_TRIDENTGMACETH_TX_65TO127_CNT_GB:

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX65TO127_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                

        case HW_TRIDENTGMACETH_TX_128TO255_CNT_GB:    

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX128TO255_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_256TO511_CNT_GB:    

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX256TO511_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_512TO1023_CNT_GB:        

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX512TO1023_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_1024TOMAX_CNT_GB:            

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX1024TOMAX_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
                
        case HW_TRIDENTGMACETH_TX_UNICAST_CNT_GB:                

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_UNICAST_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
                
        case HW_TRIDENTGMACETH_TX_MULTCST_CNT_GB:                

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_MULTCST_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_BRDCST_CNT_GB:                

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_BRDCST_GB_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_UNDERFLOW_ERR_CNT:                    

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_UNNDERFLOW_ERR_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_SINGLE_COL_CNT_G:                    

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_SINGLCOL_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_MULTICOL_COL_G:                        

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_MULTICOL_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_DEFERRED_CNT:                            

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_DEFERRED_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_LATECOL_CNT:                                

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_LATECOL_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_EXCESSCOL_CNT:                                    

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_EXCESSCOL_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_CARRIER_ERR_CNT:    

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_CARRIER_ERR_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_OCTET_CNT_G:        

                //MMC_RST_ON_RD_SAVE(pRegs,regVal);
                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_OCTET_CNT_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                    
                
                break;                
            
        case HW_TRIDENTGMACETH_TX_FRAME_CNT_G:            

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_FRM_CNT_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_EXCESSDEF_CNT:                

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_EXCESS_DEF_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;            

        case HW_TRIDENTGMACETH_TX_PAUSE_FRAMES_CNT:                    
            
                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_PAUSE_FRAMES_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                
            
        case HW_TRIDENTGMACETH_TX_VLAN_FRAMES_CNT_G:                        

                TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TX_VLAN_FRAMES_G_REG_OFFSET,\
                    pMmcReg->mmcRegVal);
                break;                            

        case HW_TRIDENTGMACETH_RX_FRM_CNT_GB:
            
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXFRM_CNT_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
           
            break;                            
            
        case HW_TRIDENTGMACETH_RX_OCTET_CNT_GB:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXOCTET_CNT_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);  

            break;                            
            
        case HW_TRIDENTGMACETH_RX_OCTET_CNT_G:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXOCTET_CNT_G_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);  

            break;                            

        case HW_TRIDENTGMACETH_RX_BRDCSTF_CNT_G:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_BRDCSTF_G_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_MULTCSTF_CNT_G:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_MULTCSTF_G_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_CRC_ERR_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_CRC_ERR_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_ALIGNMT_ERR_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_ALIGNMT_ERR_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_RUNT_ERR_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_RUNT_ERR_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_JABBER_ERR_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_JABBER_ERR_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_UNDERSIZE_CNT_G:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_UNDERSIZE_G_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;                            

        case HW_TRIDENTGMACETH_RX_OVERSIZE_CNT_G:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_OVERSIZE_G_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_64_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_64_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_65TO127_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_65TO127_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_128TO255_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_128TO255_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_256TO511_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_256TO511_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_512TO1023_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_512TO1023_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_1024TOMAX_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_1024TOMAX_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_UNICAST_CNT_G:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_UNICAST_G_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_LEN_ERR_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_LEN_ERR_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_OUTOFRANGE_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_OUTOFRANGE_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_PAUSE_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_PAUSE_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_FIFO_OVERFLOW_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_FIFO_OVERFLOW_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_VLAN_FRAMES_CNT_GB:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_VLAN_FRAMES_GB_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        case HW_TRIDENTGMACETH_RX_WATCHDOG_ERR_CNT:

            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RX_WATCHDOG_ERR_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            break;            

        /* IPC counters */
        case HW_TRIDENTGMACETH_RX_IPV4_FRMCNT_G:

            #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV4_GD_FRMS_REG_OFFSET,0);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_GD_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif            

            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_HDR_ERR_FRMCNT:

            #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV4_HDERR_FRMS_REG_OFFSET,1);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_HDERR_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_NOPPAY_FRMCNT:

            #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV4_NOPAY_FRMS_REG_OFFSET,2);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_NOPAY_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif                
            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_FRAG_FRMCNT:

           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV4_FRAG_FRMS_REG_OFFSET,3);
            #else
              TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_FRAG_FRMS_REG_OFFSET,\
              pMmcReg->mmcRegVal);
            #endif

            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_UDPCSUMDSL_FRMCNT:

           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV4_UDSBL_FRMS_REG_OFFSET,4);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_UDSBL_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV6_FRMCNT_G:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV6_GD_FRMS_REG_OFFSET,5);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV6_GD_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            break;            

        case HW_TRIDENTGMACETH_RX_IPV6_HDR_ERR_FRMCNT:

           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV6_HDERR_FRMS_REG_OFFSET,6);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV6_HDERR_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV6_NOPAY_FRMCNT:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXIPV6_NOPAY_FRMS_REG_OFFSET,7);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV6_NOPAY_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            
            break;            

        case HW_TRIDENTGMACETH_RX_UDP_FRMCNT_G:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXUDP_GD_FRMS_REG_OFFSET,8);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXUDP_GD_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            
            break;            

        case HW_TRIDENTGMACETH_RX_UDP_ERR_FRMCNT:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXUDP_ERR_FRMS_REG_OFFSET,9);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXUDP_ERR_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            break;            

        case HW_TRIDENTGMACETH_RX_TCP_FRMCNT_G:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXTCP_GD_FRMS_REG_OFFSET,10);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXTCP_GD_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif                
            break;            

        case HW_TRIDENTGMACETH_RX_TCP_ERR_FRMCNT:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXTCP_ERR_FRMS_REG_OFFSET,11);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXTCP_ERR_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            break;            

        case HW_TRIDENTGMACETH_RX_ICMP_FRMCNT_G:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXICMP_GD_FRMS_REG_OFFSET,12);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXICMP_GD_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif                
            break;            

        case HW_TRIDENTGMACETH_RX_ICMP_ERR_FRMCNT:
           #if ( IPVERSION_34_1_A)
            pMmcReg->mmcRegVal = \
            compute_ipc_value(pRegs,TMVH_TRIDENTGMACETH_MMC_RXICMP_ERR_FRMS_REG_OFFSET,13);
            #else
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXICMP_ERR_FRMS_REG_OFFSET,\
            pMmcReg->mmcRegVal);
            #endif
            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_OCTETS_G:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_GD_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);               
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_HDR_ERR_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_HDRERR_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_NOPPAY_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_NOPAY_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                

            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_FRAG_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_FRAG_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV4_UDPCSUMDSL_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV4_UDSBL_OCTECTS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV6_OCTETS_G:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV6_GD_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);     
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV6_HDR_ERR_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV6_HDRERR_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_IPV6_NOPAY_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXIPV6_NOPAY_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                

            
            break;            

        case HW_TRIDENTGMACETH_RX_UDP_OCTETS_G:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXUDP_GD_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_UDP_ERR_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_RXUDP_ERR_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_TCP_OCTETS_G:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TCP_GD_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_TCP_ERR_OCTETS:
            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_TCP_ERR_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_ICMP_OCTETS_G:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_ICMP_GD_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                
            
            break;            

        case HW_TRIDENTGMACETH_RX_ICMP_ERR_OCTETS:

            //MMC_RST_ON_RD_SAVE(pRegs,regVal);
            TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_MMC_ICMP_ERR_OCTETS_REG_OFFSET,\
                pMmcReg->mmcRegVal);
            //MMC_RST_ON_RD_RESTORE(pRegs,regVal);                

            break;            

        default:
            break;

    }

    return(TM_OK);

}
#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_VLAN)
/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetVLANTag
**
** DESCRIPTION: This function sets the VLAN tag value which is used for comparison with the
**                     received VLAN frames.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t
hwTRIDENTGMACEth_SetVLANTag (
    tmUnitSelect_t               ethUnitId ,    
    UInt32                           regValue
    )
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    regValue &=TMVH_TRIDENTGMACETH_VLANTAG_MASK;

    TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_VLANTAG_REG_OFFSET,regValue);    

    return(TM_OK);
}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_GetVLANTag
**
** DESCRIPTION: This function returns the VLAN tag value 
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetVLANTag (
    tmUnitSelect_t               ethUnitId ,    
    pUInt32                         pRegValue
    )
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_VLANTAG_REG_OFFSET,*pRegValue);    
    
    *pRegValue &=TMVH_TRIDENTGMACETH_VLANTAG_MASK;

    return(TM_OK);
}
#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_IEEE1588_TIMESTAMP)

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_GetTsCtrlVal
**
** DESCRIPTION: This function returns the control value of the time stamp registers.
**
**                         Below macros can be used to check if a corresponding bit is set/reset in 
**                         the register: 
**                          HW_TRIDENTGMACETH_TS_CTRL_ADDEND_UPDT_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_INT_TRIG_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_TS_UPDATE_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_TS_INIT_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_FINE_UPDATE_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_TIMESTAMP_EN_VAL
**
** RETURN :           TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetTsCtrlVal(
    tmUnitSelect_t               ethUnitId ,    
    pUInt32                         pRegValue
    )
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    TMVH_GEN_READ(pRegs+TMVH_TRIDENTGMACETH_TS_CTRL_REG_OFFSET,*pRegValue);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:     hwTRIDENTGMACEth_SetTsCtrl
**
** DESCRIPTION: This function enables/disables certain features of time stamp operation,
**                         depending on the value passed.
**
**                         Parameter to this function can be any of the below macros
**
**                          HW_TRIDENTGMACETH_TS_CTRL_ADDEND_UPDT_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_INT_TRIG_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_TS_UPDATE_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_TS_INIT_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_FINE_UPDATE_VAL
**                          HW_TRIDENTGMACETH_TS_CTRL_TIMESTAMP_EN_VAL
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsCtrl(
    tmUnitSelect_t               ethUnitId ,    
    UInt32                           regValue
    )
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_TS_CTRL_REG_OFFSET,regValue);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetTsSubSecInc
**
** DESCRIPTION:  API to set the value of sub-second increment register
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsSubSecInc(
    tmUnitSelect_t               ethUnitId ,    
    UInt32                           regValue
)
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_TS_SUB_SECOND_INCR_REG_OFFSET,regValue);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetTsAddend
**
** DESCRIPTION:  This API is used to set Time Stamp Addend value. This is used only when system 
**                          time is configured for Fine Update mode.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsAddend(
    tmUnitSelect_t               ethUnitId ,    
    UInt32                       regValue
)
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_TS_ADDEND_REG_OFFSET,regValue);    

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_GetSysTs
**
** DESCRIPTION:  This API fetches the current system time stamp value
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetSysTs(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_TsReg_t pRegs
)
{

    UInt32 regBase;

    regBase = GET_BASE(ethUnitId);

    TMVH_GEN_READ(regBase+TMVH_TRIDENTGMACETH_TS_HIGH_REG_OFFSET,pRegs->highVal);    
    TMVH_GEN_READ(regBase+TMVH_TRIDENTGMACETH_TS_LOW_REG_OFFSET,pRegs->lowVal);        

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetTsUpdate
**
** DESCRIPTION:  This API is used to set the values of Time Stamp update registers & the parameter
**                          passed to this function also indicates if the value is to be added to the system time
**                          or subracted from the system time.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsUpdate(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_TsUpdateReg_t pUpdate   
)
{

    UInt32 pRegs;

    pRegs = GET_BASE(ethUnitId);

    if(pUpdate->addSub == hwTRIDENTGMACEth_SubFromSysTime)
    {
        /*  Subtract this value from system time */    
        pUpdate->updateReg.lowVal |= TMVH_TRIDENTGMACETH_TS_LOW_UPDATE_PSNT_VAL;
    }
    else
    {
        /* Add this value to system time */
        pUpdate->updateReg.lowVal &= (~TMVH_TRIDENTGMACETH_TS_LOW_UPDATE_PSNT_VAL);            
    }

    TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_TS_HIGH_UPDATE_REG_OFFSET, \
                                        pUpdate->updateReg.highVal);    
    TMVH_GEN_WRITE(pRegs+TMVH_TRIDENTGMACETH_TS_LOW_UPDATE_REG_OFFSET, \
                                        pUpdate->updateReg.lowVal);        

    return(TM_OK);

}

/*-----------------------------------------------------------------------------
** FUNCTION:    hwTRIDENTGMACEth_SetTsTgtTime
**
** DESCRIPTION:  This API is used to schedule an interrupt event when the system time exceeds the
**                          value programmed in the target registers.
**
** RETURN:        TM_OK 
**
** NOTES:       
**-----------------------------------------------------------------------------
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsTgtTime(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_TsReg_t pRegs
)
{

    UInt32 regBase;

    regBase = GET_BASE(ethUnitId);

    TMVH_GEN_WRITE(regBase+TMVH_TRIDENTGMACETH_TS_TGT_TIME_HIGH_REG_OFFSET, \
                                        pRegs->highVal);    
    TMVH_GEN_WRITE(regBase+TMVH_TRIDENTGMACETH_TS_TGT_TIME_LOW_REG_OFFSET, \
                                        pRegs->lowVal);        

    return(TM_OK);

}
#endif /* End of timestamp routines */


#if (  TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC) 

     #if ( IPVERSION_34_1_A)

UInt32  compute_ipc_value (UInt32 base,UInt32 regOffset,UInt32 index)
{
        UInt32 temp, org;
        UInt32 sum;
        UInt32 regVal;

        /* Read the control & set clear on read */
        TMVH_GEN_READ(base+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,org);

        temp = org;

        if((org & TMVH_TRIDENTGMACETH_MMC_CTRL_STOP_ROLLOVER_VAL) &&
            ( (org & TMVH_TRIDENTGMACETH_MMC_CTRL_RESET_ON_RD_VAL)==0))
        {

            if(org & TMVH_TRIDENTGMACETH_MMC_CTRL_CNTR_FREZ_VAL)
            {
                return(ghwTRIDENTGMACEth_ActRegVal[index]);
            }

            /* Enable read clear */
            temp |= TMVH_TRIDENTGMACETH_MMC_CTRL_RESET_ON_RD_VAL;

            TMVH_GEN_WRITE(base+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,temp); 

            TMVH_GEN_READ(base+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,temp);

            /* If you are reading after previously returning all Fs, reset the value to zero */            
            if(ghwTRIDENTGMACEth_ActRegVal[index] == 0xFFFFFFFF)        
            {
                ghwTRIDENTGMACEth_ActRegVal[index] = 0;
            }

            /* Read the IPC counter value */        
            TMVH_GEN_READ(base+regOffset,regVal);                

            sum = ghwTRIDENTGMACEth_ActRegVal[index] + regVal;

            /* Over flow case */
            if(sum < regVal)
            {
                /* On overflow, return all Fs */
                ghwTRIDENTGMACEth_ActRegVal[index] = 0xFFFFFFFF;                    
            }
            else
            {
                /* Accumulate into a variable... until it overflows */
                ghwTRIDENTGMACEth_ActRegVal[index] += regVal;
            }

            /* Actual value to be returned */
            regVal = ghwTRIDENTGMACEth_ActRegVal[index];

            /* Restore the register settings */                
            TMVH_GEN_WRITE(base+TMVH_TRIDENTGMACETH_MMC_CTRL_REG_OFFSET,org);


        }
        else
        {
            /* When rollover disable is not set */
            TMVH_GEN_READ(base+regOffset,regVal);       
            
            /* Store the value for returning, when freeze =1 */            
             ghwTRIDENTGMACEth_ActRegVal[index] = regVal;
        }

        return(regVal);        


}
     #endif

#endif /* __IPVERSION_34_1_A__ */


