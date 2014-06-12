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
 * %filename:           mmc_stats.h  %
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

#ifndef _MMC_STATS_H_
#define _MMC_STATS_H_

#ifndef __ASSEMBLY__
#include <linux/types.h>
#endif

typedef struct trident_gmacEth_RmonCnt
{
    /* Legend: <type><class><tx/rx><name>
               type:
                     F - frame counter
                     B - byte counter
               class:
                     GG - bytes or frames from good packets only
                     BB - bytes or frames from bad/aborted packets only
                     GB - bytes or frames from both good and bad packets
                     GA - bytes or frames from packets after indicated error
               txrx:
                     TX - Transmit interface
                     RX - Receive interface
               name:
                     textual name for this counter
    */
                                       /* ADDR bb name */
    __u64 uB_GB_TX;                    /* 0114 00 txoctetcount_gb */
    __u64 uB_GG_TX;                    /* 0164 20 txoctetcount_g  */
    __u64 uF_GB_TX;                    /* 0118 01 txframecount_gb */
    __u64 uF_GG_TX;                    /* 0168 21 txframecount_g  */
    __u64 uF_GB_TX_Broadcast;          /* 0144 12 txbroadcastframes_gb */
    __u64 uF_GG_TX_Broadcast;          /* 011C 02 txbroadcastframes_g  */
    __u64 uF_GB_TX_Multicast;          /* 0140 11 txmulticastframes_gb */
    __u64 uF_GG_TX_Multicast;          /* 0120 03 txmulticastframes_g  */
    __u64 uB_GB_TX_0to64;              /* 0124 04 tx64octets_gb        */
    __u64 uB_GB_TX_65to127;            /* 0128 05 tx65to127octets_gb   */
    __u64 uB_GB_TX_128to255;           /* 012C 06 tx128to255octets_gb  */
    __u64 uB_GB_TX_256to511;           /* 0130 07 tx256to511octets_gb  */
    __u64 uB_GB_TX_512to1023;          /* 0134 08 tx512to1023octets_gb */
    __u64 uB_GB_TX_1024toMax;          /* 0138 09 tx1024tomaxoctets_gb */
    __u64 uF_GB_TX_Unicast;            /* 013C 10 txunicastframes_gb  */
    __u64 uF_GG_TX_Vlan;               /* 0174 24 txvlanframes_g */
    __u64 uF_GG_TX_Pause;              /* 0170 23 txpauseframes  */
    __u64 uF_BB_TX_Underflow;          /* 0148 13 txunderflowerror */
    __u64 uF_BB_TX_LateCollision;      /* 0158 17 txlatecol        */
    __u64 uF_BB_TX_ExcessiveCollision; /* 015C 18 txexesscol       */
    __u64 uF_BB_TX_ExcessiveDeferral;  /* 016C 22 txexcessdef      */
    __u64 uF_BB_TX_CarrierSense;       /* 0160 19 txcarriererror   */
    __u64 uF_GA_TX_SingleCollision;    /* 014C 14 txsinglecol_g  */
    __u64 uF_GA_TX_MultiCollision;     /* 0150 15 txmulticol_g   */
    __u64 uF_GA_TX_Deferred;           /* 0154 16 txdeferred     */
                                       /* ADDR bb name */
    __u64 uB_GB_RX;                    /* 0184 01 rxoctetcount_gb */
    __u64 uB_GG_RX;                    /* 0188 02 rxoctetcount_g  */
    __u64 uF_GB_RX;                    /* 0180 00 rxframecount_gb */
    __u64 uF_GG_RX_Broadcast;          /* 018C 03 rxbroadcastframes_g */
    __u64 uF_GG_RX_Multicast;          /* 0190 04 rxmulticastframes_g */
    __u64 uF_GG_RX_Unicast;            /* 01C4 17 rxunicastframes_g   */
    __u64 uB_GB_RX_0to64;              /* 01AC 11 rx64octets_gb        */
    __u64 uB_GB_RX_65to127;            /* 01B0 12 rx65to127octets_gb   */
    __u64 uB_GB_RX_128to255;           /* 01B4 13 rx128to255octets_gb  */
    __u64 uB_GB_RX_256to511;           /* 01B8 14 rx256to511octets_gb  */
    __u64 uB_GB_RX_512to1023;          /* 01BC 15 rx512to1023octets_gb */
    __u64 uB_GB_RX_1024toMax;          /* 01C0 16 rx1024tomaxoctets_gb */
    __u64 uF_GB_RX_Vlan;               /* 01D8 22 rxvlanframes_gb */
    __u64 uF_GG_RX_Pause;              /* 01D0 20 rxpauseframes   */
    __u64 uF_GG_RX_Undersize;          /* 01A4 09 rxundersize_g    */
    __u64 uF_GG_RX_Oversize;           /* 01A8 10 rxoversize_g     */
    __u64 uF_BB_RX_Crc;                /* 0194 05 rxcrcerror       */
    __u64 uF_BB_RX_Alignment;          /* 0198 06 rxalignmenterror */
    __u64 uF_BB_RX_Runt;               /* 019C 07 rxrunterror      */
    __u64 uF_BB_RX_Jabber;             /* 01A0 08 rxjabbererror    */
    __u64 uF_BB_RX_Length;             /* 01C8 18 rxlengtherror    */
    __u64 uF_BB_RX_Outofrange;         /* 01CC 19 rxoutofrangetype */
    __u64 uF_BB_RX_Fifooverflow;       /* 01D4 21 rxfifooverflow   */
    __u64 uF_BB_RX_Watchdog;           /* 01DC 23 rxwatchdogerror  */
                                       /* ADDR bb name */
    __u64 uF_GG_RX_IP4;                /* 0210 00 rxipv4_gd_frms */
    __u64 uB_GG_RX_IP4;                /* 0250 16 rxipv4_gd_octets */
    __u64 uF_BB_RX_IP4_Header;         /* 0214 01 rxipv4_hderr_frms */
    __u64 uB_BB_RX_IP4_Header;         /* 0254 17 rxipv4_hderr_octets */
    __u64 uF_GG_RX_IP4_Nopayload;      /* 0218 02 rxipv4_nopay_frms */
    __u64 uB_GG_RX_IP4_Nopayload;      /* 0258 18 rxipv4_nopay_octets */
    __u64 uF_GG_RX_IP4_Fragments;      /* 021C 03 rxipv4_frag_frms */
    __u64 uB_GG_RX_IP4_Fragments;      /* 025C 19 rxipv4_frag_octets */
    __u64 uF_GG_RX_IP4_Noudpcksum;     /* 0220 04 rxipv4_udsbl_frms */
    __u64 uB_GG_RX_IP4_Noudpcksum;     /* 0260 20 rxipv4_udsbl_octets */
    __u64 uF_GG_RX_IP6;                /* 0224 05 rxipv6_gd_frms */
    __u64 uB_GG_RX_IP6;                /* 0264 21 rxipv6_gd_octets */
    __u64 uF_BB_RX_IP6_Header;         /* 0228 06 rxipv6_hdrerr_frms */
    __u64 uB_BB_RX_IP6_Header;         /* 0268 22 rxipv6_hdrerr_octets */
    __u64 uF_GG_RX_IP6_Nopayload;      /* 022C 07 rxipv6_nopay_frms */
    __u64 uB_GG_RX_IP6_Nopayload;      /* 026C 23 rxipv6_nopay_octets */
    __u64 uF_GG_RX_UDP;                /* 0230 08 rxudp_gd_frms */
    __u64 uB_GG_RX_UDP;                /* 0270 24 rxudp_gd_octets */
    __u64 uF_BB_RX_UDP_Checksum;       /* 0234 09 rxudp_err_frms */
    __u64 uB_BB_RX_UDP_Checksum;       /* 0274 25 rxudp_err_octets */
    __u64 uF_GG_RX_TCP;                /* 0238 10 rxtcp_gd_frms */
    __u64 uB_GG_RX_TCP;                /* 0278 26 rxtcp_gd_octets */
    __u64 uF_BB_RX_TCP_Checksum;       /* 023C 11 rxtcp_err_frms */
    __u64 uB_BB_RX_TCP_Checksum;       /* 027C 27 rxtcp_err_octets */
    __u64 uF_GG_RX_ICMP;               /* 0240 12 rxicmp_gd_frms */
    __u64 uB_GG_RX_ICMP;               /* 0280 28 rxicmp_gd_octets */
    __u64 uF_BB_RX_ICMP_Checksum;      /* 0244 13 rxicmp_err_frms */
    __u64 uB_BB_RX_ICMP_Checksum;      /* 0284 29 rxicmp_err_octets */
} trident_gmacEth_RmonCnt_t, *ptrident_gmacEth_RmonCnt_t;

#endif /* _MMC_STATS_H_ */
