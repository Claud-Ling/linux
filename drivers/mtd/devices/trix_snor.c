/*
 * trix_snor.c - SPI nor-flash Controller
 *
 * Copyright (C) 2016 SigmaDeisgns
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/spi-nor.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>


#define DRIVER_NAME "trix-snor"
#define SNOR_DGB(f, x...) \
	pr_debug(DRIVER_NAME " [%s()]: " f, __func__,## x)

/* ---------------------------------------------------------------------
	SPI NOR FLASH REGISTER DEFINE
   ---------------------------------------------------------------------*/

/* macros for registers read/write */
#define snor_writel(snor, off, val)     \
        __raw_writel((val), (snor)->base + (off))

#define snor_readl(info, off)           \
        __raw_readl((snor)->base + (off))

#define SNOR_DCR             (0x00000008) /* data control register */
#define SNOR_SPI_DLY_CTRL    (0x0000020) /* SPI mode clock delay control register */
#define SNOR_SPI_TIMG_CTRL0  (0x0000024) /* SPI mode timing control register 0 */

#define SNOR_SPI_TIMG_CTRL1_OFFSET	(0x0000028) /* SPI mode timing control register 1 */
#define SNOR_SPI_TIMG_RESET_MASK	(0x14010501)
#define SNOR_RECV_DATA_LEN(x)		((x) & 0xf)
#define SNOR_RECV_DATA_LEN_SHIFT	(20)
#define SNOR_RECV_DATA_LEN_MASK		(0xf << SNOR_RECV_DATA_LEN_SHIFT) /*bit[23:20] bytes receive from SPI*/

#define SNOR_SYN_MODE0_OFFSET		(0x00000010) /* sync mode control 0 register */

#define SNOR_SYN_MODE1_OFFSET		(0x00000014) /* sync mode control 1 register */
#define SNOR_TRANS_DATA_LEN(x)		((x) & 0x1ff)

#define SNOR_SPI_MODE_CMD_OFFSET	(0x00000018) /* sync mode control 2 register */
#define SNOR_MODE_RESET_MASK		(0x04031000)
#define SNOR_MODE_HW_MODE		(0x00000100)
#define SNOR_CLK_EN			(0x00000001)
#define SNOR_SOFT_WR			(0x00000002)
#define SNOR_SOFT_RD			(0x00000004)
#define SNOR_SOFT_OP_END		(0x00000020)

#define SNOR_OP_MODES_OFFSET		(0x0000001c)
#define SNOR_TRANS_DATA_MASK		(0xff << 0)
#define SNOR_RECV_DATA_MASK		(0xff << 16)
#define SNOR_TX_FINISH			(0x1  << 24)
#define SNOR_RX_FINISH			(0x1  << 25)

#define SPI_BUFSIZ	512
#define SNOR_MAX_WAIT_SEQ_MS  100     /* SPI execution time */

//for STB enable, there is 128K offset for spi controller
#define SPI_STB_OFFSET 0x20000

enum spi_mode {
	SPI_SW_MODE = 0,
	SPI_HW_MODE
};

/*
 * struct trix_snor - Struct for NOR-flash controller
 */
struct trix_snor {
	struct device		*dev;
	struct spi_nor		spi_nor;
	enum spi_mode		mode;
	bool			is_hw_read;
	u8			buf[SPI_BUFSIZ];

	void __iomem		*base;
	void __iomem		*io;
};

/*
 * snor_msg - SPI transaction, a read/write buffer pair
 * @tx_buf: The buffer from which it's written.
 * @rx_buf: The buffer into which data is read
 * @tx_len: Number of data bytes in @tx_buf being written to
 * @rx_len: Number of data bytes in @rx_buf being read from
 */
struct snor_message {
	const void 	*tx_buf;
	void		*rx_buf;
	unsigned int	tx_len;
	unsigned int	rx_len;
};

/*
 * static partition tables
 * -----------------------
 * |  0 - 128K |  mcu    |
 * -----------------------
 * |128k- 640k |  boot   |
 * -----------------------
 */
static struct mtd_partition spi_partitions[] = {
	[0] = {
		.name	= "nor-mcu",
		.offset	= 0,
		.size	= SPI_STB_OFFSET,
	},
	[1] = {
		.name	= "nor-boot",
		.offset	= SPI_STB_OFFSET,
		.size	= 0x80000,
	},
};

static int snor_trans_init(struct trix_snor *snor)
{
	u32 val = SNOR_MODE_RESET_MASK;

	if (snor->mode == SPI_HW_MODE) {
		val |= SNOR_MODE_HW_MODE;
	} else {
		val &= ~SNOR_MODE_HW_MODE;
	}

	snor_writel(snor, SNOR_SPI_MODE_CMD_OFFSET, val);

	return 0;
}

static int snor_trans_end(struct trix_snor *snor)
{
	u32 op;

	/* SPI operation end in software mode*/
	op = SNOR_MODE_RESET_MASK | SNOR_SOFT_OP_END;
	snor_writel(snor, SNOR_SPI_MODE_CMD_OFFSET, op);

	return 0;
}

static void snor_tx(struct trix_snor *snor, u8 data)
{
	u32 val;

	val = snor_readl(snor, SNOR_OP_MODES_OFFSET);
	val &= ~(SNOR_TRANS_DATA_MASK | SNOR_TX_FINISH | SNOR_RX_FINISH);
	val |= (u32)data;
	snor_writel(snor, SNOR_OP_MODES_OFFSET, val);

}

static void snor_rx(struct trix_snor *snor, u8 *data)
{
	u32 val;

	val = snor_readl(snor, SNOR_OP_MODES_OFFSET);
	*data = ((val & SNOR_RECV_DATA_MASK) >> 16);
}

static void snor_issue_tx_cmd(struct trix_snor *snor)
{
	u32 op;

	/* toggle bit[1] to start the transmit */
	op = SNOR_MODE_RESET_MASK | SNOR_CLK_EN;
	snor_writel(snor, SNOR_SPI_MODE_CMD_OFFSET,op);

	op = SNOR_MODE_RESET_MASK | SNOR_CLK_EN | SNOR_SOFT_WR;
	snor_writel(snor, SNOR_SPI_MODE_CMD_OFFSET, op);
}

static void snor_issue_rx_cmd(struct trix_snor *snor)
{
	u32 op;

	/* toggle bit[2] to start the receiver */
	op = SNOR_MODE_RESET_MASK | SNOR_CLK_EN;
	snor_writel(snor, SNOR_SPI_MODE_CMD_OFFSET,op);

	op = SNOR_MODE_RESET_MASK | SNOR_CLK_EN | SNOR_SOFT_RD;
	snor_writel(snor, SNOR_SPI_MODE_CMD_OFFSET, op);
}

static void snor_wait_tx_finish(struct trix_snor *snor)
{
	unsigned long deadline;
	int timeout = 0;

	deadline = jiffies + msecs_to_jiffies(SNOR_MAX_WAIT_SEQ_MS);
	while(!timeout) {
		if (time_after_eq(jiffies, deadline))
			timeout = 1;
		if(snor_readl(snor, SNOR_OP_MODES_OFFSET) & SNOR_TX_FINISH)
			return;

		cond_resched();
	}

	dev_err(snor->dev, "timeout on spi tx completion\n");
}

static void snor_wait_rx_finish(struct trix_snor *snor)
{
	unsigned long deadline;
	int timeout = 0;

	deadline = jiffies + msecs_to_jiffies(SNOR_MAX_WAIT_SEQ_MS);
	while(!timeout) {
		if (time_after_eq(jiffies, deadline))
			timeout = 1;
		if(snor_readl(snor, SNOR_OP_MODES_OFFSET) & SNOR_RX_FINISH)
			return;
		cond_resched();
	}

	dev_err(snor->dev, "timeout on spi rx completion\n");
}

static int snor_start_xfer(struct trix_snor *snor, const void *txbuf, size_t n_tx)
{
	int i;
	int tx_bytes = SNOR_TRANS_DATA_LEN(n_tx);
	u8 *tx_ptr = (u8 *)txbuf;

	if (!tx_ptr) {
		/* Set the tx word if the transfer's original "tx" is not null */
		return -1;
	}

	/* total transmit bytes*/
	snor_writel(snor, SNOR_SYN_MODE1_OFFSET, tx_bytes);

	for (i=0; i < tx_bytes; i++) {
		snor_tx(snor, *tx_ptr++);
		snor_issue_tx_cmd(snor);
		snor_wait_tx_finish(snor);
	}

	return 0;
}

static int snor_start_recv(struct trix_snor *snor, void *rxbuf, size_t n_rx)
{
	u32 val;
	int i;
	int rx_bytes = SNOR_RECV_DATA_LEN(n_rx);
	u8 *rx_ptr = (u8 *)rxbuf;

	if (!rx_ptr) {
		/* Care rx only if the transfer's original "rx" is not null */
		return -1;
	}

	/* total receive bytes */
	val = SNOR_SPI_TIMG_RESET_MASK | (rx_bytes << SNOR_RECV_DATA_LEN_SHIFT);
	snor_writel(snor, SNOR_SPI_TIMG_CTRL1_OFFSET, val);

	for(i=0; i < rx_bytes; i++) {
		snor_issue_rx_cmd(snor);
		snor_wait_rx_finish(snor);
		snor_rx(snor, rx_ptr++);
	}

	return 0;
}

static int snor_do_transfer(struct trix_snor *snor, struct snor_message *msg)
{
	snor->mode = SPI_SW_MODE;

	snor_trans_init(snor);
	snor_start_xfer(snor, msg->tx_buf, msg->tx_len);
	snor_start_recv(snor, msg->rx_buf, msg->rx_len);
	snor_trans_end(snor);

	return 0;
}

static int snor_read_reg(struct spi_nor *nor, u8 code, u8 *pval, int len)
{
	struct trix_snor *snor = nor->priv;
	struct snor_message m;
	int ret;

	memset(&m, 0, sizeof(m));

	m.tx_buf	= &code;
	m.tx_len	= 1;

	m.rx_buf	= pval;
	m.rx_len	= len;

	ret = snor_do_transfer(snor, &m);
	if (ret)
		dev_err(snor->dev, "error %d reading %x\n", ret, code);

	return ret;
}

static int snor_write_reg(struct spi_nor *nor, u8 opcode, u8 *buf, int len)
{
	struct trix_snor *snor = nor->priv;
	struct snor_message m;

	memset(&m, 0, sizeof(m));

	snor->buf[0] = opcode;
	if (buf)
		memcpy(&snor->buf[1], buf, len);

	m.tx_buf = snor->buf;
	m.tx_len = len + 1;

	return snor_do_transfer(snor, &m);
}

static void snor_addr2cmd(struct spi_nor *nor, unsigned int addr, u8 *cmd)
{
	/* opcode is in cmd[0] */
	cmd[1] = addr >> (nor->addr_width * 8 - 8);
	cmd[2] = addr >> (nor->addr_width * 8 - 16);
	cmd[3] = addr >> (nor->addr_width * 8 - 24);
	cmd[4] = addr >> (nor->addr_width * 8 - 32);
}

static int snor_cmdsz(struct spi_nor *nor)
{
	return 1 + nor->addr_width;
}

static int snor_mtd_erase_sector(struct spi_nor *nor, loff_t offset)
{
	struct trix_snor *snor = nor->priv;
	struct snor_message m;

	dev_dbg(nor->dev, "%dKiB at 0x%08x\n",
		nor->mtd.erasesize / 1024, (u32)offset);

	memset(&m, 0, sizeof(m));

	/* set up command buffer*/
	snor->buf[0] = nor->erase_opcode;
	snor_addr2cmd(nor, offset, snor->buf);

	m.tx_buf = snor->buf;
	m.tx_len = snor_cmdsz(nor);

	snor_do_transfer(snor, &m);

	return 0;
}

static void snor_mtd_write(struct spi_nor *nor, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	struct trix_snor *snor = nor->priv;
	struct snor_message m;

	dev_dbg(nor->dev, "SNOR write to 0x%08x, len %zd\n", (u32)to, len);

	memset(&m, 0, sizeof(m));

	snor->buf[0] = nor->program_opcode;		/* command */
	snor_addr2cmd(nor, to, snor->buf);		/* 3-cycles address */
	memcpy(snor->buf + snor_cmdsz(nor), buf, len);	/* following the data*/

	m.tx_buf	= snor->buf;
	m.tx_len	= snor_cmdsz(nor) + len;

	snor_do_transfer(snor, &m);

	*retlen += len;
}

static int snor_flash_memcpy(struct trix_snor *snor,
				void *dest, u32 addr,
				unsigned int len)
{
	unsigned long src;

	snor->mode = SPI_HW_MODE;
	snor_trans_init(snor);

	src = (unsigned long)snor->io + addr;
	memcpy_fromio(dest, (const volatile void *)src, len);

	return 0;
}

/*
 * Read an address range from the nor chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int snor_mtd_read(struct spi_nor *nor, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	struct trix_snor *snor = nor->priv;

	dev_dbg(nor->dev, "SNOR read from 0x%08x, len %zd\n", (u32)from, len);

	if(snor->is_hw_read) {
		snor_flash_memcpy(snor, buf, from, len);
	} else {
		//so far, we only support hardware read
	}

	*retlen = len;

	return 0;
}

/*
 * set snor controller work mode
 */
static void snor_set_mode(struct trix_snor *snor)
{
	u32 val;

	val = snor_readl(snor, SNOR_DCR);
	val |= (0x33 << 24); /* select SPI HW controller */

	snor_writel(snor, SNOR_DCR, val);
}

static void snor_set_freq(void)
{
	//SNOR_SPI_DLY_CTRL [15:14] [7:6]
}

static int snor_hw_init(struct trix_snor *snor)
{
	snor_set_mode(snor);
	snor_set_freq();

	return 0;
}

static int trix_snor_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct mtd_part_parser_data ppdata;
	struct trix_snor *snor;
	struct resource *res;
	struct spi_nor *nor;
	int ret;

	if(np) {
		//DT based instantiation
		ppdata.of_node = np;
	} else {
		//non-DT based instantiation
	}

	snor = devm_kzalloc(&pdev->dev, sizeof(*snor), GFP_KERNEL);
	if (!snor)
		return -ENOMEM;

	snor->dev = &pdev->dev;
	nor = &snor->spi_nor;

	platform_set_drvdata(pdev, snor);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	snor->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(snor->base)) {
		dev_err(&pdev->dev,
			"Failed to reserve memory region %pR\n",res);
		return PTR_ERR(snor->base);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	snor->io = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(snor->io)) {
		dev_err(&pdev->dev,
			"Failed to reserve memory region %pR\n",res);
		return PTR_ERR(snor->base);
	}

	ret = snor_hw_init(snor);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialise SNOR Controller\n");
		return ret;
	}

	/* install the hooks */
	nor->read	= snor_mtd_read;
	nor->write	= snor_mtd_write;
	nor->erase	= snor_mtd_erase_sector;

	nor->write_reg	= snor_write_reg;
	nor->read_reg	= snor_read_reg;

	nor->dev	= &pdev->dev;
	nor->priv	= snor;

	snor->is_hw_read= true;

	/*
	 * board specific setup should have ensured the SPI clock used here
	 * matches what the READ command supports
 	 */
	ret = spi_nor_scan(nor, NULL, SPI_NOR_NORMAL);
	if (ret)
		return ret;

	/* register MTD device with fixed partitions table */
	return mtd_device_register(&nor->mtd, spi_partitions, ARRAY_SIZE(spi_partitions));
}

static int trix_snor_remove(struct platform_device *pdev)
{
	struct trix_snor *snor = platform_get_drvdata(pdev);
	struct spi_nor *nor = &snor->spi_nor;

	return mtd_device_unregister(&nor->mtd);
}

static const struct of_device_id trix_snor_match[] = {
        { .compatible = "sigma,spi-nor", },
        {},
};
MODULE_DEVICE_TABLE(of, trix_snor_match);

static struct platform_driver trix_snor_driver = {
	.probe		= trix_snor_probe,
	.remove 	= trix_snor_remove,
	.driver		= {
		.name	= "spi-nor",
		.of_match_table	= of_match_ptr(trix_snor_match),
	},
};
module_platform_driver(trix_snor_driver);


MODULE_AUTHOR("Claud Ling");
MODULE_DESCRIPTION("spi norflash driver");
MODULE_LICENSE("GPL");



