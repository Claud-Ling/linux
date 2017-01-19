/*******************************************************************************
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
*******************************************************************************/

#ifndef __DWMAC_H__
#define __DWMAC_H__

#define DWMAC_RESOURCE_NAME	"dwmacenet"
#define DRV_MODULE_VERSION	"2016-02"

#include <linux/clk.h>
#include <linux/phy.h>
#include <linux/ptp_clock_kernel.h>
#include "common.h"
#include <linux/gmac.h>

struct dwmac_resources {
	void __iomem *addr;	/* virtual base address */
	const char *mac;
	int wol_irq;
	int irq;
};

struct dwmac_tx_info {
	dma_addr_t buf;
	bool map_as_page;
};

struct dwmac_priv {
	/* Frequently used values are kept adjacent for cache effect */
	struct dma_extended_desc *dma_etx ____cacheline_aligned_in_smp;
	struct dma_desc *dma_tx;
	struct sk_buff **tx_skbuff;
	unsigned int cur_tx;
	unsigned int dirty_tx;
	unsigned int dma_tx_size;
	u32 tx_count_frames;
	u32 tx_coal_frames;
	u32 tx_coal_timer;
	struct dwmac_tx_info *tx_skbuff_dma;
	dma_addr_t dma_tx_phy;
	int tx_coalesce;
	int hwts_tx_en;
	spinlock_t tx_lock;
	bool tx_path_in_lpi_mode;
	struct timer_list txtimer;

	struct dma_desc *dma_rx	____cacheline_aligned_in_smp;
	struct dma_extended_desc *dma_erx;
	struct sk_buff **rx_skbuff;
	unsigned int cur_rx;
	unsigned int dirty_rx;
	unsigned int dma_rx_size;
	unsigned int dma_buf_sz;
	u32 rx_riwt;
	int hwts_rx_en;
	dma_addr_t *rx_skbuff_dma;
	dma_addr_t dma_rx_phy;

	struct napi_struct napi ____cacheline_aligned_in_smp;

	void __iomem *ioaddr;
	struct net_device *ndev;
	struct device *device;
	struct mac_device_info *hw;
	spinlock_t lock;

	struct phy_device *phydev ____cacheline_aligned_in_smp;
	int oldlink;
	int speed;
	int oldduplex;
	unsigned int flow_ctrl;
	unsigned int pause;
	struct mii_bus *mii;
	int mii_irq[PHY_MAX_ADDR];

	struct dwmac_extra_stats xstats ____cacheline_aligned_in_smp;
	struct plat_dwmacenet_data *plat;
	struct dma_features dma_cap;
	struct dwmac_counters mmc;
	int hw_cap_support;
	int synopsys_id;
	u32 msg_enable;
	int wolopts;
	int wol_irq;
	struct clk *dwmac_clk;
	struct clk *pclk;
	struct reset_control *dwmac_rst;
	int clk_csr;
	struct timer_list eee_ctrl_timer;
	int lpi_irq;
	int eee_enabled;
	int eee_active;
	int tx_lpi_timer;
	int pcs;
	unsigned int mode;
	int extend_desc;
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_clock_ops;
	unsigned int default_addend;
	struct clk *clk_ptp_ref;
	unsigned int clk_ptp_rate;
	u32 adv_ts;
	int use_riwt;
	int irq_wake;
	spinlock_t ptp_lock;

#ifdef CONFIG_DEBUG_FS
	struct dentry *dbgfs_dir;
	struct dentry *dbgfs_rings_status;
	struct dentry *dbgfs_dma_cap;
#endif
};

int dwmac_mdio_unregister(struct net_device *ndev);
int dwmac_mdio_register(struct net_device *ndev);
int dwmac_mdio_reset(struct mii_bus *mii);
void dwmac_set_ethtool_ops(struct net_device *netdev);

int dwmac_ptp_register(struct dwmac_priv *priv);
void dwmac_ptp_unregister(struct dwmac_priv *priv);
int dwmac_resume(struct net_device *ndev);
int dwmac_suspend(struct net_device *ndev);
int dwmac_dvr_remove(struct net_device *ndev);
int dwmac_dvr_probe(struct device *device,
		     struct plat_dwmacenet_data *plat_dat,
		     struct dwmac_resources *res);
void dwmac_disable_eee_mode(struct dwmac_priv *priv);
bool dwmac_eee_init(struct dwmac_priv *priv);

#endif /* __DWMAC_H__ */
