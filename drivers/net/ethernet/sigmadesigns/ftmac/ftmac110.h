/*
 * Faraday FTMAC110 10/100 Ethernet
 *
 * (C) Copyright 2009-2011 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
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
 */

#ifndef __FTMAC110_H
#define __FTMAC110_H

#define	FTMAC110_OFFSET_ISR		0x00
#define	FTMAC110_OFFSET_IMR		0x04
#define	FTMAC110_OFFSET_MAC_MADR	0x08
#define	FTMAC110_OFFSET_MAC_LADR	0x0c
#define	FTMAC110_OFFSET_MAHT0		0x10
#define	FTMAC110_OFFSET_MAHT1		0x14
#define	FTMAC110_OFFSET_TXPD		0x18
#define	FTMAC110_OFFSET_RXPD		0x1c
#define	FTMAC110_OFFSET_TXR_BADR	0x20
#define	FTMAC110_OFFSET_RXR_BADR	0x24
#define	FTMAC110_OFFSET_ITC		0x28
#define	FTMAC110_OFFSET_APTC		0x2c
#define	FTMAC110_OFFSET_DBLAC		0x30
#define	FTMAC110_OFFSET_MACCR		0x88
#define	FTMAC110_OFFSET_MACSR		0x8c
#define	FTMAC110_OFFSET_PHYCR		0x90
#define	FTMAC110_OFFSET_PHYWDATA	0x94
#define	FTMAC110_OFFSET_FCR		0x98
#define	FTMAC110_OFFSET_BPR		0x9c
#define	FTMAC110_OFFSET_TS		0xc4
#define	FTMAC110_OFFSET_DMAFIFOS	0xc8
#define	FTMAC110_OFFSET_TM		0xcc
#define	FTMAC110_OFFSET_TX_MCOL_SCOL	0xd4
#define	FTMAC110_OFFSET_RPF_AEP		0xd8
#define	FTMAC110_OFFSET_XM_PG		0xdc
#define	FTMAC110_OFFSET_RUNT_TLCC	0xe0
#define	FTMAC110_OFFSET_CRCER_FTL	0xe4
#define	FTMAC110_OFFSET_RLC_RCC		0xe8
#define	FTMAC110_OFFSET_BROC		0xec
#define	FTMAC110_OFFSET_MULCA		0xf0
#define	FTMAC110_OFFSET_RP		0xf4
#define	FTMAC110_OFFSET_XP		0xf8

/*
 * Interrupt status register & interrupt mask register
 */
#define	FTMAC110_INT_RPKT_FINISH	(1 << 0)
#define	FTMAC110_INT_NORXBUF		(1 << 1)
#define	FTMAC110_INT_XPKT_FINISH	(1 << 2)
#define	FTMAC110_INT_NOTXBUF		(1 << 3)
#define	FTMAC110_INT_XPKT_OK		(1 << 4)
#define	FTMAC110_INT_XPKT_LOST		(1 << 5)
#define	FTMAC110_INT_RPKT_SAV		(1 << 6)
#define	FTMAC110_INT_RPKT_LOST		(1 << 7)
#define	FTMAC110_INT_AHB_ERR		(1 << 8)
#define	FTMAC110_INT_PHYSTS_CHG		(1 << 9)

/*
 * Interrupt timer control register
 */
#define FTMAC110_ITC_RXINT_CNT(x)	(((x) & 0xf) << 0)
#define FTMAC110_ITC_RXINT_THR(x)	(((x) & 0x7) << 4)
#define FTMAC110_ITC_RXINT_TIME_SEL	(1 << 7)
#define FTMAC110_ITC_TXINT_CNT(x)	(((x) & 0xf) << 8)
#define FTMAC110_ITC_TXINT_THR(x)	(((x) & 0x7) << 12)
#define FTMAC110_ITC_TXINT_TIME_SEL	(1 << 15)

/*
 * Automatic polling timer control register
 */
#define	FTMAC110_APTC_RXPOLL_CNT(x)	(((x) & 0xf) << 0)
#define	FTMAC110_APTC_RXPOLL_TIME_SEL	(1 << 4)
#define	FTMAC110_APTC_TXPOLL_CNT(x)	(((x) & 0xf) << 8)
#define	FTMAC110_APTC_TXPOLL_TIME_SEL	(1 << 12)

/*
 * DMA burst length and arbitration control register
 */
#define FTMAC110_DBLAC_INCR4_EN		(1 << 0)
#define FTMAC110_DBLAC_INCR8_EN		(1 << 1)
#define FTMAC110_DBLAC_INCR16_EN	(1 << 2)
#define FTMAC110_DBLAC_RXFIFO_LTHR(x)	(((x) & 0x7) << 3)
#define FTMAC110_DBLAC_RXFIFO_HTHR(x)	(((x) & 0x7) << 6)
#define FTMAC110_DBLAC_RX_THR_EN	(1 << 9)

/*
 * MAC control register
 */
#define	FTMAC110_MACCR_XDMA_EN		(1 << 0)
#define	FTMAC110_MACCR_RDMA_EN		(1 << 1)
#define	FTMAC110_MACCR_SW_RST		(1 << 2)
#define	FTMAC110_MACCR_LOOP_EN		(1 << 3)
#define	FTMAC110_MACCR_CRC_DIS		(1 << 4)
#define	FTMAC110_MACCR_XMT_EN		(1 << 5)
#define	FTMAC110_MACCR_ENRX_IN_HALFTX	(1 << 6)
#define	FTMAC110_MACCR_RCV_EN		(1 << 8)
#define	FTMAC110_MACCR_HT_MULTI_EN	(1 << 9)
#define	FTMAC110_MACCR_RX_RUNT		(1 << 10)
#define	FTMAC110_MACCR_RX_FTL		(1 << 11)
#define	FTMAC110_MACCR_RCV_ALL		(1 << 12)
#define	FTMAC110_MACCR_CRC_APD		(1 << 14)
#define	FTMAC110_MACCR_FULLDUP		(1 << 15)
#define	FTMAC110_MACCR_RX_MULTIPKT	(1 << 16)
#define	FTMAC110_MACCR_RX_BROADPKT	(1 << 17)
#define FTMAC110_MACCR_SPEED100		(1 << 18)

/*
 * PHY control register
 */
#define FTMAC110_PHYCR_MIIRDATA		0xffff
#define FTMAC110_PHYCR_PHYAD(x)		(((x) & 0x1f) << 16)
#define FTMAC110_PHYCR_REGAD(x)		(((x) & 0x1f) << 21)
#define FTMAC110_PHYCR_MIIRD		(1 << 26)
#define FTMAC110_PHYCR_MIIWR		(1 << 27)

/*
 * PHY write data register
 */
#define FTMAC110_PHYWDATA_MIIWDATA(x)	((x) & 0xffff)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
struct ftmac110_txdes {
	unsigned int	txdes0;
	unsigned int	txdes1;
	unsigned int	txdes2;	/* TXBUF_BADR */
	unsigned int	txdes3;	/* not used by HW */
} __attribute__ ((aligned(16)));

#define	FTMAC110_TXDES0_TXPKT_LATECOL	(1 << 0)
#define	FTMAC110_TXDES0_TXPKT_EXSCOL	(1 << 1)
#define	FTMAC110_TXDES0_TXDMA_OWN	(1 << 31)

#define	FTMAC110_TXDES1_TXBUF_SIZE(x)	((x) & 0x7ff)
#define	FTMAC110_TXDES1_LTS		(1 << 27)
#define	FTMAC110_TXDES1_FTS		(1 << 28)
#define	FTMAC110_TXDES1_TX2FIC		(1 << 29)
#define	FTMAC110_TXDES1_TXIC		(1 << 30)
#define	FTMAC110_TXDES1_EDOTR		(1 << 31)

/*
 * Receive descriptor, aligned to 16 bytes
 */
struct ftmac110_rxdes {
	unsigned int	rxdes0;
	unsigned int	rxdes1;
	unsigned int	rxdes2;	/* RXBUF_BADR */
	unsigned int	rxdes3;	/* not used by HW */
} __attribute__ ((aligned(16)));

#define	FTMAC110_RXDES0_RFL		0x7ff
#define	FTMAC110_RXDES0_MULTICAST	(1 << 16)
#define	FTMAC110_RXDES0_BROADCAST	(1 << 17)
#define	FTMAC110_RXDES0_RX_ERR		(1 << 18)
#define	FTMAC110_RXDES0_CRC_ERR		(1 << 19)
#define	FTMAC110_RXDES0_FTL		(1 << 20)
#define	FTMAC110_RXDES0_RUNT		(1 << 21)
#define	FTMAC110_RXDES0_RX_ODD_NB	(1 << 22)
#define	FTMAC110_RXDES0_LRS		(1 << 28)
#define	FTMAC110_RXDES0_FRS		(1 << 29)
#define	FTMAC110_RXDES0_RXDMA_OWN	(1 << 31)

#define	FTMAC110_RXDES1_RXBUF_SIZE(x)	((x) & 0x7ff)
#define	FTMAC110_RXDES1_EDORR		(1 << 31)

#endif /* __FTMAC110_H */
