/*******************************************************************************
  DWMAC Ethtool support

  Copyright (C) 2007-2009  DWicroelectronics Ltd

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/interrupt.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/net_tstamp.h>
#include <asm/io.h>

#include "dwmac.h"
#include "dwmac_dma.h"

#define REG_SPACE_SIZE	0x1054
#define MAC100_ETHTOOL_NAME	"st_mac100"
#define GMAC_ETHTOOL_NAME	"st_gmac"

struct dwmac_stats {
	char stat_string[ETH_GSTRING_LEN];
	int sizeof_stat;
	int stat_offset;
};

#define DWMAC_STAT(m)	\
	{ #m, FIELD_SIZEOF(struct dwmac_extra_stats, m),	\
	offsetof(struct dwmac_priv, xstats.m)}

static const struct dwmac_stats dwmac_gstrings_stats[] = {
	/* Transmit errors */
	DWMAC_STAT(tx_underflow),
	DWMAC_STAT(tx_carrier),
	DWMAC_STAT(tx_losscarrier),
	DWMAC_STAT(vlan_tag),
	DWMAC_STAT(tx_deferred),
	DWMAC_STAT(tx_vlan),
	DWMAC_STAT(tx_jabber),
	DWMAC_STAT(tx_frame_flushed),
	DWMAC_STAT(tx_payload_error),
	DWMAC_STAT(tx_ip_header_error),
	/* Receive errors */
	DWMAC_STAT(rx_desc),
	DWMAC_STAT(sa_filter_fail),
	DWMAC_STAT(overflow_error),
	DWMAC_STAT(ipc_csum_error),
	DWMAC_STAT(rx_collision),
	DWMAC_STAT(rx_crc),
	DWMAC_STAT(dribbling_bit),
	DWMAC_STAT(rx_length),
	DWMAC_STAT(rx_mii),
	DWMAC_STAT(rx_multicast),
	DWMAC_STAT(rx_gmac_overflow),
	DWMAC_STAT(rx_watchdog),
	DWMAC_STAT(da_rx_filter_fail),
	DWMAC_STAT(sa_rx_filter_fail),
	DWMAC_STAT(rx_missed_cntr),
	DWMAC_STAT(rx_overflow_cntr),
	DWMAC_STAT(rx_vlan),
	/* Tx/Rx IRQ error info */
	DWMAC_STAT(tx_undeflow_irq),
	DWMAC_STAT(tx_process_stopped_irq),
	DWMAC_STAT(tx_jabber_irq),
	DWMAC_STAT(rx_overflow_irq),
	DWMAC_STAT(rx_buf_unav_irq),
	DWMAC_STAT(rx_process_stopped_irq),
	DWMAC_STAT(rx_watchdog_irq),
	DWMAC_STAT(tx_early_irq),
	DWMAC_STAT(fatal_bus_error_irq),
	/* Tx/Rx IRQ Events */
	DWMAC_STAT(rx_early_irq),
	DWMAC_STAT(threshold),
	DWMAC_STAT(tx_pkt_n),
	DWMAC_STAT(rx_pkt_n),
	DWMAC_STAT(normal_irq_n),
	DWMAC_STAT(rx_normal_irq_n),
	DWMAC_STAT(napi_poll),
	DWMAC_STAT(tx_normal_irq_n),
	DWMAC_STAT(tx_clean),
	DWMAC_STAT(tx_reset_ic_bit),
	DWMAC_STAT(irq_receive_pmt_irq_n),
	/* MMC info */
	DWMAC_STAT(mmc_tx_irq_n),
	DWMAC_STAT(mmc_rx_irq_n),
	DWMAC_STAT(mmc_rx_csum_offload_irq_n),
	/* EEE */
	DWMAC_STAT(irq_tx_path_in_lpi_mode_n),
	DWMAC_STAT(irq_tx_path_exit_lpi_mode_n),
	DWMAC_STAT(irq_rx_path_in_lpi_mode_n),
	DWMAC_STAT(irq_rx_path_exit_lpi_mode_n),
	DWMAC_STAT(phy_eee_wakeup_error_n),
	/* Extended RDES status */
	DWMAC_STAT(ip_hdr_err),
	DWMAC_STAT(ip_payload_err),
	DWMAC_STAT(ip_csum_bypassed),
	DWMAC_STAT(ipv4_pkt_rcvd),
	DWMAC_STAT(ipv6_pkt_rcvd),
	DWMAC_STAT(rx_msg_type_ext_no_ptp),
	DWMAC_STAT(rx_msg_type_sync),
	DWMAC_STAT(rx_msg_type_follow_up),
	DWMAC_STAT(rx_msg_type_delay_req),
	DWMAC_STAT(rx_msg_type_delay_resp),
	DWMAC_STAT(rx_msg_type_pdelay_req),
	DWMAC_STAT(rx_msg_type_pdelay_resp),
	DWMAC_STAT(rx_msg_type_pdelay_follow_up),
	DWMAC_STAT(ptp_frame_type),
	DWMAC_STAT(ptp_ver),
	DWMAC_STAT(timestamp_dropped),
	DWMAC_STAT(av_pkt_rcvd),
	DWMAC_STAT(av_tagged_pkt_rcvd),
	DWMAC_STAT(vlan_tag_priority_val),
	DWMAC_STAT(l3_filter_match),
	DWMAC_STAT(l4_filter_match),
	DWMAC_STAT(l3_l4_filter_no_match),
	/* PCS */
	DWMAC_STAT(irq_pcs_ane_n),
	DWMAC_STAT(irq_pcs_link_n),
	DWMAC_STAT(irq_rgmii_n),
};
#define DWMAC_STATS_LEN ARRAY_SIZE(dwmac_gstrings_stats)

/* HW MAC Management counters (if supported) */
#define DWMAC_MMC_STAT(m)	\
	{ #m, FIELD_SIZEOF(struct dwmac_counters, m),	\
	offsetof(struct dwmac_priv, mmc.m)}

static const struct dwmac_stats dwmac_mmc[] = {
	DWMAC_MMC_STAT(mmc_tx_octetcount_gb),
	DWMAC_MMC_STAT(mmc_tx_framecount_gb),
	DWMAC_MMC_STAT(mmc_tx_broadcastframe_g),
	DWMAC_MMC_STAT(mmc_tx_multicastframe_g),
	DWMAC_MMC_STAT(mmc_tx_64_octets_gb),
	DWMAC_MMC_STAT(mmc_tx_65_to_127_octets_gb),
	DWMAC_MMC_STAT(mmc_tx_128_to_255_octets_gb),
	DWMAC_MMC_STAT(mmc_tx_256_to_511_octets_gb),
	DWMAC_MMC_STAT(mmc_tx_512_to_1023_octets_gb),
	DWMAC_MMC_STAT(mmc_tx_1024_to_max_octets_gb),
	DWMAC_MMC_STAT(mmc_tx_unicast_gb),
	DWMAC_MMC_STAT(mmc_tx_multicast_gb),
	DWMAC_MMC_STAT(mmc_tx_broadcast_gb),
	DWMAC_MMC_STAT(mmc_tx_underflow_error),
	DWMAC_MMC_STAT(mmc_tx_singlecol_g),
	DWMAC_MMC_STAT(mmc_tx_multicol_g),
	DWMAC_MMC_STAT(mmc_tx_deferred),
	DWMAC_MMC_STAT(mmc_tx_latecol),
	DWMAC_MMC_STAT(mmc_tx_exesscol),
	DWMAC_MMC_STAT(mmc_tx_carrier_error),
	DWMAC_MMC_STAT(mmc_tx_octetcount_g),
	DWMAC_MMC_STAT(mmc_tx_framecount_g),
	DWMAC_MMC_STAT(mmc_tx_excessdef),
	DWMAC_MMC_STAT(mmc_tx_pause_frame),
	DWMAC_MMC_STAT(mmc_tx_vlan_frame_g),
	DWMAC_MMC_STAT(mmc_rx_framecount_gb),
	DWMAC_MMC_STAT(mmc_rx_octetcount_gb),
	DWMAC_MMC_STAT(mmc_rx_octetcount_g),
	DWMAC_MMC_STAT(mmc_rx_broadcastframe_g),
	DWMAC_MMC_STAT(mmc_rx_multicastframe_g),
	DWMAC_MMC_STAT(mmc_rx_crc_error),
	DWMAC_MMC_STAT(mmc_rx_align_error),
	DWMAC_MMC_STAT(mmc_rx_run_error),
	DWMAC_MMC_STAT(mmc_rx_jabber_error),
	DWMAC_MMC_STAT(mmc_rx_undersize_g),
	DWMAC_MMC_STAT(mmc_rx_oversize_g),
	DWMAC_MMC_STAT(mmc_rx_64_octets_gb),
	DWMAC_MMC_STAT(mmc_rx_65_to_127_octets_gb),
	DWMAC_MMC_STAT(mmc_rx_128_to_255_octets_gb),
	DWMAC_MMC_STAT(mmc_rx_256_to_511_octets_gb),
	DWMAC_MMC_STAT(mmc_rx_512_to_1023_octets_gb),
	DWMAC_MMC_STAT(mmc_rx_1024_to_max_octets_gb),
	DWMAC_MMC_STAT(mmc_rx_unicast_g),
	DWMAC_MMC_STAT(mmc_rx_length_error),
	DWMAC_MMC_STAT(mmc_rx_autofrangetype),
	DWMAC_MMC_STAT(mmc_rx_pause_frames),
	DWMAC_MMC_STAT(mmc_rx_fifo_overflow),
	DWMAC_MMC_STAT(mmc_rx_vlan_frames_gb),
	DWMAC_MMC_STAT(mmc_rx_watchdog_error),
	DWMAC_MMC_STAT(mmc_rx_ipc_intr_mask),
	DWMAC_MMC_STAT(mmc_rx_ipc_intr),
	DWMAC_MMC_STAT(mmc_rx_ipv4_gd),
	DWMAC_MMC_STAT(mmc_rx_ipv4_hderr),
	DWMAC_MMC_STAT(mmc_rx_ipv4_nopay),
	DWMAC_MMC_STAT(mmc_rx_ipv4_frag),
	DWMAC_MMC_STAT(mmc_rx_ipv4_udsbl),
	DWMAC_MMC_STAT(mmc_rx_ipv4_gd_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv4_hderr_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv4_nopay_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv4_frag_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv4_udsbl_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv6_gd_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv6_hderr_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv6_nopay_octets),
	DWMAC_MMC_STAT(mmc_rx_ipv6_gd),
	DWMAC_MMC_STAT(mmc_rx_ipv6_hderr),
	DWMAC_MMC_STAT(mmc_rx_ipv6_nopay),
	DWMAC_MMC_STAT(mmc_rx_udp_gd),
	DWMAC_MMC_STAT(mmc_rx_udp_err),
	DWMAC_MMC_STAT(mmc_rx_tcp_gd),
	DWMAC_MMC_STAT(mmc_rx_tcp_err),
	DWMAC_MMC_STAT(mmc_rx_icmp_gd),
	DWMAC_MMC_STAT(mmc_rx_icmp_err),
	DWMAC_MMC_STAT(mmc_rx_udp_gd_octets),
	DWMAC_MMC_STAT(mmc_rx_udp_err_octets),
	DWMAC_MMC_STAT(mmc_rx_tcp_gd_octets),
	DWMAC_MMC_STAT(mmc_rx_tcp_err_octets),
	DWMAC_MMC_STAT(mmc_rx_icmp_gd_octets),
	DWMAC_MMC_STAT(mmc_rx_icmp_err_octets),
};
#define DWMAC_MMC_STATS_LEN ARRAY_SIZE(dwmac_mmc)

static void dwmac_ethtool_getdrvinfo(struct net_device *dev,
				      struct ethtool_drvinfo *info)
{
	struct dwmac_priv *priv = netdev_priv(dev);

	if (priv->plat->has_gmac)
		strlcpy(info->driver, GMAC_ETHTOOL_NAME, sizeof(info->driver));
	else
		strlcpy(info->driver, MAC100_ETHTOOL_NAME,
			sizeof(info->driver));

	strlcpy(info->version, DRV_MODULE_VERSION, sizeof(info->version));
}

static int dwmac_ethtool_getsettings(struct net_device *dev,
				      struct ethtool_cmd *cmd)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	struct phy_device *phy = priv->phydev;
	int rc;

	if ((priv->pcs & DWMAC_PCS_RGMII) || (priv->pcs & DWMAC_PCS_SGMII)) {
		struct rgmii_adv adv;

		if (!priv->xstats.pcs_link) {
			ethtool_cmd_speed_set(cmd, SPEED_UNKNOWN);
			cmd->duplex = DUPLEX_UNKNOWN;
			return 0;
		}
		cmd->duplex = priv->xstats.pcs_duplex;

		ethtool_cmd_speed_set(cmd, priv->xstats.pcs_speed);

		/* Get and convert ADV/LP_ADV from the HW AN registers */
		if (!priv->hw->mac->get_adv)
			return -EOPNOTSUPP;	/* should never happen indeed */

		priv->hw->mac->get_adv(priv->hw, &adv);

		/* Encoding of PSE bits is defined in 802.3z, 37.2.1.4 */

		if (adv.pause & DWMAC_PCS_PAUSE)
			cmd->advertising |= ADVERTISED_Pause;
		if (adv.pause & DWMAC_PCS_ASYM_PAUSE)
			cmd->advertising |= ADVERTISED_Asym_Pause;
		if (adv.lp_pause & DWMAC_PCS_PAUSE)
			cmd->lp_advertising |= ADVERTISED_Pause;
		if (adv.lp_pause & DWMAC_PCS_ASYM_PAUSE)
			cmd->lp_advertising |= ADVERTISED_Asym_Pause;

		/* Reg49[3] always set because ANE is always supported */
		cmd->autoneg = ADVERTISED_Autoneg;
		cmd->supported |= SUPPORTED_Autoneg;
		cmd->advertising |= ADVERTISED_Autoneg;
		cmd->lp_advertising |= ADVERTISED_Autoneg;

		if (adv.duplex) {
			cmd->supported |= (SUPPORTED_1000baseT_Full |
					   SUPPORTED_100baseT_Full |
					   SUPPORTED_10baseT_Full);
			cmd->advertising |= (ADVERTISED_1000baseT_Full |
					     ADVERTISED_100baseT_Full |
					     ADVERTISED_10baseT_Full);
		} else {
			cmd->supported |= (SUPPORTED_1000baseT_Half |
					   SUPPORTED_100baseT_Half |
					   SUPPORTED_10baseT_Half);
			cmd->advertising |= (ADVERTISED_1000baseT_Half |
					     ADVERTISED_100baseT_Half |
					     ADVERTISED_10baseT_Half);
		}
		if (adv.lp_duplex)
			cmd->lp_advertising |= (ADVERTISED_1000baseT_Full |
						ADVERTISED_100baseT_Full |
						ADVERTISED_10baseT_Full);
		else
			cmd->lp_advertising |= (ADVERTISED_1000baseT_Half |
						ADVERTISED_100baseT_Half |
						ADVERTISED_10baseT_Half);
		cmd->port = PORT_OTHER;

		return 0;
	}

	if (phy == NULL) {
		pr_err("%s: %s: PHY is not registered\n",
		       __func__, dev->name);
		return -ENODEV;
	}
	if (!netif_running(dev)) {
		pr_err("%s: interface is disabled: we cannot track "
		"link speed / duplex setting\n", dev->name);
		return -EBUSY;
	}
	cmd->transceiver = XCVR_INTERNAL;
	rc = phy_ethtool_gset(phy, cmd);
	return rc;
}

static int dwmac_ethtool_setsettings(struct net_device *dev,
				      struct ethtool_cmd *cmd)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	struct phy_device *phy = priv->phydev;
	int rc;

	if ((priv->pcs & DWMAC_PCS_RGMII) || (priv->pcs & DWMAC_PCS_SGMII)) {
		u32 mask = ADVERTISED_Autoneg | ADVERTISED_Pause;

		/* Only support ANE */
		if (cmd->autoneg != AUTONEG_ENABLE)
			return -EINVAL;

		mask &= (ADVERTISED_1000baseT_Half |
			ADVERTISED_1000baseT_Full |
			ADVERTISED_100baseT_Half |
			ADVERTISED_100baseT_Full |
			ADVERTISED_10baseT_Half |
			ADVERTISED_10baseT_Full);

		spin_lock(&priv->lock);
		if (priv->hw->mac->ctrl_ane)
			priv->hw->mac->ctrl_ane(priv->hw, 1);
		spin_unlock(&priv->lock);

		return 0;
	}

	spin_lock(&priv->lock);
	rc = phy_ethtool_sset(phy, cmd);
	spin_unlock(&priv->lock);

	return rc;
}

static u32 dwmac_ethtool_getmsglevel(struct net_device *dev)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	return priv->msg_enable;
}

static void dwmac_ethtool_setmsglevel(struct net_device *dev, u32 level)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	priv->msg_enable = level;

}

static int dwmac_check_if_running(struct net_device *dev)
{
	if (!netif_running(dev))
		return -EBUSY;
	return 0;
}

static int dwmac_ethtool_get_regs_len(struct net_device *dev)
{
	return REG_SPACE_SIZE;
}

static void dwmac_ethtool_gregs(struct net_device *dev,
			  struct ethtool_regs *regs, void *space)
{
	int i;
	u32 *reg_space = (u32 *) space;

	struct dwmac_priv *priv = netdev_priv(dev);

	memset(reg_space, 0x0, REG_SPACE_SIZE);

	if (!priv->plat->has_gmac) {
		/* MAC registers */
		for (i = 0; i < 12; i++)
			reg_space[i] = readl(priv->ioaddr + (i * 4));
		/* DMA registers */
		for (i = 0; i < 9; i++)
			reg_space[i + 12] =
			    readl(priv->ioaddr + (DMA_BUS_MODE + (i * 4)));
		reg_space[22] = readl(priv->ioaddr + DMA_CUR_TX_BUF_ADDR);
		reg_space[23] = readl(priv->ioaddr + DMA_CUR_RX_BUF_ADDR);
	} else {
		/* MAC registers */
		for (i = 0; i < 55; i++)
			reg_space[i] = readl(priv->ioaddr + (i * 4));
		/* DMA registers */
		for (i = 0; i < 22; i++)
			reg_space[i + 55] =
			    readl(priv->ioaddr + (DMA_BUS_MODE + (i * 4)));
	}
}

static void
dwmac_get_pauseparam(struct net_device *netdev,
		      struct ethtool_pauseparam *pause)
{
	struct dwmac_priv *priv = netdev_priv(netdev);

	if (priv->pcs)	/* FIXME */
		return;

	pause->rx_pause = 0;
	pause->tx_pause = 0;
	pause->autoneg = priv->phydev->autoneg;

	if (priv->flow_ctrl & FLOW_RX)
		pause->rx_pause = 1;
	if (priv->flow_ctrl & FLOW_TX)
		pause->tx_pause = 1;

}

static int
dwmac_set_pauseparam(struct net_device *netdev,
		      struct ethtool_pauseparam *pause)
{
	struct dwmac_priv *priv = netdev_priv(netdev);
	struct phy_device *phy = priv->phydev;
	int new_pause = FLOW_OFF;
	int ret = 0;

	if (priv->pcs)	/* FIXME */
		return -EOPNOTSUPP;

	if (pause->rx_pause)
		new_pause |= FLOW_RX;
	if (pause->tx_pause)
		new_pause |= FLOW_TX;

	priv->flow_ctrl = new_pause;
	phy->autoneg = pause->autoneg;

	if (phy->autoneg) {
		if (netif_running(netdev))
			ret = phy_start_aneg(phy);
	} else
		priv->hw->mac->flow_ctrl(priv->hw, phy->duplex,
					 priv->flow_ctrl, priv->pause);
	return ret;
}

static void dwmac_get_ethtool_stats(struct net_device *dev,
				 struct ethtool_stats *dummy, u64 *data)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	int i, j = 0;

	/* Update the DMA HW counters for dwmac10/100 */
	if (!priv->plat->has_gmac)
		priv->hw->dma->dma_diagnostic_fr(&dev->stats,
						 (void *) &priv->xstats,
						 priv->ioaddr);
	else {
		/* If supported, for new GMAC chips expose the MMC counters */
		if (priv->dma_cap.rmon) {
			dwmac_mmc_read(priv->ioaddr, &priv->mmc);

			for (i = 0; i < DWMAC_MMC_STATS_LEN; i++) {
				char *p;
				p = (char *)priv + dwmac_mmc[i].stat_offset;

				data[j++] = (dwmac_mmc[i].sizeof_stat ==
					     sizeof(u64)) ? (*(u64 *)p) :
					     (*(u32 *)p);
			}
		}
		if (priv->eee_enabled) {
			int val = phy_get_eee_err(priv->phydev);
			if (val)
				priv->xstats.phy_eee_wakeup_error_n = val;
		}
	}
	for (i = 0; i < DWMAC_STATS_LEN; i++) {
		char *p = (char *)priv + dwmac_gstrings_stats[i].stat_offset;
		data[j++] = (dwmac_gstrings_stats[i].sizeof_stat ==
			     sizeof(u64)) ? (*(u64 *)p) : (*(u32 *)p);
	}
}

static int dwmac_get_sset_count(struct net_device *netdev, int sset)
{
	struct dwmac_priv *priv = netdev_priv(netdev);
	int len;

	switch (sset) {
	case ETH_SS_STATS:
		len = DWMAC_STATS_LEN;

		if (priv->dma_cap.rmon)
			len += DWMAC_MMC_STATS_LEN;

		return len;
	default:
		return -EOPNOTSUPP;
	}
}

static void dwmac_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	int i;
	u8 *p = data;
	struct dwmac_priv *priv = netdev_priv(dev);

	switch (stringset) {
	case ETH_SS_STATS:
		if (priv->dma_cap.rmon)
			for (i = 0; i < DWMAC_MMC_STATS_LEN; i++) {
				memcpy(p, dwmac_mmc[i].stat_string,
				       ETH_GSTRING_LEN);
				p += ETH_GSTRING_LEN;
			}
		for (i = 0; i < DWMAC_STATS_LEN; i++) {
			memcpy(p, dwmac_gstrings_stats[i].stat_string,
				ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		break;
	default:
		WARN_ON(1);
		break;
	}
}

/* Currently only support WOL through Magic packet. */
static void dwmac_get_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	struct dwmac_priv *priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);
	if (device_can_wakeup(priv->device)) {
		wol->supported = WAKE_MAGIC | WAKE_UCAST;
		wol->wolopts = priv->wolopts;
	}
	spin_unlock_irq(&priv->lock);
}

static int dwmac_set_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	u32 support = WAKE_MAGIC | WAKE_UCAST;

	/* By default almost all GMAC devices support the WoL via
	 * magic frame but we can disable it if the HW capability
	 * register shows no support for pmt_magic_frame. */
	if ((priv->hw_cap_support) && (!priv->dma_cap.pmt_magic_frame))
		wol->wolopts &= ~WAKE_MAGIC;

	if (!device_can_wakeup(priv->device))
		return -EINVAL;

	if (wol->wolopts & ~support)
		return -EINVAL;

	if (wol->wolopts) {
		pr_info("dwmac: wakeup enable\n");
		device_set_wakeup_enable(priv->device, 1);
		enable_irq_wake(priv->wol_irq);
	} else {
		device_set_wakeup_enable(priv->device, 0);
		disable_irq_wake(priv->wol_irq);
	}

	spin_lock_irq(&priv->lock);
	priv->wolopts = wol->wolopts;
	spin_unlock_irq(&priv->lock);

	return 0;
}

static int dwmac_ethtool_op_get_eee(struct net_device *dev,
				     struct ethtool_eee *edata)
{
	struct dwmac_priv *priv = netdev_priv(dev);

	if (!priv->dma_cap.eee)
		return -EOPNOTSUPP;

	edata->eee_enabled = priv->eee_enabled;
	edata->eee_active = priv->eee_active;
	edata->tx_lpi_timer = priv->tx_lpi_timer;

	return phy_ethtool_get_eee(priv->phydev, edata);
}

static int dwmac_ethtool_op_set_eee(struct net_device *dev,
				     struct ethtool_eee *edata)
{
	struct dwmac_priv *priv = netdev_priv(dev);

	priv->eee_enabled = edata->eee_enabled;

	if (!priv->eee_enabled)
		dwmac_disable_eee_mode(priv);
	else {
		/* We are asking for enabling the EEE but it is safe
		 * to verify all by invoking the eee_init function.
		 * In case of failure it will return an error.
		 */
		priv->eee_enabled = dwmac_eee_init(priv);
		if (!priv->eee_enabled)
			return -EOPNOTSUPP;

		/* Do not change tx_lpi_timer in case of failure */
		priv->tx_lpi_timer = edata->tx_lpi_timer;
	}

	return phy_ethtool_set_eee(priv->phydev, edata);
}

static u32 dwmac_usec2riwt(u32 usec, struct dwmac_priv *priv)
{
	unsigned long clk = clk_get_rate(priv->dwmac_clk);

	if (!clk)
		return 0;

	return (usec * (clk / 1000000)) / 256;
}

static u32 dwmac_riwt2usec(u32 riwt, struct dwmac_priv *priv)
{
	unsigned long clk = clk_get_rate(priv->dwmac_clk);

	if (!clk)
		return 0;

	return (riwt * 256) / (clk / 1000000);
}

static int dwmac_get_coalesce(struct net_device *dev,
			       struct ethtool_coalesce *ec)
{
	struct dwmac_priv *priv = netdev_priv(dev);

	ec->tx_coalesce_usecs = priv->tx_coal_timer;
	ec->tx_max_coalesced_frames = priv->tx_coal_frames;

	if (priv->use_riwt)
		ec->rx_coalesce_usecs = dwmac_riwt2usec(priv->rx_riwt, priv);

	return 0;
}

static int dwmac_set_coalesce(struct net_device *dev,
			       struct ethtool_coalesce *ec)
{
	struct dwmac_priv *priv = netdev_priv(dev);
	unsigned int rx_riwt;

	/* Check not supported parameters  */
	if ((ec->rx_max_coalesced_frames) || (ec->rx_coalesce_usecs_irq) ||
	    (ec->rx_max_coalesced_frames_irq) || (ec->tx_coalesce_usecs_irq) ||
	    (ec->use_adaptive_rx_coalesce) || (ec->use_adaptive_tx_coalesce) ||
	    (ec->pkt_rate_low) || (ec->rx_coalesce_usecs_low) ||
	    (ec->rx_max_coalesced_frames_low) || (ec->tx_coalesce_usecs_high) ||
	    (ec->tx_max_coalesced_frames_low) || (ec->pkt_rate_high) ||
	    (ec->tx_coalesce_usecs_low) || (ec->rx_coalesce_usecs_high) ||
	    (ec->rx_max_coalesced_frames_high) ||
	    (ec->tx_max_coalesced_frames_irq) ||
	    (ec->stats_block_coalesce_usecs) ||
	    (ec->tx_max_coalesced_frames_high) || (ec->rate_sample_interval))
		return -EOPNOTSUPP;

	if (ec->rx_coalesce_usecs == 0)
		return -EINVAL;

	if ((ec->tx_coalesce_usecs == 0) &&
	    (ec->tx_max_coalesced_frames == 0))
		return -EINVAL;

	if ((ec->tx_coalesce_usecs > DWMAC_MAX_COAL_TX_TICK) ||
	    (ec->tx_max_coalesced_frames > DWMAC_TX_MAX_FRAMES))
		return -EINVAL;

	rx_riwt = dwmac_usec2riwt(ec->rx_coalesce_usecs, priv);

	if ((rx_riwt > MAX_DMA_RIWT) || (rx_riwt < MIN_DMA_RIWT))
		return -EINVAL;
	else if (!priv->use_riwt)
		return -EOPNOTSUPP;

	/* Only copy relevant parameters, ignore all others. */
	priv->tx_coal_frames = ec->tx_max_coalesced_frames;
	priv->tx_coal_timer = ec->tx_coalesce_usecs;
	priv->rx_riwt = rx_riwt;
	priv->hw->dma->rx_watchdog(priv->ioaddr, priv->rx_riwt);

	return 0;
}

static const struct ethtool_ops dwmac_ethtool_ops = {
	.begin = dwmac_check_if_running,
	.get_drvinfo = dwmac_ethtool_getdrvinfo,
	.get_settings = dwmac_ethtool_getsettings,
	.set_settings = dwmac_ethtool_setsettings,
	.get_msglevel = dwmac_ethtool_getmsglevel,
	.set_msglevel = dwmac_ethtool_setmsglevel,
	.get_regs = dwmac_ethtool_gregs,
	.get_regs_len = dwmac_ethtool_get_regs_len,
	.get_link = ethtool_op_get_link,
	.get_pauseparam = dwmac_get_pauseparam,
	.set_pauseparam = dwmac_set_pauseparam,
	.get_ethtool_stats = dwmac_get_ethtool_stats,
	.get_strings = dwmac_get_strings,
	.get_wol = dwmac_get_wol,
	.set_wol = dwmac_set_wol,
	.get_eee = dwmac_ethtool_op_get_eee,
	.set_eee = dwmac_ethtool_op_set_eee,
	.get_sset_count	= dwmac_get_sset_count,
	.get_coalesce = dwmac_get_coalesce,
	.set_coalesce = dwmac_set_coalesce,
};

void dwmac_set_ethtool_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &dwmac_ethtool_ops;
}
