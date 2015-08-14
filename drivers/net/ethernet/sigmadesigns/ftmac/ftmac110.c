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

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mii.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/phy.h>

#include "ftmac110.h"

#define DRV_NAME	"ftmac110"
#define DRV_VERSION	"0.1"

#define RX_QUEUE_ENTRIES	64	/* must be power of 2 */
#define TX_QUEUE_ENTRIES	16	/* must be power of 2 */

#define MAX_PKT_SIZE		1518
#define RX_BUF_SIZE		2044	/* must be smaller than 0x7ff */

#if MAX_PKT_SIZE > 0x7ff
#error invalid MAX_PKT_SIZE
#endif

#if RX_BUF_SIZE > 0x7ff || RX_BUF_SIZE > PAGE_SIZE
#error invalid RX_BUF_SIZE
#endif

/*
 * FTMAC110 DMA design issue
 *
 * Its DMA engine has a weird restriction that its Rx DMA engine
 * accepts only 16-bits aligned address, 32-bits aligned is not
 * acceptable. However this restriction does not apply to Tx DMA.
 *
 * Conclusion:
 * (1) Tx DMA Buffer Address:
 *     1 bytes aligned: Invalid
 *     2 bytes aligned: O.K
 *     4 bytes aligned: O.K (-> ZeroCopy is possible)
 * (2) Rx DMA Buffer Address:
 *     1 bytes aligned: Invalid
 *     2 bytes aligned: O.K
 *     4 bytes aligned: Invalid
 */

/******************************************************************************
 * private data
 *****************************************************************************/
struct ftmac110_descs {
	struct ftmac110_rxdes rxdes[RX_QUEUE_ENTRIES];
	struct ftmac110_txdes txdes[TX_QUEUE_ENTRIES];
};

struct ftmac110 {
	struct resource *res;
	void __iomem *base;
	int irq;

	struct ftmac110_descs *descs;
	dma_addr_t descs_dma_addr;

	unsigned int rx_pointer;
	unsigned int tx_clean_pointer;
	unsigned int tx_pointer;
	unsigned int tx_pending;

	spinlock_t tx_lock;

	struct net_device *netdev;
	struct device *dev;
	struct napi_struct napi;

	/* platform device reference */
	struct platform_device *pdev;

	/* MII bus Interface */
	struct mii_bus* mii_bus;
	struct mii_if_info mii;

	/* PHY Interface */
	int phy_addr;
	struct phy_device *phydev;

	int old_link;
	int old_speed;
	int old_duplex;
};

static int ftmac110_alloc_rx_page(struct ftmac110 *priv,
				  struct ftmac110_rxdes *rxdes, gfp_t gfp);

/******************************************************************************
 * internal functions (MII and PHY access)
 *****************************************************************************/
static int ftmac110_mdio_read(struct net_device *netdev, int phy_id, int reg)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	unsigned int phycr;
	int i;

	phycr = FTMAC110_PHYCR_PHYAD(phy_id) |
		FTMAC110_PHYCR_REGAD(reg) |
		FTMAC110_PHYCR_MIIRD;

	iowrite32(phycr, priv->base + FTMAC110_OFFSET_PHYCR);

	for (i = 0; i < 10; i++) {
		phycr = ioread32(priv->base + FTMAC110_OFFSET_PHYCR);

		if ((phycr & FTMAC110_PHYCR_MIIRD) == 0)
			return phycr & FTMAC110_PHYCR_MIIRDATA;

		udelay(100);
	}

	netdev_err(netdev, "mdio read timed out\n");
	return 0;
}

static void ftmac110_mdio_write(struct net_device *netdev, int phy_id, int reg,
				int data)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	unsigned int phycr;
	int i;

	phycr = FTMAC110_PHYCR_PHYAD(phy_id) |
		FTMAC110_PHYCR_REGAD(reg) |
		FTMAC110_PHYCR_MIIWR;

	data = FTMAC110_PHYWDATA_MIIWDATA(data);

	iowrite32(data, priv->base + FTMAC110_OFFSET_PHYWDATA);
	iowrite32(phycr, priv->base + FTMAC110_OFFSET_PHYCR);

	for (i = 0; i < 10; i++) {
		phycr = ioread32(priv->base + FTMAC110_OFFSET_PHYCR);

		if ((phycr & FTMAC110_PHYCR_MIIWR) == 0)
			return;

		udelay(100);
	}

	netdev_err(netdev, "mdio write timed out\n");
}

static int ftmac110_mdiobus_read(struct mii_bus *bus, int phy_addr, int regnum)
{
	struct ftmac110 *priv = (struct ftmac110 *)bus->priv;
	struct net_device *netdev = priv->netdev;

	return ftmac110_mdio_read(netdev, phy_addr, regnum);
}

static int ftmac110_mdiobus_write(struct mii_bus *bus, int phy_addr, int regnum, u16 value)
{
	struct ftmac110 *priv = (struct ftmac110 *)bus->priv;
	struct net_device *netdev = priv->netdev;

	ftmac110_mdio_write(netdev, phy_addr, regnum, value);
	return 0;
}

/******************************************************************************
 * internal functions (hardware register access)
 *****************************************************************************/
#define INT_MASK_ALL_ENABLED	(FTMAC110_INT_RPKT_FINISH	| \
				 FTMAC110_INT_NORXBUF		| \
				 FTMAC110_INT_XPKT_OK		| \
				 FTMAC110_INT_XPKT_LOST		| \
				 FTMAC110_INT_RPKT_LOST		| \
				 FTMAC110_INT_AHB_ERR		| \
				 FTMAC110_INT_PHYSTS_CHG)

#define INT_MASK_ALL_DISABLED	0

static void ftmac110_enable_all_int(struct ftmac110 *priv)
{
	iowrite32(INT_MASK_ALL_ENABLED, priv->base + FTMAC110_OFFSET_IMR);
}

static void ftmac110_disable_all_int(struct ftmac110 *priv)
{
	iowrite32(INT_MASK_ALL_DISABLED, priv->base + FTMAC110_OFFSET_IMR);
}

static void ftmac110_set_rx_ring_base(struct ftmac110 *priv, dma_addr_t addr)
{
	iowrite32(addr, priv->base + FTMAC110_OFFSET_RXR_BADR);
}

static void ftmac110_set_tx_ring_base(struct ftmac110 *priv, dma_addr_t addr)
{
	iowrite32(addr, priv->base + FTMAC110_OFFSET_TXR_BADR);
}

static void ftmac110_txdma_start_polling(struct ftmac110 *priv)
{
	iowrite32(1, priv->base + FTMAC110_OFFSET_TXPD);
}

static int ftmac110_reset(struct ftmac110 *priv)
{
	struct net_device *netdev = priv->netdev;
	int i;

	/* NOTE: reset clears all registers */
	iowrite32(FTMAC110_MACCR_SW_RST, priv->base + FTMAC110_OFFSET_MACCR);

	for (i = 0; i < 5; i++) {
		unsigned int maccr;

		maccr = ioread32(priv->base + FTMAC110_OFFSET_MACCR);
		if (!(maccr & FTMAC110_MACCR_SW_RST)) {
			/*
			 * FTMAC110_MACCR_SW_RST cleared does not indicate
			 * that hardware reset completed (what the f*ck).
			 * We still need to wait for a while.
			 */
			udelay(500);
			return 0;
		}

		udelay(1000);
	}

	netdev_err(netdev, "software reset failed\n");
	return -EIO;
}

static void ftmac110_set_mac(struct ftmac110 *priv, const unsigned char *mac)
{
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	iowrite32(maddr, priv->base + FTMAC110_OFFSET_MAC_MADR);
	iowrite32(laddr, priv->base + FTMAC110_OFFSET_MAC_LADR);
}

#define MACCR_ENABLE_ALL	(FTMAC110_MACCR_XMT_EN	| \
				 FTMAC110_MACCR_RCV_EN	| \
				 FTMAC110_MACCR_XDMA_EN	| \
				 FTMAC110_MACCR_RDMA_EN	| \
				 FTMAC110_MACCR_CRC_APD	| \
				 FTMAC110_MACCR_RX_RUNT	| \
				 FTMAC110_MACCR_RX_BROADPKT)

static int ftmac110_start_hw(struct ftmac110 *priv)
{
	struct net_device *netdev = priv->netdev;
	int maccr = MACCR_ENABLE_ALL;

	if (ftmac110_reset(priv))
		return -EIO;

	/* setup ring buffer base registers */
	ftmac110_set_rx_ring_base(priv,
				  priv->descs_dma_addr +
				  offsetof(struct ftmac110_descs, rxdes));
	ftmac110_set_tx_ring_base(priv,
				  priv->descs_dma_addr +
				  offsetof(struct ftmac110_descs, txdes));

	iowrite32(FTMAC110_APTC_RXPOLL_CNT(1), priv->base + FTMAC110_OFFSET_APTC);

	ftmac110_set_mac(priv, netdev->dev_addr);

	if(SPEED_100 == priv->phydev->speed)
		maccr |= FTMAC110_MACCR_SPEED100;
	if(DUPLEX_FULL == priv->phydev->duplex)
		maccr |= FTMAC110_MACCR_FULLDUP;

	iowrite32(maccr, priv->base + FTMAC110_OFFSET_MACCR);
	return 0;
}

static void ftmac110_stop_hw(struct ftmac110 *priv)
{
	iowrite32(0, priv->base + FTMAC110_OFFSET_MACCR);
}

/******************************************************************************
 * internal functions (receive descriptor)
 *****************************************************************************/
static bool ftmac110_rxdes_first_segment(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_FRS);
}

static bool ftmac110_rxdes_last_segment(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_LRS);
}

static bool ftmac110_rxdes_owned_by_dma(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_RXDMA_OWN);
}

static void ftmac110_rxdes_set_dma_own(struct ftmac110_rxdes *rxdes)
{
	/* clear status bits */
	rxdes->rxdes0 = cpu_to_le32(FTMAC110_RXDES0_RXDMA_OWN);
}

static bool ftmac110_rxdes_rx_error(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_RX_ERR);
}

static bool ftmac110_rxdes_crc_error(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_CRC_ERR);
}

static bool ftmac110_rxdes_frame_too_long(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_FTL);
}

static bool ftmac110_rxdes_runt(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_RUNT);
}

static bool ftmac110_rxdes_odd_nibble(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_RX_ODD_NB);
}

static unsigned int ftmac110_rxdes_frame_length(struct ftmac110_rxdes *rxdes)
{
	return le32_to_cpu(rxdes->rxdes0) & FTMAC110_RXDES0_RFL;
}

static bool ftmac110_rxdes_multicast(struct ftmac110_rxdes *rxdes)
{
	return rxdes->rxdes0 & cpu_to_le32(FTMAC110_RXDES0_MULTICAST);
}

static void ftmac110_rxdes_set_buffer_size(struct ftmac110_rxdes *rxdes,
					   unsigned int size)
{
	rxdes->rxdes1 &= cpu_to_le32(FTMAC110_RXDES1_EDORR);
	rxdes->rxdes1 |= cpu_to_le32(FTMAC110_RXDES1_RXBUF_SIZE(size));
}

static void ftmac110_rxdes_set_end_of_ring(struct ftmac110_rxdes *rxdes)
{
	rxdes->rxdes1 |= cpu_to_le32(FTMAC110_RXDES1_EDORR);
}

static void ftmac110_rxdes_set_dma_addr(struct ftmac110_rxdes *rxdes,
					dma_addr_t addr)
{
	rxdes->rxdes2 = cpu_to_le32(addr);
}

static dma_addr_t ftmac110_rxdes_get_dma_addr(struct ftmac110_rxdes *rxdes)
{
	return le32_to_cpu(rxdes->rxdes2);
}

/*
 * rxdes3 is not used by hardware. We use it to keep track of page.
 * Since hardware does not touch it, we can skip cpu_to_le32()/le32_to_cpu().
 */
static void ftmac110_rxdes_set_page(struct ftmac110_rxdes *rxdes, struct page *page)
{
	rxdes->rxdes3 = (unsigned int)page;
}

static struct page *ftmac110_rxdes_get_page(struct ftmac110_rxdes *rxdes)
{
	return (struct page *)rxdes->rxdes3;
}

/******************************************************************************
 * internal functions (receive)
 *****************************************************************************/
static int ftmac110_next_rx_pointer(int pointer)
{
	return (pointer + 1) & (RX_QUEUE_ENTRIES - 1);
}

static void ftmac110_rx_pointer_advance(struct ftmac110 *priv)
{
	priv->rx_pointer = ftmac110_next_rx_pointer(priv->rx_pointer);
}

static struct ftmac110_rxdes *ftmac110_current_rxdes(struct ftmac110 *priv)
{
	return &priv->descs->rxdes[priv->rx_pointer];
}

static struct ftmac110_rxdes *
ftmac110_rx_locate_first_segment(struct ftmac110 *priv)
{
	struct ftmac110_rxdes *rxdes = ftmac110_current_rxdes(priv);

	while (!ftmac110_rxdes_owned_by_dma(rxdes)) {
		if (ftmac110_rxdes_first_segment(rxdes))
			return rxdes;

		ftmac110_rxdes_set_dma_own(rxdes);
		ftmac110_rx_pointer_advance(priv);
		rxdes = ftmac110_current_rxdes(priv);
	}

	return NULL;
}

static bool ftmac110_rx_packet_error(struct ftmac110 *priv,
				     struct ftmac110_rxdes *rxdes)
{
	struct net_device *netdev = priv->netdev;
	bool error = false;

	if (unlikely(ftmac110_rxdes_rx_error(rxdes))) {
		if (net_ratelimit())
			netdev_info(netdev, "rx err\n");

		netdev->stats.rx_errors++;
		error = true;
	}

	if (unlikely(ftmac110_rxdes_crc_error(rxdes))) {
		if (net_ratelimit())
			netdev_info(netdev, "rx crc err\n");

		netdev->stats.rx_crc_errors++;
		error = true;
	}

	if (unlikely(ftmac110_rxdes_frame_too_long(rxdes))) {
		if (net_ratelimit())
			netdev_info(netdev, "rx frame too long\n");

		netdev->stats.rx_length_errors++;
		error = true;
	} else if (unlikely(ftmac110_rxdes_runt(rxdes))) {
		if (net_ratelimit())
			netdev_info(netdev, "rx runt\n");

		netdev->stats.rx_length_errors++;
		error = true;
	} else if (unlikely(ftmac110_rxdes_odd_nibble(rxdes))) {
		if (net_ratelimit())
			netdev_info(netdev, "rx odd nibble\n");

		netdev->stats.rx_length_errors++;
		error = true;
	}

	return error;
}

static void ftmac110_rx_drop_packet(struct ftmac110 *priv)
{
	struct net_device *netdev = priv->netdev;
	struct ftmac110_rxdes *rxdes = ftmac110_current_rxdes(priv);
	bool done = false;

	if (net_ratelimit())
		netdev_dbg(netdev, "drop packet %p\n", rxdes);

	do {
		if (ftmac110_rxdes_last_segment(rxdes))
			done = true;

		ftmac110_rxdes_set_dma_own(rxdes);
		ftmac110_rx_pointer_advance(priv);
		rxdes = ftmac110_current_rxdes(priv);
	} while (!done && !ftmac110_rxdes_owned_by_dma(rxdes));

	netdev->stats.rx_dropped++;
}

static bool ftmac110_rx_packet(struct ftmac110 *priv, int *processed)
{
	struct net_device *netdev = priv->netdev;
	struct ftmac110_rxdes *rxdes;
	struct sk_buff *skb;
	struct page *page;
	dma_addr_t map;
	int length;

	rxdes = ftmac110_rx_locate_first_segment(priv);
	if (!rxdes)
		return false;

	if (unlikely(ftmac110_rx_packet_error(priv, rxdes))) {
		ftmac110_rx_drop_packet(priv);
		return true;
	}

	/*
	 * It is impossible to get multi-segment packets
	 * because we always provide big enough receive buffers.
	 */
	if (unlikely(!ftmac110_rxdes_last_segment(rxdes)))
		BUG();

	/* start processing */
	skb = netdev_alloc_skb_ip_align(netdev, 128);
	if (unlikely(!skb)) {
		if (net_ratelimit())
			netdev_err(netdev, "rx skb alloc failed\n");

		ftmac110_rx_drop_packet(priv);
		return true;
	}

	if (unlikely(ftmac110_rxdes_multicast(rxdes)))
		netdev->stats.multicast++;

	map = ftmac110_rxdes_get_dma_addr(rxdes);
	dma_unmap_page(priv->dev, map, RX_BUF_SIZE, DMA_FROM_DEVICE);

	length = ftmac110_rxdes_frame_length(rxdes);
	page = ftmac110_rxdes_get_page(rxdes);
	/* it needs to be exactly 2 bytes aligned ,so off=2 */
	skb_fill_page_desc(skb, 0, page, 2, length);
	skb->len += length;
	skb->data_len += length;

	if (length > 128) {
		skb->truesize += PAGE_SIZE;
		/* We pull the minimum amount into linear part */
		__pskb_pull_tail(skb, ETH_HLEN);
	} else {
		/* Small frames are copied into linear part to free one page */
		__pskb_pull_tail(skb, length);
	}
	ftmac110_alloc_rx_page(priv, rxdes, GFP_ATOMIC);

	ftmac110_rx_pointer_advance(priv);

	skb->protocol = eth_type_trans(skb, netdev);

	netdev->stats.rx_packets++;
	netdev->stats.rx_bytes += skb->len;

	/* push packet to protocol stack */
	netif_receive_skb(skb);

	(*processed)++;
	return true;
}

/******************************************************************************
 * internal functions (transmit descriptor)
 *****************************************************************************/
static void ftmac110_txdes_reset(struct ftmac110_txdes *txdes)
{
	/* clear all except end of ring bit */
	txdes->txdes0 = 0;
	txdes->txdes1 &= cpu_to_le32(FTMAC110_TXDES1_EDOTR);
	txdes->txdes2 = 0;
	txdes->txdes3 = 0;
}

static bool ftmac110_txdes_owned_by_dma(struct ftmac110_txdes *txdes)
{
	return txdes->txdes0 & cpu_to_le32(FTMAC110_TXDES0_TXDMA_OWN);
}

static void ftmac110_txdes_set_dma_own(struct ftmac110_txdes *txdes)
{
	/*
	 * Make sure dma own bit will not be set before any other
	 * descriptor fields.
	 */
	wmb();
	txdes->txdes0 |= cpu_to_le32(FTMAC110_TXDES0_TXDMA_OWN);
}

static bool ftmac110_txdes_excessive_collision(struct ftmac110_txdes *txdes)
{
	return txdes->txdes0 & cpu_to_le32(FTMAC110_TXDES0_TXPKT_EXSCOL);
}

static bool ftmac110_txdes_late_collision(struct ftmac110_txdes *txdes)
{
	return txdes->txdes0 & cpu_to_le32(FTMAC110_TXDES0_TXPKT_LATECOL);
}

static void ftmac110_txdes_set_end_of_ring(struct ftmac110_txdes *txdes)
{
	txdes->txdes1 |= cpu_to_le32(FTMAC110_TXDES1_EDOTR);
}

static void ftmac110_txdes_set_first_segment(struct ftmac110_txdes *txdes)
{
	txdes->txdes1 |= cpu_to_le32(FTMAC110_TXDES1_FTS);
}

static void ftmac110_txdes_set_last_segment(struct ftmac110_txdes *txdes)
{
	txdes->txdes1 |= cpu_to_le32(FTMAC110_TXDES1_LTS);
}

static void ftmac110_txdes_set_txint(struct ftmac110_txdes *txdes)
{
	txdes->txdes1 |= cpu_to_le32(FTMAC110_TXDES1_TXIC);
}

static void ftmac110_txdes_set_buffer_size(struct ftmac110_txdes *txdes,
					   unsigned int len)
{
	txdes->txdes1 |= cpu_to_le32(FTMAC110_TXDES1_TXBUF_SIZE(len));
}

static void ftmac110_txdes_set_dma_addr(struct ftmac110_txdes *txdes,
					dma_addr_t addr)
{
	txdes->txdes2 = cpu_to_le32(addr);
}

static dma_addr_t ftmac110_txdes_get_dma_addr(struct ftmac110_txdes *txdes)
{
	return le32_to_cpu(txdes->txdes2);
}

/*
 * txdes3 is not used by hardware. We use it to keep track of socket buffer.
 * Since hardware does not touch it, we can skip cpu_to_le32()/le32_to_cpu().
 */
static void ftmac110_txdes_set_skb(struct ftmac110_txdes *txdes, struct sk_buff *skb)
{
	txdes->txdes3 = (unsigned int)skb;
}

static struct sk_buff *ftmac110_txdes_get_skb(struct ftmac110_txdes *txdes)
{
	return (struct sk_buff *)txdes->txdes3;
}

/******************************************************************************
 * internal functions (transmit)
 *****************************************************************************/
static int ftmac110_next_tx_pointer(int pointer)
{
	return (pointer + 1) & (TX_QUEUE_ENTRIES - 1);
}

static void ftmac110_tx_pointer_advance(struct ftmac110 *priv)
{
	priv->tx_pointer = ftmac110_next_tx_pointer(priv->tx_pointer);
}

static void ftmac110_tx_clean_pointer_advance(struct ftmac110 *priv)
{
	priv->tx_clean_pointer = ftmac110_next_tx_pointer(priv->tx_clean_pointer);
}

static struct ftmac110_txdes *ftmac110_current_txdes(struct ftmac110 *priv)
{
	return &priv->descs->txdes[priv->tx_pointer];
}

static struct ftmac110_txdes *ftmac110_current_clean_txdes(struct ftmac110 *priv)
{
	return &priv->descs->txdes[priv->tx_clean_pointer];
}

static bool ftmac110_tx_complete_packet(struct ftmac110 *priv)
{
	struct net_device *netdev = priv->netdev;
	struct ftmac110_txdes *txdes;
	struct sk_buff *skb;
	dma_addr_t map;

	if (priv->tx_pending == 0)
		return false;

	txdes = ftmac110_current_clean_txdes(priv);

	if (ftmac110_txdes_owned_by_dma(txdes))
		return false;

	skb = ftmac110_txdes_get_skb(txdes);
	map = ftmac110_txdes_get_dma_addr(txdes);

	if (unlikely(ftmac110_txdes_excessive_collision(txdes) ||
		     ftmac110_txdes_late_collision(txdes))) {
		/*
		 * packet transmitted to ethernet lost due to late collision
		 * or excessive collision
		 */
		netdev->stats.tx_aborted_errors++;
	} else {
		netdev->stats.tx_packets++;
		netdev->stats.tx_bytes += skb->len;
	}

	dma_unmap_single(priv->dev, map, skb_headlen(skb), DMA_TO_DEVICE);
	dev_kfree_skb(skb);

	ftmac110_txdes_reset(txdes);

	ftmac110_tx_clean_pointer_advance(priv);

	spin_lock(&priv->tx_lock);
	priv->tx_pending--;
	spin_unlock(&priv->tx_lock);
	netif_wake_queue(netdev);

	return true;
}

static void ftmac110_tx_complete(struct ftmac110 *priv)
{
	while (ftmac110_tx_complete_packet(priv))
		;
}

static int ftmac110_xmit(struct ftmac110 *priv, struct sk_buff *skb,
			 dma_addr_t map)
{
	struct net_device *netdev = priv->netdev;
	struct ftmac110_txdes *txdes;
	unsigned int len = (skb->len < ETH_ZLEN) ? ETH_ZLEN : skb->len;

	txdes = ftmac110_current_txdes(priv);
	ftmac110_tx_pointer_advance(priv);

	/* setup TX descriptor */
	ftmac110_txdes_set_skb(txdes, skb);
	ftmac110_txdes_set_dma_addr(txdes, map);

	ftmac110_txdes_set_first_segment(txdes);
	ftmac110_txdes_set_last_segment(txdes);
	ftmac110_txdes_set_txint(txdes);
	ftmac110_txdes_set_buffer_size(txdes, len);

	spin_lock(&priv->tx_lock);
	priv->tx_pending++;
	if (priv->tx_pending == TX_QUEUE_ENTRIES)
		netif_stop_queue(netdev);

	/* start transmit */
	ftmac110_txdes_set_dma_own(txdes);
	spin_unlock(&priv->tx_lock);

	ftmac110_txdma_start_polling(priv);
	return NETDEV_TX_OK;
}

/******************************************************************************
 * internal functions (buffer)
 *****************************************************************************/
static int ftmac110_alloc_rx_page(struct ftmac110 *priv,
				  struct ftmac110_rxdes *rxdes, gfp_t gfp)
{
	struct net_device *netdev = priv->netdev;
	struct page *page;
	dma_addr_t map;

	page = alloc_page(gfp);
	if (!page) {
		if (net_ratelimit())
			netdev_err(netdev, "failed to allocate rx page\n");
		return -ENOMEM;
	}

	map = dma_map_page(priv->dev, page, 0, RX_BUF_SIZE, DMA_FROM_DEVICE);
	if (unlikely(dma_mapping_error(priv->dev, map))) {
		if (net_ratelimit())
			netdev_err(netdev, "failed to map rx page\n");
		__free_page(page);
		return -ENOMEM;
	}

	ftmac110_rxdes_set_page(rxdes, page);
	/* it needs to be exactly 2 bytes aligned */
	ftmac110_rxdes_set_dma_addr(rxdes, map + 2);
	ftmac110_rxdes_set_buffer_size(rxdes, RX_BUF_SIZE);
	ftmac110_rxdes_set_dma_own(rxdes);
	return 0;
}

static void ftmac110_free_buffers(struct ftmac110 *priv)
{
	int i;

	for (i = 0; i < RX_QUEUE_ENTRIES; i++) {
		struct ftmac110_rxdes *rxdes = &priv->descs->rxdes[i];
		struct page *page = ftmac110_rxdes_get_page(rxdes);
		dma_addr_t map = ftmac110_rxdes_get_dma_addr(rxdes);

		if (!page)
			continue;

		dma_unmap_page(priv->dev, map, RX_BUF_SIZE, DMA_FROM_DEVICE);
		__free_page(page);
	}

	for (i = 0; i < TX_QUEUE_ENTRIES; i++) {
		struct ftmac110_txdes *txdes = &priv->descs->txdes[i];
		struct sk_buff *skb = ftmac110_txdes_get_skb(txdes);
		dma_addr_t map = ftmac110_txdes_get_dma_addr(txdes);

		if (!skb)
			continue;

		dma_unmap_single(priv->dev, map, skb_headlen(skb), DMA_TO_DEVICE);
		dev_kfree_skb(skb);
	}

	dma_free_coherent(priv->dev, sizeof(struct ftmac110_descs),
			  priv->descs, priv->descs_dma_addr);
}

static int ftmac110_alloc_buffers(struct ftmac110 *priv)
{
	int i;
#if 1
	priv->descs = dma_alloc_coherent(priv->dev,
					 sizeof(struct ftmac110_descs),
					 &priv->descs_dma_addr,
					 GFP_KERNEL | __GFP_ZERO);
#endif
#if 0
	priv->descs = dma_alloc_coherent(NULL,
					 PAGE_ALIGN(sizeof(struct ftmac110_descs)),
					 &priv->descs_dma_addr,
					 GFP_ATOMIC);
#endif
	if (!priv->descs)
		return -ENOMEM;

	/* initialize RX ring */
	ftmac110_rxdes_set_end_of_ring(&priv->descs->rxdes[RX_QUEUE_ENTRIES - 1]);

	for (i = 0; i < RX_QUEUE_ENTRIES; i++) {
		struct ftmac110_rxdes *rxdes = &priv->descs->rxdes[i];

		if (ftmac110_alloc_rx_page(priv, rxdes, GFP_KERNEL))
			goto err;
	}

	/* initialize TX ring */
	ftmac110_txdes_set_end_of_ring(&priv->descs->txdes[TX_QUEUE_ENTRIES - 1]);
	return 0;

err:
	ftmac110_free_buffers(priv);
	return -ENOMEM;
}

/******************************************************************************
 * struct ethtool_ops functions
 *****************************************************************************/
static void ftmac110_get_drvinfo(struct net_device *netdev,
				 struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, dev_name(&netdev->dev), sizeof(info->bus_info));
}

static int ftmac110_get_settings(struct net_device *netdev, struct ethtool_cmd *cmd)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	return mii_ethtool_gset(&priv->mii, cmd);
}

static int ftmac110_set_settings(struct net_device *netdev, struct ethtool_cmd *cmd)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	return mii_ethtool_sset(&priv->mii, cmd);
}

static int ftmac110_nway_reset(struct net_device *netdev)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	return mii_nway_restart(&priv->mii);
}

static u32 ftmac110_get_link(struct net_device *netdev)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	return mii_link_ok(&priv->mii);
}

static const struct ethtool_ops ftmac110_ethtool_ops = {
	.set_settings		= ftmac110_set_settings,
	.get_settings		= ftmac110_get_settings,
	.get_drvinfo		= ftmac110_get_drvinfo,
	.nway_reset		= ftmac110_nway_reset,
	.get_link		= ftmac110_get_link,
};

/******************************************************************************
 * interrupt handler
 *****************************************************************************/
static irqreturn_t ftmac110_interrupt(int irq, void *dev_id)
{
	struct net_device *netdev = dev_id;
	struct ftmac110 *priv = netdev_priv(netdev);

	if (likely(netif_running(netdev))) {
		/* Disable interrupts for polling */
		ftmac110_disable_all_int(priv);
		napi_schedule(&priv->napi);
	}

	return IRQ_HANDLED;
}

/******************************************************************************
 * struct napi_struct functions
 *****************************************************************************/
static int ftmac110_poll(struct napi_struct *napi, int budget)
{
	struct ftmac110 *priv = container_of(napi, struct ftmac110, napi);
	struct net_device *netdev = priv->netdev;
	unsigned int status;
	bool completed = true;
	int rx = 0;

	status = ioread32(priv->base + FTMAC110_OFFSET_ISR);
	if (status & (FTMAC110_INT_RPKT_FINISH | FTMAC110_INT_NORXBUF)) {
		/*
		 * FTMAC110_INT_RPKT_FINISH:
		 *	RX DMA has received packets into RX buffer successfully
		 *
		 * FTMAC110_INT_NORXBUF:
		 *	RX buffer unavailable
		 */
		bool retry;

		do {
			retry = ftmac110_rx_packet(priv, &rx);
		} while (retry && rx < budget);

		if (retry && rx == budget)
			completed = false;
	}

	if (status & (FTMAC110_INT_XPKT_OK | FTMAC110_INT_XPKT_LOST)) {
		/*
		 * FTMAC110_INT_XPKT_OK:
		 *	packet transmitted to ethernet successfully
		 *
		 * FTMAC110_INT_XPKT_LOST:
		 *	packet transmitted to ethernet lost due to late
		 *	collision or excessive collision
		 */
		ftmac110_tx_complete(priv);
	}

	if (status & (FTMAC110_INT_NORXBUF | FTMAC110_INT_RPKT_LOST |
		      FTMAC110_INT_AHB_ERR | FTMAC110_INT_PHYSTS_CHG)) {
		if (net_ratelimit())
			netdev_info(netdev, "[ISR] = 0x%x: %s%s%s%s\n", status,
				    status & FTMAC110_INT_NORXBUF ? "NORXBUF " : "",
				    status & FTMAC110_INT_RPKT_LOST ? "RPKT_LOST " : "",
				    status & FTMAC110_INT_AHB_ERR ? "AHB_ERR " : "",
				    status & FTMAC110_INT_PHYSTS_CHG ? "PHYSTS_CHG" : "");

		if (status & FTMAC110_INT_NORXBUF) {
			/* RX buffer unavailable */
			netdev->stats.rx_over_errors++;
		}

		if (status & FTMAC110_INT_RPKT_LOST) {
			/* received packet lost due to RX FIFO full */
			netdev->stats.rx_fifo_errors++;
		}

		if (status & FTMAC110_INT_PHYSTS_CHG) {
			/* PHY link status change */
			mii_check_link(&priv->mii);
		}
	}

	if (completed) {
		/* stop polling */
		napi_complete(napi);
		ftmac110_enable_all_int(priv);
	}

	return rx;
}

/*
 *	Ethernet Link adjust
 *	-phy driver will call this
 *	-duplex, speed: change MAC configuration
 */
static void ftmac110_link_change(struct net_device *ndev)
{
	struct ftmac110 *priv = netdev_priv(ndev);
	struct phy_device *phydev = priv->phydev;

	BUG_ON(!priv->phydev);

	if (phydev->speed == priv->old_speed)
		return;

	phy_print_status(phydev);
	priv->old_link   =  phydev->link;
	priv->old_speed  =  phydev->speed;
	priv->old_duplex =  phydev->duplex;

	ftmac110_disable_all_int(priv);

	netif_stop_queue(ndev);
	ftmac110_stop_hw(priv);
	netif_start_queue(ndev);
	ftmac110_start_hw(priv);

	ftmac110_enable_all_int(priv);
}

static int setup_dma_engine(void)
{
#ifdef CONFIG_SIGMA_SOC_UXLB
	unsigned int temp;
	unsigned short v6c, v6e;

	/* Setup UMAC Agent21 for ethernet */
	temp = ReadRegWord((volatile void *)0x1502206c);
	temp &= ~0x0fff0000;
	temp |= 0x000e0000;
	WriteRegWord((volatile void *)0x1502206c, temp);

	/* Set DMA bridge endian: Little */
	WriteRegHWord((volatile void *)0x1b00016c, 0x0021);
	WriteRegHWord((volatile void *)0x1b00016e, 0x0000);

	v6c = ReadRegHWord((volatile void *)0x1b00016c);
	v6e = ReadRegHWord((volatile void *)0x1b00016e);

	/* Ethernet DMA Bridge Refresh */
	WriteRegHWord((volatile void *)0x1b00016c, v6c & ~0x8);
	WriteRegHWord((volatile void *)0x1b00016e, v6e);

	WriteRegHWord((volatile void *)0x1b00016c, v6c | 0x8);
	WriteRegHWord((volatile void *)0x1b00016e, v6e);
#endif
	return 0;
}

/*
 * Ethernet MII bus detection
 */
static int ftmac110_mii_probe(struct net_device *dev)
{
	struct ftmac110 *const priv = netdev_priv(dev);
	struct phy_device *phy_dev = NULL;
	int phy_addr;

	/*
 	 * phy scan
 	 * find the first (lowest address) PHY
 	 * on the current MAC's MII bus
 	 */
	for(phy_addr = 0 ; phy_addr < PHY_MAX_ADDR ; phy_addr++)
	{
		phy_dev = priv->mii_bus->phy_map[phy_addr];
		if(phy_dev)
			break;
	}
	if(phy_addr >= PHY_MAX_ADDR)
	{
		printk(KERN_ERR"%s: no PHY found\n",dev->name);
		return -ENXIO;
	}

	phy_dev = phy_connect(dev, dev_name(&phy_dev->dev), &ftmac110_link_change, PHY_INTERFACE_MODE_RMII);
	if(IS_ERR(phy_dev))
	{
		pr_err("%s: Could not attach to PHY\n",dev->name);
		return PTR_ERR(phy_dev);
	}

	pr_info("%s: attached PHY driver [%s] (mii_bus:phy_addr=%s, irq=%d)\n",
		dev->name, phy_dev->drv->name,
		dev_name(&phy_dev->dev), phy_dev->irq);

	priv->phy_addr	= phy_addr;
	priv->phydev	= phy_dev;

	priv->phydev->autoneg = AUTONEG_ENABLE;
	priv->phydev->state = PHY_CHANGELINK;
	phy_start(priv->phydev);

	priv->old_link   =  0;
	priv->old_speed  =  0;
	priv->old_duplex = -1;
	setup_dma_engine();

	return 0;
}

static int ftmac110_mii_init(struct net_device *dev)
{
	struct ftmac110 *const priv = netdev_priv(dev);
	int err = -ENXIO;

	priv->mii_bus = mdiobus_alloc();
	if(!priv->mii_bus)
	{
		err = -ENOMEM;
		goto err_out;
	}

	priv->mii_bus->name = "uxl_ftmac_mii_bus";
	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "%s-%x", priv->pdev->name, priv->pdev->id);
	priv->mii_bus->priv = priv;
	priv->mii_bus->read = ftmac110_mdiobus_read;
	priv->mii_bus->write = ftmac110_mdiobus_write;
	priv->mii_bus->parent = priv->pdev->dev.parent;

	if (mdiobus_register(priv->mii_bus))
	{
		pr_err("Error registering mii bus\n");
		goto err_out_free_mdiobus;
	}

	if(ftmac110_mii_probe(priv->netdev) < 0)
	{
		pr_err("Error probing mii bus\n");
		goto err_out_unregister_bus;
	}

	return 0;

err_out_unregister_bus:
	mdiobus_unregister(priv->mii_bus);
err_out_free_mdiobus:
	mdiobus_free(priv->mii_bus);
err_out:
	return err;
}


/******************************************************************************
 * struct net_device_ops functions
 *****************************************************************************/
static int ftmac110_open(struct net_device *netdev)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	int err;

	err = ftmac110_alloc_buffers(priv);
	if (err) {
		netdev_err(netdev, "failed to allocate buffers\n");
		goto err_alloc;
	}

	err = request_irq(priv->irq, ftmac110_interrupt, 0, netdev->name, netdev);
	if (err) {
		netdev_err(netdev, "failed to request irq %d\n", priv->irq);
		goto err_irq;
	}

	priv->rx_pointer = 0;
	priv->tx_clean_pointer = 0;
	priv->tx_pointer = 0;
	priv->tx_pending = 0;

	err = ftmac110_start_hw(priv);
	if (err)
		goto err_hw;

	napi_enable(&priv->napi);
	netif_start_queue(netdev);

	ftmac110_enable_all_int(priv);

	/* cause the PHY state machine to schedule a link state check */
	phy_start(priv->phydev);

	return 0;

err_hw:
	free_irq(priv->irq, netdev);
err_irq:
	ftmac110_free_buffers(priv);
err_alloc:
	return err;
}

static int ftmac110_stop(struct net_device *netdev)
{
	struct ftmac110 *priv = netdev_priv(netdev);

	ftmac110_disable_all_int(priv);
	netif_stop_queue(netdev);
	napi_disable(&priv->napi);
	ftmac110_stop_hw(priv);
	free_irq(priv->irq, netdev);
	ftmac110_free_buffers(priv);

	phy_start(priv->phydev);

	return 0;
}

static int ftmac110_hard_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	dma_addr_t map;

	if (unlikely(skb->len > MAX_PKT_SIZE)) {
		if (net_ratelimit())
			netdev_dbg(netdev, "tx packet too big\n");

		netdev->stats.tx_dropped++;
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	map = dma_map_single(priv->dev, skb->data, skb_headlen(skb), DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(priv->dev, map))) {
		/* drop packet */
		if (net_ratelimit())
			netdev_err(netdev, "map socket buffer failed\n");

		netdev->stats.tx_dropped++;
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	return ftmac110_xmit(priv, skb, map);
}

/* optional */
static int ftmac110_do_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	struct ftmac110 *priv = netdev_priv(netdev);
	struct mii_ioctl_data *data = if_mii(ifr);

	return generic_mii_ioctl(&priv->mii, data, cmd, NULL);
}

static const struct net_device_ops ftmac110_netdev_ops = {
	.ndo_open		= ftmac110_open,
	.ndo_stop		= ftmac110_stop,
	.ndo_start_xmit		= ftmac110_hard_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= ftmac110_do_ioctl,
};

/******************************************************************************
 * struct platform_driver functions
 *****************************************************************************/
static int ftmac110_probe(struct platform_device *pdev)
{
	struct resource *res;
	int irq;
	struct net_device *netdev;
	struct ftmac110 *priv;
	int err;

	if (!pdev)
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	/* setup net_device */
	netdev = alloc_etherdev(sizeof(*priv));
	if (!netdev) {
		err = -ENOMEM;
		goto err_alloc_etherdev;
	}

	SET_NETDEV_DEV(netdev, &pdev->dev);
	SET_ETHTOOL_OPS(netdev, &ftmac110_ethtool_ops);
	netdev->netdev_ops = &ftmac110_netdev_ops;

	platform_set_drvdata(pdev, netdev);

	/* setup private data */
	priv = netdev_priv(netdev);
	priv->netdev = netdev;
	priv->dev = &pdev->dev;
	priv->pdev = pdev;

	spin_lock_init(&priv->tx_lock);

	/* Fill in the fields of the device structure with ethernet values. */
	ether_setup(netdev);

	/* initialize NAPI */
	netif_napi_add(netdev, &priv->napi, ftmac110_poll, 64);

	/* map io memory */
	priv->res = request_mem_region(res->start, resource_size(res),
				       dev_name(&pdev->dev));
	if (!priv->res) {
		dev_err(&pdev->dev, "Could not reserve memory region\n");
		err = -ENOMEM;
		goto err_req_mem;
	}

	priv->base = SIGMA_IO_ADDRESS(res->start);

	priv->irq = irq;

	/* initialize struct mii_if_info */
	priv->mii.phy_id	= 0;
	priv->mii.phy_id_mask	= 0x1f;
	priv->mii.reg_num_mask	= 0x1f;
	priv->mii.dev		= netdev;
	priv->mii.mdio_read	= ftmac110_mdio_read;
	priv->mii.mdio_write	= ftmac110_mdio_write;

	/* register network device */
	err = register_netdev(netdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to register netdev\n");
		goto err_register_netdev;
	}

	netdev_info(netdev, "irq %d, mapped at %p\n", priv->irq, priv->base);

	if (!is_valid_ether_addr(netdev->dev_addr)) {
		eth_hw_addr_random(netdev);
		netdev_info(netdev, "generated random MAC address %pM\n",
			    netdev->dev_addr);
	}

	/*TODO: do minimun hardware init here to be able to probe mii bus */

	/* init mii interface and attach PHY device*/
	err = ftmac110_mii_init(netdev);
	if (err)
		goto err_register_netdev;


	return 0;

err_register_netdev:
	iounmap(priv->base);
err_req_mem:
	netif_napi_del(&priv->napi);
	platform_set_drvdata(pdev, NULL);
	free_netdev(netdev);
err_alloc_etherdev:
	return err;
}

static int __exit ftmac110_remove(struct platform_device *pdev)
{
	struct net_device *netdev;
	struct ftmac110 *priv;

	netdev = platform_get_drvdata(pdev);
	priv = netdev_priv(netdev);

	unregister_netdev(netdev);

	release_resource(priv->res);

	netif_napi_del(&priv->napi);
	platform_set_drvdata(pdev, NULL);
	free_netdev(netdev);
	return 0;
}

static struct platform_driver ftmac110_driver = {
	.probe		= ftmac110_probe,
	.remove		= __exit_p(ftmac110_remove),
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

/******************************************************************************
 * initialization / finalization
 *****************************************************************************/
static int __init ftmac110_init(void)
{
	pr_info("Loading version " DRV_VERSION " ...\n");
	return platform_driver_register(&ftmac110_driver);
}

static void __exit ftmac110_exit(void)
{
	platform_driver_unregister(&ftmac110_driver);
}

module_init(ftmac110_init);
module_exit(ftmac110_exit);

MODULE_AUTHOR("Claud Ling <Claud_Ling@sigmadesigns.com>");
MODULE_DESCRIPTION("FTMAC110 driver");
MODULE_LICENSE("GPL");
