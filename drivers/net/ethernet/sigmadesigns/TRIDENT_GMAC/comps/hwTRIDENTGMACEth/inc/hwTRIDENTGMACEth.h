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
 * All Rights Reserved.
 *
 *---------------------------------------------------------------------------
 * %filename:     hwTRIDENTGMACEth.h %
 * %pid_version:          1.5                  %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  Configuration header file for Ethernet HwApi Driver
 *
 * DOCUMENT REF: Synopsys DesignWare Cores Ethernet MAC Universal Databook 
 *                         Version 3.30a, March 22nd 2007 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/

#ifndef  HWTRIDENTGMACETH_H
#define  HWTRIDENTGMACETH_H

/*-----------------------------------------------------------------------------
* Standard include files:
*-----------------------------------------------------------------------------
*/

#include "../../../intfs/IPhy/inc/tmNxTypes.h"      /*! This is a standard type definition file for NxHome.*/
#include "../../../intfs/IPhy/inc/tmNxCompId.h"    /*! This file registers all the component IDs as defined in NxHome.*/
#include "../../../intfs/IPhy/inc/tmNxModId.h"
#include "../../../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h" /*! Configuration header file */


/*-----------------------------------------------------------------------------
** Error Codes :
**-----------------------------------------------------------------------------
*/

#define HW_TRIDENTGMACETH_COMPATIBILITY_NR (1)
/*! \def HW_TRIDENTGMACETH_COMPATIBILITY_NR
*     Compatability number         
*/

#define HW_TRIDENTGMACETH_MAJOR_VERSION_NR (1)
/*! \def HW_TRIDENTGMACETH_MAJOR_VERSION_NR
*     Major version Number         
*/

#define HW_TRIDENTGMACETH_MINOR_VERSION_NR (0)
/*! \def HW_TRIDENTGMACETH_MINOR_VERSION_NR
*     Minor Version Number         
*/

#define HW_ERR_TRIDENTGMACETH_BASE         (CID_TRIDENTGMACETH | CID_LAYER_HWAPI)
#define HW_ERR_TRIDENTGMACETH_COMP        (CID_TRIDENTGMACETH | CID_LAYER_HWAPI | \
                                                                                TM_ERR_COMP_UNIQUE_START)

/*! \def HW_ERR_TRIDENTGMACETH_COMP
*    This error code is the initial offset from which other error codes are based.
*/

#define HW_ERR_TRIDENTGMACETH_NOT_SUPPORTED  (HW_ERR_TRIDENTGMACETH_BASE+TM_ERR_NOT_SUPPORTED)

/*! \def HW_ERR_TRIDENTGMACETH_NOT_SUPPORTED
*    This error code is returned if the feature is not supported by Ethernet hardware.
*/

/* Interrupt identification flags */

/*! \brief Typedef for interrupt mask value */
typedef  UInt32  hwTRIDENTGMACEth_IntMask_t,*phwTRIDENTGMACEth_IntMask_t;

/*! \brief Typedef for interrupt status value */
typedef  UInt32  hwTRIDENTGMACEth_IntStatus_t,  *phwTRIDENTGMACEth_IntStatus_t;

/*! \brief Typedef for status value */
typedef  UInt32  hwTRIDENTGMACEth_StatusMask_t,*phwTRIDENTGMACEth_StatusMask_t;


/*-----------------------------------------------------------------------------
** Interrupt Macros :
**-----------------------------------------------------------------------------
*/

/**
 * \defgroup group0 Basic Operations
 */
/*\{*/

#define HW_TRIDENTGMACETH_DMA_INT_NIE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000)
/*! \def HW_TRIDENTGMACETH_DMA_INT_NIE_EN_VAL
*   Macro to enable Normal Interrupt Summary Interrupt
*/

#define HW_TRIDENTGMACETH_DMA_INT_AIE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000)
/*! \def HW_TRIDENTGMACETH_DMA_INT_AIE_EN_VAL
*      Macro to enable Abnormal Interrupt Summary Interrupt
*/

#define HW_TRIDENTGMACETH_DMA_INT_ERE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000)
/*! \def HW_TRIDENTGMACETH_DMA_INT_ERE_EN_VAL
*    Macro to enable Early Receive Interrupt
*/

#define HW_TRIDENTGMACETH_DMA_INT_FBE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000)
/*! \def HW_TRIDENTGMACETH_DMA_INT_FBE_EN_VAL
*    Macro to enable Fatal Bus Error interrupt
*/

#define HW_TRIDENTGMACETH_DMA_INT_ETE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400)
/*! \def HW_TRIDENTGMACETH_DMA_INT_ETE_EN_VAL
* Macro to enable Early Transmit interrupt  
*/


#define HW_TRIDENTGMACETH_DMA_INT_RWE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_DMA_INT_RWE_EN_VAL
*Macro to enable Watchdog Timeout interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_RSE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100)
/*! \def HW_TRIDENTGMACETH_DMA_INT_RSE_EN_VAL
*Macro to enable Receive Stopped interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_RUE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80)
/*! \def HW_TRIDENTGMACETH_DMA_INT_RUE_EN_VAL
* Macro to enable Receive Buffer Unavailable interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_RIE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40)
/*! \def HW_TRIDENTGMACETH_DMA_INT_RIE_EN_VAL
* Macro to enable Receive Interrupt 
*/


#define HW_TRIDENTGMACETH_DMA_INT_UNE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20)
/*! \def HW_TRIDENTGMACETH_DMA_INT_UNE_EN_VAL
* Macro to enable Underflow interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_OVE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10)
/*! \def HW_TRIDENTGMACETH_DMA_INT_OVE_EN_VAL
*Macro to enable Overflow interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_TJE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_DMA_INT_TJE_EN_VAL
* Macro to enable Transmit Jabber Timeout interrupt
*/

#define HW_TRIDENTGMACETH_DMA_INT_TUE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_DMA_INT_TUE_EN_VAL
* Macro to enable Transmit Buffer Unavailable interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_TSE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_DMA_INT_TSE_EN_VAL
* Macro to enable Transmit Stopped interrupt 
*/

#define HW_TRIDENTGMACETH_DMA_INT_TIE_EN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_DMA_INT_TIE_EN_VAL
* Macro to enable Transmit Interrupt  
*/

#define HW_TRIDENTGMACETH_INTR_PCS_AN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_INTR_PCS_AN_VAL
* Macro to enable/disable interrupt generation due to auto negotiation completion event
*/

#define HW_TRIDENTGMACETH_INTR_LNKSTAT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_INTR_LNKSTAT_VAL
* Macro to enable/disable interrupt generation due to change in link status event
*/

#define HW_TRIDENTGMACETH_INTR_RGMII_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_INTR_RGMII_VAL
* Macro to enable/disable interrupt generation due to the setting of RGMII status bit
*/

/* DMA status register macros. Some are Read Only & Some are Read Clear */

#define HW_TRIDENTGMACETH_DMA_STATUS_GPI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x10000000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_GPI_VAL
* Macro to check if the cause of the interrupt is due to the Power Management Unit
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_GMI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x8000000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_GMI_VAL
* Macro to check if the cause of the interrupt is due to MMC unit 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_GLI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x4000000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_GLI_VAL
* Macro to check if the cause of the interrupt is due to PCS interface block  
*/

/* The below bits are valid only when the fatal bus error interrupt is set */

#define HW_TRIDENTGMACETH_DMA_STATUS_EB_MSK ((hwTRIDENTGMACEth_StatusMask_t)0x3800000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_EB_MSK
* Mask to extract the Error bits 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_EBTX_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x800000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_EBTX_VAL
* Macro to check if the Error is due to data transfer by TxDMA 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_EBRD_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x1000000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_EBRD_VAL
* Macro to check if there is an Error during read transfer 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_EBDESC_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x2000000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_EBDESC_VAL
* Macro to check if there is an Error during descriptor access 
*/

/* DMA Transmit process status */

#define HW_TRIDENTGMACETH_DMA_STATUS_TS_MSK ((hwTRIDENTGMACEth_StatusMask_t)0x700000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TS_MSK
* DMA Transmit process state mask  
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TS_TXDESC_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x100000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TS_TXDESC_VAL
* DMA state is running and it is Fetching Transmit Transfer Descriptor   
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TS_TXSTAT_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x200000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TS_TXSTAT_VAL
* DMA state is running and it is  Waiting for status   
*/


#define HW_TRIDENTGMACETH_DMA_STATUS_TS_RDDAT_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x300000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TS_RDDAT_VAL
* DMA state is running: It is  Reading data from host memory buffer and queuing 
* it to transmit buffer (TX FIFO)  
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TS_SUSP_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x600000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TS_SUSP_VAL
* DMA state is Suspended: Transmit Descriptor Unavailable or Transmit Buffer Underflow 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TS_CTXDESC_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x700000)  
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TS_CTXDESC_VAL
* DMA is running: Closing Transmit Descriptor
*/

/* Receive process status 
**
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RS_MSK ((hwTRIDENTGMACEth_StatusMask_t)0xE0000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RS_MSK
* Receive process state mask
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RS_RXDESC_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x20000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RS_RXDESC_VAL
* DMA state is Running: Fetching Receive Transfer Descriptor 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RS_WTRX_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x60000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RS_WTRX_VAL
* DMA state is Running: Waiting for receive packet
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RS_NO_RXDESC_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x80000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RS_NO_RXDESC_VAL
 * DMA state is Suspended: No Rx Descriptor available 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RS_CLOSE_RXDESC_VAL ((hwTRIDENTGMACEth_StatusMask_t)0xA0000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RS_CLOSE_RXDESC_VAL
* DMA state is Running: Closing receive descriptor 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RS_TXFRDATA_VAL ((hwTRIDENTGMACEth_StatusMask_t)0xE0000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RS_TXFRDATA_VAL
* DMA state is Running: Transferring the receive packet data from receive 
* buffer to host memory
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_NIS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x10000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_NIS_VAL
*  Normal Interrupt Summary status. To clear this bit, write a 1 to this bit position
*  after clearing the cause of NIS interrupt
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_AIS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x8000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_AIS_VAL
*  Abnormal Interrupt Summary 
*/


#define HW_TRIDENTGMACETH_DMA_STATUS_ERI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x4000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_ERI_VAL
*  Early Receive Interrupt macro which  indicates that the DMA had 
*  filled the first data buffer of the packet.
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_FBI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x2000)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_FBI_VAL
* Macro to indicate that a Bus Error occurred.  
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_ETI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x400)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_ETI_VAL
* Early Transmit Interrupt : This macro indicates that the frame 
* to be transmitted was fully transferred to the MTL Transmit FIFO 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RWT_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RWT_VAL
* Receive Watchdog Timeout indicates a frame with a length greater than 
* 2048 bytes was received 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RPS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x100)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RPS_VAL
* Receive Process Stopped: Receive process has entered the stopped state
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RU_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x80)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RU_VAL
*    Receive Buffer Unavailable indicates the next descriptor is owned by host. 
*    DMA enters suspended state 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_RI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x40)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_RI_VAL
*     Receive Interrupt indicates completion of frame reception 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_UNF_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x20)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_UNF_VAL
*     Transmit Underflow: Transmit buffer had an underflow during transmission 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_OVF_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x10)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_OVF_VAL
*     Receive Overflow: There was an overflow during frame reception 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TJT_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TJT_VAL
*     Transmit Jabber Timeout: The transmitter has been excessively active 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TU_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TU_VAL
*     Transmit Buffer Unavailable: The next descriptor in the transmit list is 
*     owned by host and cannot be acquired by DMA 
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TPS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TPS_VAL
*     Macro indicates Transmit Process is in Stopped state
*/

#define HW_TRIDENTGMACETH_DMA_STATUS_TI_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_DMA_STATUS_TI_VAL
*     Transmit Interrupt: Status to indicate frame transmission is complete
*/


#define HW_TRIDENTGMACETH_FLOWCTRL_FCBBPA_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_FLOWCTRL_FCBBPA_VAL
* Macro for checking the FCB/BPA bit status
*/

/* Additional macros to check the status */
#define HW_TRIDENTGMACETH_INTR_PCS_AN_STAT_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_INTR_PCS_AN_STAT_VAL
* Macro for checking the staus of auto negotiation completion in TBI/RTBI/SGMII PHY interface
*/

#define HW_TRIDENTGMACETH_INTR_PCS_LS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_INTR_PCS_LS_VAL
* Macro for checking the change in the link staus in the TBI/RTBI/SGMII PHY interface
*/

#define HW_TRIDENTGMACETH_INTR_RGMII_STAT_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_INTR_RGMII_STAT_VAL
* Macro for checking the change in the link status for RGMII PHY interface
*/

/*\}*/ /* Group0 definition end */


#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC)

/*
*  MAC management Interrupt status values. All the bits are clear on read 
*  Receive interrupt register macros 
*/

/**
 * \defgroup group3 MAC Management Counters
 */
/*\{*/

#define HW_TRIDENTGMACETH_INTR_MMC_STAT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000000)
/*! \def HW_TRIDENTGMACETH_INTR_MMC_STAT_VAL
* Macro to check if the interrupt is due to MMC unit
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_WD_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_WD_VAL
* Macro to check if the interrupt is due to watchdog error counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_VLANGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_VLANGB_VAL
* Macro to check if  the interrupt is due to rxvlanframes_gb counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_OVERFLOW_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_OVERFLOW_VAL
* Macro to check if  the interrupt is due to rxfifooverflow counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_PAUSE_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_PAUSE_VAL
* Macro to check if  the interrupt is due to rxpauseframes counter reaching 
* half the maximum value
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RX_OUTOFR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_OUTOFR_VAL
* Macro to check if  the interrupt is due to rxoutofrange counter reaching 
* half the maximum value 
*/



#define HW_TRIDENTGMACETH_MMC_INTR_RX_LEN_ERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_LEN_ERR_VAL
* Macro to check if  the interrupt is due to rxlengtherror counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_UNICASTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_UNICASTGB_VAL
* Macro to check if  the interrupt is due to rxunicastframes_gb counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RX_1024TOMAXGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_1024TOMAXGB_VAL
* Macro to check if  the interrupt is due to rx1024tomaxoctects_gb counter reaching 
* half the maximum value  
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_512TO1023GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_512TO1023GB_VAL
* Macro to check if  the interrupt is due to rx512to1023octects_gb counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_256TO511GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_256TO511GB_VAL
* Macro to check if  the interrupt is due to rx256to511octects_gb counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_128TO255GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_128TO255GB_VAL
* Macro to check if  the interrupt is due to rx128to255octects_gb counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_65TO127GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_65TO127GB_VAL
* Macro to check if  the interrupt is due to rx65to127octects_gb counter 
* reaching half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_64GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_64GB_VAL
* Macro to check if  the interrupt is due to rx64octects_gb counter 
* reaching half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_OVERSIZEG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_OVERSIZEG_VAL
* Macro to check if  the interrupt is due to rxoversize_g counter reaching 
* half the maximum value
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RX_UNDERSIZEG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_UNDERSIZEG_VAL
* Macro to check if  the interrupt is due to rxundersize_g counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_JABBER_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_JABBER_VAL
* Macro to check if  the interrupt is due to rxjabbererror counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RX_RUNTERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_RUNTERR_VAL
* Macro to check if  the interrupt is due to rxrunterror counter reaching half the 
* maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_ALIGN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_ALIGN_VAL
* Macro to check if  the interrupt is due to rxalignmenterror counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_CRC_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_CRC_VAL
* Macro to check if  the interrupt is due to rxcrcerror counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_MULTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_MULTG_VAL
* Macro to check if  the interrupt is due to rxmulticastframes_g counter 
* reaching half the maximum value
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RX_BDCSTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_BDCSTG_VAL
* Macro to check if  the interrupt is due to rxbroadcastframes_g  counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_OCTCNTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_OCTCNTG_VAL
* Macro to check if  the interrupt is due to rxoctetcount_g counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_OCTCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_OCTCNTGB_VAL
* Macro to check if  the interrupt is due to rxoctetcount_gb counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RX_FRMCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RX_FRMCNTGB_VAL
* Macro to check if  the interrupt is due to rxframecount_gb counter reaching 
* half the maximum value
*/

/* Checksum offload interrupt status register macros. Register is clear on read */

#define HW_TRIDENTGMACETH_MMC_INTR_RXICMP_ERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXICMP_ERROCT_VAL
* Macro to check if  the interrupt is due to rxicmp_err_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXICMP_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000000) 
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXICMP_GDOCT_VAL
* Macro to check if  the interrupt is due to rxicmp_gd_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXTCP_ERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXTCP_ERROCT_VAL
* Macro to check if  the interrupt is due to rxtcp_err_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXTCP_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000000 ) 
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXTCP_GDOCT_VAL
* Macro to check if  the interrupt is due to rxtcp_gd_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXUDP_ERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXUDP_ERROCT_VAL
* Macro to check if  the interrupt is due to rxudp_err_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXUDP_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXUDP_GDOCT_VAL
* Macro to check if  the interrupt is due to rxudp_gd_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_NOPAYOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_NOPAYOCT_VAL
* Macro to check if  the interrupt is due to rxipv6_nopay_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_HDRERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_HDRERROCT_VAL
* Macro to check if  the interrupt is due to rxipv6_hdrerr_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_GDOCT_VAL
* Macro to check if  the interrupt is due to rxipv6_gd_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_UDPDISOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_UDPDISOCT_VAL
* Macro to check if  the interrupt is due to rxipv4_udsbl_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_FRAGOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_FRAGOCT_VAL
* Macro to check if  the interrupt is due to rxipv4_frag_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_NOPAYOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_NOPAYOCT_VAL
* Macro to check if  the interrupt is due to rxipv4_nopay_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_HDRERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_HDRERROCT_VAL
* Macro to check if  the interrupt is due to rxipv4_hdrerr_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_GDOCT_VAL
* Macro to check if  the interrupt is due to rxipv4_gd_octets counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXICMP_ERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXICMP_ERR_FRMS_VAL
* Macro to check if  the interrupt is due to rxicmp_err_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXICMP_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXICMP_GD_FRMS_VAL
* Macro to check if  the interrupt is due to rxicmp_gd_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXTCP_ERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXTCP_ERR_FRMS_VAL
* Macro to check if  the interrupt is due to rxtcp_err_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXTCP_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXTCP_GD_FRMS_VAL
* Macro to check if  the interrupt is due to rxtcp_gd_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXUDP_ERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXUDP_ERR_FRMS_VAL
* Macro to check if  the interrupt is due to rxudp_err_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXUDP_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXUDP_GD_FRMS_VAL
* Macro to check if  the interrupt is due to rxudp_err_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_NOPAY_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_NOPAY_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv6_nopay_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_HDRERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_HDRERR_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv6_hdrerr_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV6_GD_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv6_gd_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_UDPDIS_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_UDPDIS_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv4_udsbl_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_FRAG_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_FRAG_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv4_frag_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_NOPAY_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_NOPAY_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv4_nopay_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_HDRERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_HDRERR_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv4_hdrerr_frms counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXIPV4_GD_FRMS_VAL
* Macro to check if  the interrupt is due to rxipv4_gd_frms counter reaching 
* half the maximum value
*/


/* MMC Transmit interrupt register status macros */

#define HW_TRIDENTGMACETH_MMC_INTR_TX_VLANG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_VLANG_VAL
* Macro to check if  the interrupt is due to txvlanframes_g counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_PAUSEERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_PAUSEERR_VAL
* Macro to check if  the interrupt is due to txpauseframes error counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_EXSDEF_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_EXSDEF_VAL
* Macro to check if  the interrupt is due to txoexcessdef  counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_FRMCNTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_FRMCNTG_VAL
* Macro to check if  the interrupt is due to txframecount_g  counter reaching 
* half the maximum value
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_OCTCNTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_OCTCNTG_VAL
* Macro to check if  the interrupt is due to txoctectcount_g   counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_CARERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_CARERR_VAL
* Macro to check if  the interrupt is due to txcarriererror counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_EXSCOL_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_EXSCOL_VAL
* Macro to check if  the interrupt is due to txexcesscol counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_LATECOL_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_LATECOL_VAL
* Macro to check if  the interrupt is due to txlatecol counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_DEFCNT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_DEFCNT_VAL
* Macro to check if  the interrupt is due to txdeferred counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_MULTICOLG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_MULTICOLG_VAL
* Macro to check if  the interrupt is due to txmulticol_g counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_SINGLECOL_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_SINGLECOL_VAL
* Macro to check if  the interrupt is due to txsinglecol_g counter reaching half 
* the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_UNFLWERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_UNFLWERR_VAL
* Macro to check if  the interrupt is due to txunderflowerror counter 
* reaching half the maximum value  
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_BRDCSTFMGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_BRDCSTFMGB_VAL
* Macro to check if  the interrupt is due to txbroadcastframes_gb counter reaching 
* half the maximum value
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_MULTCSTFMGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_MULTCSTFMGB_VAL
* Macro to check if  the interrupt is due to txmulticastframes_gb counter 
* reaching half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_UNICSTFMGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_UNICSTFMGB_VAL
* Macro to check if  the interrupt is due to txunicastframes_gb counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_1024TOMAXGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_1024TOMAXGB_VAL
* Macro to check if  the interrupt is due to tx1024tomaxoctects_gb counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_512TO1023GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_512TO1023GB_VAL
* Macro to check if  the interrupt is due to tx512to1023octects_gb counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_256TO511GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_256TO511GB_VAL
* Macro to check if  the interrupt is due to tx256to511octects_gb counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_128TO255GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_128TO255GB_VAL
* Macro to check if  the interrupt is due to tx128to255octects_gb counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_65TO127GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_65TO127GB_VAL
* Macro to check if  the interrupt is due to tx65to127octects_gb counter reaching 
* half the maximum value 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_64GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_64GB_VAL
* Macro to check if  the interrupt is due to tx64to127octects_gb counter reaching 
* half the maximum value   
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_MULTCSTFRMG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_MULTCSTFRMG_VAL
* Macro to check if  the interrupt is due to txmulticastframes_g counter reaching 
* half the maximum value   
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_BDCSTFRMG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_BDCSTFRMG_VAL
* Macro to check if  the interrupt is due to txbroadcastframes_g counter reaching 
* half the maximum value   
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TX_FRMCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_FRMCNTGB_VAL
* Macro to check if  the interrupt is due to txframecount_gb counter reaching
* half the maximum value   
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TX_OCTCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TX_OCTCNTGB_VAL
* Macro to check if  the interrupt is due to txoctectcount_gb counter reaching 
* half the maximum value 
*/


/* MMC bit masks to disable interrupts in Rx Directon */

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_WD_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_WD_VAL
* Macro to mask the  generation of rxwatchdog interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_VLANGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_VLANGB_VAL
* Macro to mask the  generation of rxvlanframes_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OVERFLOW_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OVERFLOW_VAL
* Macro to mask the  generation of rxfifooverflow interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_PAUSE_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_PAUSE_VAL
* Macro to mask the  generation of rxpauseframes interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OUTOFR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OUTOFR_VAL
* Macro to mask the  generation of  rxoutofrange interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_LEN_ERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_LEN_ERR_VAL
* Macro to mask the  generation of rxlengtherror interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_UNICASTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_UNICASTGB_VAL
* Macro to mask the  generation of rxunicastframes_gb interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_1024TOMAXGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_1024TOMAXGB_VAL
* Macro to mask the  generation of rx1024tomaxoctects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_512TO1023GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_512TO1023GB_VAL
* Macro to mask the  generation of rx512to1023octects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_256TO511GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_256TO511GB_VAL
* Macro to mask the  generation of rx256to511octects_gb interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_128TO255GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_128TO255GB_VAL
* Macro to mask the  generation of rx128to255octects_gb interrupt 
*/

/* Macro to mask the  generation of rx65to127octects_gb interrupt */
#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_65TO127GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_65TO127GB_VAL
* Macro to check if  the interrupt is due to rxcrcerror counter reaching 
* half the maximum value 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_64GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_64GB_VAL
* Macro to mask the  generation of rx64octects_gb interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OVERSIZEG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OVERSIZEG_VAL
* Macro to mask the  generation of rxoversize_g interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_UNDERSIZEG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_UNDERSIZEG_VAL
* Macro to mask the  generation of rxundersize_g interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_JABBER_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_JABBER_VAL
* Macro to mask the  generation of rxjabbererror  interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_RUNTERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_RUNTERR_VAL
* Macro to mask the  generation of rxrunterror interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_ALIGN_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_ALIGN_VAL
* Macro to mask the  generation of rxalignmenterror interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_CRC_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_CRC_VAL
* Macro to mask the  generation of  rxcrcerror interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_MULTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_MULTG_VAL
* Macro to mask the  generation of rxmulticastframes_g interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_BDCSTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_BDCSTG_VAL
* Macro to mask the  generation of rxbroadcastframes_g interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OCTCNTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OCTCNTG_VAL
* Macro to mask the  generation of rxoctetcount_g interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OCTCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_OCTCNTGB_VAL
* Macro to mask the  generation of rxoctetcount_gb interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_RXMASK_FRMCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_RXMASK_FRMCNTGB_VAL
* Macro to mask the  generation of rxframecount_gb interrupt 
*/

/* MMC Transmit interrupt mask register bits */

#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_VLANG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_VLANG_VAL
* Macro to mask the  generation of  txvlanframes_g interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_PAUSEERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_PAUSEERR_VAL
* Macro to mask the  generation of  txpauseframes interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_EXSDEF_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_EXSDEF_VAL
* Macro to mask the  generation of  txoexcessdef interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_FRMCNTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_FRMCNTG_VAL
* Macro to mask the  generation of  txframecount_g   interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_OCTCNTG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_OCTCNTG_VAL
* Macro to mask the  generation of  txoctectcount_g  interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_CARERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_CARERR_VAL
* Macro to mask the  generation of  txcarriererror interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_EXSCOL_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_EXSCOL_VAL
* Macro to mask the  generation of  txexcesscol interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_LATECOL_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_LATECOL_VAL
* Macro to mask the  generation of  txlatecol interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_DEFCNT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_DEFCNT_VAL
* Macro to mask the  generation of  txdeferred interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_MULTICOLG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_MULTICOLG_VAL
* Macro to mask the  generation of  txmulticol_g interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_SINGLECOL_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_SINGLECOL_VAL
* Macro to mask the  generation of txsinglecol_g   interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_UNFLWERR_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_UNFLWERR_VAL
* Macro to mask the  generation of  txunderflowerror interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_BRDCSTFMGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_BRDCSTFMGB_VAL
* Macro to mask the  generation of  txbroadcastframes_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_MULTCSTFMGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_MULTCSTFMGB_VAL
* Macro to mask the  generation of  txmulticastframes_gb interrupt 
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_UNICSTFMGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_UNICSTFMGB_VAL
* Macro to mask the  generation of  txunicastframes_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_1024TOMAXGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_1024TOMAXGB_VAL
* Macro to mask the  generation of  tx1024tomaxoctects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_512TO1023GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_512TO1023GB_VAL
* Macro to mask the  generation of  tx512to1023octects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_256TO511GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_256TO511GB_VAL
* Macro to mask the  generation of  tx256to511octects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_128TO255GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_128TO255GB_VAL
* Macro to mask the  generation of  tx128to255octects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_65TO127GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_65TO127GB_VAL
* Macro to mask the  generation of  tx65to127octects_gb interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_64GB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_64GB_VAL
* Macro to mask the  generation of  tx64to127octects_gb interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_MULTCSTFRMG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_MULTCSTFRMG_VAL
* Macro to mask the  generation of  txmulticastframes_g interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_BDCSTFRMG_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_BDCSTFRMG_VAL
* Macro to mask the  generation of  txbroadcastframes_g interrupt
*/


#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_FRMCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_FRMCNTGB_VAL
*Macro to mask the  generation of  txframecount_gb interrupt 
*/

#define HW_TRIDENTGMACETH_MMC_INTR_TXMASK_OCTCNTGB_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_TXMASK_OCTCNTGB_VAL
* Macro to mask the  generation of  txoctectcount_gb interrupt
*/

/* Mask to disable generation of interrupts due to ipv4, ipv6, tcp/udp/icmp counters reaching half 
** the maximum value
*/    

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_ERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_ERROCT_VAL
* Macro to mask the  generation of  rxicmp_err_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000000) 
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_GDOCT_VAL
* Macro to mask the  generation of  rxicmp_gd_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_ERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8000000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_ERROCT_VAL
* Macro to mask the  generation of  rxtcp_err_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4000000 ) 
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_GDOCT_VAL
* Macro to mask the  generation of  rxtcp_gd_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_ERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_ERROCT_VAL
* Macro to mask the  generation of  rxudp_err_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000000)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_GDOCT_VAL
* Macro to mask the  generation of  rxudp_gd_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_NOPAYOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_NOPAYOCT_VAL
* Macro to mask the  generation of  rxipv6_nopay_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_HDRERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_HDRERROCT_VAL
* Macro to mask the  generation of  rxipv6_hdrerr_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_GDOCT_VAL
* Macro to mask the  generation of  rxipv6_gd_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_UDPDISOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_UDPDISOCT_VAL
* Macro to mask the  generation of  rxipv4_udsbl_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_FRAGOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_FRAGOCT_VAL
* Macro to mask the  generation of  rxipv4_frag_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_NOPAYOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_NOPAYOCT_VAL
* Macro to mask the  generation of  rxipv4_nopay_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_HDRERROCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_HDRERROCT_VAL
* Macro to mask the  generation of  rxipv4_hdrerr_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_GDOCT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_GDOCT_VAL
* Macro to mask the  generation of  rxipv4_gd_octets interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_ERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_ERR_FRMS_VAL
* Macro to mask the  generation of  rxicmp_err_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1000 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXICMP_GD_FRMS_VAL
* Macro to mask the  generation of  rxicmp_gd_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_ERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x800 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_ERR_FRMS_VAL
* Macro to mask the  generation of  rxtcp_err_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x400 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXTCP_GD_FRMS_VAL
* Macro to mask the  generation of  rxtcp_gd_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_ERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_ERR_FRMS_VAL
* Macro to mask the  generation of  rxudp_err_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x100 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXUDP_GD_FRMS_VAL
* Macro to mask the  generation of  rxudp_gd_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_NOPAY_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x80 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_NOPAY_FRMS_VAL
* Macro to mask the  generation of  rxipv6_nopay_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_HDRERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x40 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_HDRERR_FRMS_VAL
* Macro to mask the  generation of  rxipv6_hdrerr_frms counter interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x20 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV6_GD_FRMS_VAL
* Macro to mask the  generation of  rxipv6_hdrerr_frms counter interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_UDPDIS_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x10 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_UDPDIS_FRMS_VAL
* Macro to mask the  generation of  rxipv4_udsbl_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_FRAG_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_FRAG_FRMS_VAL
* Macro to mask the  generation of  rxipv4_frag_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_NOPAY_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x4 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_NOPAY_FRMS_VAL
* Macro to mask the  generation of  rxipv4_nopay_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_HDRERR_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x2 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_HDRERR_FRMS_VAL
* Macro to mask the  generation of  rxipv4_hdrerr_frms interrupt
*/

#define HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_GD_FRMS_VAL ((hwTRIDENTGMACEth_IntMask_t)0x1 )
/*! \def HW_TRIDENTGMACETH_MMC_INTR_MSKRXIPV4_GD_FRMS_VAL
* Macro to mask the  generation of  rxipv4_gd_frms interrupt
*/

/* Macros to read various MAC management counters */

#define HW_TRIDENTGMACETH_TX_OCTET_CNT_GB (0x0U)
/*! \def HW_TRIDENTGMACETH_TX_OCTET_CNT_GB
* Macro used to read the txoctetcount_gb counter
*/


#define HW_TRIDENTGMACETH_TX_FRAME_CNT_GB (0x1U)
/*! \def HW_TRIDENTGMACETH_TX_FRAME_CNT_GB
* Macro used to read the txframecount_gb counter
*/

#define HW_TRIDENTGMACETH_TX_BRDCST_CNT_G (0x2U)
/*! \def HW_TRIDENTGMACETH_TX_BRDCST_CNT_G
* Macro used to read the txbroadcastframes_g counter
*/

#define HW_TRIDENTGMACETH_TX_MULTCST_CNT_G (0x3U)
/*! \def HW_TRIDENTGMACETH_TX_MULTCST_CNT_G
* Macro used to read the txmulticastframes_g counter 
*/

#define HW_TRIDENTGMACETH_TX_64_CNT_GB (0x4U)
/*! \def HW_TRIDENTGMACETH_TX_64_CNT_GB
* Macro used to read the tx64octets_gb counter
*/

#define HW_TRIDENTGMACETH_TX_65TO127_CNT_GB (0x5U)
/*! \def HW_TRIDENTGMACETH_TX_65TO127_CNT_GB
* Macro used to read the tx65to127octets_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_128TO255_CNT_GB (0x6U)
/*! \def HW_TRIDENTGMACETH_TX_128TO255_CNT_GB
* Macro used to read the tx128to255octets_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_256TO511_CNT_GB (0x7U)
/*! \def HW_TRIDENTGMACETH_TX_256TO511_CNT_GB
* Macro used to read the tx256to511octets_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_512TO1023_CNT_GB (0x8U)
/*! \def HW_TRIDENTGMACETH_TX_512TO1023_CNT_GB
* Macro used to read the tx512to1023octets_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_1024TOMAX_CNT_GB (0x9U)
/*! \def HW_TRIDENTGMACETH_TX_1024TOMAX_CNT_GB
* Macro used to read the tx1024tomaxoctets_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_UNICAST_CNT_GB (0xAU)
/*! \def HW_TRIDENTGMACETH_TX_UNICAST_CNT_GB
* Macro used to read the txunicastframes_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_MULTCST_CNT_GB (0xBU)
/*! \def HW_TRIDENTGMACETH_TX_MULTCST_CNT_GB
*  Macro used to read the txmulticastcastframes_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_BRDCST_CNT_GB (0xCU)
/*! \def HW_TRIDENTGMACETH_TX_BRDCST_CNT_GB
* Macro used by MMC API to read the txbroadcastframes_gb counter 
*/

#define HW_TRIDENTGMACETH_TX_UNDERFLOW_ERR_CNT (0xDU)
/*! \def HW_TRIDENTGMACETH_TX_UNDERFLOW_ERR_CNT
* Macro used by MMC API to read the txunderflowerror counter 
*/

#define HW_TRIDENTGMACETH_TX_SINGLE_COL_CNT_G (0xEU)
/*! \def HW_TRIDENTGMACETH_TX_SINGLE_COL_CNT_G
* Macro used by MMC API to read the txsinglecol_g counter 
*/

#define HW_TRIDENTGMACETH_TX_MULTICOL_COL_G (0xFU)
/*! \def HW_TRIDENTGMACETH_TX_MULTICOL_COL_G
* Macro used by MMC API to read the txmulticol_g counter 
*/

#define HW_TRIDENTGMACETH_TX_DEFERRED_CNT (0x10U)
/*! \def HW_TRIDENTGMACETH_TX_DEFERRED_CNT
* Macro used by MMC API to read the txdeferred counter 
*/

#define HW_TRIDENTGMACETH_TX_LATECOL_CNT (0x11U)
/*! \def HW_TRIDENTGMACETH_TX_LATECOL_CNT
* Macro used by MMC API to read txlatecol counter
*/

#define HW_TRIDENTGMACETH_TX_EXCESSCOL_CNT (0x12U)
/*! \def HW_TRIDENTGMACETH_TX_EXCESSCOL_CNT
* Macro used by MMC API to read txexcesscol counter 
*/

#define HW_TRIDENTGMACETH_TX_CARRIER_ERR_CNT (0x13U)
/*! \def HW_TRIDENTGMACETH_TX_CARRIER_ERR_CNT
* Macro used by MMC API to read txcarriererror counter 
*/

#define HW_TRIDENTGMACETH_TX_OCTET_CNT_G (0x14U)
/*! \def HW_TRIDENTGMACETH_TX_OCTET_CNT_G
* Macro used by MMC API to read txoctetcount_g counter 
*/

#define HW_TRIDENTGMACETH_TX_FRAME_CNT_G (0x15U)
/*! \def HW_TRIDENTGMACETH_TX_FRAME_CNT_G
* Macro used by MMC API to read txframecount_g counter 
*/

#define HW_TRIDENTGMACETH_TX_EXCESSDEF_CNT (0x16U)
/*! \def HW_TRIDENTGMACETH_TX_EXCESSDEF_CNT
* Macro used by MMC API to read txexcessdef counter
*/

#define HW_TRIDENTGMACETH_TX_PAUSE_FRAMES_CNT (0x17U)
/*! \def HW_TRIDENTGMACETH_TX_PAUSE_FRAMES_CNT
* Macro used by MMC API to read txpauseframes counter 
*/

#define HW_TRIDENTGMACETH_TX_VLAN_FRAMES_CNT_G (0x18U)
/*! \def HW_TRIDENTGMACETH_TX_VLAN_FRAMES_CNT_G
* Macro used by MMC API to read txvlanframes_g counter 
*/

/* Receive frame statistics*/

 #define HW_TRIDENTGMACETH_RX_FRM_CNT_GB (0x32U)
/*! \def HW_TRIDENTGMACETH_RX_FRM_CNT_GB
* Macro used by MMC API to read rxframecount_gb counter 
*/

#define HW_TRIDENTGMACETH_RX_OCTET_CNT_GB (0x33U)
/*! \def HW_TRIDENTGMACETH_RX_OCTET_CNT_GB
* Macro used by MMC API to read rxoctetcount_gb counter 
*/

#define HW_TRIDENTGMACETH_RX_OCTET_CNT_G (0x34U)
/*! \def HW_TRIDENTGMACETH_RX_OCTET_CNT_G
* Macro used by MMC API to read rxoctetcount_g counter 
*/

#define HW_TRIDENTGMACETH_RX_BRDCSTF_CNT_G (0x35U)
/*! \def HW_TRIDENTGMACETH_RX_BRDCSTF_CNT_G
* Macro used by MMC API to read rxbroadcastframes_g counter
*/

#define HW_TRIDENTGMACETH_RX_MULTCSTF_CNT_G (0x36U)
/*! \def HW_TRIDENTGMACETH_RX_MULTCSTF_CNT_G
* Macro used by MMC API to read rxmulticastframes_g counter
*/

#define HW_TRIDENTGMACETH_RX_CRC_ERR_CNT (0x37U)
/*! \def HW_TRIDENTGMACETH_RX_CRC_ERR_CNT
* Macro used by MMC API to read rxcrcerror counter
*/

#define HW_TRIDENTGMACETH_RX_ALIGNMT_ERR_CNT (0x38U)
/*! \def HW_TRIDENTGMACETH_RX_ALIGNMT_ERR_CNT
* Macro used by MMC API to read rxalignmenterror counter
*/

#define HW_TRIDENTGMACETH_RX_RUNT_ERR_CNT (0x39U)
/*! \def HW_TRIDENTGMACETH_RX_RUNT_ERR_CNT
* Macro used by MMC API to read rxrunterror counter
*/

#define HW_TRIDENTGMACETH_RX_JABBER_ERR_CNT (0x3AU)
/*! \def HW_TRIDENTGMACETH_RX_JABBER_ERR_CNT
* Macro used by MMC API to read rxjabbererror counter
*/

#define HW_TRIDENTGMACETH_RX_UNDERSIZE_CNT_G (0x3BU)
/*! \def HW_TRIDENTGMACETH_RX_UNDERSIZE_CNT_G
* Macro used by MMC API to read rxundersize_g counter
*/


#define HW_TRIDENTGMACETH_RX_OVERSIZE_CNT_G (0x3CU)
/*! \def HW_TRIDENTGMACETH_RX_OVERSIZE_CNT_G
* Macro used by MMC API to read rxoversize_g counter
*/

#define HW_TRIDENTGMACETH_RX_64_CNT_GB (0x3DU)
/*! \def HW_TRIDENTGMACETH_RX_64_CNT_GB
* Macro used by MMC API to read rx64octets_gb counter
*/



#define HW_TRIDENTGMACETH_RX_65TO127_CNT_GB (0x3EU)
/*! \def HW_TRIDENTGMACETH_RX_65TO127_CNT_GB
* Macro used by MMC API to read rx65to127octets_gb counter
*/

#define HW_TRIDENTGMACETH_RX_128TO255_CNT_GB (0x3FU)
/*! \def HW_TRIDENTGMACETH_RX_128TO255_CNT_GB
* Macro used by MMC API to read rx128to255octets_gb counter
*/

#define HW_TRIDENTGMACETH_RX_256TO511_CNT_GB (0x40U)
/*! \def HW_TRIDENTGMACETH_RX_256TO511_CNT_GB
* Macro used by MMC API to read rx256to511octets_gb counter
*/

#define HW_TRIDENTGMACETH_RX_512TO1023_CNT_GB (0x41U)
/*! \def HW_TRIDENTGMACETH_RX_512TO1023_CNT_GB
* Macro used by MMC API to read rx512to1023octets_gb counter
*/

#define HW_TRIDENTGMACETH_RX_1024TOMAX_CNT_GB (0x42U)
/*! \def HW_TRIDENTGMACETH_RX_1024TOMAX_CNT_GB
* Macro used by MMC API to read rx1024tomaxoctets_g counter
*/

#define HW_TRIDENTGMACETH_RX_UNICAST_CNT_G (0x43U)
/*! \def HW_TRIDENTGMACETH_RX_UNICAST_CNT_G
* Macro used by MMC API to read rxunicastframes_g counter
*/

#define HW_TRIDENTGMACETH_RX_LEN_ERR_CNT (0x44U)
/*! \def HW_TRIDENTGMACETH_RX_LEN_ERR_CNT
* Macro used by MMC API to read rxlengtherror counter
*/

#define HW_TRIDENTGMACETH_RX_OUTOFRANGE_CNT (0x45U)
/*! \def HW_TRIDENTGMACETH_RX_OUTOFRANGE_CNT
* Macro used by MMC API to read rxoutofrangetype counter
*/

#define HW_TRIDENTGMACETH_RX_PAUSE_CNT (0x46U)
/*! \def HW_TRIDENTGMACETH_RX_PAUSE_CNT
* Macro used by MMC API to read rxpauseframes counter
*/

#define HW_TRIDENTGMACETH_RX_FIFO_OVERFLOW_CNT (0x47U)
/*! \def HW_TRIDENTGMACETH_RX_FIFO_OVERFLOW_CNT
* Macro used by MMC API to read rxfifooverflow counter
*/

#define HW_TRIDENTGMACETH_RX_VLAN_FRAMES_CNT_GB (0x48U)
/*! \def HW_TRIDENTGMACETH_RX_VLAN_FRAMES_CNT_GB
* Macro used by MMC API to read rxvlanframes_gb counter
*/

#define HW_TRIDENTGMACETH_RX_WATCHDOG_ERR_CNT (0x49U)
/*! \def HW_TRIDENTGMACETH_RX_WATCHDOG_ERR_CNT
* Macro used by MMC API to read rxwatchdogerror counter 
*/

/* Macros to read checksum offload counters on the receive side */

#define HW_TRIDENTGMACETH_RX_IPV4_FRMCNT_G (0x5AU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_FRMCNT_G
* Macro used by MMC API to read number of ipv4 good frames received
*/

#define HW_TRIDENTGMACETH_RX_IPV4_HDR_ERR_FRMCNT (0x5BU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_HDR_ERR_FRMCNT
* Macro used by MMC API to read number ipv4 packets with header errors
*/

#define HW_TRIDENTGMACETH_RX_IPV4_NOPPAY_FRMCNT (0x5CU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_NOPPAY_FRMCNT
* Macro used by MMC API to read number of ipv4 frames without TCP/UDP/ICMP payloads
*/

#define HW_TRIDENTGMACETH_RX_IPV4_FRAG_FRMCNT (0x5DU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_FRAG_FRMCNT
* Macro used by MMC API to read number of good IPv4 datagrams with fragmentation
*/

#define HW_TRIDENTGMACETH_RX_IPV4_UDPCSUMDSL_FRMCNT (0x5EU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_UDPCSUMDSL_FRMCNT
* Macro used by MMC API to read number of UDP frames without checksum
*/

#define HW_TRIDENTGMACETH_RX_IPV6_FRMCNT_G (0x5FU)
/*! \def HW_TRIDENTGMACETH_RX_IPV6_FRMCNT_G
* Macro used by MMC API to read number of ipv6 frames 
*/

#define HW_TRIDENTGMACETH_RX_IPV6_HDR_ERR_FRMCNT (0x60U)
/*! \def HW_TRIDENTGMACETH_RX_IPV6_HDR_ERR_FRMCNT
* Macro used by MMC API to read number of ipv6 frames with error in header
*/

#define HW_TRIDENTGMACETH_RX_IPV6_NOPAY_FRMCNT (0x61U)
/*! \def HW_TRIDENTGMACETH_RX_IPV6_NOPAY_FRMCNT
* Macro used by MMC API to read number of ipv6 frames without TCP/ICMP/UDP payload
*/

#define HW_TRIDENTGMACETH_RX_UDP_FRMCNT_G (0x62U)
/*! \def HW_TRIDENTGMACETH_RX_UDP_FRMCNT_G
* Macro used by MMC API to read number of UDP frames received
*/

#define HW_TRIDENTGMACETH_RX_UDP_ERR_FRMCNT (0x63U)
/*! \def HW_TRIDENTGMACETH_RX_UDP_ERR_FRMCNT
* Macro used by MMC API to read number of UDP frames received with checksum errors 
*/

#define HW_TRIDENTGMACETH_RX_TCP_FRMCNT_G (0x64U)
/*! \def HW_TRIDENTGMACETH_RX_TCP_FRMCNT_G
* Macro used by MMC API to read number of good TCP frames 
*/

#define HW_TRIDENTGMACETH_RX_TCP_ERR_FRMCNT (0x65U)
/*! \def HW_TRIDENTGMACETH_RX_TCP_ERR_FRMCNT
* Macro used by MMC API to read  of TCP frames received with checksum errors 
*/

#define HW_TRIDENTGMACETH_RX_ICMP_FRMCNT_G (0x66U)
/*! \def HW_TRIDENTGMACETH_RX_ICMP_FRMCNT_G
* Macro used by MMC API to read number of good ICMP frames received
*/

#define HW_TRIDENTGMACETH_RX_ICMP_ERR_FRMCNT (0x67U)
/*! \def HW_TRIDENTGMACETH_RX_ICMP_ERR_FRMCNT
* Macro used by MMC API to read number of ICMP frames received with checksum errors 
*/

/* Octets */
#define HW_TRIDENTGMACETH_RX_IPV4_OCTETS_G (0x68U)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_OCTETS_G
* Macro used by MMC API to read number of ipv4 octets received
*/

#define HW_TRIDENTGMACETH_RX_IPV4_HDR_ERR_OCTETS (0x69U)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_HDR_ERR_OCTETS
* Macro used by MMC API to read number of octets received in ipv4 packets with 
*     header errors
*/

#define HW_TRIDENTGMACETH_RX_IPV4_NOPPAY_OCTETS (0x6AU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_NOPPAY_OCTETS
* Macro used by MMC API to read no. octets received in ipv4 frame with payload 
*     other than TCP/UDP/ICMP 
*/

#define HW_TRIDENTGMACETH_RX_IPV4_FRAG_OCTETS (0x6BU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_FRAG_OCTETS
* Macro used by MMC API to read number of octects in the fragmented IP packet 
*/

#define HW_TRIDENTGMACETH_RX_IPV4_UDPCSUMDSL_OCTETS (0x6CU)
/*! \def HW_TRIDENTGMACETH_RX_IPV4_UDPCSUMDSL_OCTETS
* Macro used by MMC API to read number of octets received in a UDP segment that 
*     had the UDP checksum disabled
*/


#define HW_TRIDENTGMACETH_RX_IPV6_OCTETS_G (0x6DU)
/*! \def HW_TRIDENTGMACETH_RX_IPV6_OCTETS_G
*      Number of bytes received in good IPv6 datagrams encapsulating TCP, UDP or 
*      ICMPv6 data
*/

#define HW_TRIDENTGMACETH_RX_IPV6_HDR_ERR_OCTETS (0x6EU)
/*! \def HW_TRIDENTGMACETH_RX_IPV6_HDR_ERR_OCTETS
* Number of bytes received in IPv6 datagrams with header errors (length, version mismatch).
*/

#define HW_TRIDENTGMACETH_RX_IPV6_NOPAY_OCTETS (0x6FU)
/*! \def HW_TRIDENTGMACETH_RX_IPV6_NOPAY_OCTETS
* Number of bytes received in IPv6 datagrams that did not have a TCP, UDP, or
*     ICMP payload.
*/

#define HW_TRIDENTGMACETH_RX_UDP_OCTETS_G (0x70U)
/*! \def HW_TRIDENTGMACETH_RX_UDP_OCTETS_G
* Number of bytes received in a good UDP segment
*/

#define HW_TRIDENTGMACETH_RX_UDP_ERR_OCTETS (0x71U)
/*! \def HW_TRIDENTGMACETH_RX_UDP_ERR_OCTETS
* Number of bytes received in a UDP segment that had checksum errors
*/

#define HW_TRIDENTGMACETH_RX_TCP_OCTETS_G (0x72U)
/*! \def HW_TRIDENTGMACETH_RX_TCP_OCTETS_G
* Number of bytes received in a good TCP segment
*/

#define HW_TRIDENTGMACETH_RX_TCP_ERR_OCTETS (0x73U)
/*! \def HW_TRIDENTGMACETH_RX_TCP_ERR_OCTETS
* Number of bytes received in a TCP segment with checksum errors
*/

#define HW_TRIDENTGMACETH_RX_ICMP_OCTETS_G (0x74U)
/*! \def HW_TRIDENTGMACETH_RX_ICMP_OCTETS_G
* Number of bytes received in a good ICMP segment
*/

#define HW_TRIDENTGMACETH_RX_ICMP_ERR_OCTETS (0x75U)
/*! \def HW_TRIDENTGMACETH_RX_ICMP_ERR_OCTETS
* Number of bytes received in an ICMP segment with checksum errors
*/

/*\}*/ /* end of group3 */

#endif /* End of MAC management group */

/* Power management related interrupts */

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_POWER)
/**
 * \defgroup group1 Power Management 
 */
/*\{*/

/* Macro to detect the reception of wakeup frame */
#define HW_TRIDENTGMACETH_PWRMGMT_WKUPFM_RECVD_VAL (0x40U)
/*! \def HW_TRIDENTGMACETH_PWRMGMT_WKUPFM_RECVD_VAL
*       Macro to detect the reception of wakeup frame
*/

/* Macro to detect the reception of magic packet */
#define HW_TRIDENTGMACETH_PWRMGMT_MAGICPKT_RECVD_VAL (0x20U)
/*! \def HW_TRIDENTGMACETH_PWRMGMT_MAGICPKT_RECVD_VAL
*       Macro to detect the reception of magic packet
*/

#define HW_TRIDENTGMACETH_INTR_PMT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_INTR_PMT_VAL
* Macro to enable/disable interrupt generation from power management
*/

#define HW_TRIDENTGMACETH_INTR_PMT_STAT_VAL ((hwTRIDENTGMACEth_IntMask_t)0x8)
/*! \def HW_TRIDENTGMACETH_INTR_PMT_STAT_VAL
* Macro to check if the PMT bit in interrupt status register is set
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD_MSK (0xFU)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD_MSK
* Mask to extract the command value
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD_EN_VAL (0x1U)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD_EN_VAL
* Macro to check if the PMT bit in interrupt status register is set
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD_MULT_VAL (0x8U)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD_MULT_VAL
* Macro to enable multicast wakeup frame 
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD1_POS (8)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD1_POS
* Macro for command value for wakeup framefilter1 register
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD2_POS (16)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD2_POS
* Macro for command value for wakeup framefilter2 register
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD3_POS (24)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CMD3_POS
* Macro for command value for wakeup framefilter3 register
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET_MSK (0xFFU)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET_MSK
* Wakeup frame filter register offset mask
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET1_POS (8)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET1_POS
* Wakeup frame filter register offset position for frame filter1
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET2_POS (16)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET2_POS
* Wakeup frame filter register offset position for frame filter2
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET3_POS (24)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_OFFSET3_POS
* Wakeup frame filter register offset position for frame filter3
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CRC_MSK (0xFFFFU)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CRC_MSK
* Wakeup frame filter register crc mask
*/

#define HW_TRIDENTGMACETH_WKUP_FMFILTER_CRC_POS (16)
/*! \def HW_TRIDENTGMACETH_WKUP_FMFILTER_CRC_POS
* Wakeup frame filter register crc offset for filter 1 and 3
*/

/*\}*/
#endif /* End of power management */

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_IEEE1588_TIMESTAMP)

/**
 * \defgroup group6 IEEE 1588 Time Stamp option
 */
/*\{*/

#define HW_TRIDENTGMACETH_TS_CTRL_ADDEND_UPDT_VAL (0x20U)
/*! \def HW_TRIDENTGMACETH_TS_CTRL_ADDEND_UPDT_VAL
*  Macro to enable addend register update. The Time Stamp Addend register’s contents are 
*  updated to the PTP block for fine correction.
*/

#define HW_TRIDENTGMACETH_TS_CTRL_INT_TRIG_VAL (0x10U)
/*! \def HW_TRIDENTGMACETH_TS_CTRL_INT_TRIG_VAL
*   Macro to enable time stamp interrupt when the system time becomes greater than the value 
*   written in Target Time register.
*/

#define HW_TRIDENTGMACETH_TS_CTRL_TS_UPDATE_VAL (0x8U)
/*! \def HW_TRIDENTGMACETH_TS_CTRL_TS_UPDATE_VAL
*   Used when the system time needs to be  updated (added to or subtracted from) with the
*   value specified in the Time Stamp High Update and Time Stamp Low Update registers.
*/

#define HW_TRIDENTGMACETH_TS_CTRL_TS_INIT_VAL (0x4U)
/*! \def HW_TRIDENTGMACETH_TS_CTRL_TS_INIT_VAL
*  When used  the system time is initialized (overwritten) with the value specified in the 
*  Time Stamp High Update and Time Stamp Low Update registers.
*/

#define HW_TRIDENTGMACETH_TS_CTRL_FINE_UPDATE_VAL (0x2)
/*! \def HW_TRIDENTGMACETH_TS_CTRL_FINE_UPDATE_VAL
*  This macro is used when system time stamp is to be updated using fine update method
*/

#define HW_TRIDENTGMACETH_TS_CTRL_TIMESTAMP_EN_VAL (0x1U)
/*! \def HW_TRIDENTGMACETH_TS_CTRL_TIMESTAMP_EN_VAL
*  This macro is used to enable time stamping for transmit and receive frames
*/

/* Interrupt status related macros w.r.t time stamp options */

#define HW_TRIDENTGMACETH_TS_INT_TRIG_STATUS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x20000000)
/*! \def HW_TRIDENTGMACETH_TS_INT_TRIG_STATUS_VAL
*     Time stamp interrupt trigger status value. This is a read only bit.
*/

#define HW_TRIDENTGMACETH_TS_INT_STATUS_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_TS_INT_STATUS_VAL
*     Time stamp interrupt status. This is a clear on read bit.
*/

/* Macro to disable Time stamp interrupts */

#define HW_TRIDENTGMACETH_TS_INT_MSK_VAL ((hwTRIDENTGMACEth_StatusMask_t)0x200)
/*! \def HW_TRIDENTGMACETH_TS_INT_MSK_VAL
*     Macro used to mask time stamp interrupt generation.
*/

/*\}*/

#endif

/*-----------------------------------------------------------------------------
** Data structures:
**-----------------------------------------------------------------------------
*/

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
/**
 * \defgroup group5 Debug/Support Functions
 */
/*\{*/

typedef struct hwTRIDENTGMACEth_DmaHostRegs
/*! Current Host register values */
{
    UInt32 curHostTxDesc ;          /*!< Current Host transmit descriptor */
    UInt32 curHostRxDesc;           /*!< Current Host Receive descriptor */
    UInt32 curHostTxBufferAdr;   /*!< Current Host Transmit Buffer address */
    UInt32 curHostRxBufferAdr;   /*!< Current Host Receive Buffer address */
}hwTRIDENTGMACEth_DmaHostRegs_t,*phwTRIDENTGMACEth_DmaHostRegs_t;

typedef struct hwTRIDENTGMACEth_DmaMissedFrmCnt
/*!Missed Frames Counter */    
{
    UInt32 fifoOverflowCnt;    
    UInt32 buffNotAvlCnt;    
} hwTRIDENTGMACEth_DmaMissedFrmCnt_t,*phwTRIDENTGMACEth_DmaMissedFrmCnt_t;

/*\}*/

#endif

/*! \addtogroup group0
   *  
   *  \{
   */
/* MDC clock values */
typedef enum hwTRIDENTGMACEth_ClkDiv
/*! This enum is useful while deciding the MDIO clock frequency */
{
    hwTRIDENTGMACEth_ClkDiv_0,  /*!< clk_csr_i/42 */    
    hwTRIDENTGMACEth_ClkDiv_1,  /*!< clk_csr_i/62 */      
    hwTRIDENTGMACEth_ClkDiv_2,  /*!< clk_csr_i/16 */
    hwTRIDENTGMACEth_ClkDiv_3,  /*!< clk_csr_i/26 */
    hwTRIDENTGMACEth_ClkDiv_4,  /*!< clk_csr_i/102 */
    hwTRIDENTGMACEth_ClkDiv_5   /*!< clk_csr_i/122 */    

}hwTRIDENTGMACEth_ClkDiv_t,*phwTRIDENTGMACEth_ClkDiv_t;

/* Flow control structures */
typedef enum  hwTRIDENTGMACEth_IFG
/*! This enum is useful for selecting the interframe gap duration */
{
    hwTRIDENTGMACEth_IFG96bits=0,  /*!< Inter frame gap of 96 bit times */    
    hwTRIDENTGMACEth_IFG88bits,      /*!< Inter frame gap of 88 bit times */    
    hwTRIDENTGMACEth_IFG80bits,      /*!< Inter frame gap of 80 bit times */       
    hwTRIDENTGMACEth_IFG72bits,      /*!< Inter frame gap of 72 bit times */          
    hwTRIDENTGMACEth_IFG64bits,      /*!< Inter frame gap of 64 bit times */    
    hwTRIDENTGMACEth_IFG56bits,      /*!< Inter frame gap of 56 bit times */      
    hwTRIDENTGMACEth_IFG48bits,      /*!< Inter frame gap of 48 bit times */           
    hwTRIDENTGMACEth_IFG40bits       /*!< Inter frame gap of 40 bit times */              
}hwTRIDENTGMACEth_IFG_t,*phwTRIDENTGMACEth_IFG_t;

typedef enum  hwTRIDENTGMACEth_BackOffLimit
/*! This enum decides the random integer of slot delays the GMAC waits
*     before rescheduling a transmission attempt after collision 
*     n = Retransmission attempt
*     Range of r : 0 <= r < 2^k
*/
{
    hwTRIDENTGMACEth_BackoffVal0=0,  /*!< k= min(n,10)*/
    hwTRIDENTGMACEth_BackoffVal1,      /*!< k= min(n,8)*/
    hwTRIDENTGMACEth_BackoffVal2,      /*!< k= min(n,4)*/   
    hwTRIDENTGMACEth_BackoffVal3       /*!< k= min(n,1)*/ 
   
}hwTRIDENTGMACEth_BackOffLimit_t,*phwTRIDENTGMACEth_BackOffLimit_t;

typedef struct hwTRIDENTGMACEth_PhyInfo
/*! Structure  to store PHY & Clk Csr value */
{
    UInt32 phyID;
    UInt32 phyAddr;    /*!< PHY address  */
    UInt32 clkCsrVal;  /*!< MDC clock    */
}hwTRIDENTGMACEth_PhyInfo_t, *phwTRIDENTGMACEth_PhyInfo_t;

/*! \brief Constant pointer to constant structure of type hwTRIDENTGMACEth_PhyInfo_t */
typedef const hwTRIDENTGMACEth_PhyInfo_t* const hwTRIDENTGMACEth_PhyInfo_Kpk_t;

typedef struct hwTRIDENTGMACEth_StationAddress
/*! Structure  to store the station address */
{
    UInt32 adrHigh; /*!< Upper 2 bytes of station address */
    UInt32 adrLow;  /*!< Lower 4 bytes of station address*/  

}hwTRIDENTGMACEth_StationAddress_t, *phwTRIDENTGMACEth_StationAddress_t;

typedef struct  hwTRIDENTGMACEth_DevConfig
/*! Ethernet configuration structure */
{
    Bool txConfiginfo;                /*!< When True, MAC transfers info to PHY in SGMII/RGMII ports*/                  
    Bool wdTimer;                      /*!< When True, disables Watchdog timer */                  
    Bool jabberTimer;                /*!< When True, disables Jabber timer */
    Bool frameBurst;                  /*!< When True, enables frame bursting in 1G mode */
    Bool jumboFrame;                /*!< When True, enables Jumbo frame transmission */
    hwTRIDENTGMACEth_IFG_t ifg;/*!< Interframe gap selection */
    Bool miiSelect;                      /*!< When True, selects MII Interface */
    Bool disableCS;                     /*!< When True, MAC ignores CRS signal during txmn in Half duplex mode */
    Bool speed100Mbps;            /*!< When True, selects 100Mbps mode, False selects 10 Mbps */
    Bool duplexMode;                 /*!< When True, selects duplex mode */    
    Bool disableReceiveOwn;    /*!<  Disable receive Own Enable/Disable */ 
    Bool ipChecksumOffload;     /*!<  Enable IP checksum offload & IPv4 checksum verification */
    Bool disableRetry;                /*!<  When set, GMAC attempts only 1 transmission */
    Bool linkup;                           /*!<  Read only bit. Valid only during get configruation */ 
    Bool autoPadCRC;                 /*!<  GMAC Hw calculates the CRC */ 
    hwTRIDENTGMACEth_BackOffLimit_t backOffLim; /*!< Decides random backoff value */ 
    Bool deferralCheck;             /*!<  Enable/Disable Deferral check */
    hwTRIDENTGMACEth_ClkDiv_t clockSelect; /*!<  MDIO Clock selection */
    UInt32  phyAddress;            /*!< PHY addresss */
    hwTRIDENTGMACEth_StationAddress_t station; /*!< Station Address */
} hwTRIDENTGMACEth_DevConfig_t, *phwTRIDENTGMACEth_DevConfig_t;
/*! \brief Constant pointer to constant structure of type hwTRIDENTGMACEth_DevConfig_t  */
typedef const hwTRIDENTGMACEth_DevConfig_t* const hwTRIDENTGMACEth_DevConfig_Kpk_t;

typedef struct  hwTRIDENTGMACEth_PerfectAdrConfig
/*! Perfect Address filter configuration structure */
{
    Bool addressEnable;  /*!< Enable this perfect address filter */
    Bool srcAddrCmp;      /*!< Compare source address in the received frame */   
    UInt32 addrMask;      /*!< Bytes to ignore while comparison */       
    UInt32 macAddrHigh; /*!< Upper 2 bytes of MAC address */
    UInt32 macAddrlow;  /*!< First 4 bytes of MAC address  */
} hwTRIDENTGMACEth_PerfectAdrConfig_t,*phwTRIDENTGMACEth_PerfectAdrConfig_t;

/*! \brief Constant pointer to constant structure of type hwTRIDENTGMACEth_PerfectAdrConfig_t  */
typedef const hwTRIDENTGMACEth_PerfectAdrConfig_t* const hwTRIDENTGMACEth_Kpk_PerAdrCfg_t;

typedef enum hwTRIDENTGMACEth_PCF
/*! Enum to configure the reception of Control Frames */
{
   hwTRIDENTGMACEth_PCF_DisableAll0=0,  /*!< Disable reception of Pause frames */  
   hwTRIDENTGMACEth_PCF_DisableAll1,      /*!< Disable reception of Pause frames */  
   hwTRIDENTGMACEth_PCF_PassAll,             /*!< Pass all Pause frames */  
   hwTRIDENTGMACEth_PCF_EnableFilter      /*!< Pass Pause frames after passing filter */  
} hwTRIDENTGMACEth_PCF_t, *phwTRIDENTGMACEth_PCF_t;

typedef struct hwTRIDENTGMACEth_FilterConfig
/*! General filter configuration structure */
{
    Bool receiveAllEnable;                            /*!<  Receive all frames with status updation */
    Bool srcAdrFilterEnable;                         /*!<  Enable/Disable source address filtering */
    Bool srcAdrInvFilterEnable;                   /*!<  Enable/Disable source address inverse filtering */
    Bool hashNPerfectFilterEn;                    /*!<  Enable/Disable both perfect and hash filtering */
    hwTRIDENTGMACEth_PCF_t pauseSetting;/*!<  Pause frame filter configuration */
    Bool filterBroadCastFrames;                  /*!<  Enable/Disable all broadcast frames*/
    Bool recvAllMulticast;                             /*!<  Enable/Disable reception of all Multicast frames */
    Bool destAdrInvFiltering;                       /*!<  Enable/Disable Destination Inverse filtering*/
    Bool hashMulticastEnable;                      /*!<  Enable/Disable Hash Multicast filtering */
    Bool hashUnicastEnable;                        /*!<  Enable/Disable Hash Unicast filtering */
    Bool passAllFrames;                               /*!<  Receive all frames without status updation */
}hwTRIDENTGMACEth_FilterConfig_t,*phwTRIDENTGMACEth_FilterConfig_t;

/*! \brief Constant pointer to constant structure of type hwTRIDENTGMACEth_FilterConfig_t  */
typedef const hwTRIDENTGMACEth_FilterConfig_t* const hwTRIDENTGMACEth_FilterConfig_Kpk_t;


typedef enum hwTRIDENTGMACEth_EnableDisable
/*! Enum to enable disable a particular option */
{
    hwTRIDENTGMACEth_Disable=0,  /*!< Disable */
    hwTRIDENTGMACEth_Enable=1    /*!< Enable */
}hwTRIDENTGMACEth_EnableDisable_t,*phwTRIDENTGMACEth_EnableDisable_t;

typedef enum hwTRIDENTGMACEth_Dir
/*! Enum to select the direction */
{
    hwTRIDENTGMACEth_Dir_Tx,      /*!< Transmit Direction */
    hwTRIDENTGMACEth_Dir_Rx,      /*!< Receive Direction */
    hwTRIDENTGMACEth_Dir_TxRx   /*!< Transmit & Receive Directions */
}hwTRIDENTGMACEth_Dir_t,*phwTRIDENTGMACEth_Dir_t;

typedef enum hwTRIDENTGMACEth_PLT
/*! Enum to select Pause Low Threshold value */
{
    hwTRIDENTGMACEth_PLT_4slotTime=0,  /*!<  PLT of 4 Slot times */
    hwTRIDENTGMACEth_PLT_28slotTime,    /*!<  PLT of 28 Slot times */
    hwTRIDENTGMACEth_PLT_144slotTime, /*!<  PLT of 144 Slot times */
    hwTRIDENTGMACEth_PLT_256slotTime  /*!<  PLT of 256 Slot times */

}hwTRIDENTGMACEth_PLT_t,*phwTRIDENTGMACEth_PLT_t;

typedef struct hwTRIDENTGMACEth_FlowCtrlConfig
/*! Structure for flow control configuration */
{
    UInt32                         pauseTime;                    /*!< Pause Time Value (16 Bit) */
    Bool                             zeroQuanta;                   /*!< Disable automatic generation of zero quanta pause */
    hwTRIDENTGMACEth_PLT_t pauseLowThreshold;/*!< Pause Low Threshold */
    Bool                             unicastPsDetect;            /*!< Enable/Disable Unicast Pause Frame */
    Bool                             rxFlowCtrlEn;                 /*!< Enable/Disable rx flow control */
    Bool                             txFlowCtrlEn;                 /*!< Enable/Disable tx flow control */

}hwTRIDENTGMACEth_FlowCtrlConfig_t,*phwTRIDENTGMACEth_FlowCtrlConfig_t;

/*! \brief Constant pointer to constant data of type hwTRIDENTGMACEth_FlowCtrlConfig_t  */
typedef const hwTRIDENTGMACEth_FlowCtrlConfig_t* const hwTRIDENTGMACEth_Kpk_FlowCtrlCfg_t;


typedef enum hwTRIDENTGMACEth_DmaPriority
/*! Enum to choose the Rx:Tx DMA priority ratio */
{
    hwTRIDENTGMACEth_DmaPriority0,/*!< 1:1 */
    hwTRIDENTGMACEth_DmaPriority1,/*!< 2:1 */
    hwTRIDENTGMACEth_DmaPriority2,/*!< 3:1 */
    hwTRIDENTGMACEth_DmaPriority3/*!< 4:1 */
}hwTRIDENTGMACEth_DmaPriority_t,*phwTRIDENTGMACEth_DmaPriority_t;

typedef enum hwTRIDENTGMACEth_PBL
/*! Enum to choose Programmable Burst Length */
{
    hwTRIDENTGMACEth_PBL_Val1 =1,     /*!< PBL of 1 */
    hwTRIDENTGMACEth_PBL_Val2 =2,     /*!< PBL of 2 */
    hwTRIDENTGMACEth_PBL_Val4 =4,     /*!< PBL of 4 */
    hwTRIDENTGMACEth_PBL_Val8 =8,     /*!< PBL of 8 */
    hwTRIDENTGMACEth_PBL_Val16 =16, /*!< PBL of 16 */       
    hwTRIDENTGMACEth_PBL_Val32 =32  /*!< PBL of 32 */          

}hwTRIDENTGMACEth_PBL_t,*phwTRIDENTGMACEth_PBL_t ; 

/* For configuring DMA in transmit direction */
typedef enum hwTRIDENTGMACEth_TTC
/*! Enum to configure the transmit threshold control */
{
    hwTRIDENTGMACEth_TTC_64bytes,    /*!< 64 bytes */
    hwTRIDENTGMACEth_TTC_128bytes,  /*!< 128 bytes */
    hwTRIDENTGMACEth_TTC_192bytes,  /*!< 192 bytes */   
    hwTRIDENTGMACEth_TTC_256bytes,  /*!< 256 bytes */
    hwTRIDENTGMACEth_TTC_40bytes,    /*!< 40 bytes */ 
    hwTRIDENTGMACEth_TTC_32bytes,    /*!< 32 bytes */
    hwTRIDENTGMACEth_TTC_24bytes,    /*!< 24 bytes */
    hwTRIDENTGMACEth_TTC_16bytes     /*!< 16 bytes */

}hwTRIDENTGMACEth_TTC_t,*phwTRIDENTGMACEth_TTC_t;

typedef enum hwTRIDENTGMACEth_RTC
/*! Enum to configure the receive threshold control */
{
    hwTRIDENTGMACEth_RTC_64bytes,    /*!< 64 bytes */
    hwTRIDENTGMACEth_RTC_32bytes,    /*!< 32 bytes */  
    hwTRIDENTGMACEth_RTC_96bytes,    /*!< 96 bytes */       
    hwTRIDENTGMACEth_RTC_128bytes   /*!< 128 bytes */

}hwTRIDENTGMACEth_RTC_t,*phwTRIDENTGMACEth_RTC_t;

typedef struct hwTRIDENTGMACEth_EnTxfr
/*! Structure  to Enable/Disable Tx/Rx transfers */
{
    hwTRIDENTGMACEth_EnableDisable_t  enFlag; /*!< Enable/Disable flag */
    hwTRIDENTGMACEth_Dir_t                    dirFlag; /*!<  Direction flag */
}hwTRIDENTGMACEth_EnTxfr_t,*phwTRIDENTGMACEth_EnTxfr_t;

/*! \brief Constant pointer to constant data refering hwTRIDENTGMACEth_EnTxfr_t */
typedef const hwTRIDENTGMACEth_EnTxfr_t * const  hwTRIDENTGMACEth_EnTxfr_Kpk_t;

typedef struct hwTRIDENTGMACEth_Int
/*! Structure  to store the interrupt register values */
{
    UInt32 dmaIntVal;      /*!<  Variable to store DMA interrupt mask/status values */
    UInt32 gmacIntVal;     /*!<  Variable to store GMAC interrupt mask/status values */
}hwTRIDENTGMACEth_Int_t,*phwTRIDENTGMACEth_Int_t;

/*! \brief Constant pointer to constant data refering hwTRIDENTGMACEth_Int_t */
typedef const hwTRIDENTGMACEth_Int_t * const  hwTRIDENTGMACEth_Int_Kpk_t;

typedef enum hwTRIDENTGMACEth_RFD
/*! Enum to select the Flow control deactivating threshold value */
{
    hwTRIDENTGMACEth_RFD_1K,  /*!< Deassert flow control after Full-1K */
    hwTRIDENTGMACEth_RFD_2K,  /*!< Deassert flow control after Full-2K */      
    hwTRIDENTGMACEth_RFD_3K,  /*!< Deassert flow control after Full-3K */          
    hwTRIDENTGMACEth_RFD_4K,   /*!< Deassert flow control after Full-4K */
    hwTRIDENTGMACEth_RFD_5K,   /*!< Deassert flow control after Full-5K */    
    hwTRIDENTGMACEth_RFD_6K,   /*!< Deassert flow control after Full-6K */        
    hwTRIDENTGMACEth_RFD_7K    /*!< Deassert flow control after Full-7K */            

}hwTRIDENTGMACEth_RFD_t,*phwTRIDENTGMACEth_RFD_t;

typedef enum hwTRIDENTGMACEth_RFA
/*! Enum to select the Flow control activating threshold value */
{
    hwTRIDENTGMACEth_RFA_1K,  /*!<  Activate flow control when fifo fill level is (Full-1K) */
    hwTRIDENTGMACEth_RFA_2K,  /*!<  Activate flow control when fifo fill level is (Full-2K) */      
    hwTRIDENTGMACEth_RFA_3K,  /*!<  Activate flow control when fifo fill level is (Full-3K) */          
    hwTRIDENTGMACEth_RFA_4K,  /*!<  Activate flow control when fifo fill level is (Full-4K) */
    hwTRIDENTGMACEth_RFA_5K,  /*!<  Activate flow control when fifo fill level is (Full-5K) */    
    hwTRIDENTGMACEth_RFA_6K,  /*!<  Activate flow control when fifo fill level is (Full-6K) */        
    hwTRIDENTGMACEth_RFA_7K   /*!<  Activate flow control when fifo fill level is (Full-7K) */            
}hwTRIDENTGMACEth_RFA_t,*phwTRIDENTGMACEth_RFA_t;

typedef struct hwTRIDENTGMACEth_DmaCfg
/*! Structure to select the DMA configuration */
{
    

    Bool mixedBurst; /*!< Enabled mixed burst mode */    
    Bool addrAlignedBtsEn;                             /*!< Address aligned beats enable */    
    Bool fixedBurstEn;                                     /*!< Enable/Disable Fixed Bursting */
    hwTRIDENTGMACEth_DmaPriority_t priority;/*!< Select the DMA priority */
    Bool pBL4xmode;                                      /*!< When set, effective PBL becomes (4* burstLen) */
    Bool differentPBL;                                     /*!<  Use separate PBL for tx & rx */
    hwTRIDENTGMACEth_PBL_t rxPBL;              /*!<  PBL for receive direction */    
    hwTRIDENTGMACEth_PBL_t burstLen;          /*!< Programmable Burst Length */
    Bool enableAltDescSize;                  /*!< Enable alternate descriptor size of 32 bytes */                       
    UInt32 descSkipLen;                                 /*!< Descriptor skip length */
    Bool dmaArbitration;                                 /*!< Round Robin or Priority of Rx > Tx */

    /* Transmit side configuration */
    Bool                                    storeNforwardEn;  /*!< Enable/Disable Store and Forward Mechanism */
    hwTRIDENTGMACEth_TTC_t   txThreshold;           /*!< Transmit Threshold control */
    hwTRIDENTGMACEth_RTC_t   rxThreshold;           /*!< Receive Threshold control */
    Bool                                    txSecondFrameEn;  /*!< Transmit second frame enable/disable  */
    UInt32                                txDescListBaseAdr; /*!< Transmit descriptor list base address */

     /* Receive side configuration */
    hwTRIDENTGMACEth_RFA_t actRxThreshold;	/*!< Receive Flow control activate value */
    hwTRIDENTGMACEth_RFD_t deactRxThreshold;/*!< Receive Flow control Deactivate value */
    Bool                                   hwFlowCtrlEn; /*!< Enable/Disable Hardware Flow control */
    Bool                                   errFramesEn; /*!< Enable/Disable reception of error frames */
    Bool                                   underSizedGdFramesEn; /*!< Enable/Disable reception of undersized good frames */
    UInt32                               rxDescListBaseAdr;   /*!< Receive descriptor list base address */ 
    Bool disableFrameFlush;                         /*!< Disable frame flushing, when descriptors are not available */
    Bool rxStoreNforwardEn; /*!< Enable/Disable receive store and Forward Mechanism */ 
    Bool recvTcpIpErrFrms; /*!< Enable/Disable reception of TCP/IP error frames */       

 }hwTRIDENTGMACEth_DmaCfg_t,*phwTRIDENTGMACEth_DmaCfg_t;

/*! \brief Constant pointer to constant data of type hwTRIDENTGMACEth_DmaCfg_t */
typedef const hwTRIDENTGMACEth_DmaCfg_t* const hwTRIDENTGMACEth_DmaCfg_Kpk_t;


/*! \} */ /* End of group0 */


#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_HASH )

/**
 * \defgroup group2 Hash Filter Related
 */
/*\{*/

typedef struct hwTRIDENTGMACEth_HashFilter
/*! Hash Filter configuration Structure */
{
    UInt32 hashFilterH;  /*!< Hash Filter High Register */
    UInt32 hashFilterL;  /*!<  Hash Filter Low Register */   
}hwTRIDENTGMACEth_HashFilter_t,*phwTRIDENTGMACEth_HashFilter_t;

/*! \brief Constant pointer to constant data of type hwTRIDENTGMACEth_HashFilter_t */
typedef const hwTRIDENTGMACEth_HashFilter_t* const hwTRIDENTGMACEth_HashFilter_Kpk_t;


/*\}*/ /* End of group2 */

#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC)

/*! \addtogroup group3
   *  \{
   */
typedef struct hwTRIDENTGMACEth_MmcCtrl
/*! MAC management counters configuration */
{
    Bool freezeCntrs;          /*!< Enable/Disable freezing of counters */
    Bool resetOnRdEn;        /*!< Enable/Disable reset on Read */
    Bool rollOverDisable;    /*!< Enable/Disable rollover */
    Bool resetCounters;       /*!< Reset counters */
}hwTRIDENTGMACEth_MmcCtrl_t,*phwTRIDENTGMACEth_MmcCtrl_t;

/*! \brief Contant pointer to constant data of type hwTRIDENTGMACEth_MmcCtrl_t */
typedef const hwTRIDENTGMACEth_MmcCtrl_t* const hwTRIDENTGMACEth_MmcCtrl_Kpk_t;

typedef struct hwTRIDENTGMACEth_MmcIntStat
/*! MAC management Interrupt status structure */
{
    UInt32                               rxCsumStatus; /*!< Rx checksum offload status */
    UInt32                               intStat;             /*!< Interrupt status */
    hwTRIDENTGMACEth_Dir_t    dir;                   /*!< Tx/Rx direction  */
}hwTRIDENTGMACEth_MmcIntStat_t, *phwTRIDENTGMACEth_MmcIntStat_t;

typedef struct hwTRIDENTGMACEth_MmcIntr
/*! Structure to enable and disable MAC interrupts */
{
    UInt32                          rxCsumIntVal; /*!< Rx checksum offload interrupts */
    UInt32                          intrVal;             /*!< Interrupts to Enable or disable */
    hwTRIDENTGMACEth_Dir_t dir;                 /*!< Tx/Rx direction  */
}hwTRIDENTGMACEth_MmcIntr_t, *phwTRIDENTGMACEth_MmcIntr_t;

/*! \brief Constant pointer to constant data of type hwTRIDENTGMACEth_MmcIntr_t */
typedef const hwTRIDENTGMACEth_MmcIntr_t* const hwTRIDENTGMACEth_MmcIntr_Kpk_t;

typedef struct hwTRIDENTGMACEth_MmcRegVal
/*! MAC management structure to read MAC counters */
{
    UInt32                        mmcRegVal; /*!< Register Value read */
    UInt32                        regToRd;       /*!< Register to be read */
}hwTRIDENTGMACEth_MmcRegVal_t, *phwTRIDENTGMACEth_MmcRegVal_t;

/*! \} */ /* End of group3 */
#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_POWER)

/* Power management related */
/*! \addtogroup group1
   *  \{
   */
typedef struct hwTRIDENTGMACEth_WkupCfg
/*! Structure to store the Wakeup on LAN configuration */
{
    Bool      rstRegptr;               /*!< Reset wakeup frame filter register pointer */
    Bool      globalUnicastEn;    /*!< Enable/Disable Wakeup on Unicast frame reception */
    Bool      wkupFrameEn;       /*!< Enable/Disable Wakeup Frame */
    Bool      magicPktEn;           /*!< Enable/Disable Wakeup on Magic Packet reception */
    UInt32  filterMask[4];         /*!< Mask to be applied for the incoming frame */
    UInt32  filterCommand;      /*!< Multicast/Unicast and Enable/disable filter options */
    UInt32  filterOffset;            /*!<  Offset to start sampling the bytes for calucating CRC */
    UInt32  filterCrcVal6;         /*!< CRC value to be used by filter 0 and filter 1 */
    UInt32  filterCrcVal7;         /*!< CRC value to be used by filter 2 and filter 3 */    
}hwTRIDENTGMACEth_WkupCfg_t,*phwTRIDENTGMACEth_WkupCfg_t;
/*! \} */ /* End of group1 */

/*! \brief Constant pointer to constant data of type hwTRIDENTGMACEth_WkupCfg_t */
typedef const hwTRIDENTGMACEth_WkupCfg_t* const hwTRIDENTGMACEth_WkupCfg_Kpk_t;


#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_IEEE1588_TIMESTAMP)

/*! \addtogroup group6
   *  \{
   */
typedef struct hwTRIDENTGMACEth_TsReg
/*! Current Host register values */
{
    UInt32 highVal;/*!< Higher 32 bit value of the register */
    UInt32 lowVal; /*!< Lower 32 bit value of the register */
}hwTRIDENTGMACEth_TsReg_t,*phwTRIDENTGMACEth_TsReg_t;

typedef enum hwTRIDENTGMACEth_Ts_Psnt
/*! Enum to indicate if the value in the update registers is to be added or 
**   subtracted from the system time.
*/
{
    hwTRIDENTGMACEth_AddToSysTime,
    hwTRIDENTGMACEth_SubFromSysTime        
}hwTRIDENTGMACEth_Ts_Psnt_t, *phwTRIDENTGMACEth_Ts_Psnt_t;

typedef struct hwTRIDENTGMACEth_TsUpdateReg
/*! Value in the Time Stamp Update registers is added to the system time
** or subtracted depending on addSub flag
*/
{
    hwTRIDENTGMACEth_TsReg_t updateReg;
    hwTRIDENTGMACEth_Ts_Psnt_t addSub;    

}hwTRIDENTGMACEth_TsUpdateReg_t, *phwTRIDENTGMACEth_TsUpdateReg_t;

/*! \} */ /* End of group6 */
#endif

/*-----------------------------------------------------------------------------*/
/* Exported functions:*/
/*-----------------------------------------------------------------------------*/

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)

/*! \addtogroup group5
   *  Additional support functions
   *  \{
   */

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetSWVersion (ptmSWVersion_t  pEthGmacVersion);
* This function gets the sofware version of the driver 
*  \param[out] pEthGmacVersion: Pointer to structure tmSWVersion_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetSWVersion (
    ptmSWVersion_t  pEthGmacVersion    
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetHWVersion(tmUnitSelect_t ethUnitId, pUInt32 pHWVersion);
* This function returns the hardware version number of the ethernet unit
*  \param[in] ethUnitId: Ethernet Unit ID
*  \param[out] pHWVersion: Pointer to variable, where the version number is to be stored
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetHWVersion(
    tmUnitSelect_t                    ethUnitId,  
    pUInt32                             pHWVersion
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_Deinit(tmUnitSelect_t ethUnitId);
* This function disables the GMAC hardware
*  \param[in] ethUnitId: Ethernet Unit ID
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_Deinit(
    tmUnitSelect_t  ethUnitId   
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetConfig(tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_DevConfig_t pEthConfig);
* This function gets the current configuration of the GMAC device
*  \param[in] ethUnitId: Ethernet Unit ID
*  \param[out] pEthConfig : Fills the information in the pEthConfig pointer passed as parameter
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetConfig(
    tmUnitSelect_t                      ethUnitId ,    
    phwTRIDENTGMACEth_DevConfig_t    pEthConfig
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaFlushTxFifo (tmUnitSelect_t ethUnitId);
* This function is used to flush the contents of a Transmit FIFO
* \param[in] ethUnitId: GMAC unit number 
* \return TM_OK - successful
*/

tmErrorCode_t  
hwTRIDENTGMACEth_DmaFlushTxFifo(
    tmUnitSelect_t  ethUnitId 
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaGetCurrentHostRegs(tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_DmaHostRegs_t pDmaHostReg);
* This function is used to retrieve the DMA host registers for debug purposes
* \param[in] ethUnitId: GMAC unit number 
* \param[out] pDmaHostReg: Pointer to structure hwTRIDENTGMACEth_DmaHostRegs_t for storing the read values 
* \return TM_OK - successful
*/
tmErrorCode_t 
hwTRIDENTGMACEth_DmaGetCurrentHostRegs(
    tmUnitSelect_t                                 ethUnitId,
    phwTRIDENTGMACEth_DmaHostRegs_t     pDmaHostReg       
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaGetMissedFrameCount (tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_DmaMissedFrmCnt_t pMissedFrmCnt);
* This function is used to clear the Missed Frames counter
* \param[in] ethUnitId: GMAC unit number 
* \param[out] pMissedFrmCnt: Pointer to structure phwTRIDENTGMACEth_DmaMissedFrmCnt_t
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_DmaGetMissedFrameCount(
    tmUnitSelect_t                                 ethUnitId,
    phwTRIDENTGMACEth_DmaMissedFrmCnt_t pMissedFrmCnt
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaMissedFrameCounterClear (tmUnitSelect_t ethUnitId);
* This function is used to clear the Missed Frames counter
* \param[in] ethUnitId: GMAC unit number 
* \return TM_OK - successful
*/
tmErrorCode_t 
hwTRIDENTGMACEth_DmaMissedFrameCounterClear(
    tmUnitSelect_t                                 ethUnitId
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_FilterGetConfig (tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_FilterConfig_t pFilterConfig);
* This function is used to get the receive filter configuration
* \param[in] ethUnitId: GMAC unit number 
* \param[out] pFilterConfig: Pointer to hwTRIDENTGMACEth_FilterConfig_t structure for storing the read values
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_FilterGetConfig(
    tmUnitSelect_t                      ethUnitId , 
    phwTRIDENTGMACEth_FilterConfig_t        pFilterConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_PerfectAdrGetConfig (tmUnitSelect_t ethUnitId,UInt32 regNum,phwTRIDENTGMACEth_PerfectAdrConfig_t pAdrConfig);
* This function is used to get the perfect address filter configuration
* \param[in] ethUnitId: GMAC unit number
* \param[in] regNum: Perfect address filter register number
* \param[out] pAdrConfig: Pointer to structure hwTRIDENTGMACEth_PerfectAdrConfig_t
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_PerfectAdrGetConfig(
    tmUnitSelect_t                      ethUnitId , 
    UInt32                                 regNum,    
    phwTRIDENTGMACEth_PerfectAdrConfig_t pAdrConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_FlowCtrlGetConfig (tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_FlowCtrlConfig_t pFlowCtrlCfg);
* This function is used to do flow control configuration
* \param[in] ethUnitId: GMAC unit number 
* \param[in] pFlowCtrlCfg: Pointer to hwTRIDENTGMACEth_FlowCtrlConfig_t structure
* \return TM_OK - successful
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlGetConfig(
    tmUnitSelect_t                                    ethUnitId ,
    phwTRIDENTGMACEth_FlowCtrlConfig_t        pFlowCtrlCfg
);
/*! \} */ /* Group5 */

#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_POWER)
/*! \addtogroup group1
   *  Power Management Functions
   *  \{
   */

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetPowerState (tmUnitSelect_t ethUnitId,ptmPowerState_t pEthPowerState);
* This function gets the current power state of the GMAC device
*  \param[in] ethUnitId: Ethernet unit ID
*  \param[out] pEthPowerState: Pointer to tmPowerState_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetPowerState(
    tmUnitSelect_t    ethUnitId ,
    ptmPowerState_t pEthPowerState
    );
/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetPowerState(tmUnitSelect_t  ethUnitId, tmPowerState_t ethPowerState);
* This function sets/resets power down bit of the GMAC. Function hwTRIDENTGMACEth_WoLConfig() is to be 
* called before putting the device into power down mode.
* \param[in] ethUnitId: GMAC Unit number.
* \param[in] ethPowerState: Power On or Power Off
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetPowerState(
    tmUnitSelect_t   ethUnitId  , 
    tmPowerState_t  ethPowerState
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_WoLConfig(tmUnitSelect_t  ethUnitId, hwTRIDENTGMACEth_WkupCfg_Kpk_t pWolConfig);
* This function sets the wakeup method for the GMAC when putting the device into power down mode. 
* \param[in] ethUnitId: Instance number of the device
* \param[in] pWolConfig: Pointer to structure hwTRIDENTGMACEth_WkupCfg_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_WoLConfig(
    tmUnitSelect_t   ethUnitId  , 
    hwTRIDENTGMACEth_WkupCfg_Kpk_t  pWolConfig
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetWakeupStatus(tmUnitSelect_t  ethUnitId, pUInt32 pRegVal);
* This function is used to get the cause of a wakeup event 
* \param[in] ethUnitId: Instance number of the device
* \param[out] pRegVal: Pointer to variable to store the status
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_GetWakeupStatus(
    tmUnitSelect_t       ethUnitId ,    
    pUInt32                  pRegVal
    );

/*! \} */ /* group1 */
#endif 

/*! \addtogroup group0
   *  Basic API set
   *  \{
   */

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_Init (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_DevConfig_Kpk_t pPhyinfo);
* This function initializes the GMAC with default values
* \param[in] ethUnitId: GMAC unit number
* \param[in] pPhyinfo: PHY address & Clk Csr value
* \param[in] miiSelect: MII interface then True
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_Init(
    tmUnitSelect_t  ethUnitId,
    hwTRIDENTGMACEth_PhyInfo_Kpk_t pPhyinfo,
    Bool miiSelect
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetConfig (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_DevConfig_Kpk_t pEthConfig);
* This function configures the GMAC device with the values passed.
* \param[in] ethUnitId: GMAC unit number
* \param[in] pEthConfig: Pointer to structure hwTRIDENTGMACEth_DevConfig_t
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetConfig(
    tmUnitSelect_t                  ethUnitId ,    
    hwTRIDENTGMACEth_DevConfig_Kpk_t pEthConfig
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_IntGetStatus (tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_Int_t pIntStat);
* This function gets DMA interrupt status
* \param[in] ethUnitId: GMAC unit number
* \param[out] pIntStat: Pointer to structure hwTRIDENTGMACEth_Int_t 
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_IntGetStatus (
    tmUnitSelect_t                               ethUnitId ,    
    phwTRIDENTGMACEth_Int_t                  pIntStat
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_IntEnable (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_Int_Kpk_t pIntEn);
* This function enables the device interrupts
* \param[in] ethUnitId: GMAC unit number
* \param[in] pIntEn: Interrupts to be enabled
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_IntEnable (
    tmUnitSelect_t              ethUnitId ,    
    hwTRIDENTGMACEth_Int_Kpk_t pIntEn
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_IntDisable (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_Int_Kpk_t pIntDis);
* This function disables the device interrupts
* \param[in] ethUnitId: GMAC unit number
* \param[in] pIntDis: Interrupts to be disabled
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_IntDisable (
    tmUnitSelect_t                       ethUnitId ,    
    hwTRIDENTGMACEth_Int_Kpk_t     pIntDis
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_IntClear (tmUnitSelect_t ethUnitId,UInt32 ethIntstatus);
* This function clears the device interrupts by writing 1s to the status register
* \param[in] ethUnitId: GMAC unit number
* \param[in] ethIntstatus: Interrupts to be cleared
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_IntClear (
    tmUnitSelect_t              ethUnitId ,    
    UInt32                         ethIntstatus
    );

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_PerfectAdrSetConfig (tmUnitSelect_t ethUnitId,UInt32 regNum,hwTRIDENTGMACEth_Kpk_PerAdrCfg_t pAdrConfig);
* This function is used to configure the perfect address filters of GMAC
* \param[in] ethUnitId: GMAC unit number
* \param[in] regNum: Perfect address filter register number
* \param[in] pAdrConfig: Pointer to structure hwTRIDENTGMACEth_PerfectAdrConfig_t
* \return TM_OK - successful
*/
tmErrorCode_t 
hwTRIDENTGMACEth_PerfectAdrSetConfig(
    tmUnitSelect_t                      ethUnitId , 
    UInt32                                 regNum,
    hwTRIDENTGMACEth_Kpk_PerAdrCfg_t pAdrConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_FilterSetConfig (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_FilterConfig_Kpk_t pFilterConfig);
* This function is used to set the filter configuration
* \param[in] ethUnitId: GMAC unit number 
* \param[in] pFilterConfig: Pointer to hwTRIDENTGMACEth_FilterConfig_t structure
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_FilterSetConfig(
    tmUnitSelect_t                          ethUnitId , 
    hwTRIDENTGMACEth_FilterConfig_Kpk_t  pFilterConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GmacEnableDisable (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_EnTxfr_Kpk_t pEndir);
* This function is used to enable or disable GMAC in transmit or receive or both the directions
* \param[in] ethUnitId: GMAC unit number 
* \param[in] pEndir: Pointer to hwTRIDENTGMACEth_EnTxfr_t structure
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_GmacEnableDisable (
    tmUnitSelect_t                              ethUnitId ,
    hwTRIDENTGMACEth_EnTxfr_Kpk_t      pEndir    
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_LpbkEnableDisable (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_EnableDisable_t enableDisable);
* This function is used to enable loopback at the MAC level
* \param[in] ethUnitId: GMAC unit number 
* \param[in] enableDisable: Enum to enable or disable
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_LpbkEnableDisable (
    tmUnitSelect_t                              ethUnitId ,
    hwTRIDENTGMACEth_EnableDisable_t     enableDisable
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_FlowCtrlSetConfig (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_Kpk_FlowCtrlCfg_t pFlowCtrlCfg);
* This function is used to do flow control configuration
* \param[in] ethUnitId: GMAC unit number 
* \param[in] pFlowCtrlCfg: Pointer to hwTRIDENTGMACEth_FlowCtrlConfig_t structure
* \return TM_OK - successful
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlSetConfig(
    tmUnitSelect_t                                    ethUnitId ,
    hwTRIDENTGMACEth_Kpk_FlowCtrlCfg_t        pFlowCtrlCfg
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_FlowCtrlEnableDisable (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_EnableDisable_t enableDisable);
* This function is used to enable the generation of pause frame in full-duplex mode, and in half duplex
* to activate/deactivate backpressure operation.
* \param[in] ethUnitId: GMAC unit number 
* \param[in] enableDisable: Enum to enable or disable
* \return TM_OK - successful
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlEnableDisable(
    tmUnitSelect_t                          ethUnitId ,
    hwTRIDENTGMACEth_EnableDisable_t enableDisable
) ;

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_FlowCtrlStatus (tmUnitSelect_t ethUnitId,pUInt32 pRegVal);
* This function is used to check if the FCA/BP bit is cleared after a pause frame transmission
* \param[in] ethUnitId : Ethernet unit number 
* \param[out] pRegVal: Pointer to variable to store the status 
* \return TM_OK - successful
*/

tmErrorCode_t  
hwTRIDENTGMACEth_FlowCtrlStatus(
    tmUnitSelect_t                          ethUnitId,
    pUInt32                                    pRegVal
); 

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaConfig (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_DmaCfg_Kpk_t pDmaConfig);
* This function is used to do the DMA configuration.
* \param[in] ethUnitId : Ethernet unit number
* \param[in] pDmaConfig: Pointer to hwTRIDENTGMACEth_DmaCfg_t structure
* \return TM_OK - successful
*/
tmErrorCode_t  
hwTRIDENTGMACEth_DmaConfig(
    tmUnitSelect_t                               ethUnitId ,
    hwTRIDENTGMACEth_DmaCfg_Kpk_t        pDmaConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaEnableDisable (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_EnTxfr_Kpk_t pTxfr);
* This function is used to Enable/Disable DMA in a particular direction
* \param[in] ethUnitId: GMAC unit number 
* \param[in] pTxfr: Pointer to structure hwTRIDENTGMACEth_EnTxfr_t
* \return TM_OK - successful
*/
tmErrorCode_t  
hwTRIDENTGMACEth_DmaEnableDisable(
    tmUnitSelect_t  ethUnitId, 
    hwTRIDENTGMACEth_EnTxfr_Kpk_t pTxfr
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_DmaPollDesc(tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_Dir_t dir);
* This function is used to restart the DMA transmission/reception if it is in suspended state 
* \param[in] ethUnitId: GMAC unit number 
* \param[in] dir: Enum, which indicates the direction
* \return TM_OK - successful
*/

tmErrorCode_t  
hwTRIDENTGMACEth_DmaPollDesc(
    tmUnitSelect_t          ethUnitId,
    hwTRIDENTGMACEth_Dir_t       dir
);

tmErrorCode_t hwTRIDENTGMACEth_get_phyInfo(tmUnitSelect_t  ethUnitId, UInt32* phyID, UInt32* phyAddr);

/*! \} */ /*End of  group0*/

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_HASH)
/*! \addtogroup group2
   *  Hash Filter APIs
   *  \{
   */

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_HashFilterSetConfig (tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_HashFilter_Kpk_t pHashConfig);
* This function is used to set the hash filter configuration
* \param[in] ethUnitId: GMAC unit number 
* \param[in] pHashConfig: Pointer to hwTRIDENTGMACEth_HashFilter_t structure
* \return TM_OK - successful
*/
tmErrorCode_t 
hwTRIDENTGMACEth_HashFilterSetConfig(
    tmUnitSelect_t                          ethUnitId ,
    hwTRIDENTGMACEth_HashFilter_Kpk_t    pHashConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_HashFilterGetConfig (tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_HashFilter_t pHashConfig);
* This function is used to get the hash filter configuration
* \param[in] ethUnitId: GMAC unit number 
* \param[out] pHashConfig: Pointer to hwTRIDENTGMACEth_HashFilter_t structure
* \return TM_OK - successful
*/

tmErrorCode_t 
hwTRIDENTGMACEth_HashFilterGetConfig(
    tmUnitSelect_t                                ethUnitId ,
    phwTRIDENTGMACEth_HashFilter_t          pHashConfig
);

/*! \} */ /*End of  group2 */

#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC )

/*! \addtogroup group3
   *  APIs for MAC Management Counters 
   *  \{
   */

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_MMCConfig(tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_MmcCtrl_Kpk_t pCtrlConfig);
* This function is used to configure the behaviour of MAC Management counters and also to reset counters
* \param[in] ethUnitId: GMAC unit number
* \param[in] pCtrlConfig: Pointer to structure hwTRIDENTGMACEth_MmcCtrl_t
* \return TM_OK - successful
*/
tmErrorCode_t 
hwTRIDENTGMACEth_MMCConfig(
    tmUnitSelect_t                      ethUnitId,
    hwTRIDENTGMACEth_MmcCtrl_Kpk_t           pCtrlConfig
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_MMCIntStatus(tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_MmcIntStat_t pMmcStat);
* This function is used to read the interrupt status register for transmit or receive direction
* \param[in] ethUnitId: GMAC unit number
* \param[in] pMmcStat: Pointer to structure hwTRIDENTGMACEth_MmcIntStat_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_MMCIntStatus(
    tmUnitSelect_t                            ethUnitId ,    
    phwTRIDENTGMACEth_MmcIntStat_t   pMmcStat
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_MMCIntEnable(tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_MmcIntr_Kpk_t pMmcIntEn);
* This function is used to enable the MMC interrupts in transmit or receive direction
* \param[in] ethUnitId: GMAC unit number
* \param[in] pMmcIntEn: Pointer to structure hwTRIDENTGMACEth_MmcIntr_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_MMCIntEnable(
    tmUnitSelect_t                            ethUnitId ,    
    hwTRIDENTGMACEth_MmcIntr_Kpk_t   pMmcIntEn
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_MMCIntDisable(tmUnitSelect_t ethUnitId,hwTRIDENTGMACEth_MmcIntr_Kpk_t pMmcIntDis);
* This function is used to disable the MMC interrupts in transmit or receive direction
* \param[in] ethUnitId: GMAC unit number
* \param[in] pMmcIntDis: Pointer to structure hwTRIDENTGMACEth_MmcIntr_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_MMCIntDisable(
    tmUnitSelect_t                            ethUnitId ,    
    hwTRIDENTGMACEth_MmcIntr_Kpk_t   pMmcIntDis
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_MMCCountersRead(tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_MmcRegVal_t pMmcReg);
* This function is used to read the Mac Management Counters in transmit or receive direction
* \param[in] ethUnitId: GMAC unit number
* \param[in] pMmcReg: Pointer to structure hwTRIDENTGMACEth_MmcRegVal_t
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_MMCCountersRead(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_MmcRegVal_t pMmcReg
);

/*! \} */ /* End of group 3*/
#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_VLAN)

/**
 * \defgroup group4 VLAN Tag APIs
 */
/*\{*/

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetVLANTag(tmUnitSelect_t ethUnitId,UInt32 regValue);
* This function is used to set the VLAN tag value.
* \param[in] ethUnitId: GMAC unit number
* \param[in] regValue: VLAN tag value
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_SetVLANTag (
    tmUnitSelect_t    ethUnitId ,    
    UInt32            regValue
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetVLANTag(tmUnitSelect_t ethUnitId,pUInt32 pRegValue);
* This function is used to get the VLAN tag value.
* \param[in] ethUnitId: GMAC unit number
* \param[out] pRegValue: Pointer to store the VLAN tag value read 
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetVLANTag (
    tmUnitSelect_t               ethUnitId ,    
    pUInt32                         pRegValue
);

/*\}*/ /* end of group4 */

#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_IEEE1588_TIMESTAMP)

/**
 * \addtogroup group6
 */
/*\{*/

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetTsCtrlVal(tmUnitSelect_t ethUnitId,pUInt32 pRegValue);
* This function returns the control value of the time stamp registers.
* Below macros can be used to check if a corresponding bit is set/reset in 
* the register: 
* HW_TRIDENTGMACETH_TS_CTRL_ADDEND_UPDT_VAL
* HW_TRIDENTGMACETH_TS_CTRL_INT_TRIG_VAL
* HW_TRIDENTGMACETH_TS_CTRL_TS_UPDATE_VAL
* HW_TRIDENTGMACETH_TS_CTRL_TS_INIT_VAL
* HW_TRIDENTGMACETH_TS_CTRL_FINE_UPDATE_VAL
* HW_TRIDENTGMACETH_TS_CTRL_TIMESTAMP_EN_VAL
* \param[in] ethUnitId: GMAC unit number
* \param[out] pRegValue: Pointer to store the control value read 
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetTsCtrlVal(
    tmUnitSelect_t               ethUnitId ,    
    pUInt32                         pRegValue
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetTsCtrl(tmUnitSelect_t ethUnitId,UInt32 regValue);
* This function enables/disables certain features of time stamp operation,depending on the value 
* passed. 
* Parameter to this function can be any of the below macros:
* HW_TRIDENTGMACETH_TS_CTRL_ADDEND_UPDT_VAL
* HW_TRIDENTGMACETH_TS_CTRL_INT_TRIG_VAL
* HW_TRIDENTGMACETH_TS_CTRL_TS_UPDATE_VAL
* HW_TRIDENTGMACETH_TS_CTRL_TS_INIT_VAL
* HW_TRIDENTGMACETH_TS_CTRL_FINE_UPDATE_VAL
* HW_TRIDENTGMACETH_TS_CTRL_TIMESTAMP_EN_VAL
* \param[in] ethUnitId: GMAC unit number
* \param[in] regValue: Time stamp control value to set 
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_SetTsCtrl(
    tmUnitSelect_t               ethUnitId ,    
    UInt32                           regValue
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetTsSubSecInc(tmUnitSelect_t ethUnitId,UInt32 regValue);
* API to set sub-second increment value
* \param[in] ethUnitId: GMAC unit number
* \param[in] regValue: Sub-Second increment value
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_SetTsSubSecInc(
    tmUnitSelect_t               ethUnitId ,    
    UInt32                           regValue
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetTsAddend(tmUnitSelect_t ethUnitId,UInt32 regValue);
* This API is used to set Time Stamp Addend value. This is used only when system time is configured 
* for Fine Update mode.
* \param[in] ethUnitId: GMAC unit number
* \param[in] regValue:  Addend value
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsAddend(
    tmUnitSelect_t               ethUnitId ,    
    UInt32                           regValue
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_GetSysTs(tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_TsReg_t pRegs);
* This API fetches the current system time stamp value. 
* \param[in] ethUnitId: GMAC unit number
* \param[out] pRegs: Pointer to structure to store the values read
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_GetSysTs(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_TsReg_t pRegs
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetTsUpdate(tmUnitSelect_t ethUnitId, phwTRIDENTGMACEth_TsUpdateReg_t pUpdate);
* This API is used to set the values of Time Stamp update registers & the parameter passed to this 
* function also indicates if the value is to be added to the system time or subracted from the system 
* time.
* \param[in] ethUnitId: GMAC unit number
* \param[in] pUpdate: Pointer to structure that stores the time stamp values & flag that indicates 
*   whether to add or subtract this value from system time.
* \return TM_OK - successful
*/
tmErrorCode_t
hwTRIDENTGMACEth_SetTsUpdate(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_TsUpdateReg_t pUpdate   
);

/*!
* \fn tmErrorCode_t hwTRIDENTGMACEth_SetTsTgtTime(tmUnitSelect_t ethUnitId,phwTRIDENTGMACEth_TsReg_t pRegs);
* This API is used to set the target time stamp register values.
* \param[in] ethUnitId: GMAC unit number
* \param[in] pRegs: Pointer to structure which contains  target high & targe low register values
* \return TM_OK - successful
*/

tmErrorCode_t
hwTRIDENTGMACEth_SetTsTgtTime(
    tmUnitSelect_t               ethUnitId ,    
    phwTRIDENTGMACEth_TsReg_t pRegs
);

/*\}*/ /* end of group6 */
#endif

/* End of function prototypes */

#endif  /* HWTRIDENTGMACETH_H */



