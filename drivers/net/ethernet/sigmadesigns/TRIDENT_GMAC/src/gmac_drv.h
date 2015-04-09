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
 * %filename:           gmac_drv.h  %
 * %pid_version:       1.2              %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  Header file for Linux Driver for LIPP_6100ETH ethernet subsystem
 *
 * DOCUMENT REF:
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/

#ifndef _GMAC_DRV_H_
#define _GMAC_DRV_H_

#include <linux/interrupt.h>

#ifndef __ASSEMBLY__
#include <linux/types.h>
#endif

/* Macros to define speed of the link */
#define LX_SPEED_1000  (1000)
#define LX_SPEED_100 (100)
#define LX_SPEED_10 (10)

#define LX_MODE_FULL_DUPLEX (1)
#define LX_MODE_HALF_DUPLEX (0)

/* Autonegotiation enable/disable macros */
#define LX_AUTONEG_ENABLE (1)
#define LX_AUTONEG_DISABLE (0)

#define LX_FLOW_CONTROL_ENABLED (1)
#define LX_FLOW_CONTROL_DISABLED (0)

#define LX_RX_FLOW_CONTROL (0x02)
#define LX_TX_FLOW_CONTROL (0x04)

#define LX_FILTER_TYPE_SW (0)
#define LX_FILTER_TYPE_HW (1)

#define LX_PROMISC_ENABLED (1)
#define LX_PROMISC_DISABLED (0)

#define LX_4SLOT_TIMES (0)
#define LX_28SLOT_TIMES (1)
#define LX_144SLOT_TIMES (2)
#define LX_256SLOT_TIMES (3)

/* Fill level of receive FIFO for activating/deactivating
** hardware flow control. Used when the macro ENABLE_HW_FLOW_CONTROL
** is set to 1.
*/
#define FIFO_FILL_LEVEL_1K (0)
#define FIFO_FILL_LEVEL_2K (1)
#define FIFO_FILL_LEVEL_3K (2)
#define FIFO_FILL_LEVEL_4K (3)
#define FIFO_FILL_LEVEL_5K (4)
#define FIFO_FILL_LEVEL_6K (5)
#define FIFO_FILL_LEVEL_7K (6)

/* Programmable burst length values */
#define BURST_LENGTH_1 (1)
#define BURST_LENGTH_2 (2)
#define BURST_LENGTH_4 (4)
#define BURST_LENGTH_8 (8)
#define BURST_LENGTH_16 (16)
#define BURST_LENGTH_32 (32)

/* DMA transmit threshold values */
#define TX_DMA_THRESHOLD_VAL_64 (0)
#define TX_DMA_THRESHOLD_VAL_128 (1)
#define TX_DMA_THRESHOLD_VAL_192 (2)
#define TX_DMA_THRESHOLD_VAL_256 (3)
#define TX_DMA_THRESHOLD_VAL_40 (4)
#define TX_DMA_THRESHOLD_VAL_32 (5)
#define TX_DMA_THRESHOLD_VAL_24 (6)
#define TX_DMA_THRESHOLD_VAL_16 (7)

/* DMA receive threshold values */
#define RX_DMA_THRESHOLD_VAL_64 (0)
#define RX_DMA_THRESHOLD_VAL_32 (1)
#define RX_DMA_THRESHOLD_VAL_96 (2)
#define RX_DMA_THRESHOLD_VAL_128 (3)

/* DMA priorities in Rx:Tx ratio */
#define DMA_RX1_TX1 (0)
#define DMA_RX2_TX1 (1)
#define DMA_RX3_TX1 (2)
#define DMA_RX4_TX1 (3)

/*********************** configuration options **********************/

/* Rx Frame size should be multiple of 4/8/16 depending on bus width.
 * The frame size should be at least (MTU+14) bytes after meeting the above
 * condition
 */
#define MAX_ETH_FRAME_SIZE (1536)

/* Descriptor alignment required. 4/8/16 for 32/64/128 bit buses respectively */

/* Set the alignment value to (alignment - 1)
*   Ex: For 16, set to 15,
*          For   8, set to 7,
*          For 4 set to 3
*/
#define DMA_DESC_ALIGNMENT (31) /* 32-1 */

#ifdef CONFIG_GMAC_MODE_RGMII
#define GMAC0_MII_SEL False
#else
#define GMAC0_MII_SEL True
#endif

#ifdef CONFIG_GMAC_MODE_RGMII
#define GMAC1_MII_SEL False
#else
#define GMAC1_MII_SEL True
#endif

/* Speed of the ethernet link */
#define ETH_LINK_SPEED (LX_SPEED_100)

/* Mode of the ethernet link */
#define ETH_LINK_MODE (LX_MODE_FULL_DUPLEX)

/* Macro to enable/disable autonegotiation */
#define ETH_AUTO_NEGOTIATION (LX_AUTONEG_DISABLE)

/* HW or SW filtering */
#define ETH_FILTER_TYPE (LX_FILTER_TYPE_HW)

#define MULTICAST_FILTER_DISABLE (1)
#define MULTICAST_FILTER_ENABLE (0)

#define MULTICAST_FILTER (MULTICAST_FILTER_DISABLE)

/* Software based flow control -Enable/Disable */
#define ETH_FLOW_CTRL (LX_FLOW_CONTROL_ENABLED)

/* Macro to enable hardware flow control if the receive FIFO size
 * is greater than 4K. When set to 1 it is enabled.
 */
#define ENABLE_HW_FLOW_CONTROL (1)

/* Activate flow control when the empty space falls below this value */
#define RFA_THRESHOLD (hwTRIDENTGMACEth_RFA_1K)

/* Deactivate flow control when FIFO fill level is less than this value */
#define RFD_THRESHOLD (hwTRIDENTGMACEth_RFA_2K)

/* Disable frame flushing on rx side */
#define ETH_DISABLE_FRAME_FLUSH (0)

/* Enable Operate on second frame. 1-Enable, 0-Disable */
#define ETH_ENABLE_OSF (1)

/* Store & forward mode for transmit side */
#define ETH_STORE_FWD_ENABLE (1)

#define ETH_TX_THRESHOLD (TX_DMA_THRESHOLD_VAL_256)

#define ETH_RX_THRESHOLD (RX_DMA_THRESHOLD_VAL_128)

/* 4*X PBL mode enable/disable */
#define ETH_4XPBL_ENABLE (0)

/* Separate PBL for Tx & Rx */
#define ETH_DIFF_PBL_ENABLE (0)

#define ETH_FIXED_BURST_ENABLE (0)

/* Programmable burst length value */
#define ETH_TX_PBL_VAL (BURST_LENGTH_32)

#define ETH_RX_PBL_VAL (BURST_LENGTH_32)

/* Enable/Disable DMA arbitration scheme */
#define ETH_DMA_ARBITRATION (0)

/* When DA bit is reset, the values are valid */
#define ETH_DMA_PRIORITY (DMA_RX1_TX1)

/* Flow control direction Tx/Rx or Both */
#define ETH_FLOW_CTRL_DIR (LX_RX_FLOW_CONTROL | LX_TX_FLOW_CONTROL)

/* Promiscuous mode enable/disable */
#define ETH_PROMISC_MODE (LX_PROMISC_DISABLED)

/* Time sent in a pause frame in slot times. Max value is 0xFFFF slot time
** Each slot time is 512 bit times in GMII/MII mode
*/
#define LX_PAUSE_TIMER_VALUE (256)

/* Pause low threshold value  */
#define ETH_PLT_VALUE (LX_4SLOT_TIMES)

/* To find if there are enough descriptors are available to receive frames */
#define LX_DESC_GAP (10)

/* Ethtool API selection */
#define ENABLE_ETH_TOOL

#define RX_TX_BUF_LEN   (1 << 10)
/* Number of descriptors for transmit side */
#define HW_DESCR_QUEUE_LEN_TX  RX_TX_BUF_LEN  // (80)

/* Number of descriptors for receive side */
#define HW_DESCR_QUEUE_LEN_RX RX_TX_BUF_LEN   // (80)

/* At a time, the txmt isr processes these many tx desc */
#define MAX_TX_PKTS_TO_PROCESS  RX_TX_BUF_LEN // (80)

/* At a time, the receive isr processes these many rx desc */
#define MAX_RX_PKTS_TO_PROCESS  RX_TX_BUF_LEN // (80)

#define ETH_ENABLE_JUMBO_FRAME (0)

/* Maximum jumbo frame size in bytes */
#define TRIDENT_GMAC_JUMBO_MTU (2000)

#define NAPI_DEV_WEIGHT (64)

/* For periodic data, define this macro */
#undef  CONTROL_INTR_FREQ

/*To support enhanced DMA descriptor of the IP */
#define __ENHANCED_DESCRIPTOR__

/* VLAN TAGGING 8021Q */
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define TRIDENT_GMAC_VLAN_TAG
#endif

/* Frequency at which the transmit side is interrupted */
#define ETH_TX_INTR_FREQ (1)

/* Frequency at which the receive side is interrupted */
#define ETH_RX_INTR_FREQ (1)

/* Autonegotiation retry count */
#define AUTO_NEG_RETRY_COUNT (3)

/* Periodic status check for ethernet link */
#define TIMEOUT_VALUE   (2*HZ)

/* Watch dog timeout for transmission  */
#define TX_TIMEOUT (5*HZ)

/* Number of perfect address filters available on the hardware
** Perfect Filter register 0 is used for MAC address
** Remaining Perfect Filter registers (1-31) are used for programming multicast addresses
*/
#define ETH_NUM_OF_PER_ADR_FILTERS (32)

/* Debug and error messages */
#define ENABLE_PRINT (1)

/* Enable printing of error messages */
#define ENABLE_PRINT_ERR (1)

/* Enable printing of Debug messages  */
#define ENABLE_PRINT_DBG (0)

/* Messages from interrupt context  */
#define ENABLE_PRINT_INT (0)

/* For printing link up/down messages */
#define ENABLE_LINK_STATUS_PRINT (0)

#if (ENABLE_PRINT == 1)

    #if ( ENABLE_PRINT_DBG == 1)
    #define GMAC_PRINT_DBG(args...) printk(KERN_INFO"GMAC:" args)
    #else
    #define GMAC_PRINT_DBG(args...)
    #endif

    #if ( ENABLE_PRINT_ERR == 1)
    #define GMAC_PRINT_ERR(args...) printk(KERN_ERR"GMAC:" args)
    #else
    #define GMAC_PRINT_ERR(args...)
    #endif

    #if (ENABLE_PRINT_INT == 1)
    #define GMAC_PRINT_INT(args...) printk(KERN_INFO"GMAC:" args)
    #else
    #define GMAC_PRINT_INT(args...)
    #endif

    #if ( ENABLE_LINK_STATUS_PRINT == 1)
    #define GMAC_STAT_PRINT(args...) printk(KERN_INFO"GMAC:" args)
    #else
    #define GMAC_STAT_PRINT(args...)
    #endif

#else
    #define GMAC_PRINT_DBG(args...)
    #define GMAC_PRINT_ERR(args...)
    #define GMAC_PRINT_INT(args...)
    #define GMAC_STAT_PRINT(args...)
#endif /* #if (ENABLE_PRINT == 1) */

/* configuration options ends */
/***************************************************************/

/* Transmit descriptor macros */
#define TXDESC_TDES0_OWN_VAL (0x80000000)
#define TXDESC_TDES0_IHE_VAL (0x10000)

#define TXDESC_TDES0_ES_VAL (0x8000)
#define TXDESC_TDES0_JABTIMEOUT_VAL (0x4000)
#define TXDESC_TDES0_FRMFLUSH_VAL (0x2000)

#define TXDESC_TDES0_IPPAYLD_VAL (0x1000)
#define TXDESC_TDES0_LOSSOFCAR_VAL (0x800)
#define TXDESC_TDES0_NOCAR_VAL (0x400)
#define TXDESC_TDES0_LATECOL_VAL (0x200)
#define TXDESC_TDES0_EXCOL_VAL (0x100)
#define TXDESC_TDES0_VLAN_VAL (0x80)
#define TXDESC_TDES0_COLCNT_MSK (0x78)
#define TXDESC_TDES0_COLCNT_POS (3)
#define TXDESC_TDES0_EXDEF_VAL (0x4)
#define TXDESC_TDES0_UNDERFLOW_VAL (0x2)
#define TXDESC_TDES0_DEF_BIT_VAL (0x1)
#define TXDESC_TDES0_TTSS_VAL (0x20000)

#ifdef __ENHANCED_DESCRIPTOR__
/* Control  Bits 30:26 */
#define TXDESC_TDES0_INT_VAL (0x40000000)
#define TXDESC_TDES0_LASTSEG_VAL (0x20000000)
#define TXDESC_TDES0_FIRSTSEG_VAL (0x10000000)
#define TXDESC_TDES0_CRC_DISABLE_VAL (0x8000000)
#define TXDESC_TDES0_PAD_DISABLE_VAL (0x4000000)

/* Control  Bits  Bits 23:20 */
#define TXDESC_TDES0_TTSE_EN_VAL (0x2000000)
#define TXDESC_TDES0_CIC_CLR (0xFF3FFFFF)
#define TXDESC_TDES0_CIC_POS (22)

/* Different CIC options */
#define TXDESC_TDES0_CIC_IPHDR_CSUM (0x400000)
#define TXDESC_TDES0_CSUM_NO_PSEUDO (0x800000)
#define TXDESC_TDES0_CSUM_WITH_PSEUDO (0xC00000)

#define TXDESC_TDES0_END_OF_RING_VAL (0x200000)
#define TXDESC_TDES0_SEC_ADR_CHAIN_VAL (0x100000)

#endif /*__ENHANCED_DESCRIPTOR__*/

#ifdef __ENHANCED_DESCRIPTOR__
/* Enhanced descriptor format */
#define TXDESC_TDES1_TX_BUF2_SIZE_CLR (0xE000FFFF)
#define TXDESC_TDES1_TX_BUF2_SIZE_POS (16)
#define TXDESC_TDES1_TX_BUF1_SIZE_CLR (0xFFFFE000)
#define TXDESC_TDES1_TX_BUF1_SIZE_MSK (0x1FFF)
#else
/* TDES1. Old format */
#define TXDESC_TDES1_INT_VAL (0x80000000)
#define TXDESC_TDES1_LASTSEG_VAL (0x40000000)
#define TXDESC_TDES1_FIRSTSEG_VAL (0x20000000)
#define TXDESC_TDES1_CRC_DISABLE_VAL (0x4000000)
#define TXDESC_TDES1_END_OF_RING_VAL (0x2000000)
#define TXDESC_TDES1_SEC_ADR_CHAIN_VAL (0x1000000)
#define TXDESC_TDES1_PAD_DISABLE_VAL (0x800000)

#define TXDESC_TDES1_TX_BUF2_SIZE_MSK (0x3ff800)
#define TXDESC_TDES1_TX_BUF2_SIZE_POS (11)

#define TXDESC_TDES1_TX_BUF1_SIZE_MSK (0x7ff)
#define TXDESC_TDES1_TX_BUF1_SIZE_CLR (0xfffff800)
#endif /* __ENHANCED_DESCRIPTOR__ */

/* Receive descriptor macros */
#define RXDESC_RDES0_OWN_VAL (0x80000000)
#define RXDESC_RDES0_DST_ADR_FAIL_VAL (0x40000000)

#define RXDESC_RDES0_FRM_LEN_MSK (0x3fff0000)
#define RXDESC_RDES0_FRM_LEN_POS (16)

#define RXDESC_RDES0_ERR_SUM_VAL (0x8000)
#define RXDESC_RDES0_DESC_ERR_VAL (0x4000)
#define RXDESC_RDES0_SRC_ADR_FAIL_VAL (0x2000)
#define RXDESC_RDES0_LEN_ERR_VAL (0x1000)
#define RXDESC_RDES0_OVERFLOW_ERR_VAL (0x800)
#define RXDESC_RDES0_VLAN_TAG_VAL (0x400)
#define RXDESC_RDES0_FIRST_DESC_VAL (0x200)
#define RXDESC_RDES0_LAST_DESC_VAL (0x100)
#define RXDESC_RDES0_TS_IPC_GIANT_VAL (0x80)
#define RXDESC_RDES0_LATE_COL_VAL (0x40)
#define RXDESC_RDES0_FRM_TYP_VAL (0x20)
#define RXDESC_RDES0_WDOG_VAL (0x10)
#define RXDESC_RDES0_RX_ERR_VAL (0x8)
#define RXDESC_RDES0_DRIBBLE_VAL (0x4)
#define RXDESC_RDES0_CRC_ERR_VAL (0x2)
#define RXDESC_RDES0_EXTDSTAT_DAMATCH (0x1)

/* RDES1 */
#define RXDESC_RDES1_DIS_IOC_VAL (0x80000000)
#define RXDESC_RDES1_DIS_IOC_CLR (0x7FFFFFFF)

#ifdef __ENHANCED_DESCRIPTOR__
/* Enhanced descriptor */
#define RXDESC_RDES1_END_OF_RING_VAL (0x8000)
#define RXDESC_RDES1_SEC_ADR_CHN_VAL (0x4000)
#define RXDESC_RDES1_BUF2_LEN_CLR (0xE000FFFF)
#define RXDESC_RDES1_BUF2_LEN_POS (16)
#define RXDESC_RDES1_BUF1_LEN_MSK (0x1FFF)
#define RXDESC_RDES1_BUF1_LEN_CLR (0xFFFFE000)
#else
/* Old format */
#define RXDESC_RDES1_END_OF_RING_VAL (0x2000000)
#define RXDESC_RDES1_SEC_ADR_CHN_VAL (0x1000000)
#define RXDESC_RDES1_BUF2_LEN_MSK (0x3ff800)
#define RXDESC_RDES1_BUF2_LEN_POS (11)
#define RXDESC_RDES1_BUF1_LEN_MSK (0x7ff)
#endif

/* RDES4 -- When Type 2 checksum engine or Time Stamp feature is enabled
** From version 3.50a onwards
*/
#define RXDESC_RDES4_IPV6_PKT (0x00000080)
#define RXDESC_RDES4_IPV4_PKT (0x00000040)
#define RXDESC_RDES4_IPCSUM_BYPASS (0x00000020)
#define RXDESC_RDES4_IPPAYLD_ERR (0x00000010)
#define RXDESC_RDES4_IPHDR_ERR (0x00000008)

/* Descriptor size is 32 bytes */
#define TCPIP_CSUM_ERRCHK (RXDESC_RDES4_IPPAYLD_ERR | RXDESC_RDES4_IPHDR_ERR)

#define RXDESC_RDES4_IPPAYLD_TYPE_MSK (0x00000007)
#define RXDESC_RDES4_PKT_BYPASS (0x0)
#define RXDESC_RDES4_PKT_UDP (0x1)
#define RXDESC_RDES4_PKT_TCP (0x2)
#define RXDESC_RDES4_PKT_ICMP (0x3)
#define RXDESC_RDES4_PKT_RSVD (0x8)

/* Macro to mask all the DMA interrupts */
#define DMA_MASK_ALL_INTS (0xFFFF)

#define GMAC_MASK_ALL_INTS (0xF)

/* Macro to clear the status bits. Bits are clear on write */
#define DMA_CLR_ALL_INTS (0xE7FF)

#define INT_STATUS_CHECK (HW_TRIDENTGMACETH_DMA_STATUS_GPI_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_GMI_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_NIS_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_AIS_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_FBI_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_RI_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_OVF_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_TI_VAL | \
                            HW_TRIDENTGMACETH_DMA_STATUS_UNF_VAL )

/* Macro for clearing receive status */
#define RX_STAT_MSK (HW_TRIDENTGMACETH_DMA_STATUS_RI_VAL | \
                                  HW_TRIDENTGMACETH_DMA_STATUS_RU_VAL | \
                                  HW_TRIDENTGMACETH_DMA_STATUS_OVF_VAL | \
                                  HW_TRIDENTGMACETH_DMA_STATUS_NIS_VAL | \
                                  HW_TRIDENTGMACETH_DMA_STATUS_AIS_VAL)

/* Receive interrupts macro for disabling interrupts for NAPI */
#define RX_INT_MSK (HW_TRIDENTGMACETH_DMA_INT_RIE_EN_VAL |  \
                                HW_TRIDENTGMACETH_DMA_INT_RUE_EN_VAL | \
                                HW_TRIDENTGMACETH_DMA_INT_OVE_EN_VAL )

/* Desired transmit interrupts */
/* Fatal bus error interrupt, Normal Interrupt summary,Abnormal Interrupt summary
** Underflow interrupt enable, Transmit interrupt enable
*/
#define TX_INTR_VAL (HW_TRIDENTGMACETH_DMA_INT_FBE_EN_VAL | \
                     HW_TRIDENTGMACETH_DMA_INT_NIE_EN_VAL  | \
                     HW_TRIDENTGMACETH_DMA_INT_AIE_EN_VAL  | \
                     HW_TRIDENTGMACETH_DMA_INT_UNE_EN_VAL | \
                     HW_TRIDENTGMACETH_DMA_INT_TIE_EN_VAL )

/* Fatal Bus Error Interrupt, Over run error interrupt, Normal Interrupt Summary,
** Abnormal Interrupt Summary, Receive Interrupt Enable
*/
#define RX_INTR_VAL (HW_TRIDENTGMACETH_DMA_INT_FBE_EN_VAL | \
                     HW_TRIDENTGMACETH_DMA_INT_OVE_EN_VAL | \
                     HW_TRIDENTGMACETH_DMA_INT_NIE_EN_VAL | \
                     HW_TRIDENTGMACETH_DMA_INT_AIE_EN_VAL | \
                     HW_TRIDENTGMACETH_DMA_INT_RIE_EN_VAL )

/* Early Transmit and Early Receive Interrupts */
#define ERE_ETE_INT_VAL ( HW_TRIDENTGMACETH_DMA_INT_ERE_EN_VAL | \
                          HW_TRIDENTGMACETH_DMA_INT_ETE_EN_VAL )


/* Interrupt mask register of GMAC. Enable PMT interrupt  */
#define HW_TRIDENTGMACETH_TS_INT_MSK_VAL (0x200)

#define HW_TRIDENTGMACETH_PMT_INT_MSK_VAL (0x8)

#define HW_TRIDENTGMACETH_PCSAN_INT_MSK_VAL (0x4)
#define HW_TRIDENTGMACETH_PCSLS_INT_MSK_VAL (0x2)
#define HW_TRIDENTGMACETH_RGMII_INT_MSK_VAL (0x1)

#define GMAC_INT_MASK_VAL (HW_TRIDENTGMACETH_TS_INT_MSK_VAL | \
                                                     HW_TRIDENTGMACETH_PCSAN_INT_MSK_VAL | \
                                                     HW_TRIDENTGMACETH_PCSLS_INT_MSK_VAL | \
                                                     HW_TRIDENTGMACETH_RGMII_INT_MSK_VAL)


#define PHY_RW_MASK(_regno_,_rw_,_phy_adr_,_csr_val_) ((_phy_adr_ << 11) | \
                                  (_regno_ << 6) | \
                                  (_csr_val_ << 2) | \
                                  (_rw_<< 1) | 1 )

/* structure for TX descriptors */
typedef struct TX_DESCR
{
    volatile __u32 TDES0;
    volatile __u32 TDES1;
    volatile __u32 TDES2;
    volatile __u32 TDES3;

    volatile __u32 TDES4;
    volatile __u32 TDES5;
    volatile __u32 TDES6;
    volatile __u32 TDES7;

} TX_DESCR_t ;

typedef struct RX_DESCR
{
    volatile __u32 RDES0;
    volatile __u32 RDES1;
    volatile __u32 RDES2;
    volatile __u32 RDES3;

    volatile __u32 RDES4;
    volatile __u32 RDES5;
    volatile __u32 RDES6;
    volatile __u32 RDES7;

} RX_DESCR_t ;

typedef struct trident_gmacEth_WorkQ
{
    struct work_struct workq;       /* Work queue structure */
    struct net_device* pNetDev; /* Pointer to network device */
} trident_gmacEth_WorkQ_t,*ptrident_gmacEth_WorkQ_t;


typedef struct trident_gmacEth_Napi
{
    struct napi_struct napi;
    struct net_device* pDev; /* Pointer to network device */
} trident_gmacEth_Napi_t,*ptrident_gmacEth_Napi_t;

typedef struct tridentgmacEth_Counters
{
   /* good counters */
   /*   0 */ __u64 ullRxPackets;
   /*   1 */ __u64 ullRxMulticast;
   /*   2 */ __u64 ullRxVlanFrame;
   /*   3 */ __u64 ullTxPackets;

   /* bad rx counters */
   /* 150 */ __u64 ullRxIpPayloadError;
   /* 151 */ __u64 ullRxIpHeaderError;
   /* 152 */ __u64 ullRxIPChecksumError;
   /* 153 */ __u64 ullRxDescriptorError;
   /* 154 */ __u64 ullRxMTLOverflowError;
   /* 155 */ __u64 ullRxTimeStamped;
   /* 156 */ __u64 ullRxLateCollisionError;
   /* 157 */ __u64 ullRxWdogTruncatedPktError;
   /* 158 */ __u64 ullRxGmiiRxError;
   /* 159 */ __u64 ullRxCrcError;
   /* 160 */ __u64 ullRxDribbleError;
   /* 161 */ __u64 ullRxBadIeeeLengthError;
   /* 162 */ __u64 ullRxFailedSAFilter;
   /* 163 */ __u64 ullRxFailedDAFilter;

   /* bad tx counters */
   /* 100 */ __u64 ullTxError;
   /* 101 */ __u64 ullTxUnderflowError;
   /* 102 */ __u64 ullTxExcessiveDeferralError;
   /* 103 */ __u64 ullTxExcessiveCollisionsError;
   /* 104 */ __u64 ullTxLateCollisionError;
   /* 105 */ __u64 ullTxNoCarrierError;
   /* 106 */ __u64 ullTxLossOfCarrierError;
   /* 107 */ __u64 ullTxIpPayloadError;
   /* 108 */ __u64 ullTxFrameFlushError;
   /* 109 */ __u64 ullTxJabberTimeoutError;
   /* 110 */ __u64 ullTxIpHeaderError;
   /* 111 */ __u64 ullTxCollisionError;

   /* dropped counters by software reasons */
   /* 200 */ __u64 ullTxDroppedOnLinkDown;
   /* 201 */ __u64 ullTxDroppedOnHardStart;
   /* 202 */ __u64 ullRxDroppedPacketFragment;
   /* 203 */ __u64 ullRxMulticastDropped;
   /* 204 */ __u64 ullRxTotalDropped;

} trident_gmacEth_Counters_t, *ptrident_gmacEth_Counters_t;

extern void MystiSetPair(int id, int reg, int val);
extern void MystiGetPair(int id, int *preg, int *pval);

/*
 * GMAC private structure
 */
typedef struct trident_gmacEth_PRIV
{

    void *  p_vtx ; /* allocated memory address for TX virtual address */
    dma_addr_t  p_tx; /* physical address for TX  */
    TX_DESCR_t *p_vtx_descr ; /* Tx descriptor array virtual address, bus width aligned */
    dma_addr_t p_tx_descr ;/* Physical Tx descriptor array address, bus width aligned */
    void * p_vrx ; /* allocated memory address for RX virtual address */
    dma_addr_t p_rx ; /* allocated memory address for RX physical address */
    RX_DESCR_t * p_vrx_descr ;	/* Rx descriptor array virtual desc address, bus width aligned */
    dma_addr_t  p_rx_descr ; /* Rx descriptor array physical desc address, bus width aligned */
    struct sk_buff * p_vtx_skb_list[ HW_DESCR_QUEUE_LEN_TX ] ;
    struct sk_buff * p_vrx_skb_list[ HW_DESCR_QUEUE_LEN_RX ] ; /* SKB buffers corresponding to RX descriptors */
    dma_addr_t p_eth_buf; /* allocated memory for Ethernet buffers physical address */
    __s8 * p_veth_buf ; /* allocated memory for Ethernet buffers virtual address */
    __u32 tx_produce_index ; /* Pointer to keep track of pkts submitted from application */
    __u32 tx_consume_index ; /* Pointer to keep track of pkts processed*/
    __u32 tx_submit_count; /* Pkts submitted for transmission on the wire */
    __u32 rx_consume_index ; /* Pointer to keep track of receive descriptors */
    __u32 u_speed ; /* speed setting 10/100/1000 */
    __u32 u_mode ; /* mode setting Full Duplex, Half Duplex */
    __u32 u_autoneg ; /* auto negotiation enable/disable */
    __u32 u_flow_control; /* flow control enabled/disabled */
    __u32 u_mc_filter_type ; /* multicast address filter type HW_FILTER, SW_FILTER */
    __u32 u_all_multi ; /* allow all multicast packets */
    __u32 u_rx_buf_size ; /* holds the maximum RX buf size */
    __u32 u_rx_tx_fc ; /* specifies RX and TX  flow control enabled/disabled */
    struct timer_list *phy_timer ; /* timer for probing the link status */
    __u32 linkStatus; /* Physical link status */
    wait_queue_head_t waitQ; /* For WoL  ioctl */
    __u32 wolFlag; /* For WoL  ioctl */
    struct net_device_stats stats; /* status (counters) structure */
    spinlock_t lock ; /* serialize access to this structure */
    spinlock_t tx_lock ; /* transmit sync lock */
    __u32 hwUnitNum; /* Hardware unit  < Combination of GMAC & PHY >*/
    trident_gmacEth_WorkQ_t autoNegWork; /* For autonegotiating when the cable is connected/disconnected */
    trident_gmacEth_WorkQ_t wdTimeoutTxWork; /* Watchdog timeout  */
    __u32 dma_enabled;

    trident_gmacEth_Napi_t napiInfo;
    __u32 autoneg_adv; /* Current fields advertized during autonegotiation */

    #ifdef __TRIDENT_GMAC_DEBUG__
    __u32 enable_mac_loopback;
    __u32 enable_phy_loopback;
    #endif

    __u32 enable_jumbo;
    #ifdef  CONTROL_INTR_FREQ
    __u32 tx_int_enable_cnt; /* Frequency at which the interrupt should be generated by MAC for tx */
    __u32 rx_intr_freq; /* Frequency at which the interrupt should be generated by MAC for rx*/
    #endif

    #ifdef TRIDENT_GMAC_VLAN_TAG
    struct vlan_group *vlgrp;
    #endif

    __u32 phy_addr_val; /* PHY address value */
    __u32 phy_id; 	/* phy_id: UID for this device found during discovery */

    __u32 clk_csr_val;  /* Clock used for reading & writing PHY registers */

    __u32 shutdown;

    trident_gmacEth_Counters_t counters;

} trident_gmacEth_PRIV_t ;

//
// driver kernel context - simulating the way a net driver works
//
struct _driver_kernel_context_
{
    char *name ;
    void *priv ;    // pointer to data context
};
typedef struct _driver_kernel_context_ dk_context_t ;

//
// driver data context - simulating the way an ethernet driver works
//
struct _driver_data_context_
{
    void *p_dg_ctx ;    // pointer to driver gpl context
};
typedef struct _driver_data_context_ dd_context_t ;

extern irqreturn_t trident_gmacEth_isr( __s32 irq, void *dev_id) ;

#endif /* _GMAC_DRV_H_ */
