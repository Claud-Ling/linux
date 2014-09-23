/*
 *
 *  (c)copyright 2011 trident microsystem ,inc. all rights reserved.
 *     claud ling  Claud_Ling@sigmadesigns.com
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
 * %filename:          gmac_drv.c %
 * %pid_version:           1.2        %
 *---------------------------------------------------------------------------
 * DESCRIPTION:      Linux driver source file for LIPP_6100ETH ethernet subsystem
 *
 * DOCUMENT REF:
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
 */

/*--------------------------------------------------------------------------*/
/* Standard include files:                                                  */
/*--------------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/completion.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/sockios.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/highmem.h>
#include <linux/dma-mapping.h>
#include <linux/if_ether.h>
#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include "../intfs/IPhy/inc/tmNxTypes.h"   /* Common include for HwApi & PHY device */
#include "../intfs/IPhy/inc/Phy.h"    /* Generic PHY header file */
#include "../comps/hwTRIDENTGMACEth/cfg/hwTRIDENTGMACEth_Cfg.h" /* GMAC HwApi configuration header file */
#include "../comps/hwTRIDENTGMACEth/inc/hwTRIDENTGMACEth.h" /* GMAC HwApi header file */
#include "../src/gmac_drv.h"  /* Driver header & configuration file */
#include "../src/remap.h"
#include <plat/gmac_eth_drv.h>
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#include <linux/if_vlan.h>
#endif
/*--------------------------------------------------------------------------*/
/* Ethernet defines:                                                        */
/*--------------------------------------------------------------------------*/

#define DRV_NAME "SIGMA_TRIx_ETHERNET"
#define DRV_VERSION "1.0"
#define DRV_RELDATE "2013/04/23"

MODULE_AUTHOR("Claud_Ling@sigmadesigns.com");
MODULE_DESCRIPTION("Trident GMAC driver for Ethernet Subsystem" DRV_VERSION " " DRV_RELDATE);
MODULE_LICENSE("GPL");

/* Macros */
#define NETDEV_PRIV( dev ) ( ( trident_gmacEth_PRIV_t * ) netdev_priv( dev ) )

#define SIZEOF_TX_DESCS(x) ( sizeof( TX_DESCR_t ) * (x) )

#define SIZEOF_RX_DESCS(x) ( sizeof( RX_DESCR_t ) * (x) )

/*--------------------------------------------------------------------------*/
/* Function prototypes:                                                     */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_probe(struct platform_device *pdev);

static __s32 trident_gmacEth_remove(struct platform_device *pdev);

static __s32 __init trident_gmacEth_init_module( void ) ;

static void __exit trident_gmacEth_cleanup_module( void );

/*--------------------------------------------------------------------------*/
/* Standard driver handlers:                                                     */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_open(struct net_device *dev);

static __s32 trident_gmacEth_stop(struct net_device *dev);

static __s32 trident_gmacEth_hard_start_xmit(struct sk_buff *skb, \
                                                struct net_device *dev);

//static void trident_gmacEth_multicast_list(struct net_device *dev);

//static __s32 trident_gmacEth_set_mac_address(struct net_device *dev, void *addr);
//static int trident_gmacEth_change_mtu (struct net_device *dev, int new_mtu);
static __s32 trident_gmacEth_do_ioctl(struct net_device *dev, struct ifreq *ifr, \
                                        __s32 cmd);

static void trident_gmacEth_tx_timeout_isr(struct net_device *dev);

static void trident_gmacEth_work_reset_link(struct work_struct *pWork);

static struct net_device_stats* trident_gmacEth_get_stats(struct net_device *dev);

static void trident_gmacEth_timer( unsigned long data ) ;

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
static __s32 trident_gmacEth_napi(struct napi_struct *pNapi, __s32 budget);
#endif

#ifdef TRIDENT_GMAC_VLAN_TAG
static void trident_gmaceth_vlan_rx_register(struct net_device *dev, struct vlan_group *grp);
#endif

irqreturn_t trident_gmacEth_isr( __s32 irq, void *dev_id) ;

#ifdef ENABLE_ETH_TOOL
/*--------------------------------------------------------------------------*/
/* Extended Ethtool driver handlers:                                        */
/*--------------------------------------------------------------------------*/
static __s32 trident_gmacEth_ethtool_get_settings(struct net_device *dev, \
                                                      struct ethtool_cmd *ecmd);

static __s32 trident_gmacEth_ethtool_set_settings(struct net_device *dev, \
                                                      struct ethtool_cmd *ecmd);

static void trident_gmacEth_ethtool_get_drvinfo(struct net_device *dev, \
                                                    struct ethtool_drvinfo *drvinfo);

static void trident_gmacEth_ethtool_get_pauseparam(struct net_device *dev, \
                                            struct ethtool_pauseparam* pauseparam);

static __s32 trident_gmacEth_ethtool_set_pauseparam(struct net_device *dev, \
                                             struct ethtool_pauseparam* pauseparam);

static __u32 trident_gmacEth_ethtool_get_link_status(struct net_device *dev);

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD
//static __u32 trident_gmacEth_ethtool_get_rx_csum(struct net_device *dev);
#endif
#endif
void trident_gmacEth_isr_poll(struct net_device* dev_id);
/*--------------------------------------------------------------------------*/
/* Internal functions prototypes:                                           */
/*--------------------------------------------------------------------------*/
static __s32 alloc_dma_descriptors( struct net_device *dev );
static void free_dma_descriptors( trident_gmacEth_PRIV_t * priv );
static __s32 setup_dma_descriptors( trident_gmacEth_PRIV_t * priv );
static __s32 down_trident_gmacEth( struct net_device *dev );
static __s32 up_trident_gmacEth( struct net_device *dev );
static __s32 setup_phy( struct net_device *dev );
static __s32 setup_mac( struct net_device *dev );
static __s32 setup_filter_config( struct net_device *dev );
//static __s32 generate_crc(char *data, __s32 data_len);
static void perfect_hash_filter_config( struct net_device *dev, \
                                     phwTRIDENTGMACEth_HashFilter_t rx_hash_filter );
static __s32 is_mc_filtered_packet( struct net_device * dev, __u32 idx );
static __s32 handle_receive_packets( struct net_device * dev, __s32 *pBudget);
static void handle_tx_packets( struct net_device * dev);
static __s32 check_n_enable_tx_flow_control( struct net_device *dev );
static __s32 disable_tx_flow_control( struct net_device *dev );

#ifdef __TRIDENT_GMAC_DEBUG__
/* Manipulate ARP & UDP data to run the UDP client/server application  in loopback mode */
static void man_arp_udp_data(struct sk_buff * skb,__u32 len );
#endif /* __TRIDENT_GMAC_DEBUG__ */

#ifdef CONFIG_PM
/* Power Management functions */
static int trident_gmacEth_resume(struct platform_device *pdev);
static int trident_gmacEth_suspend(struct platform_device *pdev, pm_message_t state);
#endif

//#define DEBUG_PACKET 
#ifdef DEBUG_PACKET
static void print_packet(unsigned char *buf, int length)
{
	int i;
	int remainder;
	int lines;

	printk("Packet of length %d \n", length);

	lines = length >> 4;
	remainder = length & 15;

	for (i = 0; i < lines; i++) {
		int cur;
		for (cur = 0; cur < 8; cur++) {
			unsigned char a, b;
			a = *(buf++);
			b = *(buf++);
			printk("%02x%02x ", a, b);
		}
		printk("\n");
	}
	for (i = 0; i < remainder / 2; i++) {
		unsigned char a, b;

		a = *(buf++);
		b = *(buf++);
		printk("%02x%02x ", a, b);
	}
	printk("\n");
}
#endif 
/*--------------------------------------------------------------------------*/
/* global variables used :                                                  */
/*--------------------------------------------------------------------------*/

/* Ethernet MAC address */
static __u8 mac_addr[HWTRIDENTGMACETH_NUM_UNITS][6]={{0,0}};
extern unsigned char stb_mac_address[1][6];
/*Save phy address come from cmdline ,sx6 default Phy addr is 0x0*/
UInt32 _phy_addr_ = 0x0;
/* Timer structure, for PHYs which doesnot support interrupt mechanism */
static struct timer_list trident_gmacEth_timer_struct[HWTRIDENTGMACETH_NUM_UNITS];

#ifdef ENABLE_ETH_TOOL
/* ethtool ioctl handlers. Refer Ethtool.h for other ioctls */
static struct ethtool_ops trident_gmacEth_ethtool_ops =
{
    .get_settings = trident_gmacEth_ethtool_get_settings,
    .set_settings = trident_gmacEth_ethtool_set_settings,
    .get_drvinfo = trident_gmacEth_ethtool_get_drvinfo,
    .get_pauseparam = trident_gmacEth_ethtool_get_pauseparam,
    .set_pauseparam = trident_gmacEth_ethtool_set_pauseparam,
    .get_link = trident_gmacEth_ethtool_get_link_status,
} ;
#endif

static const struct net_device_ops trident_gmacEth_netdev_ops = {
	.ndo_open		= trident_gmacEth_open,
	.ndo_stop		= trident_gmacEth_stop,
	.ndo_start_xmit		= trident_gmacEth_hard_start_xmit,
	.ndo_tx_timeout         = trident_gmacEth_tx_timeout_isr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address    = eth_mac_addr,
	.ndo_do_ioctl		= trident_gmacEth_do_ioctl,
	.ndo_get_stats		= trident_gmacEth_get_stats,
	.ndo_validate_addr      = eth_validate_addr,
#ifdef TRIDENT_GMAC_VLAN_TAG
	.ndo_vlan_rx_register = trident_gmaceth_vlan_rx_register,
#endif	
};

/* Global pointer to get the PHY function pointers */
static PhyConfig_t * gpPhyInterface[HWTRIDENTGMACETH_NUM_UNITS];

/**
* Ethernet TRIDENT_GMAC driver registration
*/
static struct platform_driver trident_gmaceth_driver =
{
    .probe      = trident_gmacEth_probe,
    .remove     = trident_gmacEth_remove,
#ifdef CONFIG_PM
	.suspend	= trident_gmacEth_suspend,
	.resume		= trident_gmacEth_resume,
#endif
    .driver     =   {
                        .name   = "SIGMA_Trix_GMAC",
                        .owner  = THIS_MODULE,
                    }

};

/* Flag to check if the MAC address is chosen from kernel command line */
static __u32 mac_addr_set[HWTRIDENTGMACETH_NUM_UNITS]={1};

static int macIdx=0;

/* FUNCTION: trident_gmacEth_init_mac0:
 * DESCRIPTION: For accepting the MAC address from kernel command line
 * RETURN:
 *
 * NOTES:
 */
/*--------------------------------------------------------------------------*/

static int __init trident_gmacEth_init_mac0(char *line)
{
    unsigned int mac_temp[6];
    int i;
    sscanf (line, "%x:%x:%x:%x:%x:%x",
    &mac_temp[0], &mac_temp[1], &mac_temp[2],
    &mac_temp[3], &mac_temp[4], &mac_temp[5]);

    for (i = 0; i < 6; i++) {
        mac_addr[macIdx][i] = mac_temp[i];
    }

    /* Static value to copy the next MAC addrress from cmd line */
    mac_addr_set[macIdx++] = 1;

    return 1;

}

/* FUNCTION: trident_gmacEth_init_mac1:
 * DESCRIPTION: For accepting the MAC address from kernel command line
 * RETURN:
 *
 * NOTES:
 */
/*--------------------------------------------------------------------------*/

__setup("ethaddr=",trident_gmacEth_init_mac0);


/* FUNCTION: trident_gmacEth_init_module:
 * DESCRIPTION: Register ethernet driver as platform driver
 * RETURN:
 *  0 on  Success, Non-zero value on Error
 *
 * NOTES:
 */
/*--------------------------------------------------------------------------*/

static __s32 __init trident_gmacEth_init_module( void )
{

    return platform_driver_register(&trident_gmaceth_driver);

}

/* FUNCTION:	trident_gmacEth_cleanup_module:
 * DESCRIPTION:  This function is called wth the driver is unloaded
 * RETURN:
 *	 None
 *
 * NOTES:
 */
/*--------------------------------------------------------------------------*/
static void __exit trident_gmacEth_cleanup_module( void )
{
    platform_driver_unregister(&trident_gmaceth_driver);

}

/*
 *Get PHY address for cmdline 
 *for some phy scanning not 
 *stable
 *
 *
 *
 */
#ifndef MODULE
static int __init set_phy_addr(char *p)
{
	if((p == NULL) || (*p == '\0')){
		_phy_addr_ = 0x04;
	}else{
		_phy_addr_ = simple_strtoull(p, &p, 0);
		if(_phy_addr_ > 0x1f ) {
			printk("Got wrong phyaddr from cmdline is %ld\n",_phy_addr_);
			_phy_addr_ = 0x04;
		}
	}
	printk("_phy_addr_ is %ld\n",_phy_addr_);

	return 0;
}
#endif

/*--------------------------------------------------------------------------*/
/* Control functions:                                                                                              */
/*--------------------------------------------------------------------------*/

/*
 * FUNCTION:	trident_gmacEth_probe:
 *  DESCRIPTION:	This function will initializes the hardware and registers the
 *	device with the kernel, stores the corresponding device paramter information in private
 *     structure.
 * RETURN:
 *	0	- success
 *	ENXIO	- IO error
 *	ENOMEM	- Memory allocation failure
 *	EINVAL	- Invalid arguments
 *
 * NOTES:
 **
 */

/*--------------------------------------------------------------------------*/
static __s32 trident_gmacEth_probe(struct platform_device *pdev)
{
    struct net_device *	      dev = NULL;
    trident_gmacEth_PRIV_t *    priv ;
    __s32                      ret_val ;
    __u32 unitNum = pdev->id;
    pgmac_platform_data_t pPlatData=NULL;
    hwTRIDENTGMACEth_PhyInfo_t phyInfo;
    __u8 *pMacAdr=NULL;
    __u8 tMAC[6];
    hwTRIDENTGMACEth_Int_t IntDis;
    __u32 Ref_Clk, IntrMask;
#ifdef CONFIG_GMAC_MODE_RGMII
    __u32 phy_intr_en,ChipVer;
#endif

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
    __u32 hwVer;
#endif
#if 0   //pcl used external phy clk  
/*Set the clock input mux */
    Ref_Clk = ReadRegWord((volatile void*)PINSHARE_REG_A0);
    Ref_Clk &= ~0x38; 	
    WriteRegWord((volatile void *)PINSHARE_REG_A0, Ref_Clk | 0x10);
#endif 
#ifdef CONFIG_GMAC_MODE_RGMII
    phy_intr_en = ReadRegWord((volatile void*)0x15031f00);
    phy_intr_en &= ~0x07; 	
    WriteRegWord((volatile void *)0x15031f00,phy_intr_en | 0x01);
#endif
#if 0
    ChipVer = ReadRegWord((volatile void*)0x1500e000);
    printk("Chip version is 0x%x\n",ChipVer);
    if( ChipVer & 0x01 ){    
	    IntrMask = ReadRegWord((volatile void*)INTERRUPTCTL_REG);
	    IntrMask |= 0x40; 	
	    WriteRegWord((volatile void *)INTERRUPTCTL_REG, IntrMask );
    }
#endif
	 IntrMask = ReadRegWord((volatile void*)INTERRUPTCTL_REG);
	 IntrMask |= 0x40; 	
	 WriteRegWord((volatile void *)INTERRUPTCTL_REG, IntrMask );

#if defined(CONFIG_SIGMA_SOC_SX7)
	 WriteRegByte((volatile void *)0xf500ea2c, 0x7f); //GBE_TXEN
	 WriteRegByte((volatile void *)0xf500ea2d, 0x7f); //GBE_TXC
	 WriteRegByte((volatile void *)0xf500ea2e, 0x7f); //GBE_TXD0
	 WriteRegByte((volatile void *)0xf500ea2f, 0x7f); //GBE_TXD1
	 WriteRegByte((volatile void *)0xf500ea30, 0x7f); //GBE_TXD2
	 WriteRegByte((volatile void *)0xf500ea31, 0x7f); //GBE_TXD3
	 WriteRegByte((volatile void *)0xf500ea38, 0x7f); //GBE_MDC
#endif
    dev = alloc_etherdev(sizeof( trident_gmacEth_PRIV_t));

    if( NULL == dev ) {
        GMAC_PRINT_ERR("Unable to allocate etherdev, aborting\n");
        ret_val = -ENOMEM ;
        goto _dev_alloc_failed;
    }

    SET_NETDEV_DEV(dev, &pdev->dev);

    /* Store the network device structure */
    platform_set_drvdata(pdev, dev);

    /* To fetch platform data like clk_csr, phy addr & MAC address */
    pPlatData = pdev->dev.platform_data;

    /* Remap base addresses for HwApi base addresses */
    ret_val = remapBaseAdrs(pdev);

    if(0 != ret_val) {
        GMAC_PRINT_ERR("Error in Remapped Base Addresses\n");
        goto _abort_init;
    }

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
    hwTRIDENTGMACEth_GetHWVersion(unitNum,(pUInt32)&hwVer);
    GMAC_PRINT_DBG("HwVersion: %x\n",hwVer);
#endif

    phyInfo.clkCsrVal = pPlatData->clk_csr_val;
    pMacAdr = &pPlatData->mac_addr[0];
    phyInfo.phyAddr = _phy_addr_;
    /* Initialize the ethernet MAC, do a reset */
    if( hwTRIDENTGMACEth_Init(unitNum,&phyInfo, unitNum?\
			    GMAC1_MII_SEL:GMAC0_MII_SEL) != TM_OK ) {
        GMAC_PRINT_ERR("Error in Ethernet MAC H/W initialization, aborting\n");
        ret_val = -ENXIO ;
        goto _abort_init;
    }

    /* Get the PHY interface function pointers */
    PhyGetInterface(unitNum, phyInfo.phyID, pPlatData->isExternal,\
		    &gpPhyInterface[unitNum]);

    GMAC_STAT_PRINT("%s PHY being used with GMAC%d\n",
                    gpPhyInterface[unitNum]->phyName,unitNum);

    /* Disable unnecessary interrupts */
    IntDis.dmaIntVal = ERE_ETE_INT_VAL;
    IntDis.gmacIntVal = GMAC_INT_MASK_VAL;
    hwTRIDENTGMACEth_IntDisable(unitNum, &IntDis);

    /* Register interrupts, after MAC is reset. PHY is reset in open fun */
    ret_val = registerInterrupts(pdev);

    if(0 != ret_val) {
        GMAC_PRINT_ERR("Error while registering ethernet ISRs\n");
        goto _abort_init_int;
    }

    /* If MAC address is set from the command line, change the ptr */
    if(mac_addr_set[unitNum] == 1U ) {
         pMacAdr = &mac_addr[unitNum][0];
    } else {/* If the MAC address is not in command line, look for ATAG list */ 
         pMacAdr = &stb_mac_address[unitNum][0];
    }

	switch(unitNum)
	{
		case 0:	/* Primary Ethernet */
		{
			if(pMacAdr != NULL)
			{
				/* If MAC Address is empty, Use Platform Data */
				if((pMacAdr[0] == 0x0) && (pMacAdr[1] == 0x0) && (pMacAdr[2] == 0x0)
				&& (pMacAdr[3] == 0x0) && (pMacAdr[4] == 0x0) && (pMacAdr[5] == 0x0))
				{
					pMacAdr = &pPlatData->mac_addr[0];
				}
			}
			else
			{
				tMAC[0] = pPlatData->mac_addr[0];
				tMAC[1] = pPlatData->mac_addr[1];
				tMAC[2] = pPlatData->mac_addr[2];
				tMAC[3] = pPlatData->mac_addr[3];
				tMAC[4] = pPlatData->mac_addr[4];
				tMAC[5] = pPlatData->mac_addr[5];

				pMacAdr = &tMAC[0];
			}

			break;
		}

		case 1: /* Secondary Ethernet */
		{
			if(pMacAdr != NULL)
			{
				/* If MAC Address is empty, Use Platform Data */
				if((pMacAdr[0] == 0x0) && (pMacAdr[1] == 0x0) && (pMacAdr[2] == 0x0)
				&& (pMacAdr[3] == 0x0) && (pMacAdr[4] == 0x0) && (pMacAdr[5] == 0x0));
				{
					tMAC[0] = pPlatData->mac_addr[0];
					tMAC[1] = pPlatData->mac_addr[1];
					tMAC[2] = pPlatData->mac_addr[2];
					tMAC[3] = pPlatData->mac_addr[3];
					tMAC[4] = pPlatData->mac_addr[4];
					tMAC[5] = pPlatData->mac_addr[5];

					if(tMAC[5] == 0xFF)
						tMAC[5] = 0x0;
					else
						tMAC[5] = tMAC[5] + 0x01;

					pMacAdr = &tMAC[0];
				}
			}
			else
			{
				tMAC[0] = pPlatData->mac_addr[0];
				tMAC[1] = pPlatData->mac_addr[1];
				tMAC[2] = pPlatData->mac_addr[2];
				tMAC[3] = pPlatData->mac_addr[3];
				tMAC[4] = pPlatData->mac_addr[4];
				tMAC[5] = pPlatData->mac_addr[5];

				if(tMAC[5] == 0xFF)
					tMAC[5] = 0x0;
				else
					tMAC[5] = tMAC[5] + 0x01;

				pMacAdr = &tMAC[0];
			}

			break;
		}

		default:
			break;
	}

	printk("GMAC[%s:%d] Mac Address : %02x:%02x:%02x:%02x:%02x:%02x\n", pdev->name, unitNum
																	  , pMacAdr[0], pMacAdr[1], pMacAdr[2]
																	  , pMacAdr[3], pMacAdr[4], pMacAdr[5]);

    /* MAC address is not set in kernel cmd line.Set the default ones*/
    memcpy(dev->dev_addr,pMacAdr,ETH_ALEN);

    if (!is_valid_ether_addr(dev->dev_addr))
	    GMAC_PRINT_DBG("\tNo valid MAC address;"
		    "please, use ifconfig or fix MAC Address!\n");

    /* Get pointer to private structure */
    priv = netdev_priv(dev);

    /* zero out the private structure */
    memset(priv, 0x00, sizeof(trident_gmacEth_PRIV_t )) ;

    /* Fill in the fields of the device structure with Ethernet-generic values. */
    ether_setup(dev);

    /*
    * Initialize the timer, that checks the status of the link for every 500 ms
    * report any error to the kernel
    */
    trident_gmacEth_timer_struct[unitNum].function = trident_gmacEth_timer;

    trident_gmacEth_timer_struct[unitNum].data = (__u32)dev;

    trident_gmacEth_timer_struct[unitNum].expires = jiffies + TIMEOUT_VALUE;

    /* Store the hardware device unit number */
    priv->hwUnitNum = unitNum;

    priv->phy_timer = &trident_gmacEth_timer_struct[unitNum];

    init_timer(&trident_gmacEth_timer_struct[unitNum]);

    spin_lock_init( &priv->lock ) ;
    spin_lock_init( &priv->tx_lock ) ;

    priv->u_autoneg = ETH_AUTO_NEGOTIATION;
    priv->u_speed = pPlatData->max_speed;
    priv->clk_csr_val = pPlatData->clk_csr_val;
    priv->phy_addr_val = phyInfo.phyAddr;

    #ifdef __TRIDENT_GMAC_DEBUG__
    priv->enable_mac_loopback=0;
    priv->enable_phy_loopback =0;
    #endif

    priv->u_mode = ETH_LINK_MODE ;
    priv->u_flow_control = ETH_FLOW_CTRL ;
    priv->u_rx_tx_fc = ETH_FLOW_CTRL_DIR;

    /* Rx Frame size should be multiple of 4/8/16 depending on bus width */
    priv->u_rx_buf_size = MAX_ETH_FRAME_SIZE ;

    /* When set, doesnot pass through the multicast filtering */
    priv->u_all_multi = MULTICAST_FILTER ;

    /* Hardware or software filtering */
    priv->u_mc_filter_type = ETH_FILTER_TYPE ;

    priv->dma_enabled = 0;
    dev->if_port = PORT_MII;

    /* Set this macro to enable/disable */
    priv->enable_jumbo = ETH_ENABLE_JUMBO_FRAME;

    if(priv->enable_jumbo == 1U) {
        /* Set the receive buffer size to multiple of bus-width */
        if ( (TRIDENT_GMAC_JUMBO_MTU & (~DMA_DESC_ALIGNMENT))
            != TRIDENT_GMAC_JUMBO_MTU ) {
            priv->u_rx_buf_size =
                        (TRIDENT_GMAC_JUMBO_MTU & (~DMA_DESC_ALIGNMENT)) +
                         DMA_DESC_ALIGNMENT +1;
        }

    }

    /* assign the driver handlers */
    dev->netdev_ops = &trident_gmacEth_netdev_ops;

    dev->watchdog_timeo = TX_TIMEOUT;

    priv->autoNegWork.pNetDev = dev;

    INIT_WORK(&priv->autoNegWork.workq,trident_gmacEth_work_reset_link);

    priv->wdTimeoutTxWork.pNetDev = dev;
    INIT_WORK(&priv->wdTimeoutTxWork.workq,trident_gmacEth_work_reset_link);

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    priv->napiInfo.pDev = dev;

    netif_napi_add(dev,&priv->napiInfo.napi,trident_gmacEth_napi,NAPI_DEV_WEIGHT);
#endif

#ifdef TRIDENT_GMAC_VLAN_TAG
    /* Supports receive VLAN Tag filtering */
    dev->features = NETIF_F_HW_VLAN_RX;
#endif

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD
    /* Can checksum TCP/UDP packets over IPv4 & IPv6 */
    //dev->features |= NETIF_F_ALL_CSUM | NETIF_F_SG ;
    dev->features |= NETIF_F_HIGHDMA | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_FRAGLIST;
#endif /* CONFIG_SIGMA_GMAC_CSUMOFFLOAD */

#ifdef ENABLE_ETH_TOOL
    dev->ethtool_ops = &trident_gmacEth_ethtool_ops;
#endif

    /* Receive all multicast packets */
    dev->flags |= IFF_ALLMULTI;

    /* Pass all frames */
    if( ETH_PROMISC_MODE == LX_PROMISC_ENABLED ) {
        dev->flags |= IFF_PROMISC ;
    } else {
        dev->flags &= ~(IFF_PROMISC) ;
    }

    dev->base_addr = 0 ;

    if (register_netdev(dev) != 0) {

        GMAC_PRINT_ERR("Unable to register the network device, aborting\n");
        ret_val = -ENXIO ;
        goto _abort_init;
    }

    /* For woL support */
    GMAC_STAT_PRINT("Ethernet interface %s registered\n",dev->name);
#ifdef ALLOCATE_MEMORY_AT_INIT
    /*
    * Allocate memory buffer for TX, RX descriptor and allocate
    * buffers for all the descriptors
    *  in the receive side.
    */
    if( alloc_dma_descriptors( dev ) != 0 ) {
        GMAC_PRINT_ERR("Unable to allocate memory, aborting\n");
        return -ENOMEM ;
    }

    /* Initialize Rx descriptors, assign buffers to each rx descriptor.
    ** Since ring mode is used, the descriptors should be contiguous
    ** and the addresses should be aligned to bus width.
    */
    if( setup_dma_descriptors( priv ) != 0 ) {
        goto _err_free_mem ;
    }
#endif

    init_waitqueue_head(&priv->waitQ);
    return 0 ;

_abort_init_int:
    unregisterInterrupts(pdev);

_abort_init:

    GMAC_PRINT_ERR("Aborting Driver Initialization\n");

    unmapBaseAdrs(pdev);

    /* Free the netdevice structure */
    free_netdev(dev);
#ifdef ALLOCATE_MEMORY_AT_INIT
_err_free_mem :

    /* free the memory */
    free_dma_descriptors( priv ) ;
#endif

_dev_alloc_failed:

    return ret_val;

}

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:    trident_gmacEth_remove:
 * DESCRIPTION: This function will un-initialize the hardware and unregisters the net_device structure
 *                      and frees the memory for device structure
 *
 * RETURN:      None
 *
 * NOTES: This function is called when the driver needs to be unloaded from the system.
 *            Ex: Call to 'rmmod' removes the loadable module from the system and this function
 *             is invoked at that point.
 */
/*--------------------------------------------------------------------------*/
static __s32 trident_gmacEth_remove(struct platform_device *pdev)

{
    struct net_device * dev;
    trident_gmacEth_PRIV_t *    priv ;

    dev = dev_get_drvdata(&pdev->dev);
    priv = NETDEV_PRIV(dev);

    /* Deinit the PHY device */
//    gpPhyInterface[unitNum]->deinitFunc(priv->phy_addr_val);

#if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_OTHERS)
    hwTRIDENTGMACEth_Deinit(unitNum);
#endif
    netif_carrier_off(dev);

    /* Unregister the network device */
    unregister_netdev(dev) ;

    /* Unmap base address */
    unmapBaseAdrs(pdev);

    unregisterInterrupts(pdev);

    /* Free the memory for device structure */
    free_netdev(dev);

    return 0;

}
/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:	alloc_dma_descriptors
 * DESCRIPTION: This function allocates the memory required for transmit and receive descriptors,
 * allocates SKBs to store received ethernet frames . If memory is not allocated successfully,
 * it frees the memory allocated.
 *
 * PARAMETERS:
 * 	priv	- trident_gmacEth_PRIV_t private structure
 * RETURN:	0 - success
 *                -ENOMEM - on failure
 * NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static __s32 alloc_dma_descriptors( struct net_device *dev )
{
    __s32 i=0;
    trident_gmacEth_PRIV_t * priv = netdev_priv(dev) ;

    /* Virtual Adr for transmission */
    priv->p_vtx = dma_alloc_coherent( NULL,
    					PAGE_ALIGN(SIZEOF_TX_DESCS( HW_DESCR_QUEUE_LEN_TX)),
    					&priv->p_tx, /* Physical address for transmission */
    					GFP_ATOMIC ) ;

    /* Virtual Adr for reception */
    priv->p_vrx = dma_alloc_coherent( NULL,
    					PAGE_ALIGN(SIZEOF_RX_DESCS( HW_DESCR_QUEUE_LEN_RX)),
    					&priv->p_rx, /* Physical address for reception */
    					GFP_ATOMIC ) ;

    if( ( NULL ==  priv->p_vtx ) || ( NULL ==  priv->p_vrx ) ) {
        goto _free_txrx_mem ;
    }

    GMAC_PRINT_DBG("p_vtx:%08x p_tx: %08x\n",(unsigned int)priv->p_vtx, priv->p_tx);
    GMAC_PRINT_DBG("p_vrx:%08x p_rx: %08x\n",(unsigned int)priv->p_vrx, priv->p_rx);
    /* memory is allocated, just align the physical addresses and assign.
        ** it to trident_gmacEth_PRIV_t->p_[rt]x_* members
        */

    /* The descriptors need to be aligned to the bus width. The descriptors are aligned to
        **  128bits. ( Descriptor Address should be divisible by 16)
        */
    priv->p_vtx_descr = (TX_DESCR_t *)(((__u32)priv->p_vtx + DMA_DESC_ALIGNMENT) & \
                                        (~DMA_DESC_ALIGNMENT));
    priv->p_tx_descr  = (dma_addr_t)(((__u32)priv->p_tx + DMA_DESC_ALIGNMENT) & \
                                        (~DMA_DESC_ALIGNMENT));
    priv->p_vrx_descr = (RX_DESCR_t *)(((__u32)priv->p_vrx + DMA_DESC_ALIGNMENT) & \
                                        (~DMA_DESC_ALIGNMENT));
    priv->p_rx_descr = (dma_addr_t)(((__u32)priv->p_rx + DMA_DESC_ALIGNMENT) & \
                                        (~DMA_DESC_ALIGNMENT));

    GMAC_PRINT_DBG("priv->p_vtx_descr:%08x p_tx: %08x\n",(unsigned int)priv->p_vtx_descr, priv->p_tx);
    GMAC_PRINT_DBG("priv->p_vrx_descr:%08x p_rx: %08x\n",(unsigned int)priv->p_vrx_descr, priv->p_rx);

  /*
    * Now allocate memory for RX ethernet buffers
    * for poll mode : RX ethernet buffer with size of 1536 bytes using dma_alloc_coherent()
    * and store the physical address into RX_DESCR_t[index]->packet, and store the virtual address
    * into priv->rx_skb_list[index].
    * [for interrupt mode] : Rx SKBs with MTU size of 1536+2 bytes using alloc_skb()
    * and store the physical address of SKB->data() into RX_DESCR_t[index]->packet, and store the
    * SKB address into priv->rx_skb_list[index].
    */

    /* Interrupt Mode */
    /* Allocate SKB memory buffers and store them in the arry priv->p_vrx_skb_list[] */
	for( i =0  ; i < HW_DESCR_QUEUE_LEN_RX ; i++ ) {
		priv->p_vrx_skb_list[ i ] = netdev_alloc_skb_ip_align(dev, priv->u_rx_buf_size);

		if( NULL == priv->p_vrx_skb_list[ i ] ) {
			/* free up the memory allocated for SKBs */
			goto _free_rx_skb_list ;
		}
		

	}

	return 0 ;

_free_rx_skb_list:

    for( --i ; i >= 0 ; i-- ) {
        dev_kfree_skb( priv->p_vrx_skb_list[ i ] ) ;
    }

_free_txrx_mem:

    if( priv->p_vtx != NULL ) {
        dma_free_coherent( NULL,
                                PAGE_ALIGN(SIZEOF_TX_DESCS( HW_DESCR_QUEUE_LEN_TX)),
                                priv->p_vtx,
                                priv->p_tx ) ;
    }

    if( priv->p_vrx != NULL ) {
        dma_free_coherent( NULL,PAGE_ALIGN(SIZEOF_RX_DESCS( HW_DESCR_QUEUE_LEN_RX)),
                                    priv->p_vrx,
                                    priv->p_rx ) ;
    }

    return -ENOMEM ;

}


/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:    free_dma_descriptors
 * DESCRIPTION: This function frees the memory allocated for transmit and receive descriptors,
 * SKBs allocated for receiving ethernet frames, transmit frames queued but not yet transmitted.
 *
 * PARAMETERS:
 * 	priv	- trident_gmacEth_PRIV_t private structure
 * RETURN:	None
 * NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static void free_dma_descriptors( trident_gmacEth_PRIV_t * priv )
{

    __s32 i =	0 ;

    /* free the SKBs allocated and the ethernet buffers */

    for( i = 0 ; i < HW_DESCR_QUEUE_LEN_RX ; i++ ) {
        if(priv->p_vrx_skb_list[i] != NULL ) {
            dev_kfree_skb_any(priv->p_vrx_skb_list[ i ]) ;
        }
    }

    /* free the SKBs which are pending for transmission */
    for( i = 0 ; i < HW_DESCR_QUEUE_LEN_TX ; i++ ) {
        if( priv->p_vtx_skb_list[ i ] ) {
            dev_kfree_skb_any( priv->p_vtx_skb_list[ i ] ) ;
        }
    }

    /* free the memory allocated to RX, TX descriptors */
    if( priv->p_vtx != NULL ) {
        dma_free_coherent( NULL,
        PAGE_ALIGN(SIZEOF_TX_DESCS( HW_DESCR_QUEUE_LEN_TX)),
        priv->p_vtx,
        priv->p_tx ) ;
    }

    if( priv->p_vrx != NULL ) {
        dma_free_coherent( NULL,
        PAGE_ALIGN(SIZEOF_RX_DESCS( HW_DESCR_QUEUE_LEN_RX)),
        priv->p_vrx,
        priv->p_rx ) ;
    }

    return  ;

}

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION: setup_dma_descriptors
 * DESCRIPTION:	This function initializes Transmit and Receive descriptors for DMA
 * PARAMETERS:
 * 	priv	- trident_gmacEth_PRIV_t private structure
 * RETURN:
 * 0 - success
 * NOTES: Forms TX & RX DMA descriptor ring, enables/disables interrupt on
 * frame transmission/ reception.
 */
/*--------------------------------------------------------------------------*/
static __s32 setup_dma_descriptors( trident_gmacEth_PRIV_t * priv )
{

     __s32 i;

    /* Initialize the Tx & Rx Descriptor indices */
    priv->tx_produce_index = 0;
    priv->tx_consume_index = 0;
    priv->rx_consume_index = 0;
    priv->tx_submit_count =0;

    /*Clear the TX descriptors */
    memset(priv->p_vtx_descr,0,sizeof(TX_DESCR_t)*HW_DESCR_QUEUE_LEN_TX);

    /* Set the Transmit End of Ring bit */
    #ifdef __ENHANCED_DESCRIPTOR__
    priv->p_vtx_descr[HW_DESCR_QUEUE_LEN_TX-1].TDES0|= TXDESC_TDES0_END_OF_RING_VAL;
    #else
    priv->p_vtx_descr[HW_DESCR_QUEUE_LEN_TX-1].TDES1 |= TXDESC_TDES1_END_OF_RING_VAL;
    #endif /* __ENHANCED_DESCRIPTOR__ */

    /*Clear the RX descriptors */
    memset(priv->p_vrx_descr,0,sizeof(RX_DESCR_t)*HW_DESCR_QUEUE_LEN_RX);

    for(i=0;i<HW_DESCR_QUEUE_LEN_RX;i++) {

        /* Only Buffer1 is utilized. Assign the physical address of the buffer
        ** to descriptor
        */
        priv->p_vrx_descr[i].RDES2 = (__u32) dma_map_single( NULL,
        					(priv->p_vrx_skb_list[i])->data,
        					priv->u_rx_buf_size,
        					DMA_FROM_DEVICE );

        /* Set the own bit. DMA can process the descriptors only with own bit set */
        priv->p_vrx_descr[i].RDES0 |= RXDESC_RDES0_OWN_VAL;
        priv->p_vrx_descr[i].RDES1 |= priv->u_rx_buf_size;
    }

    /* Set the end of ring bit & length field */
    priv->p_vrx_descr[i-1].RDES1 |= RXDESC_RDES1_END_OF_RING_VAL;

    return 0 ;

}

/*--------------------------------------------------------------------------
 * FUNCTION: down_trident_gmacEth:
 * DESCRIPTION: This function disables the TX and RX path and frees up the SKBs
 * which are not transmitted and zeros out all TX status and RX status for all DMA descriptors
 * PARAMETERS:
 *    dev - net_device structure for the device
 * RETURN:
 *  0 - success
 * -1 - Failure
 * NOTES: netif_carrier_off() and netif_stop_queue() should be called before invoking the function
 *
 *--------------------------------------------------------------------------*/
 static __s32 down_trident_gmacEth( struct net_device *dev )
{
    __s32 i = 0 ;
    unsigned long flags ;
    hwTRIDENTGMACEth_EnTxfr_t txfr;
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;
    hwTRIDENTGMACEth_Int_t IntDis;

    GMAC_PRINT_DBG("down_trident_gmacEth called for %s\n", dev->name);

    local_irq_save( flags );

    IntDis.dmaIntVal = DMA_MASK_ALL_INTS;
    IntDis.gmacIntVal = GMAC_INT_MASK_VAL;

    /* Disable all the interrupts */
    hwTRIDENTGMACEth_IntDisable( priv->hwUnitNum,&IntDis);

    local_irq_restore( flags ) ;

    txfr.dirFlag = hwTRIDENTGMACEth_Dir_TxRx;
    txfr.enFlag = hwTRIDENTGMACEth_Disable;

    /* Disable DMA and GMAC state machines */
    hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&txfr);
    hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&txfr);

    /* This flag is set to 1 during transmission once */
    priv->dma_enabled =0;

    for( i = 0 ; i < HW_DESCR_QUEUE_LEN_TX ; i++ ) {
        /* Free all the Tx SKB buffers */
        if( priv->p_vtx_skb_list[ i ] != NULL ) {
            dev_kfree_skb( priv->p_vtx_skb_list[ i ] ) ;
            priv->p_vtx_skb_list[ i ] = NULL ;
            priv->stats.tx_dropped ++ ;
            priv->counters.ullTxDroppedOnLinkDown++;
        }

        /* Clear the transmit status */
        priv->p_vtx_descr[ i ].TDES0 = 0;
    }

    #ifdef __ENHANCED_DESCRIPTOR__
    /* Set the end of ring bit */
    priv->p_vtx_descr[ i-1 ].TDES0 |= TXDESC_TDES0_END_OF_RING_VAL;
    #endif /* __ENHANCED_DESCRIPTOR__ */

    for( i = 0 ; i < HW_DESCR_QUEUE_LEN_RX ; i++ ) {
        /* Clear the receive status, dont clear the OWN bit */
        priv->p_vrx_descr[ i ].RDES0 = RXDESC_RDES0_OWN_VAL;

    }

    /* Reset all the indices */
    priv->rx_consume_index = 0;
    priv->tx_consume_index = 0;
    priv->tx_produce_index = 0;
    priv->tx_submit_count = 0;

    /* Delete the timer */
    del_timer( priv->phy_timer );

    return 0 ;

}


/*--------------------------------------------------------------------------*/
/*
 * FUNCTION: up_trident_gmacEth
 *DESCRIPTION: This function sets up the hardware (PHY and MAC) and enables it for
 * reception of ethernet frames
 *PARAMETERS:
 * dev - net_device structure for the device
 *RETURN:
 * 0 - success
 * -1 - Failure
 *NOTES:
 */
/*--------------------------------------------------------------------------*/
static __s32 up_trident_gmacEth( struct net_device *dev )
{
    unsigned long flags ;
    hwTRIDENTGMACEth_EnTxfr_t Txfer;
    __s32 ret_val;
    __u32 Status;
    hwTRIDENTGMACEth_Int_t IntEn;
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;

    GMAC_PRINT_DBG("up_trident_gmacEth called for %s\n", dev->name);

    /* Normal operation of the driver  */
    /* Do autonegotiation or setup the PHY parameters to fixed values */
    if( setup_phy(dev) != 0 ) {
        GMAC_PRINT_ERR("Error while Setting up PHY\n");
        goto _err_up_trident_gmacEth ;
    }

    /* depending upon the options set in the dev structure, set the device parameters */
    if( setup_mac( dev ) != 0 ) {
        goto _err_up_trident_gmacEth ;
    }

    /* setup the filter configuration */
    if( setup_filter_config( dev ) != 0 ) {
        goto _err_up_trident_gmacEth ;
    }

    local_irq_save( flags ) ;

    if( hwTRIDENTGMACEth_IntClear( priv->hwUnitNum, DMA_CLR_ALL_INTS )  != TM_OK ) {
        local_irq_restore( flags ) ;
        goto _err_up_trident_gmacEth ;
    }

    IntEn.dmaIntVal =  TX_INTR_VAL | RX_INTR_VAL;
    IntEn.gmacIntVal =0x0;

    if(hwTRIDENTGMACEth_IntEnable(priv->hwUnitNum,&IntEn) != TM_OK ) {
        local_irq_restore( flags ) ;
        goto _err_up_trident_gmacEth ;
    }

    /* Disable unnecessary interrupts */
    IntEn.dmaIntVal = ERE_ETE_INT_VAL;
    IntEn.gmacIntVal = GMAC_INT_MASK_VAL;
    hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntEn);

    local_irq_restore( flags ) ;

    Txfer.dirFlag = hwTRIDENTGMACEth_Dir_Rx;
    Txfer.enFlag = hwTRIDENTGMACEth_Enable;

    /* Reads the register and sets the corresponding bit */
    ret_val = hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&Txfer);

    if(ret_val != TM_OK) {
        local_irq_restore( flags ) ;
        goto _err_up_trident_gmacEth ;
    }

    ret_val = hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&Txfer);

    if(ret_val != TM_OK) {
        goto _err_up_trident_gmacEth ;
    }

    /* If the receive process is in suspended state, write to receive poll demand register
        ** to start reception
        */
    hwTRIDENTGMACEth_IntGetStatus(priv->hwUnitNum,&IntEn);

    Status = IntEn.dmaIntVal;

    if(((Status & HW_TRIDENTGMACETH_DMA_STATUS_RS_NO_RXDESC_VAL) >> 17) == 0x4) {
        hwTRIDENTGMACEth_DmaPollDesc(priv->hwUnitNum,hwTRIDENTGMACEth_Dir_Rx);
    }

    /* Set the link to up state. Used in timer for checking link status */
    priv->linkStatus = 1;

    /* Initialize timer to check the link status */
    priv->phy_timer->expires = jiffies + TIMEOUT_VALUE ;

    add_timer( priv->phy_timer) ;

    return(0);

_err_up_trident_gmacEth:

    Txfer.dirFlag = hwTRIDENTGMACEth_Dir_TxRx;
    Txfer.enFlag = hwTRIDENTGMACEth_Disable;

    /* error in hardware.. Try to disable the hardware and return an error */
    hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&Txfer);
    hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&Txfer);

    IntEn.dmaIntVal = DMA_MASK_ALL_INTS;
    IntEn.gmacIntVal = GMAC_INT_MASK_VAL;

    hwTRIDENTGMACEth_IntDisable( priv->hwUnitNum,&IntEn) ;

    GMAC_PRINT_ERR("Unable to setup the hardware, disabling the hardware and aborting\n");

    return -1 ;

}

/*--------------------------------------------------------------------------
 * FUNCTION: setup_phy:
 * DESCRIPTION: This function sets up the PHY hardware depending on the options
 * chosen and performs autonegotiation or forced usage of a particular speed and duplex mode
 * PARAMETERS:
 * dev - net_device structure for the device
 * RETURN:
 *  0 - success
 * -1 - Failure
 * NOTES:       None
 *--------------------------------------------------------------------------
 */

static __s32 setup_phy( struct net_device *dev )
{    
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;
    u16 bmcr,bmsr;
    int advert;
    tmErrorCode_t ret_val;
    unsigned long timeout;
    unsigned long try_count = AUTO_NEG_RETRY_COUNT;

    GMAC_PRINT_DBG("setup_phy called for %s\n", dev->name);

    /* Bring the PHY out of reset / do software reset / enable clocks to MAC */
    ret_val = gpPhyInterface[priv->hwUnitNum]->initFunc(priv->phy_addr_val);

    if(ret_val != TM_OK ) {
        GMAC_PRINT_ERR("Error in PHY H/W init,ERR:%x\n\n",(__u32)ret_val);
        ret_val = -ENXIO ;
    }

#if defined(CONFIG_GMAC_MODE_RGMII)
    priv->u_speed = LX_SPEED_1000;
    priv->u_mode = LX_MODE_FULL_DUPLEX;
#elif defined(CONFIG_GMAC_MODE_RMII)
    if(priv->u_autoneg == LX_AUTONEG_DISABLE){
	/*auto-negotiation disable ,we force to 100M Full*/
	return 0;
    }
start_Auto_Neg:
    /* Set Auto-Neg Advertisement capabilities to 10/100 half/full */
    Write_Phy_Reg(priv->phy_addr_val,MII_ADVERTISE,0x1E1);
    bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
    Write_Phy_Reg(priv->phy_addr_val,MII_BMCR,bmcr);

    timeout = round_jiffies(jiffies + TIMEOUT_VALUE);
    GMAC_PRINT_DBG("wait for PHY auto-negotiation to complete ");
    while(time_before(jiffies,timeout))
    {
	Read_Phy_Reg(priv->phy_addr_val,MII_BMSR,&bmsr);
	if (bmsr & BMSR_ANEGCOMPLETE) {
		break;
	}
	//try again after 1ms.
	udelay(1000);
    }    
    if (!(bmsr & BMSR_ANEGCOMPLETE)){
        GMAC_PRINT_DBG("TIME OUT!\n");
	if(try_count--){
		GMAC_PRINT_DBG("Try again ,");
		goto start_Auto_Neg;
	}
	else{
	//when failed, use 100M full by default.
		GMAC_PRINT_DBG("Auto-Neg failed. Use 100M-Full by default\n");
		return 0;
	}
    }
    else{
	GMAC_PRINT_DBG("done.\n");
    }

    
	Read_Phy_Reg(priv->phy_addr_val,MII_LPA,(pUInt16)&advert);
	GMAC_PRINT_DBG("Link Partner Ability = %x \n",advert);
	priv->u_speed = (advert & LPA_100)? SPEED_100:SPEED_10;
	priv->u_mode = (advert & (LPA_10FULL | LPA_100FULL))?DUPLEX_FULL:DUPLEX_HALF;
	/* ignore maxtxpkt, maxrxpkt for now */

#elif defined(CONFIG_GMAC_MODE_MII)
    /*
    * Now check the parameters that are stored in private structure and
    * accordingly configure the Phy and Mac
    */
#endif
	return 0;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION : setup_mac:
 *DESCRIPTION : This function sets up the MAC hardware depending on the options chosen
 *PARAMETERS:
 * dev - net_device structure for the device
 *RETURN:
 *  0 - success
 * -1 - Failure
 *NOTES: None
 */
/*--------------------------------------------------------------------------*/
static __s32 setup_mac( struct net_device *dev )
{
    trident_gmacEth_PRIV_t * 		priv = NETDEV_PRIV( dev ) ;
    hwTRIDENTGMACEth_DevConfig_t	eth_cfg ;
    hwTRIDENTGMACEth_DmaCfg_t dmaCfg;
    hwTRIDENTGMACEth_FlowCtrlConfig_t flowCtrl;
    hwTRIDENTGMACEth_Int_t IntDis;

    #if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC)
    hwTRIDENTGMACEth_MmcIntr_t MmcIntDis;
    #endif

    GMAC_PRINT_DBG("setup_mac called for %s\n", dev->name);

    /* Clear the structure */
    memset(&eth_cfg,0,sizeof(hwTRIDENTGMACEth_DevConfig_t));
    memset(&dmaCfg,0,sizeof(hwTRIDENTGMACEth_DmaCfg_t));
    memset(&flowCtrl,0,sizeof(hwTRIDENTGMACEth_FlowCtrlConfig_t));

    /* Do the ethernet MAC configuration */
    eth_cfg.autoPadCRC = True;
    eth_cfg.backOffLim = hwTRIDENTGMACEth_BackoffVal0;
    eth_cfg.clockSelect = priv->clk_csr_val;
    eth_cfg.ifg = hwTRIDENTGMACEth_IFG96bits;
    eth_cfg.phyAddress = priv->phy_addr_val;

    if(priv->enable_jumbo == 1U) {
        /* Disable jabber on tx & watchdog on rx, for jumbo frames */
        eth_cfg.jabberTimer = True;
        eth_cfg.wdTimer = True;
    }

    /* TODO: A new HwAPI API need to be called here to set the interface type */
    if(priv->hwUnitNum == 0)
        eth_cfg.miiSelect = GMAC0_MII_SEL;
    else
        eth_cfg.miiSelect = GMAC1_MII_SEL;

    /* After auto negotiation/PHY configuration, the values were stored in the Private data structure.
    ** Use those values and configure MAC
    */
    if( priv->u_mode == LX_MODE_FULL_DUPLEX ) {
        eth_cfg.duplexMode = True;
    }

    if( priv->u_speed == LX_SPEED_100 ) {
        eth_cfg.speed100Mbps = True;
    }

    eth_cfg.station.adrLow =  ( dev->dev_addr[0])     |
                              (dev->dev_addr[1] << 8) |
                              (dev->dev_addr[2] << 16)|
                              (dev->dev_addr[3] << 24);

    eth_cfg.station.adrHigh =(dev->dev_addr[4]) |
                             (dev->dev_addr[5] << 8);

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD
    /* Enables IPv4 checksum checking for received frame payloads,
    ** TCP/UDP/ICMP headers
    */
    eth_cfg.ipChecksumOffload = True;
#endif /* CONFIG_SIGMA_GMAC_CSUMOFFLOAD */

    hwTRIDENTGMACEth_SetConfig( priv->hwUnitNum, &eth_cfg);

    /* Set loopback, only after doing basic configuration */

    #ifdef __TRIDENT_GMAC_DEBUG__
    if(priv->enable_mac_loopback == 1U) {
        hwTRIDENTGMACEth_LpbkEnableDisable(priv->hwUnitNum,hwTRIDENTGMACEth_Enable);
    }
    #endif

   if( priv->u_flow_control == LX_FLOW_CONTROL_ENABLED ) {

        if( priv->u_rx_tx_fc & LX_TX_FLOW_CONTROL ) {
            flowCtrl.txFlowCtrlEn = True;

            /* Do the flow control configuration for TX  */
            flowCtrl.pauseTime = LX_PAUSE_TIMER_VALUE;
            flowCtrl.pauseLowThreshold = LX_4SLOT_TIMES;
        }

        if( priv->u_rx_tx_fc & LX_RX_FLOW_CONTROL ) {
            flowCtrl.rxFlowCtrlEn = True;

            /* For FIFO size greater than 4K */
            if(1 == ENABLE_HW_FLOW_CONTROL) {
                dmaCfg.hwFlowCtrlEn = True;
                dmaCfg.actRxThreshold = RFA_THRESHOLD;
                dmaCfg.deactRxThreshold = RFD_THRESHOLD;
            }

        }

        /* Flow control settings */
        hwTRIDENTGMACEth_FlowCtrlSetConfig(priv->hwUnitNum,&flowCtrl);

    }

    /* DMA transmits packets in store and forward mode */
    dmaCfg.storeNforwardEn = ETH_STORE_FWD_ENABLE;

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD

    /* For checksum offload, this is true always */
    dmaCfg.storeNforwardEn = True;

    /* When enableAltDescSize is set, DMA descriptor size is 32 bytes
    ** rather than 16 bytes
    */
    dmaCfg.enableAltDescSize = True;

#endif /*CONFIG_SIGMA_GMAC_CSUMOFFLOAD */

    dmaCfg.rxThreshold = ETH_RX_THRESHOLD;
    dmaCfg.txThreshold = ETH_TX_THRESHOLD;

    dmaCfg.disableFrameFlush  = ETH_DISABLE_FRAME_FLUSH;
    dmaCfg.fixedBurstEn = ETH_FIXED_BURST_ENABLE;

    dmaCfg.differentPBL = ETH_DIFF_PBL_ENABLE;
    dmaCfg.pBL4xmode = ETH_4XPBL_ENABLE;
    dmaCfg.burstLen = ETH_TX_PBL_VAL;
    dmaCfg.rxPBL = ETH_RX_PBL_VAL;

    dmaCfg.txSecondFrameEn = ETH_ENABLE_OSF;

    dmaCfg.dmaArbitration = ETH_DMA_ARBITRATION;
    dmaCfg.priority = ETH_DMA_PRIORITY;

    /* Set the base addresses in the DMA registers */
    dmaCfg.txDescListBaseAdr = priv->p_tx_descr;
    dmaCfg.rxDescListBaseAdr = priv->p_rx_descr;

    hwTRIDENTGMACEth_DmaConfig(priv->hwUnitNum,&dmaCfg);

    IntDis.dmaIntVal = DMA_MASK_ALL_INTS;
    IntDis.gmacIntVal = GMAC_INT_MASK_VAL;
    hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntDis);

   #if (TMFL_SD_ALL || TMFL_TRIDENTGMACETHSD_MMC)
    /* Disable MMC interrupts */
    MmcIntDis.dir = hwTRIDENTGMACEth_Dir_Tx;
    MmcIntDis.intrVal = 0x1FFFFFF;
    hwTRIDENTGMACEth_MMCIntDisable(priv->hwUnitNum,&MmcIntDis);

    MmcIntDis.dir = hwTRIDENTGMACEth_Dir_Rx;
    MmcIntDis.intrVal = 0xFFFFFF;
    MmcIntDis.rxCsumIntVal = 0x3FFF3FFF;
    hwTRIDENTGMACEth_MMCIntDisable(priv->hwUnitNum,&MmcIntDis);
   #endif

#ifdef TRIDENT_GMAC_VLAN_TAG
    /* Setup VLAN Tag without filtering */
    hwTRIDENTGMACEth_SetVLANTag(priv->hwUnitNum, 0);
#endif

    return 0 ;

}

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:	setup_filter_config:
 * DESCRIPTION:	This function sets up the filter configuration for the MAC
 * depending upon the options present in priv structure
 * PARAMETERS:
 * dev - net_device structure for the device
 * RETURN:
 * 0 - success
  * NOTES: This function utilizes all the perfect address filter registers in GMAC
  *             to program multicast addresses. If all the multicast addresses doesnot fit
  *             into these, it uses multicast hash table high/low registers.
 */
/*--------------------------------------------------------------------------*/

static __s32 setup_filter_config( struct net_device *dev )
{

    trident_gmacEth_PRIV_t *   priv = NETDEV_PRIV( dev ) ;
    hwTRIDENTGMACEth_HashFilter_t rx_hash_filter ;
    hwTRIDENTGMACEth_FilterConfig_t filterCfg;

    /* Default, the hw receives Unicast and broadcast packets */
    memset(&rx_hash_filter,0,sizeof(hwTRIDENTGMACEth_HashFilter_t));

    memset(&filterCfg,0,sizeof(hwTRIDENTGMACEth_FilterConfig_t));

    if( dev->flags & IFF_PROMISC ) {
        /* It is enabled to receive all packets. Just enable the promiscous mode and return */
        filterCfg.passAllFrames = True;
        /* Skip mulitcast frame filtering in reception */
        priv->u_all_multi = 1 ;
    } else if( dev->flags & IFF_ALLMULTI ) {
        /* allow all multicast packets to receive */
        filterCfg.recvAllMulticast = True;

        /* Skip mulitcast frame filtering in reception */
        priv->u_all_multi = 1;
    } else if( dev->flags & IFF_MULTICAST ) {
        /*
        *  Unless the driver sets this flag in dev->flags, the interface wont be asked to handle
        *  multicast packets.
        */

        /* Pass the received multicast frames via a  filter */
        priv->u_all_multi = 0 ;

        if( priv->u_mc_filter_type == LX_FILTER_TYPE_HW ) {
            /*
            * It is an hardware filter. So, just generate the hash value for the
            * multicast addresses and write into the hash filter register
            */
            perfect_hash_filter_config( dev, &rx_hash_filter ) ;

            /* If the Hash table registers have been programmed */
            if((0 != rx_hash_filter.hashFilterH) || (0 != rx_hash_filter.hashFilterL)) {
                filterCfg.hashMulticastEnable = True;
            }

        } else {
            /* Enable software filtering */
            priv->u_mc_filter_type = LX_FILTER_TYPE_SW;
        }

    }

    /* Write into filter control register */
    hwTRIDENTGMACEth_FilterSetConfig( priv->hwUnitNum,&filterCfg);

    return 0 ;

}

static __u32 crc_table[256];
static __s32 first =0;

static void gen_table(void)                /* build the crc table */
{
    __u32 crc, poly;
    __s32	i, j;

    poly = 0xEDB88320L;
    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 8; j > 0; j--)
            {
            if (crc & 1)
                crc = (crc >> 1) ^ poly;
            else
                crc >>= 1;
            }
        crc_table[i] = crc;
        }
}

/* Bitwise reversal of CRC obtained */
static __u32 reverse_crc(__u32 crc)
{
    __s32 i;
    __u32 val=0;

    for(i=0;i<32;i++) {
        if((crc & 0x1) > 0) {
            val |= 1 << (31-i);
        }

        crc >>=1;
    }

    return val;

}

__u32 get_crc32( unsigned char *fp,__s32 data_len)    /* calculate the crc value */
{
    register __u32 crc;
    __s32 i;

    crc = 0xFFFFFFFF;

    if(first == 0) {
        gen_table();
        first =1;
    }

    for(i=0;i<data_len; i++) {
        crc = (crc>>8) ^ crc_table[ (crc^(*fp++)) & 0xFF ];
    }


    crc ^=0xFFFFFFFF;
    crc = reverse_crc(crc);

    return(crc);

}

#if 0
/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:    generate_crc
 *DESCRIPTION:	This function generates the CRC using CRC-32 for Ethernet GMAC core
 *PARAMETERS:
 *	data - data for the crc calculation. Here it is 6 byte destination address
 *	data_len	- length of the data in bytes
 *RETURN:
 * 	CRC value
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static __s32 generate_crc(char *data, __s32 data_len)
{
     __s32 crc;

    crc = get_crc32(data,data_len);

    return crc;
}
#endif

/*--------------------------------------------------------------------------*/
/*
* FUNCTION:	perfect_hash_filter_config
* DESCRIPTION: This function programs the multicast addresses in the perfect address registers.
* If the multicast addresses exceed the perfect address registers, then hash table is programmed.
*
* PARAMETERS:
* dev - net_device structure for the device
* pRx_HashVal - Hash value to be written to hash table high and hash table low registers
* RETURN: None
* NOTES:
*/
/*--------------------------------------------------------------------------*/
static void perfect_hash_filter_config(
    struct net_device *dev,
    phwTRIDENTGMACEth_HashFilter_t pRx_HashVal
)
{
#if 0	
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev) ;
    __s32  crc ;
    u32 filtercnt;
    hwTRIDENTGMACEth_PerfectAdrConfig_t perAdrFilter;
//    struct dev_mc_list * p_mc_list = dev->mc_list ;
//    struct dev_mc_list * p_mc_list = dev->mc;
    struct netdev_hw_addr_list * p_mc_list = dev->mc;
//    __s32 mc_count = dev->mc_count ;
    __s32 mc_count = netdev_mc_count(dev); //dev->mc_count ;

    memset(&perAdrFilter,0,sizeof(hwTRIDENTGMACEth_PerfectAdrConfig_t));

    perAdrFilter.addressEnable = False;

    /* Disable all the perfect address filters. For (1-31) for an IP with 32 perfect address filters */
    for(filtercnt = 1; filtercnt < ETH_NUM_OF_PER_ADR_FILTERS;filtercnt++)
    {
        hwTRIDENTGMACEth_PerfectAdrSetConfig(priv->hwUnitNum,filtercnt,&perAdrFilter);
    }

    /* Set the enable flag again */
    perAdrFilter.addressEnable = True;

    for( filtercnt = 1; (( (mc_count > 0) && ( p_mc_list != NULL )) && (filtercnt < ETH_NUM_OF_PER_ADR_FILTERS)) ; \
            p_mc_list = p_mc_list->next, mc_count--,filtercnt++ )
    {

        /* Upper 2 bytes of address */
        perAdrFilter.macAddrHigh = (p_mc_list->dmi_addr[5] << 8) |
                                             (p_mc_list->dmi_addr[4]);

        /* Lower 4 bytes of address */
        perAdrFilter.macAddrlow = (p_mc_list->dmi_addr[3] << 24) |
                                            (p_mc_list->dmi_addr[2] << 16) |
                                            (p_mc_list->dmi_addr[1] << 8) |
                                            (p_mc_list->dmi_addr[0]);

        hwTRIDENTGMACEth_PerfectAdrSetConfig(priv->hwUnitNum,filtercnt,&perAdrFilter);

    }

    /* Program the remaining multicast address in the hash filter. Calculate the bit values */
    for( ;(mc_count > 0) && ( p_mc_list != NULL); p_mc_list = p_mc_list->next, mc_count--)
    {

        crc = generate_crc( &p_mc_list->dmi_addr[0], ETH_ALEN ) ;

        /* Upper most 6 bits are valid for hash filter */
        crc>>= 26;

        if( crc < 32 )
        {
            /* set the bit corresponding to crc value in to lsw */
             pRx_HashVal->hashFilterL |= ( 1 << crc ) ;
        }
        else
        {
            /* set the bit corresponding to crc value in to lsw */
            pRx_HashVal->hashFilterH |= ( 1 << crc ) ;
        }

    }

    /* Write the hash value into the two registers */
    hwTRIDENTGMACEth_HashFilterSetConfig(priv->hwUnitNum,pRx_HashVal);
#endif 
    return ;

}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	is_mc_filtered_packet
 *DESCRIPTION: This function checks if the given packet is in the list of multicast addresses
 * in the net_device structure.
 *
 *PARAMETERS:
 *	dev - net_device structure for the device
 *	idx - index to the descriptor from where the receive buffer is to be obtained
 *
 *RETURN:
 *	0	- if packet is not in the list of multicast address
 *	1	- if packet is in the list of multicast address
 *NOTES: This is a software method of filtering multicast frames
 *
 */
/*--------------------------------------------------------------------------*/

static __s32 is_mc_filtered_packet( struct net_device * dev, __u32 idx )
{
#if 0
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;
    struct ethhdr          * p_eth_hdr ;
    __s32				   mc_count ;
//    struct dev_mc_list * p_mc_list ;
    struct netdev_hw_addr_list * p_mc_list ;

    /*
    * search the multicast address list and find out whether the received packet
    * is one of the list of multicast addresses to be filtered out to submit kernel
    */

    /* Ethernet header is present in SKB->data */
    p_eth_hdr = ( struct ethhdr * ) (priv->p_vrx_skb_list[ idx ])->data ;
//    mc_count = dev->mc_count ;
    mc_count = netdev_mc_count(dev) ;
    p_mc_list = dev->mc;

    for( ; p_mc_list && mc_count > 0 ; p_mc_list = p_mc_list->next, --mc_count )
    {
		/* compare the source MAC addresses */
        if( memcmp( &p_mc_list->dmi_addr[0], p_eth_hdr->h_dest, ETH_ALEN ) == 0 )
        {
            /* yep! this address is on the list of multicast addresses to receive */
            return 1 ;
        }

    }
#endif 
    /* received multicast packet does not have address in the multicast filter list. return error */
    return 0 ;

}

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION: trident_gmacEth_open
 *
 * DESCRIPTION: 'open' driver handler for ethernet device. This function
 *                     will set up the hardware allocates memory and enables the hardware
 *                     for reception.
 * PARAMETERS:
 *  dev - net_device structure for the device
 *  RETURN:
 *  0 - success
 *  ENOMEM - memory allocation error
 *  ENXIO - IO error
 *  NOTES:   This function is called when the interface is brought up
 */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_open(struct net_device *dev)
{
    __s32 ret_val = -ENXIO ;
    hwTRIDENTGMACEth_EnTxfr_t    txfr;
    hwTRIDENTGMACEth_Int_t IntDis;
    trident_gmacEth_PRIV_t *priv = NETDEV_PRIV( dev ) ;

    GMAC_PRINT_DBG("trident_gmacEth_open called for %s\n", dev->name);

    /* Check that the MAC address is valid.  If its not, refuse
     * to bring the device up. The user must specify an
     * address using the following linux command:
     *      ifconfig eth0 hw ether xx:xx:xx:xx:xx:xx  */
    if (!is_valid_ether_addr(dev->dev_addr)) {
            random_ether_addr(dev->dev_addr);
            GMAC_PRINT_ERR("%s: generated random MAC address %pM\n", dev->name,
                    dev->dev_addr);
    }

#ifndef ALLOCATE_MEMORY_AT_INIT
    /*
    * Allocate memory buffer for TX, RX descriptor and allocate
    * buffers for all the descriptors
    *  in the receive side.
    */
    if( alloc_dma_descriptors( dev ) != 0 ) {
        GMAC_PRINT_ERR("Unable to allocate memory, aborting\n");
        return -ENOMEM ;
    }

    /* Initialize Rx descriptors, assign buffers to each rx descriptor.
    ** Since ring mode is used, the descriptors should be contiguous
    ** and the addresses should be aligned to bus width.
    */
    if( setup_dma_descriptors( priv ) != 0 ) {
        goto _err_free_mem ;
    }
#endif

    GMAC_PRINT_DBG("trident_gmacEth_open: setup_dma_descriptors Done\n");

    /* reset the stats structure */
    memset( &priv->stats, 0, sizeof( struct net_device_stats ) ) ;

    /* Bring up the interface */
    if( up_trident_gmacEth( dev ) != 0 ) {
        goto _err_free_intr ;
    }

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
        napi_enable(&priv->napiInfo.napi);
#endif

    //netif_carrier_on(dev);
    netif_start_queue( dev);

    GMAC_PRINT_DBG("trident_gmacEth_open: up_trident_gmacEth Done\n");

    /* Print the speed, duplex, flow control etc.. of successfull connection */
    GMAC_STAT_PRINT("%s Up : Speed: %s Mbps %s Duplex\n", dev->name,
               ((priv->u_speed == LX_SPEED_100) ? "100":((priv->u_speed == LX_SPEED_1000) ? "1000":"10")),
               ((priv->u_mode == LX_MODE_FULL_DUPLEX)? "Full": "Half") );

    return 0;

_err_free_intr :

    GMAC_PRINT_ERR("%s:unable to setup the hardware, aborting\n",__func__);

    hwTRIDENTGMACEth_IntClear( priv->hwUnitNum, DMA_CLR_ALL_INTS );

    IntDis.dmaIntVal = DMA_MASK_ALL_INTS;
    IntDis.gmacIntVal = GMAC_INT_MASK_VAL;
    hwTRIDENTGMACEth_IntDisable( priv->hwUnitNum,&IntDis) ;

    txfr.dirFlag = hwTRIDENTGMACEth_Dir_TxRx;
    txfr.enFlag = hwTRIDENTGMACEth_Disable;
    hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&txfr) ;
    hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&txfr) ;

    /* free the interrupt */
    free_irq( dev->irq, dev ) ;
#ifndef ALLOCATE_MEMORY_AT_INIT
_err_free_mem :

    /* free the memory */
    free_dma_descriptors( priv ) ;
#endif
    return ret_val ;

}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_stop
 *
 *DESCRIPTION:	'stop' driver handler for ethernet device. This function will
 *  disable the hardware frees the memory allocated
 *PARAMETERS:
 *  dev - net_device structure for the device
 *RETURN:
 *  0	- success
 *
 *NOTES: This function is called when the interface is brought down
 */
/*--------------------------------------------------------------------------*/
static __s32 trident_gmacEth_stop(struct net_device *dev)
{

    hwTRIDENTGMACEth_EnTxfr_t txfr;
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev);
    hwTRIDENTGMACEth_Int_t IntDis;

    GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);

    netif_stop_queue(dev);
    netif_carrier_off(dev);

    (void)down_trident_gmacEth(dev);

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    napi_disable(&priv->napiInfo.napi);
#endif

    /* disable the Rx/Tx and disable the interrupts */
    IntDis.dmaIntVal = DMA_MASK_ALL_INTS;
    IntDis.gmacIntVal = GMAC_INT_MASK_VAL;
    hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntDis);

    txfr.dirFlag = hwTRIDENTGMACEth_Dir_TxRx;
    txfr.enFlag = hwTRIDENTGMACEth_Disable;
    hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&txfr);
    hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&txfr);

    priv->dma_enabled = 0;

#ifndef ALLOCATE_MEMORY_AT_INIT
    /* free the memory */
    free_dma_descriptors( priv ) ;
#endif
    return 0 ;

}

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:    trident_gmacEth_hard_start_xmit
 *
 * DESCRIPTION: 'hard_start_xmit' driver handler for ethernet device. This
 *	function will add the given SKB into the TX descriptor list if it has
 *	a place for a TX packet. If TX descriptor ring is Full it returns a 1.
 *
 *PARAMETERS:
 *	skb	-packet to be transmitted
 *	dev	-net_device structure for the device
 *RETURN:
 *	   0 - success
 *      1 - If the packet is not successfully queued
 *
 *   NOTES : None
 */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    trident_gmacEth_PRIV_t * 	priv = NETDEV_PRIV(dev) ;
    hwTRIDENTGMACEth_EnTxfr_t txfer;
    __u32 EthStatus;
    hwTRIDENTGMACEth_Int_t IntEn;
    unsigned long flags;

    dev->trans_start = jiffies;

    GMAC_PRINT_INT("%s:%s\n",__func__, dev->name);

#ifdef __TRIDENT_GMAC_DEBUG__
    if((priv->enable_mac_loopback == 1U) ||
       (priv->enable_phy_loopback == 1U)) {
	man_arp_udp_data(skb,skb->len);
    }
#endif
	
    spin_lock_irqsave(&priv->tx_lock,flags);

    /* Check if the own bit is cleared for the current transmit index.
        ** Transmit index loops back after reaching the max descriptor length
        ** For transmit descriptor, only end of ring is set in setup_descriptors()
        */

    if( (0 == (priv->p_vtx_descr[priv->tx_produce_index].TDES0 &\
				    TXDESC_TDES0_OWN_VAL)) &&\
		    priv->tx_submit_count < HW_DESCR_QUEUE_LEN_TX ){

        /* Buffer pointer to the frame */
        priv->p_vtx_descr[priv->tx_produce_index].TDES2 =
                            dma_map_single( NULL, skb->data, skb->len, DMA_TO_DEVICE );

        /* Store the buffers transmitted, so that it can be freed once the
                ** transmit process is complete
                */
        priv->p_vtx_skb_list[priv->tx_produce_index] = skb;

        priv->tx_submit_count++;

        /* Clear the length field */
        priv->p_vtx_descr[priv->tx_produce_index].TDES1 &= TXDESC_TDES1_TX_BUF1_SIZE_CLR;

#ifdef __ENHANCED_DESCRIPTOR__
        priv->p_vtx_descr[priv->tx_produce_index].TDES0 |=
                                                        #ifndef CONTROL_INTR_FREQ
                                                        TXDESC_TDES0_INT_VAL |
                                                        #endif
                                                        TXDESC_TDES0_LASTSEG_VAL |
                                                        TXDESC_TDES0_FIRSTSEG_VAL;

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD
        if(skb->ip_summed == CHECKSUM_PARTIAL) {
            priv->p_vtx_descr[priv->tx_produce_index].TDES0 |= TXDESC_TDES0_CSUM_WITH_PSEUDO;
        } else {
            /* Clear CIC bits, as CSUM is not reqd. END OF RING is set in isr */
            priv->p_vtx_descr[priv->tx_produce_index].TDES0 &= TXDESC_TDES0_CIC_CLR;            
        }
#endif

        priv->p_vtx_descr[priv->tx_produce_index].TDES1 = skb->len;
        #else
        /*Enable interrupt after pkt txmn. This is the first and last segment of the frame.
                ** Set the length field passed from upper layers
                */
        priv->p_vtx_descr[priv->tx_produce_index].TDES1 |=
                                                        #ifndef CONTROL_INTR_FREQ
                                                        TXDESC_TDES1_INT_VAL |
                                                        #endif
                                                        TXDESC_TDES1_LASTSEG_VAL |
                                                        TXDESC_TDES1_FIRSTSEG_VAL |
                                                        skb->len;
#endif /* __ENHANCED_DESCRIPTOR__ */

        /* Set the own bit value as the last statement. Otherwise, there could be a race condition */
        priv->p_vtx_descr[priv->tx_produce_index].TDES0 |= TXDESC_TDES0_OWN_VAL;

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
        /* Interrupts to be enabled for transmit operation */
        IntEn.dmaIntVal = TX_INTR_VAL;

        IntEn.gmacIntVal =0;
        hwTRIDENTGMACEth_IntEnable(priv->hwUnitNum,&IntEn);

        /* Disable unwanted interrupts */
        IntEn.dmaIntVal = ERE_ETE_INT_VAL;
        IntEn.gmacIntVal = GMAC_INT_MASK_VAL;
        /* Disable unnecessary interrupts */
        hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntEn);
#endif
        /* If DMA is in suspended state, write to Transmit poll demand register
        ** Otherwise, enable GMAC and DMA
        */
        hwTRIDENTGMACEth_IntGetStatus(priv->hwUnitNum,&IntEn);
        EthStatus = IntEn.dmaIntVal;

        if( (EthStatus & HW_TRIDENTGMACETH_DMA_STATUS_TS_MSK) == HW_TRIDENTGMACETH_DMA_STATUS_TS_SUSP_VAL) {
            hwTRIDENTGMACEth_DmaPollDesc(priv->hwUnitNum, hwTRIDENTGMACEth_Dir_Tx);
        }

        if(priv->dma_enabled == 0) {
            txfer.dirFlag = hwTRIDENTGMACEth_Dir_Tx;
            txfer.enFlag = hwTRIDENTGMACEth_Enable;
            hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&txfer);
            hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&txfer);
            priv->dma_enabled = 1;
        }

       /* Get the next produce index to set to */
        if( priv->tx_produce_index >= ( HW_DESCR_QUEUE_LEN_TX - 1 )) {
            priv->tx_produce_index = 0 ;
        } else {
            priv->tx_produce_index++;
        }

    } else {
        GMAC_PRINT_DBG("trident_gmacEth_hard_start_xmit: Pkt Dropped");

        netif_stop_queue(dev);

        /* Drop the packet, keep the index constant */
        priv->stats.tx_dropped++ ;
        priv->counters.ullTxDroppedOnHardStart++;
    	spin_unlock_irqrestore(&priv->tx_lock,flags);
        return NETDEV_TX_BUSY;
    }

    spin_unlock_irqrestore(&priv->tx_lock,flags);
    return 0 ;

}

/*--------------------------------------------------------------------------
*
* FUNCTION:	trident_gmacEth_multicast_list
* DESCRIPTION: 'set_multicast_list' driver handler for ethernet device.
*                    This function will program the hardware for multicast filtering
*                    This function is called when there is a change in flags. Any corresponding
*                    action is to be taken in this function.
*
* PARAMETERS:
*	dev	- net_device structure for the device
* RETURN:	None
* NOTES:	None
*
*--------------------------------------------------------------------------
*/
#if 0
static void trident_gmacEth_multicast_list(struct net_device *dev)
{
	trident_gmacEth_PRIV_t *	priv = NETDEV_PRIV( dev ) ;
	unsigned long flags;
	/* lock the spinlock */
	spin_lock_irqsave( &priv->lock ,flags ) ;

	setup_filter_config( dev ) ;

	/* unlock the spinlock */
	spin_unlock_irqrestore( &priv->lock , flags) ;

	return ;
}

/*--------------------------------------------------------------------------*/
/*
 *  FUNCTION:	trident_gmacEth_set_mac_address
 *  DESCRIPTION:	'set_mac_address' driver handler for ethernet device. This
 *	function will assign the given MAC address to the hardware
 *  PARAMETERS:
 *	dev	- net_device structure for the device
 *	addr	-address of MAC
 *  RETURN:
 *	0	- success
 *	ENXIO	- IO error
 *  NOTES:		None
 */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_set_mac_address(struct net_device *dev, void *addr)
{
    struct  sockaddr *	hw_addr = ( struct  sockaddr * ) addr ;
		unsigned long flags;
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;

    /* lock the spinlock */
    spin_lock_irqsave( &priv->lock, flags );

    /* If the interface is not in running state */
    if (0 == netif_running(dev))
    {
        /* new MAC address will be valid only when the device starts next time */
        memcpy(dev->dev_addr, hw_addr->sa_data, dev->addr_len);
		spin_unlock_irqrestore( &priv->lock );
        return 0;
    }

    /* Inform upper layer that link is down */
    netif_stop_queue( dev );
    netif_carrier_off(dev);


    /* bring down the interface */
    down_trident_gmacEth( dev ) ;

    /* copy the MAC address onto dev structure */
    memcpy(dev->dev_addr, hw_addr->sa_data, dev->addr_len);

    /* bring up the interface */
    if( up_trident_gmacEth( dev ) != 0 )
    {
        goto _err_set_mac_address ;
    }

    /* start accepting the packets from n/w stack */
    netif_carrier_on(dev);
    netif_start_queue( dev );

    /* unlock the spinlock */
    spin_unlock_irqrestore( &priv->lock ,flags) ;

    return 0 ;

    _err_set_mac_address:
    GMAC_PRINT_ERR("Error in setting up the ethernet hardware\n") ;

    /* start accepting the packets from n/w stack */
    netif_carrier_on(dev);
    netif_start_queue( dev );

    /* unlock the spinlock */
    spin_unlock_irqrestore( &priv->lock ,flags) ;

    return -ENXIO ;

}

static int trident_gmacEth_change_mtu (struct net_device *dev, int new_mtu)
{

    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev) ;
		unsigned long flags;

    if (new_mtu > TRIDENT_GMAC_JUMBO_MTU)
        return -EINVAL;

    spin_lock_irqsave( &priv->lock ,flags) ;

    if (!netif_running(dev))
    {
        printk(KERN_INFO "%s:not up",dev->name);
        /* If device is not in running state, return */
        spin_unlock_irqrestore( &priv->lock ,flags) ;
        return 0;
    }

    netif_stop_queue( dev );

    netif_carrier_off(dev);

    /* Delay to close the transactions */
    mdelay(10);

    /* bring down the interface */
    down_trident_gmacEth(dev) ;

    if (new_mtu > ETH_DATA_LEN)
    {

        if (priv->enable_jumbo == 0U)
        {
            printk(KERN_INFO "%s: Enabling Jumbo frame "
                             "support\n", dev->name);

            priv->enable_jumbo = 1U;

        }
        else
        {
            /* Change in jumbo frame size ? */
        }

    }
    else
    {
        /* Standard size of 1500 bytes or less */
        priv->enable_jumbo = 0U;
    }

    /* Reallocate buffers only if jumbo frame size is greater than
    ** previous value OR there is a switch from jumbo to normal frames
    */
    if( (( new_mtu > dev->mtu ) && (priv->enable_jumbo == 1U)) ||
        (( dev->mtu > ETH_DATA_LEN ) && (priv->enable_jumbo == 0)))
    {

        free_dma_descriptors(priv);

        /* Receive buffer size should be multiple of bus width (4,8,16 - bytes)
        ** Round off to next size, which is divisible by bus width
        */
        priv->u_rx_buf_size = new_mtu;

        if ( (new_mtu & (~DMA_DESC_ALIGNMENT)) != new_mtu )
        {
            priv->u_rx_buf_size = (new_mtu & (~DMA_DESC_ALIGNMENT)) +
                                   DMA_DESC_ALIGNMENT +1;
        }

        /* Allocate receive buffers, with new size */
        alloc_dma_descriptors(priv);

        /* Assign the new buffers to DMA descriptors */
        setup_dma_descriptors(priv);

    }

    /* Let stack know the new MTU */
    dev->mtu = new_mtu;

    /* bring up the interface */
    if( up_trident_gmacEth( dev ) != 0 ) {

    }

    /* start accepting the packets */
    netif_carrier_on(dev);
    netif_start_queue( dev ) ;

    /* unlock the spinlock */
    spin_unlock_irqrestore( &priv->lock ,flags) ;

    return 0;
}
#endif

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:	trident_gmacEth_do_ioctl
 * DESCRIPTION:	'do_ioctl' driver handler for ethernet device. It is used for the following two
 * purposes:
 *                             1. To set the ethernet core to power down mode
 *                             2. To access registers of MAC for debug purposes (Compile option)
 *
 * PARAMETERS:
 *	dev	- net_device structure for the device
 *	ifr	-interface structure
 *	cmd	-IOCTL command
 * RETURN:
 *	 0	- success, -1 on Error
 *
 * NOTES:		None
 */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_do_ioctl(struct net_device *dev, struct ifreq *ifr, __s32 cmd)
{

	__s32 rc = 0;
	__u16 val;
	trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;
	struct mii_ioctl_data * mii_data = if_mii(ifr);
	GMAC_PRINT_DBG("%s %d\n", __func__,__LINE__);
	switch(cmd) {
		case SIOCGMIIPHY:
			break;

		case SIOCGMIIREG:
			GMAC_PRINT_DBG("%s %d\n", __func__,__LINE__);
			GMAC_PRINT_DBG("reg [0x%x] = 0x%x\n", mii_data->reg_num,mii_data->val_in);
			Read_Phy_Reg(priv->phy_addr_val,mii_data->reg_num,&mii_data->val_out);
			break;

		case SIOCSMIIREG: 
			GMAC_PRINT_DBG("%s %d\n", __func__,__LINE__);
			GMAC_PRINT_DBG("reg [0x%x] = 0x%x\n", mii_data->reg_num,mii_data->val_in);
			val = mii_data->val_in;
			Write_Phy_Reg(priv->phy_addr_val,mii_data->reg_num,val);
			break;
		default:
			rc = -EOPNOTSUPP;
			break;
		}

	return rc;

}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_tx_timeout_isr
 *DESCRIPTION:	'tx_timeout' watchdog isr for ethernet device.
 * This function schedules a thread for restarting the interface in case of transmission timeout
 * given  by macro 'TX_TIMEOUT'. Typically this value is (5*HZ)
 *
 *PARAMETERS:
 *	dev	- net_device structure for the device
 *
 *RETURN:	None
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/

static void trident_gmacEth_tx_timeout_isr(struct net_device *dev)
{
    __s32 ret_val;
    trident_gmacEth_PRIV_t *		priv = NETDEV_PRIV( dev ) ;

    /* Queue the work */
    ret_val = schedule_work(&priv->wdTimeoutTxWork.workq);

    if(0 == ret_val) {
        GMAC_PRINT_INT("WdTxTimeout Work Not Queued\n");
    }


}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_work_reset_link
 *DESCRIPTION:	'tx_timeout' driver handler for ethernet device.
 * This function restarts the interface in process context
 *
 *PARAMETERS:
 *	dev	- net_device structure for the device
 *
 *RETURN:	None
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/

static void trident_gmacEth_work_reset_link(struct work_struct *pWork)
{

    /* Parameters to container_of are as follows:
       1.Ptr to variable inside structure
       2.Parent structure type, in which (1) is embedded
       3.Exact variable name inside the structure. The pointer to this
         is in step 1.
       4. The result would be a pointer to the parent structure, where (1)
         is a member
    */

    ptrident_gmacEth_WorkQ_t pLippWorkq = container_of(pWork,trident_gmacEth_WorkQ_t,workq);

    struct net_device * dev = pLippWorkq->pNetDev;

    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev) ;
	hwTRIDENTGMACEth_PhyInfo_t phyInfo;

    __s32 ret_val =0;
    unsigned long flags;

    netif_stop_queue(dev);
    netif_carrier_off(dev);

    GMAC_PRINT_DBG("trident_gmacEth_work_reset_link: Start\n");

    spin_lock_irqsave(&priv->lock,flags);

    /* Bring down the interface */
    down_trident_gmacEth(dev);

    /* Bring back the interface again */
//    gpPhyInterface[priv->hwUnitNum]->softResetFunc(priv->phy_addr_val);
    phyInfo.clkCsrVal = priv->clk_csr_val;
    phyInfo.phyAddr = priv->phy_addr_val;

    hwTRIDENTGMACEth_Init(priv->hwUnitNum,&phyInfo, priv->hwUnitNum? GMAC1_MII_SEL:GMAC0_MII_SEL);
    /* If autonegotiation is enabled, negotiate
    ** to the highest speed & mode, when cable is unplugged & plugged back
    */
    priv->u_autoneg = ETH_AUTO_NEGOTIATION;
    priv->u_mode = ETH_LINK_MODE;
    ret_val = up_trident_gmacEth(dev);

    spin_unlock_irqrestore(&priv->lock,flags);

    if(ret_val < 0 ) {
        GMAC_PRINT_ERR("trident_gmacEth_work_reset_link: Bringing back interface failed\n");
       goto  _err_tx_timeout;
    }

    netif_carrier_on(dev);
    netif_start_queue(dev);

    /* For reset & autonegotiation. Dont allow wol ioctl until link is up & running */
    priv->linkStatus = 1;

    GMAC_PRINT_DBG("trident_gmacEth_work_reset_link: End\n");

    GMAC_STAT_PRINT("%s: Up Speed : %s Mbps %s Duplex\n", dev->name,
               ((priv->u_speed == LX_SPEED_100) ? "100":"10"),
               ((priv->u_mode == LX_MODE_FULL_DUPLEX)? "Full": "Half") );

    GMAC_PRINT_DBG("trident_gmacEth_work_reset_link: End\n");

    return ;

_err_tx_timeout:

    GMAC_PRINT_ERR("trident_gmacEth_work_reset_link: Error in setting up of hardware\n" );

    return ;

}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_get_stats
 *DESCRIPTION:	'get_stats' driver handler for ethernet device. This function
 *	returns the address of net_device_stats structure that contains the
 *	statistics for the ethernet device
 *PARAMETERS:
 *	dev	- net_device structure for the device
 *RETURN:	Address of the net_device_stats structre
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static struct net_device_stats* trident_gmacEth_get_stats(struct net_device *dev)
{
	trident_gmacEth_PRIV_t *	priv = NETDEV_PRIV(dev) ;

	/* return the address priv->stats structure that contain the counters */
	return &(priv->stats) ;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION: trident_gmacEth_timer
 *DESCRIPTION: This function will inform the kernel about the link status every
 *                   'TIMEOUT_VALUE' seconds. Typically (2*HZ) seconds
 *PARAMETERS:
 *      data	- net_device structure for the device
 *RETURN: None
 *NOTES:
 */
/*--------------------------------------------------------------------------*/
static void trident_gmacEth_timer( unsigned long data)
{
		
    struct net_device* dev = (struct net_device*)data;
    PhyEnableDisable_t curr_link_status;
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev);
		unsigned long flags;
		
    #ifdef __NULL_PHY__
        return; /* No LANR register present */
    #endif

    #ifdef __TRIDENT_GMAC_DEBUG__
    if(priv->enable_mac_loopback || priv->enable_phy_loopback) {
        return;
    }
    #endif/* __TRIDENT_GMAC_DEBUG__ */
    
    spin_lock_irqsave(&priv->lock,flags);

    /*This function checks whether the carrier is present for the phy or not and depending
    * upon the status of carrier, it informs kernel about the link
    */

    if(gpPhyInterface[priv->hwUnitNum]->getLinkStatusFunc(priv->phy_addr_val,
                                                          &curr_link_status)
                                                          != TM_OK ) {
	printk("curr_link_status = %x\n",curr_link_status );						  
        goto _err_timer;
    }
    /*
    * Now check the parameters that are stored in private structure and
    * accordingly configure the Phy and Mac
    */
    /* Common parameters. Enabling the mask disables the feature */
    if(PhyEnable == curr_link_status) {
        if(!netif_carrier_ok(dev)) {
            priv->linkStatus = 1;            
            /* Bring up the interface */
            if( up_trident_gmacEth( dev ) != 0 ) {
                GMAC_PRINT_ERR("Error in Timer up_trident_gmacEth");
                goto _err_timer;
            }
            
            netif_carrier_on(dev);
            netif_wake_queue(dev);
            GMAC_STAT_PRINT("%s : Link is Up\n",dev->name);

        }
    } else {
        if (netif_carrier_ok(dev)) {
            priv->linkStatus = 0;
            
            netif_stop_queue(dev);
            netif_carrier_off(dev);

            /* Bring down the interface */
            down_trident_gmacEth(dev);
            
            if(priv->wolFlag == 1) {
                priv->wolFlag = 3;
                wake_up_interruptible_all(&priv->waitQ);
            }
            
            GMAC_STAT_PRINT("%s: Link is down\n",dev->name );
        }
     }

    priv->phy_timer = &trident_gmacEth_timer_struct[priv->hwUnitNum];
    
    priv->phy_timer->expires = round_jiffies(jiffies + TIMEOUT_VALUE);
    
    mod_timer(priv->phy_timer,priv->phy_timer->expires);
    spin_unlock_irqrestore(&priv->lock,flags);

    return;

_err_timer :

    	spin_unlock_irqrestore(&priv->lock,flags);
        GMAC_PRINT_ERR("Link Status Error\n") ;

    return;

}

/*--------------------------------------------------------------------------*/
/*FUNCTION:	handle_receive_packets
 *DESCRIPTION: This function handles the received packets and if valid submits
 *	                 to the kernel
 *PARAMETERS:
 *	dev - net_device structure for the device
 *	pBudget - number of packets to be received (valid only if NAPI is enabled )
 *RETURN:
 *	0	- Success
 *	1	- Still some packets are present in queue to handle (valid only
 *		   if NAPI is enabled)
 *	-1	- error
 *NOTES: None
 */
/*--------------------------------------------------------------------------*/
static __s32 handle_receive_packets( struct net_device * dev, __s32 *pBudget )
{
    struct sk_buff * skb = NULL;
    __s32 frameSize=0;
    __s32 ret_val =0;
    __s32 bFoundErrors = 0;
    trident_gmacEth_PRIV_t*priv = NETDEV_PRIV(dev);
#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    __s32 npackets = 0 ;
#else
    unsigned long flags;
    __u32 max_rx_pkts =0;
    hwTRIDENTGMACEth_Int_t IntEn ={0,0};    
#endif

	(void) pBudget;
    /* If there is not enough free space, transmit a pause frame
    ** or enable back pressure.
    */
    if( check_n_enable_tx_flow_control( dev ) != 0 ) {
        goto _err_handle_receive_packets ;
    }

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    /* Interrupts to be disabled for receive operation */
    IntEn.dmaIntVal = HW_TRIDENTGMACETH_DMA_INT_RIE_EN_VAL;
    IntEn.gmacIntVal =0;
    hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntEn);
#endif
        while(1) {

            /* If the descriptor is not processed by DMA then break */
            if((priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_OWN_VAL) != 0) {

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
                if(max_rx_pkts >= MAX_RX_PKTS_TO_PROCESS) {
                    break;
                }
#endif
                break;
            }

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            if(*pBudget <=0) {   /* Met the quota, return */
                goto _return_from_napi;
            }            
#endif

            /* If the error summary bit is set, check for errors */
            if((priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_ERR_SUM_VAL) &&
               (priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_LAST_DESC_VAL)) {

                /* Check if Extended Status Available */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_EXTDSTAT_DAMATCH) {
                    /* If IP Payload checksum failed */
                    if(priv->p_vrx_descr[priv->rx_consume_index].RDES4 & RXDESC_RDES4_IPPAYLD_ERR) {
                        GMAC_PRINT_DBG("ERR RX IP Payload: Index %d\n",priv->rx_consume_index);
                        priv->stats.rx_crc_errors++ ;
                        priv->counters.ullRxIpPayloadError++ ;
                        bFoundErrors++;
                    }

                    /* If IP HDR checksum failed */
                    if(priv->p_vrx_descr[priv->rx_consume_index].RDES4 & RXDESC_RDES4_IPHDR_ERR) {
                        GMAC_PRINT_DBG("ERR RX IP Header: Index %d\n",priv->rx_consume_index);
                        priv->stats.rx_crc_errors++ ;
                        priv->counters.ullRxIpHeaderError++ ;
                        bFoundErrors++;
                    }
                }
                
                /* Check if the frame is truncated */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_DESC_ERR_VAL) {
                    GMAC_PRINT_DBG("ERR RX Descriptor: Index %d\n",priv->rx_consume_index);
                    priv->stats.rx_frame_errors++ ;
                    priv->counters.ullRxDescriptorError++ ;
                    bFoundErrors++;
                }

                /* Check if the received frame was damaged due to buffer overflow in MTL. */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_OVERFLOW_ERR_VAL) {
                    GMAC_PRINT_DBG("ERR RX Overflow: Index %d\n",priv->rx_consume_index);
                    priv->stats.rx_frame_errors++ ;
                    priv->counters.ullRxMTLOverflowError++ ;
                    bFoundErrors++;
                }

                /* Check if a late collision has occurred while receiving the frame in Half-Duplex mode */
		        if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_LATE_COL_VAL) {
                    GMAC_PRINT_DBG("ERR RX Late Collision: Index %d\n", priv->rx_consume_index);
                    priv->stats.rx_frame_errors++ ;
                    priv->counters.ullRxLateCollisionError++ ;
                    bFoundErrors++;
                }

                /* Check if Receive Watchdog Timer has expired while receiving the current frame */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_WDOG_VAL) {
                    GMAC_PRINT_DBG("ERR RX Watchdog Truncated: Index %d\n", priv->rx_consume_index);
                    priv->stats.rx_frame_errors++ ;
                    priv->counters.ullRxWdogTruncatedPktError++ ;
                    bFoundErrors++;
                }

                /* Check if gmii_rxer_i signal is asserted */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_RX_ERR_VAL) {
                    GMAC_PRINT_DBG("ERR RX GMII RXERR: Index %d\n", priv->rx_consume_index);
                    priv->stats.rx_frame_errors++ ;
                    priv->counters.ullRxGmiiRxError++ ;
                    bFoundErrors++;
                }

                /* Check if CRC Error occurred on the received frame */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_CRC_ERR_VAL) {
                    GMAC_PRINT_DBG("ERR RX CRC: Index %d\n", priv->rx_consume_index);
                    priv->stats.rx_crc_errors++ ;
                    priv->counters.ullRxCrcError++ ;
                    bFoundErrors++;
                }

            }

            /* Check the Time Stamp Bit -- note that this bit may be or'ed
             * into summary status  */
            if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_TS_IPC_GIANT_VAL) {
                //This is not an error -- IP configured as:
                //    ATS=1   (advanced timestamp feature is present)
                //    IPC2=1  (IPC checksum offload type 2 present)
                //
                //priv->stats.rx_crc_errors++ ;
                priv->counters.ullRxTimeStamped++ ;
            }


	    /* Check if the received frame has a non-integer multiple of bytes */
            if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_DRIBBLE_VAL) {
                GMAC_PRINT_DBG("ERR RX Dribble: Index %d\n", priv->rx_consume_index);
                priv->stats.rx_length_errors++ ;
                priv->counters.ullRxDribbleError++ ;
                bFoundErrors++;
            }

            /* Check if the actual length of the frame received and that the Length/Type field does not match*/
            if((priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_LEN_ERR_VAL) &&
               (!(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_FRM_TYP_VAL))) {
                GMAC_PRINT_DBG("ERR RX: Bad IEEE Length: Index %d\n", priv->rx_consume_index);
                priv->stats.rx_length_errors++ ;
                priv->counters.ullRxBadIeeeLengthError++ ;
                bFoundErrors++;
            }

	        /* Check if the SA field of frame failed the SA Filter in the GMAC Core */
            if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_SRC_ADR_FAIL_VAL) {
                GMAC_PRINT_DBG("ERR RX: SAF: Index %d\n", priv->rx_consume_index);
                priv->stats.rx_frame_errors++ ;
                priv->counters.ullRxFailedSAFilter++ ;
                bFoundErrors++;
            }

	        /* Check if frame failed in the DA Filter in the GMAC Core */
            if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_DST_ADR_FAIL_VAL) {
                //GMAC_PRINT_DBG("ERR RX DAF: Index %d\n", priv->rx_consume_index);
                priv->stats.rx_frame_errors++ ;
                priv->counters.ullRxFailedDAFilter++ ;
                bFoundErrors++;
            }

#ifdef TRIDENT_GMAC_VLAN_TAG
            if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_VLAN_TAG_VAL) {
               priv->counters.ullRxVlanFrame++ ;
            }            
#endif

            /* If first and last bit are set for the frame, it is a valid frame.
            ** Otherwise, discard the frame
            */
            if((priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_FIRST_DESC_VAL) &&
                (priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_LAST_DESC_VAL)) {

                /* Get the pointer to SKB */
                skb = priv->p_vrx_skb_list[priv->rx_consume_index];
                
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES2)
                    dma_unmap_single(NULL, priv->p_vrx_descr[priv->rx_consume_index].RDES2, priv->u_rx_buf_size, DMA_FROM_DEVICE);

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD

                /* Check if the frames passes the IP & TCP/UDP/ICMP checksum test */
                if(priv->p_vrx_descr[priv->rx_consume_index].RDES0 & RXDESC_RDES0_EXTDSTAT_DAMATCH) {

                    /* If IP checksum & TCP/UDP checksum is successful */
                    if((priv->p_vrx_descr[priv->rx_consume_index].RDES4 & TCPIP_CSUM_ERRCHK) == 0) {
                        /* Passed the checksum test done by IP, inform not repeat again in stack */
                        skb->ip_summed = CHECKSUM_UNNECESSARY;
                    }
                    else if(priv->p_vrx_descr[priv->rx_consume_index].RDES4 & RXDESC_RDES4_IPCSUM_BYPASS) {
                        /* Bypassed checksum test, inform stack to do the necessary processing */
                        skb->ip_summed = CHECKSUM_NONE;

                    } else {
                        GMAC_PRINT_DBG("RX ERR Cksum failed\n");
                        /* Drop the frame, as it failed checksum test */
                        priv->counters.ullRxIPChecksumError++ ;
                        bFoundErrors++;

                    }

                } else {/* Frame type value is 0 & other conditions, let kernel handle */ 
                    skb->ip_summed = CHECKSUM_NONE;
                }

#endif /* CONFIG_SIGMA_GMAC_CSUMOFFLOAD*/
            } else {
                GMAC_PRINT_INT("handle_receive_packets: Dropped Pkt..EOF not set\n");
                priv->counters.ullRxDroppedPacketFragment++;
                bFoundErrors++;
            }

            if(bFoundErrors) {
                goto _err_drop_packet ;
            }

            /* Submit the frame to the Kernel */
            frameSize = (priv->p_vrx_descr[priv->rx_consume_index].RDES0 &
                        RXDESC_RDES0_FRM_LEN_MSK) >> RXDESC_RDES0_FRM_LEN_POS;

#ifdef CONFIG_SIGMA_GMAC_CSUMOFFLOAD
            frameSize -=4;/* Exclude ethernet CRC */
#endif
            
            priv->p_vrx_skb_list[priv->rx_consume_index]=NULL;
            skb_put( skb,frameSize);            
#ifdef DEBUG_PACKET
	    printk("Receive packet begin\n");
	    print_packet(skb->data, skb->len);
	    printk("Reveive packet end\n");
#endif 

#ifndef CONFIG_SIGMA_GMAC_CSUMOFFLOAD
            skb->ip_summed = CHECKSUM_COMPLETE;
#endif /* CONFIG_SIGMA_GMAC_CSUMOFFLOAD */

            skb->len = frameSize;
            skb->dev = dev ;
            skb->protocol = eth_type_trans(skb,dev) ;

            if(skb->pkt_type == PACKET_MULTICAST) {
                if( priv->u_all_multi == 0) {
                    priv->stats.multicast++ ;
                    priv->counters.ullRxMulticast++;

                    if( priv->u_mc_filter_type == LX_FILTER_TYPE_SW ) {
                        /* Check the validity of the packet by filtering through software filtering */
                        if( is_mc_filtered_packet( dev, priv->rx_consume_index ) ) {
                            /* valid multicast packet, submit to kernel */
                        } else {
                            /* Invalid multicast packet, just drop it */
                            priv->counters.ullRxMulticastDropped++;
                            priv->stats.rx_dropped++ ;
                            goto _clear_rx_status;
                        }

                    }

                }

            }

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            /* Submit the packet to the kernel */
            ret_val = netif_receive_skb(skb);
            npackets++;
            (*pBudget)--;
            dev->last_rx = jiffies;
            priv->stats.rx_packets++ ;
            priv->counters.ullRxPackets++;
            priv->stats.rx_bytes += frameSize;
#else
            /* submit the packet to kernel */
            ret_val = netif_rx(skb);
            dev->last_rx = jiffies;
            priv->stats.rx_packets++ ;
            priv->counters.ullRxPackets++;
            priv->stats.rx_bytes += frameSize;
#endif  /* #ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI */

            /*
            * Now packet is submitted, just allocate a new SKB with maximum frame size and update
            * relevant descr, status and skb_list
            */
            skb = netdev_alloc_skb_ip_align(dev, priv->u_rx_buf_size);
            if( skb == NULL ) {
                /* error in memory allocation. set the descriptor to NULL. */
                priv->p_vrx_descr[priv->rx_consume_index ].RDES2 = 0 ;
                goto _err_drop_packet ;
            }

            /* Store the SKB in the rx SKB list array */
            priv->p_vrx_skb_list[priv->rx_consume_index] = skb;

            /* Assign buffer address to descriptor */
            priv->p_vrx_descr[priv->rx_consume_index].RDES2= \
                                          dma_map_single(NULL,
                                                         skb->data,
                                                         priv->u_rx_buf_size,
                                                         DMA_FROM_DEVICE);

            /* packet is submitted to kernel, now clear the status and continue processing
                          * remaining packets */
            goto _clear_rx_status ;

            /* invalid packet, drop the packet */
    _err_drop_packet :

            priv->stats.rx_errors++ ;
            priv->counters.ullRxTotalDropped++;

    _clear_rx_status :
            priv->p_vrx_descr[priv->rx_consume_index].RDES0 = RXDESC_RDES0_OWN_VAL;

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            spin_lock_irqsave(&priv->lock,flags);
#endif
            if(priv->rx_consume_index >= (HW_DESCR_QUEUE_LEN_RX-1)) {
                priv->rx_consume_index = 0;
            } else {
                priv->rx_consume_index++;
            }

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            spin_unlock_irqrestore(&priv->lock,flags);
            max_rx_pkts++;
#endif
        }

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
      /* Re-enable receive interrupts */
      IntEn.dmaIntVal = HW_TRIDENTGMACETH_DMA_INT_RIE_EN_VAL;
      IntEn.gmacIntVal =0;
      hwTRIDENTGMACEth_IntEnable(priv->hwUnitNum,&IntEn);
#endif

    /*
    * If TX flow control is enabled, just disable it as now enough space is present in
    * RX descriptor chain to receive packets
    */
	disable_tx_flow_control( dev ) ;

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
_return_from_napi:
    return npackets ;
#else
    return 0;
#endif /* CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI */

_err_handle_receive_packets :
	/*
	 * if TX flow control is enabled, just disable it as now enough space is present in
	 * RX descriptor chain to receive packets
	 */
	disable_tx_flow_control( dev ) ;

	return -1 ;

}


#ifdef __TRIDENT_GMAC_DEBUG__
/* Manipulate ARP & UDP data to run the UDP client/server application */
static void man_arp_udp_data(struct sk_buff * skb, __u32 framelen )
{
    __u8 *data = skb->data;
    __u16 proto;
    __u8 pcip[4];
    __u8 tgtip[4];
    __u8 pcmac[6] = {0xaa,0xbb,0xcc,0xdd,0xee,0xf0};
    __u8 tgtmac[6];

    proto = (*(data+12) << 8) | *(data+13);

    /* If ARP packet, create ARP reply */
    if(proto == 0x806) {
         /* Sent by this IP address */
        memcpy(tgtip,data+0x1C,4);

        /* Sent to this IP address */
        memcpy(pcip,data+0x26,4);

        /* Store tgt mac address */
        memcpy(tgtmac,data+6,6); /* 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff */

        /* Replace broadcast adr with tgtmac */
        memcpy(data,tgtmac,6);

        /* Replace the from address with pc mac  */
        memcpy(data+6,pcmac,6);

        /* Set code to ARP reply */
        *(data+0x15)=2;

        /* Sender is a pc now. Copy PC mac address  */
        memcpy(data+0x16,pcmac,6);

        /* Sender IP address : IP address of imaginary PC */
        memcpy(data+0x1c,pcip,4);

        /* Sent to Target ip mac & ip address */
        memcpy(data+0x20,tgtmac,6);
        memcpy(data+0x26,tgtip,4);

    }
    else if(proto == 0x800) {

        /* Swap the MAC addresses as well */
        memcpy(tgtmac,data,6);
        memcpy(data,data+6,6);
        memcpy(data+6,tgtmac,6);

        /* Just swap the IP address for UDP packets & Port number */
        memcpy(tgtip,data+0x1a,4);
        memcpy(data+0x1a,data+0x1e,4);
        memcpy(data+0x1e,tgtip,4);

        /* Server port = 9999 */
        *(skb->data+0x25) = 0x0F;

        /* Reset UDP checksum. Is there a sock api to do this ? */
        *(skb->data+0x28) =0;
        *(skb->data+0x29) =0;

    } else {
    }

}

#endif /* __TRIDENT_GMAC_LOOPBACK__ */


/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	handle_tx_packets
 *DESCRIPTION: This function handles the packets transmitted and frees the
 *	SKB of the transmitted packet
 *PARAMETERS:
 *	priv	- trident_gmacEth_PRIV_t structure
 *RETURN:	None
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static void handle_tx_packets( struct net_device * dev)
{

    trident_gmacEth_PRIV_t * 		priv = NETDEV_PRIV( dev ) ;
    TX_DESCR_t *		ptx_descr ;
    struct sk_buff *	skb ;
    unsigned long flags;
#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    __u32 max_tx_pkts =0;
    hwTRIDENTGMACEth_Int_t IntEn;
#endif
    spin_lock_irqsave(&priv->tx_lock,flags);

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    /* Interrupts to be disabled for transmit operation */
    IntEn.dmaIntVal = HW_TRIDENTGMACETH_DMA_INT_TIE_EN_VAL;
    IntEn.gmacIntVal =0;
    hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntEn);
#endif
    if(priv->tx_submit_count > MAX_TX_PKTS_TO_PROCESS)
	   printk("priv->tx_submit_count = 0x%x \n",priv->tx_submit_count);
    do {
/* Loop till the consume index is not equal to produce index AND own bit is zero
** When there are no packets to transmit, at that time, all own bits will be zero
** Compare against produce index also
*/

/* If OWN bit is set OR no packets were submitted for transmission, break */
        if((priv->p_vtx_descr[priv->tx_consume_index].TDES0 & TXDESC_TDES0_OWN_VAL) ||
            (priv->tx_submit_count == 0)
#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            )
#else            
            ||(max_tx_pkts >= MAX_TX_PKTS_TO_PROCESS))
#endif
        {
            break;
        }

        /* Decrement number of packets transmitted */
        if(priv->tx_submit_count > 0) {
#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            spin_lock_irqsave(&priv->lock,flags);
#endif
            priv->tx_submit_count--;
#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
            spin_unlock_irqrestore(&priv->lock,flags);
#endif
        } else {
            break;
        }

        ptx_descr = &(priv->p_vtx_descr[ priv->tx_consume_index]);

        /* Buffer transmitted */
        skb = priv->p_vtx_skb_list[priv->tx_consume_index];

        /* Check whether any error is present for transmitted frames by
        ** checking the status
        */
        if( ptx_descr->TDES0 & TXDESC_TDES0_ES_VAL) {
            //GMAC_PRINT_INT("handle_tx_packets:Tx Error\n");
            priv->stats.tx_errors++;
            priv->counters.ullTxError++;

            if( ptx_descr->TDES0 & TXDESC_TDES0_UNDERFLOW_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:Underflow Error\n");
                priv->counters.ullTxUnderflowError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_EXDEF_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:Excessive Deferral Error\n");
                priv->counters.ullTxExcessiveDeferralError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_EXCOL_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:Excessive Collision Error\n");
                priv->counters.ullTxExcessiveCollisionsError++;
            }

            if( (ptx_descr->TDES0 & TXDESC_TDES0_LATECOL_VAL) &&
                (!(ptx_descr->TDES0 & TXDESC_TDES0_UNDERFLOW_VAL)) ) {
                //GMAC_PRINT_INT("handle_tx_packets:Late Collision Error\n");
                priv->counters.ullTxLateCollisionError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_NOCAR_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:No Carrier Error\n");
                priv->counters.ullTxNoCarrierError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_LOSSOFCAR_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:Loss of Carrier Error\n");
                priv->counters.ullTxLossOfCarrierError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_IPPAYLD_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:IP Payload Error\n");
                priv->counters.ullTxIpPayloadError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_FRMFLUSH_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:Frame Flush Error\n");
                priv->counters.ullTxFrameFlushError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_JABTIMEOUT_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:Jabber Timeout Error\n");
                priv->counters.ullTxJabberTimeoutError++;
            }

            if( ptx_descr->TDES0 & TXDESC_TDES0_IHE_VAL) {
                //GMAC_PRINT_INT("handle_tx_packets:IP Header Error\n");
                priv->counters.ullTxIpHeaderError++;
            }

        } else {           
            /* packet is transmitted. Update the counter and free the skb */
            priv->stats.tx_packets++;
            priv->counters.ullTxPackets++;
            priv->stats.tx_bytes += ptx_descr->TDES1 & TXDESC_TDES1_TX_BUF1_SIZE_MSK ;
        }

        {
           __u32 colcnt = ((ptx_descr->TDES0 & TXDESC_TDES0_COLCNT_MSK) >>
                                       TXDESC_TDES0_COLCNT_POS) ;
           priv->stats.collisions += colcnt;
           priv->counters.ullTxCollisionError += colcnt;
        }

        if(ptx_descr->TDES2 != 0) {
            /* Buffer 1 address pointer */
            dma_unmap_single( NULL,ptx_descr->TDES2,skb->len,DMA_TO_DEVICE ) ;
        }

        if(skb != NULL) {
            dev_kfree_skb_any(skb);
        }

        priv->p_vtx_skb_list[priv->tx_consume_index] = NULL ;

        /* Dont clear TDES1 as it contains end of ring set */
        /* Clear the buffer1 pointer */
        ptx_descr->TDES2 =0;

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
        spin_lock_irqsave(&priv->lock,flags);
#endif
        /* Get the next descriptor index to be processed */
        if(priv->tx_consume_index  >= (HW_DESCR_QUEUE_LEN_TX-1)) {
            #ifdef __ENHANCED_DESCRIPTOR__
            /* Set the end of ring value again */
            ptx_descr->TDES0 |= TXDESC_TDES0_END_OF_RING_VAL;
            #endif /* __ENHANCED_DESCRIPTOR__ */

            priv->tx_consume_index = 0;
        } else {

            priv->tx_consume_index++;
        }
	GMAC_PRINT_DBG("priv->tx_consume_index=%d\n ",priv->tx_consume_index );
#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
        spin_unlock_irqrestore(&priv->lock,flags);
        max_tx_pkts++;
#endif
   }while( 1 ) ;

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    /* Re-enable transmit interrupts */
    IntEn.dmaIntVal = HW_TRIDENTGMACETH_DMA_INT_TIE_EN_VAL;
    IntEn.gmacIntVal =0;
    hwTRIDENTGMACEth_IntEnable(priv->hwUnitNum,&IntEn);
#endif

    /* Free space is available. Enable the tx queue */
	if(netif_queue_stopped(dev)) {
		netif_wake_queue(dev);
	}

    spin_unlock_irqrestore(&priv->tx_lock,flags);
    return ;

}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	check_n_enable_tx_flow_control
 *DESCRIPTION: This function checks if there are enough descriptors available for the DMA.
 * If the number of descriptors has fallen below the configured threshold, it activates
 * flow control mechanism.
 *
 *PARAMETERS:
 *	dev	- net_device structure for the device
 *RETURN:
 *	 0	 - Success
 *	-1	 - failure
 *NOTES: None
 */
/*--------------------------------------------------------------------------*/
static __s32 check_n_enable_tx_flow_control( struct net_device *dev )
{
    trident_gmacEth_PRIV_t *priv = NETDEV_PRIV( dev );
    __u32 probeIdx = 0;

    /* Check if flow control is enabled */
    if( ( priv->u_flow_control == 0 ) ||
        ((priv->u_rx_tx_fc & LX_TX_FLOW_CONTROL) == 0) ) {
        return 0 ;
    }

    /* If all the descriptors are full, enable tx Flow control.
    ** Check if the isr is too slow in processing & enable flow control
    ** If the own bit is not set in the descriptor pointing to probeIdx,
    ** enable flow control.
    */

    if(priv->rx_consume_index >= LX_DESC_GAP) {
        probeIdx = priv->rx_consume_index - LX_DESC_GAP;
    } else if ( priv->rx_consume_index < LX_DESC_GAP) {
        probeIdx = (HW_DESCR_QUEUE_LEN_RX - LX_DESC_GAP) + priv->rx_consume_index;
    }

    if( 0 == (priv->p_vrx_descr[probeIdx].RDES0 & RXDESC_RDES0_OWN_VAL)) {
        /* Enable flow control */
        hwTRIDENTGMACEth_FlowCtrlEnableDisable(priv->hwUnitNum, hwTRIDENTGMACEth_Enable);
    }

    return 0 ;

}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	disable_tx_flow_control
 *DESCRIPTION:	This function disables the TX flow control mechanism by disabling the
 *                       backpressure in half duplex operation.
 *PARAMETERS:
 *	dev	- net_device structure for the device
 *RETURN:
 *	0	- Success
 *	-1	- failure
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static __s32 disable_tx_flow_control( struct net_device *dev )
{
    trident_gmacEth_PRIV_t * priv = NETDEV_PRIV( dev ) ;

    if( ( priv->u_flow_control == 0 ) ||
        ((priv->u_rx_tx_fc & LX_TX_FLOW_CONTROL) == 0) ) {
		return 0 ;
    }

    /* The bit is self clearing in Full Duplex mode. For half duplex, explicitly clear the bit */

    if(LX_MODE_HALF_DUPLEX == priv->u_mode ) {
        hwTRIDENTGMACEth_FlowCtrlEnableDisable(priv->hwUnitNum,hwTRIDENTGMACEth_Disable);
    }

    return 0 ;

}

void trident_gmacEth_isr_poll(struct net_device* dev_id)
{

    struct net_device * dev = (struct net_device*) dev_id ;
    trident_gmacEth_PRIV_t *priv = NETDEV_PRIV(dev);
    __u32                          intr_status;
    hwTRIDENTGMACEth_Int_t   IntDis;
    __u32 wolStatus;
    hwTRIDENTGMACEth_WkupCfg_t wolCfg;
    hwTRIDENTGMACEth_EnTxfr_t txfer;

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    struct napi_struct * pNapi = &priv->napiInfo.napi;
#else
    /* In loop process all interrupts */
    while( 1 ) {
#endif	
        /* Read the interrupt status */
        hwTRIDENTGMACEth_IntGetStatus( priv->hwUnitNum, &IntDis);

        intr_status = IntDis.dmaIntVal;
        /* Check if there is update on necessary interrupts. If not break from loop  */
        hwTRIDENTGMACEth_IntClear(priv->hwUnitNum,intr_status);
        if((INT_STATUS_CHECK & intr_status) == 0) {
            return ;
        }

        /* Check if it is a power management interrupt */
        if(intr_status & HW_TRIDENTGMACETH_DMA_STATUS_GPI_VAL) {

            /* This is due to reception of wake up frame. Clear the status. Its read on clear reg */
            hwTRIDENTGMACEth_GetWakeupStatus(priv->hwUnitNum,(pUInt32)&wolStatus);
            wolCfg.globalUnicastEn = False;
            wolCfg.magicPktEn = False;
            wolCfg.wkupFrameEn = False;
            wolCfg.rstRegptr = False;

            /* Disable recognizing any further magic/wakeup frames received */
            hwTRIDENTGMACEth_WoLConfig(priv->hwUnitNum,&wolCfg);

            txfer.dirFlag = hwTRIDENTGMACEth_Dir_Tx;
            txfer.enFlag = hwTRIDENTGMACEth_Enable;

            /* Enable transmission again */
            hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&txfer);
            hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&txfer);

            netif_wake_queue(dev);
            netif_carrier_on(dev);
            priv->wolFlag = 2;
            wake_up_interruptible_all(&priv->waitQ);
            return ;

        }

        /* Due to MAC Management counters */
        if(intr_status & HW_TRIDENTGMACETH_DMA_STATUS_GMI_VAL) {
//            GMAC_PRINT_INT("MMC Int not supported by driver\n");
        }

        if(intr_status & HW_TRIDENTGMACETH_DMA_STATUS_FBI_VAL) {
  //          GMAC_PRINT_INT("ERROR: FATAL BUS ERROR Interrupt\n");
        }

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
        /* NAPI mode of operation */
        if(napi_schedule_prep(pNapi)) {
            // Mask all interrupts DMA & GMAC, even PMT interrupt 
            IntDis.dmaIntVal = DMA_MASK_ALL_INTS;
            IntDis.gmacIntVal = 0x20F;
            hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntDis);
        
            /* Schedule a polling routine */
            __napi_schedule(pNapi);
        }
#else
        /* Handle receive interrupts if  any */
        if( intr_status & ( HW_TRIDENTGMACETH_DMA_STATUS_OVF_VAL |
                            HW_TRIDENTGMACETH_DMA_STATUS_RI_VAL |
                            HW_TRIDENTGMACETH_DMA_STATUS_RU_VAL )) {

           /* Handle the received packets in interrupt mode */
            if( handle_receive_packets( dev, NULL ) ) {
                goto _err_isr ;
            }
        }

        if( intr_status & (HW_TRIDENTGMACETH_DMA_STATUS_UNF_VAL |
                           HW_TRIDENTGMACETH_DMA_STATUS_TI_VAL )) {
                handle_tx_packets(dev) ;
        }
    }
#endif

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
_err_isr :
    GMAC_PRINT_INT("Error in handling the interrupt for ethernet\n") ;
#endif

    return ;
}
/*--------------------------------------------------------------------------*/
/*
 * FUNCTION:	trident_gmacEth_isr
 * DESCRIPTION:	This function handles the interrupt events from GMAC hardware for
 *	the packets transmitted or received
 * PARAMETERS:
 *	irq	- interrupt number
 *	dev_id	- net_device structure for the device
 *	regs	- registers structure
 * RETURN:
 *	IRQ_HANDLED	- interrupt handled
 *	IRQ_NONE	- invalid interrupt
 * NOTES:	None
 */
/*--------------------------------------------------------------------------*/
irqreturn_t trident_gmacEth_isr( __s32 irq, void *dev_id)
{

    struct net_device * dev = (struct net_device*) dev_id ;
    trident_gmacEth_PRIV_t *priv = NETDEV_PRIV(dev);
    __u32                          intr_status;
    hwTRIDENTGMACEth_Int_t   IntDis;
    __u32 wolStatus;
    hwTRIDENTGMACEth_WkupCfg_t wolCfg;
    hwTRIDENTGMACEth_EnTxfr_t txfer;

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
    struct napi_struct * pNapi = &priv->napiInfo.napi;
#else
    /* In loop process all interrupts */
    while( 1 ) {
#endif	
        /* Read the interrupt status */
        hwTRIDENTGMACEth_IntGetStatus( priv->hwUnitNum, &IntDis);
        intr_status = IntDis.dmaIntVal;
        /* Check if there is update on necessary interrupts. If not break from loop  */
        hwTRIDENTGMACEth_IntClear(priv->hwUnitNum,intr_status);
        if((INT_STATUS_CHECK & intr_status) == 0) {
            return IRQ_HANDLED;
        }

        /* Check if it is a power management interrupt */
        if(intr_status & HW_TRIDENTGMACETH_DMA_STATUS_GPI_VAL) {
	/* This is due to reception of wake up frame.
	 * Clear the status. Its read on clear reg
	 */
            hwTRIDENTGMACEth_GetWakeupStatus(priv->hwUnitNum,(pUInt32)&wolStatus);
            wolCfg.globalUnicastEn = False;
            wolCfg.magicPktEn = False;
            wolCfg.wkupFrameEn = False;
            wolCfg.rstRegptr = False;

            /* Disable recognizing any further magic/wakeup frames received */
            hwTRIDENTGMACEth_WoLConfig(priv->hwUnitNum,&wolCfg);

            txfer.dirFlag = hwTRIDENTGMACEth_Dir_Tx;
            txfer.enFlag = hwTRIDENTGMACEth_Enable;

            /* Enable transmission again */
            hwTRIDENTGMACEth_GmacEnableDisable(priv->hwUnitNum,&txfer);
            hwTRIDENTGMACEth_DmaEnableDisable(priv->hwUnitNum,&txfer);

            netif_wake_queue(dev);
            netif_carrier_on(dev);
            priv->wolFlag = 2;
            wake_up_interruptible_all(&priv->waitQ);
            return IRQ_HANDLED ;

        }

        /* Due to MAC Management counters */
        if(intr_status & HW_TRIDENTGMACETH_DMA_STATUS_GMI_VAL) {
            GMAC_PRINT_INT("MMC Int not supported by driver\n");
        }

        if(intr_status & HW_TRIDENTGMACETH_DMA_STATUS_FBI_VAL) {
            GMAC_PRINT_INT("ERROR: FATAL BUS ERROR Interrupt\n");
        }

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
        /* NAPI mode of operation */
        if(napi_schedule_prep(pNapi)) {
            // Mask all interrupts DMA & GMAC, even PMT interrupt 
            IntDis.dmaIntVal = DMA_MASK_ALL_INTS;
            IntDis.gmacIntVal = 0x20F;
            hwTRIDENTGMACEth_IntDisable(priv->hwUnitNum,&IntDis);
        
            /* Schedule a polling routine */
            __napi_schedule(pNapi);
        }
#else
        /* Handle receive interrupts if  any */
        if( intr_status & ( HW_TRIDENTGMACETH_DMA_STATUS_OVF_VAL |
                            HW_TRIDENTGMACETH_DMA_STATUS_RI_VAL |
                            HW_TRIDENTGMACETH_DMA_STATUS_RU_VAL )) {

           /* Handle the received packets in interrupt mode */
            if( handle_receive_packets( dev, NULL ) ) {
                goto _err_isr ;
            }
        }

        if( intr_status & (HW_TRIDENTGMACETH_DMA_STATUS_UNF_VAL |
                           HW_TRIDENTGMACETH_DMA_STATUS_TI_VAL )) {
                handle_tx_packets(dev) ;
        }
    }
#endif

#ifndef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
_err_isr :
    GMAC_PRINT_INT("Error in handling the interrupt for ethernet\n") ;
#endif

    return IRQ_HANDLED ;
}

#ifdef TRIDENT_GMAC_VLAN_TAG
static void trident_gmaceth_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
    trident_gmacEth_PRIV_t*priv = netdev_priv(dev);
		unsigned long flags;
		
    spin_lock_irqsave(&priv->lock,flags);
    priv->vlgrp = grp;
    spin_lock_unirqrestore(&priv->lock,flags);

}
#endif /* TRIDENT_GMAC_VLAN_TAG */

#ifdef CONFIG_ETH_SIGMA_TRIX_GMAC_NAPI
static __s32 trident_gmacEth_napi(struct napi_struct *pNapi, __s32 budget)
{

    hwTRIDENTGMACEth_Int_t   IntDis;
    
    ptrident_gmacEth_Napi_t pNapiInfo = container_of(pNapi,trident_gmacEth_Napi_t,napi);
    struct net_device * dev = pNapiInfo->pDev;
    trident_gmacEth_PRIV_t *priv = NETDEV_PRIV(dev) ;
    __u32 intr_status;
    __u32 unitNum = priv->hwUnitNum;
    unsigned long flags;
    __s32 ret_val=0;

    /* Read the interrupt status & clear it */
    hwTRIDENTGMACEth_IntGetStatus( unitNum, &IntDis);
    intr_status = IntDis.dmaIntVal;
    hwTRIDENTGMACEth_IntClear(unitNum,intr_status);

    ret_val = handle_receive_packets(dev,&budget);

#ifdef TRIDENT_GMAC_LRO_SUPPORT
    if(ret_val == 0) {
        lro_flush_all(&priv->lro_mgr);
    }
#endif /* TRIDENT_GMAC_LRO_SUPPORT */

    handle_tx_packets(dev);

    if(ret_val < budget) {   
        /* Exit condition
        ** 1. Submitted budget no. frames. So stop polling. OR
        ** 2. There are no interrupts to process
        */

#ifdef TRIDENT_GMAC_LRO_SUPPORT
        lro_flush_all(&priv->lro_mgr);
#endif
        hwTRIDENTGMACEth_IntGetStatus( unitNum, &IntDis);

        spin_lock_irqsave(&priv->lock, flags);
        napi_complete(pNapi);

        /* Enable  interrupts, for next schedule */
        IntDis.dmaIntVal = TX_INTR_VAL | RX_INTR_VAL;
        IntDis.gmacIntVal = HW_TRIDENTGMACETH_PMT_INT_MSK_VAL;

        hwTRIDENTGMACEth_IntEnable(priv->hwUnitNum,&IntDis);  
        spin_unlock_irqrestore(&priv->lock,flags);

    }
    
    return ret_val;

}
#endif

#ifdef ENABLE_ETH_TOOL
/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_ethtool_get_settings
 *DESCRIPTION:	'get_settings" ethtool driver handler. This function returns
 *	the current settings of the link.
 *PARAMETERS:
 *	dev	- net_device structure
 *	cmd	- ethtool command and data
 *RETURN:
 *	0	- success
 *NOTES: This function is used for debug purposes
 */
/*--------------------------------------------------------------------------*/

static __s32 trident_gmacEth_ethtool_get_settings(struct net_device *dev,
                                               struct ethtool_cmd *ecmd)
{
	trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev) ;
	u16 bmcr, bmsr;
	u32 nego;
	u32 result = 0;
	int advert;

	GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);

	ecmd->supported =
	    (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
	     SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
	     SUPPORTED_Autoneg | SUPPORTED_TP | SUPPORTED_MII);
	
	/* only supports twisted-pair */
	ecmd->port = dev->if_port;

	/* only supports internal transceiver */
	ecmd->transceiver = XCVR_INTERNAL;

	/* this isn't fully supported at higher layers */
	ecmd->phy_address = priv->phy_addr_val;

	ecmd->advertising = ADVERTISED_TP | ADVERTISED_MII;

	Read_Phy_Reg(priv->phy_addr_val,MII_BMCR,&bmcr);
	GMAC_PRINT_DBG("MII_BMCR :  %x\n",bmcr);
	Read_Phy_Reg(priv->phy_addr_val,MII_BMSR,&bmsr);
	GMAC_PRINT_DBG("MII_BMSR :  %x\n",bmsr);

	if (bmcr & BMCR_ANENABLE) {
		GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
		ecmd->advertising |= ADVERTISED_Autoneg;
		ecmd->autoneg = AUTONEG_ENABLE;
		Read_Phy_Reg(priv->phy_addr_val,MII_ADVERTISE,(pUInt16)&advert);
		if (advert & LPA_LPACK)
			result |= ADVERTISED_Autoneg;
		if (advert & ADVERTISE_10HALF)
			result |= ADVERTISED_10baseT_Half;
		if (advert & ADVERTISE_10FULL)
			result |= ADVERTISED_10baseT_Full;
		if (advert & ADVERTISE_100HALF)
			result |= ADVERTISED_100baseT_Half;
		if (advert & ADVERTISE_100FULL)
			result |= ADVERTISED_100baseT_Full;
		ecmd->advertising |= result;
		if (bmsr & BMSR_ANEGCOMPLETE) {
			GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
			Read_Phy_Reg(priv->phy_addr_val,MII_LPA,(pUInt16)&advert);
			if (advert & LPA_LPACK)
				result |= ADVERTISED_Autoneg;
			if (advert & ADVERTISE_10HALF)
				result |= ADVERTISED_10baseT_Half;
			if (advert & ADVERTISE_10FULL)
				result |= ADVERTISED_10baseT_Full;
			if (advert & ADVERTISE_100HALF)
				result |= ADVERTISED_100baseT_Half;
			if (advert & ADVERTISE_100FULL)
				result |= ADVERTISED_100baseT_Full;
			ecmd->lp_advertising = result;
		} else {
			GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
			ecmd->lp_advertising = 0;
		}

		nego = ecmd->advertising & ecmd->lp_advertising;

		if (nego & (ADVERTISED_100baseT_Full |
			ADVERTISED_100baseT_Half)) {
			GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
			ecmd->speed = SPEED_100;
			ecmd->duplex = !!(nego & ADVERTISED_100baseT_Full);
		} else {
			GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
			ecmd->speed = SPEED_10;
			ecmd->duplex = !!(nego & ADVERTISED_10baseT_Full);
		}
	} else {
		GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
		ecmd->autoneg = AUTONEG_DISABLE;
		ecmd->speed = ((bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10);
		ecmd->duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
	}


	/* ignore maxtxpkt, maxrxpkt for now */

	return 0;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_ethtool_set_settings
 *DESCRIPTION:	'set_settings" ethtool driver handler. This function sets the
 *	link parameters
 *PARAMETERS:
 *	dev	- net_device structure
 *	cmd	- ethtool command and data
 *RETURN:
 *	0	- success
 *	ENXIO	- IO error
 *NOTES: This function is used for debug purposes
 */
/*--------------------------------------------------------------------------*/
static __s32 trident_gmacEth_ethtool_set_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
	u16 bmcr, advert, tmp;
	trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev) ;
	GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);
	
	if (ecmd->speed != SPEED_10 &&
	    ecmd->speed != SPEED_100)
		return -EINVAL;
	if (ecmd->duplex != DUPLEX_HALF && ecmd->duplex != DUPLEX_FULL)
		return -EINVAL;
	if (ecmd->port != PORT_MII)
		return -EINVAL;
	if (ecmd->transceiver != XCVR_INTERNAL)
		return -EINVAL;
	if (ecmd->phy_address != priv->phy_addr_val )
		return -EINVAL;
	if (ecmd->autoneg != AUTONEG_DISABLE && ecmd->autoneg != AUTONEG_ENABLE)
		return -EINVAL;

	/* ignore supported, maxtxpkt, maxrxpkt */

	if (ecmd->autoneg == AUTONEG_ENABLE) {
		/* advertise only what has been requested */
		Read_Phy_Reg(priv->phy_addr_val,MII_ADVERTISE,&advert);
		GMAC_PRINT_DBG("%s called  %d\n",__func__, __LINE__);
		tmp = advert & ~(ADVERTISE_ALL | ADVERTISE_100BASE4);
		if (ecmd->advertising & ADVERTISED_10baseT_Half)
			tmp |= ADVERTISE_10HALF;
		if (ecmd->advertising & ADVERTISED_10baseT_Full)
			tmp |= ADVERTISE_10FULL;
		if (ecmd->advertising & ADVERTISED_100baseT_Half)
			tmp |= ADVERTISE_100HALF;
		if (ecmd->advertising & ADVERTISED_100baseT_Full)
			tmp |= ADVERTISE_100FULL;
		if (advert != tmp) {
			Write_Phy_Reg(priv->phy_addr_val,MII_ADVERTISE, tmp & 0xffff);
		}

		/* turn on autonegotiation, and force a renegotiate */
		Read_Phy_Reg(priv->phy_addr_val,MII_BMCR,&bmcr);
		bmcr |= (BMCR_ANENABLE | BMCR_ANRESTART);
		Write_Phy_Reg(priv->phy_addr_val,MII_BMCR, bmcr & 0xffff);

	} else {
		/* turn off auto negotiation, set speed and duplexity */
		Read_Phy_Reg(priv->phy_addr_val,MII_BMCR,&bmcr);
		GMAC_PRINT_DBG("MII_BMCR :  %x\n",bmcr);
		tmp = bmcr & ~(BMCR_ANENABLE | BMCR_SPEED100 |
			       BMCR_FULLDPLX);
		if (ecmd->speed == SPEED_100)
			tmp |= BMCR_SPEED100;
		if (ecmd->duplex == DUPLEX_FULL) 
			tmp |= BMCR_FULLDPLX;
		GMAC_PRINT_DBG("%s called  %x\n",__func__, __LINE__);
		GMAC_PRINT_DBG("tmp :  %x\n",tmp);
		if (bmcr != tmp)
			Write_Phy_Reg(priv->phy_addr_val,MII_BMCR, tmp & 0xffff );
		Read_Phy_Reg(priv->phy_addr_val,MII_BMCR,&bmcr);
		GMAC_PRINT_DBG("MII_BMCR :  %x\n",bmcr);
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_ethtool_get_drvinfo
 *DESCRIPTION:	'get_drvinfo" ethtool driver handler. This function returns
 *	the driver information
 *PARAMETERS:
 *	dev	- net_device structure
 *	cmd	- ethtool command and data
 *RETURN:	None
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/

static void trident_gmacEth_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *drvinfo)
{
    GMAC_PRINT_DBG("trident_gmacEth_ethtool_get_drvinfo called for %s\n", dev->name);

	strcpy(drvinfo->driver,  DRV_NAME);
	strcpy(drvinfo->version, DRV_VERSION) ;
	drvinfo->fw_version[0] = '\0';
	drvinfo->n_stats = 0 ;
	drvinfo->testinfo_len = 0 ;
	drvinfo->regdump_len = 0 ;
	drvinfo->eedump_len = 0 ;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_ethtool_get_link_status
 *DESCRIPTION:	Returns the link status
 *PARAMETERS:
 *	dev	- net_device structure
 *RETURN:	1 if ethernet link is up, 0 if ethernet link is down
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/

static __u32 trident_gmacEth_ethtool_get_link_status(struct net_device *dev)
{
    trident_gmacEth_PRIV_t *    priv ;
    UInt16 lstatus = 0;

	GMAC_PRINT_DBG("%s for %s\n",__func__,dev->name);

    priv = netdev_priv(dev);
    Read_Phy_Reg(priv->phy_addr_val,MII_BMSR,&lstatus);
    if( lstatus & BMSR_LSTATUS )
		return 1;
    else 
		return 0;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_ethtool_get_pauseparam
 *DESCRIPTION:	'get_pauseparam" ethtool driver handler. This function returns
 * the flow control parameters to the caller
 *PARAMETERS:
 * dev	- net_device structure
 * pauseparam	- ethtool flow control strucutre
 *RETURN:	None
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static void trident_gmacEth_ethtool_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam* pauseparam)
{
	/* - need to implement the flow control */
	trident_gmacEth_PRIV_t *		priv = NETDEV_PRIV( dev ) ;

	GMAC_PRINT_DBG("%s called for %s\n", __func__,dev->name);

	pauseparam->autoneg = priv->u_autoneg ;
	pauseparam->tx_pause = priv->u_flow_control == ( LX_FLOW_CONTROL_ENABLED ) && ( priv->u_rx_tx_fc & LX_TX_FLOW_CONTROL ) ;
	pauseparam->rx_pause = priv->u_flow_control == ( LX_FLOW_CONTROL_ENABLED ) && ( priv->u_rx_tx_fc & LX_RX_FLOW_CONTROL ) ;
	return ;
}

/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:    trident_gmacEth_ethtool_set_pauseparam
 *DESCRIPTION: 'set_pauseparam" ethtool driver handler. This function sets
 *	the flow control parameters to the hardware
 *PARAMETERS:
 *	dev	- net_device structure
 *	pauseparam	- ethtool flow control strucutre
 *RETURN:
 *	0	- success
 *	-ENXIO	- IO error
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/
static __s32 trident_gmacEth_ethtool_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam* pauseparam)
{

    trident_gmacEth_PRIV_t *		priv = NETDEV_PRIV( dev ) ;
		unsigned long flags;
		
    GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);
		
    /* lock the spinlock */
    spin_lock_irqsave( &priv->lock ,flags) ;

    priv->u_autoneg = pauseparam->autoneg ;

    if( priv->u_autoneg == LX_AUTONEG_ENABLE ) {
        priv->u_flow_control = LX_FLOW_CONTROL_ENABLED ;
    }

    priv->u_rx_tx_fc |= ( pauseparam->tx_pause ) ? LX_TX_FLOW_CONTROL : 0 ;
    priv->u_rx_tx_fc |= ( pauseparam->rx_pause ) ? LX_RX_FLOW_CONTROL : 0 ;

    if( priv->u_rx_tx_fc ) {
        priv->u_flow_control = LX_FLOW_CONTROL_ENABLED ;
    }

    if (!netif_running(dev)) {
        /* new settings will be used when device starts netxt time */
        /* unlock the spinlock */
        spin_unlock_irqrestore( &priv->lock ,flags) ;
        return 0;
    }

    netif_stop_queue( dev );
    netif_carrier_off(dev);
    /* bring down the interface */
    down_trident_gmacEth( dev ) ;

    /* bring up the interface */
    if( up_trident_gmacEth( dev ) != 0 ) {
        goto _err_set_pauseparam ;
    }


    /* start accepting the packets */
    netif_carrier_on(dev);
    netif_start_queue( dev ) ;

    /* unlock the spinlock */
    spin_unlock_irqrestore( &priv->lock ,flags) ;

    return 0 ;

    _err_set_pauseparam:
    GMAC_PRINT_ERR("Error in setting up the ethernet hardware\n") ;

    /* start accepting the packets from n/w stack */
    netif_carrier_on(dev);
    netif_start_queue( dev );

    /* unlock the spinlock */
    spin_unlock_irqrestore( &priv->lock ,flags) ;

    return -ENXIO ;

}

#ifdef CONFIG_TRIDENT_GMAC_CSUMOFFLOAD
/*--------------------------------------------------------------------------*/
/*
 *FUNCTION:	trident_gmacEth_ethtool_get_rx_csum
 *DESCRIPTION:	Returns the rx csum
 *PARAMETERS:
 *	dev	- net_device structure
 *RETURN:	1 if ethernet rx csum is enable, 0 if ethernet rx csum is disabled
 *NOTES:	None
 */
/*--------------------------------------------------------------------------*/

#if 0
static __u32 trident_gmacEth_ethtool_get_rx_csum(struct net_device *dev)
{
    GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);

    return 1;
}
#endif

#endif
#endif

#ifdef CONFIG_PM
static int trident_gmacEth_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct net_device * dev = dev_get_drvdata(&pdev->dev);
	trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev) ;
	unsigned long flags;

	GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);
    
	if (!dev || !netif_running(dev))
    	return 0;
    
    spin_lock_irqsave(&priv->lock,flags);

	if (state.event == PM_EVENT_SUSPEND) {
		netif_device_detach(dev);

		napi_disable(&priv->napiInfo.napi);

		/* Inform upper layer that link is down */
		netif_stop_queue( dev );
		netif_carrier_off(dev);
#if 0                
		/* Power down PHY here. PHY is not accessible, if GMAC is disabled
		** in next step
		*/
		
		gpPhyInterface[priv->hwUnitNum]->setPowerStateFunc(priv->phy_addr_val,tmPowerOff);
#endif 
		/*Below operation also disables GMAC & DMA */
		hwTRIDENTGMACEth_SetPowerState(priv->hwUnitNum,tmPowerOff);

		/* Free all the tx & rx buffers,DMA descriptors, deletes the PHY timer,
		** disables GMAC, GMAC DMA
		*/
		(void)down_trident_gmacEth(dev);

	} else {
		priv->shutdown = 1;
		trident_gmacEth_stop(dev);
	}

	spin_unlock_irqrestore(&priv->lock,flags);

	return 0;

}

static int trident_gmacEth_resume(struct platform_device *pdev)
{
	struct net_device * dev = dev_get_drvdata(&pdev->dev);
	trident_gmacEth_PRIV_t * priv = NETDEV_PRIV(dev);
	hwTRIDENTGMACEth_PhyInfo_t phyInfo;
	unsigned long flags, IntrMask;

	GMAC_PRINT_DBG("%s called for %s\n",__func__, dev->name);

	MWriteRegByte((volatile void*)0x1500ec17, 0x8, 0x8); //bit3 = 1 enable REFCLK for sx6 revb,50MHz clock output

	 IntrMask = ReadRegWord((volatile void*)INTERRUPTCTL_REG);
	 IntrMask |= 0x40; 	
	 WriteRegWord((volatile void *)INTERRUPTCTL_REG, IntrMask );
	
	if (!netif_running(dev))
		return 0;

	spin_lock_irqsave(&priv->lock,flags);

    if(priv->shutdown) {
        trident_gmacEth_open(dev);
        goto out_resume;
    }

    netif_device_attach(dev);

	phyInfo.clkCsrVal = priv->clk_csr_val;
	phyInfo.phyAddr = priv->phy_addr_val;

	/* Reset MAC. Resetting will bring the MAC out of power down.
	** PHY is also reset in setup_phy, to come out of powerdown mode
	*/
	hwTRIDENTGMACEth_Init(priv->hwUnitNum,&phyInfo, priv->hwUnitNum? GMAC1_MII_SEL:GMAC0_MII_SEL);

	/* Allocates DMA descriptors, receive buffers, adds the PHY timer,
	** enables GMAC, GMAC DMA
	*/
	if( up_trident_gmacEth(dev) != 0 ) {
		goto _err_trident_gmacEth_resume;
	}

    napi_enable(&priv->napiInfo.napi);

	/* start accepting the packets from n/w stack */
	netif_carrier_on(dev);
	netif_start_queue( dev );

out_resume:
	spin_unlock_irqrestore(&priv->lock,flags);
	return 0;

_err_trident_gmacEth_resume:
	spin_unlock_irqrestore(&priv->lock,flags);
	return 1;
}
#endif  /* CONFIG_PM */

__setup("phyaddr=",set_phy_addr);
module_init(trident_gmacEth_init_module);
module_exit(trident_gmacEth_cleanup_module);


