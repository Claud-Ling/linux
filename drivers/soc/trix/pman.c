/*
 *  linux/drivers/soc/trix/pman.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns DTV SoC PMAN Driver
 *
 */
#define pr_fmt(fmt) "pmandrv: " fmt

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/log2.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>

#include <linux/soc/sigma-dtv/io.h>

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#define PMAN_PROC_DIR	"pman/"
#endif

/*
 * PMan registers map (IP_1040_SEC)
 */
#define PMAN_BLOCKED_CMD	0x20
# define PMAN_BLOCK_CMD_BIT_DEFAULT_REGION	(1 << 17)
# define PMAN_BLOCK_CMD_BIT_EXEC		(1 << 2)
# define PMAN_BLOCK_CMD_BIT_READ		(1 << 0)
#define PMAN_BLOCKED_CMD_ID	0X24
#define PMAN_BLOCKED_CMD_BS	0x28
#define PMAN_BLOCKED_CMD_LINES	0x2c
#define PMAN_BLOCKED_CMD_STRIDE	0x30
#define PMAN_BLOCKED_CMD_ADDR	0x34
#define PMAN_BLOCKED_CMD_GROUP	0x38
#define PMAN_BLOCKED_REGION	0x3c

#define PMAN_INT_CLR_ENABLE	0xfd8
#define PMAN_INT_SET_ENABLE	0xfdc
#define PMAN_INT_STATUS		0xfe0
#define PMAN_INT_ENABLE		0xfe4
#define PMAN_INT_CLR_STATUS	0xfe8
#define PMAN_INT_SET_STATUS	0xfec
#define PMAN_MODULE_ID		0xffc

#define PMAN_REGION_ADDR_LOW(n)		(0x100 + (n << 5))
#define PMAN_REGION_ADDR_HIGH(n)	(0x104 + (n << 5))
#define PMAN_REGION_SEC_ACCESS(n)	(0x108 + (n << 5))
#define PMAN_REGION_ATTR(n)		(0x10c + (n << 5))

#define MAX_NR_SEC_GROUPS	32
#define MAX_PRINT_LIMITS	5000

struct pman_sec_group {
	const char **sec_groups_names;
	int nr_sec_groups;
	u32 mask;		/* PMan Hub security group to be checked, one bit per group */

	/* statistics for each group */
	int violat_stat[MAX_NR_SEC_GROUPS];
	int violat_stat_unknown;
};

struct pman_hub_priv {
	phys_addr_t phys_base;
	int irq;
	int nr_region;
	struct pman_sec_group *sec_group; /* chip specific security group information */

	spinlock_t lock;
	struct device *dev;
};

#define LOG2_PMAN_REGION_GRANULARITY	12
#define PMAN_REGION_GRANULARITY		(1 << LOG2_PMAN_REGION_GRANULARITY)
struct pman_sec_region {
	u32 addr_lo;
	u32 addr_hi;
	u32 sec_access;
	u32 attr;
};

static const char *sx7_sec_groups_names[] = {
    "JTAG_MMIO",            /*0*/
    "AV_CPU",               /*1*/
    "PLF-ARMOR",            /*2*/
    "PLF-ARM",              /*3*/
    "DISP_CPU",             /*4*/
    "MVED_CPU",             /*5*/
    "ISDB-Demod",           /*6*/
    "DMA_GATE",             /*7*/
    "GFX_3D/GE2D/EGC2350",  /*8*/
    "MALONE/HEVC/VP9",      /*9*/
    "DEMUX",                /*10*/
    "DEMUX-Read",           /*11*/
    "FLASH",                /*12*/
    "TURING",               /*13*/
    "TURING-Read",          /*14*/
    "ACP",                  /*15*/
    "USB_1/USB_2/USB_3",    /*16*/
    "ETHERNET",             /*17*/
    "SDIO_1/2/SPI",         /*18*/
    "NR",                   /*19*/
    "MCNR_W/MCNR_R1",       /*20*/
    "PIP_CAP",              /*21*/
    "MP_CAP",               /*22*/
    "OSD",                  /*23*/
    "BL/PP_W/PP_R/PIP_CAP", /*24*/
    "NONE",                 /*25*/
    "TCD",                  /*26*/
    "AUDIO",                /*27*/
    "NONE",                 /*28*/
    "GME/2DD",              /*29*/
    "NONE",                 /*30*/
};

static const char *sx8_sec_groups_names[] = {
    "NULL",                                                       /*0*/
    "NULL",                                                       /*1*/
    "PLF_CPU_M0",                                                 /*2*/
    "PLF_CPU_M0",                                                 /*3*/
    "AV_CPU",                                                     /*4*/
    "DISP_CPU",                                                   /*5*/
    "MVED_CPU",                                                   /*6*/
    "ISDB-Demod",                                                 /*7*/
    "DMA_GATE",                                                   /*8*/
    "JTAG_MMIO",                                                  /*9*/
    "GFX_3D/GE2D/EGC2350",                                        /*10*/
    "MVED malone",                                                /*11*/
    "Winsor/VDMA_W",                                              /*12*/
    "hevc_mbus/hevc_direct",                                      /*13*/
    "V_LOGO/LOGO",                                                /*14*/
    "VP9",                                                        /*15*/
    "DEMUX/cip_msp",                                              /*16*/
    "DEMUX/cip_msp",                                              /*17*/
    "AUDIO/ADSP/HBR",                                             /*18*/
    "ACP",                                                        /*19*/
    "FLASH",                                                      /*20*/
    "TUNING",                                                     /*21*/
    "USB_1/2/3/ETHERNET",                                         /*22*/
    "OSD_HEADER/V_BLENDER/OSD",                                   /*23*/
    "SDIO_1/SDIO_2",                                              /*24*/
    "SPI/V_PANEL/2DD",                                            /*25*/
    "V_FRCA/VBUF_ME/V_FRCB/PATH A/B",                             /*26*/
    "V_INCAP/EDR MetaData Write/Read/V_BLENDER",                  /*27*/
    "V_NR/CAP Read",                                              /*28*/
    "V_PANEL/3D PR/HDMi420@UHD120",                               /*29*/
    "V_NR/TNR Write/Read/V_DEINT/DeInt Write/Read/V_DEINT/Alpha", /*30*/
    "V_INCAP/MP write/PIP write",                                 /*31*/
};

static const char *union_sec_groups_names[] = {
    "JTAG_MMIO",                                                  /*0*/
    "AV_CPU",                                                     /*1*/
    "PLF_CPU_M0",                                                 /*2*/
    "PLF_CPU_M0",                                                 /*3*/
    "DISP_CPU",                                                   /*4*/
    "NULL",                                                       /*5*/
    "ISDB-Demod",                                                 /*6*/
    "DMA_GATE",                                                   /*7*/
    "GFX_3D/GE2D/EGC2350",                                        /*8*/
    "hevc_mbus/hevc_direct/VP9",                                  /*9*/
    "DEMUX/cip_msp",                                              /*10*/
    "DEMUX/cip_msp",                                              /*11*/
    "FLASH",                                                      /*12*/
    "TUNING",                                                     /*13*/
    "TUNING",                                                     /*14*/
    "NULL",                                                       /*15*/
    "USB_1/2",                                                    /*16*/
    "ETHERNET",                                                   /*17*/
    "SDIO_1/SDIO_2/SPI",                                          /*18*/
    "DETN_CAP_READ",                                              /*19*/
    "DETN_NEXT_WRITE/DETN_CURRENT_WRITE/DETN_PRE_WRITE/DETN_PPRE_READ_ALPHA", /*20*/
    "PIP_CAP",                                                    /*21*/
    "MP_CAP",                                                     /*22*/
    "OSD",                                                        /*23*/
    "PP_W/PP_R",                                                  /*24*/
    "NULL",                                                       /*25*/
    "TCD",                                                        /*26*/
    "AUDIO",                                                      /*27*/
};

static struct pman_sec_group sx7_pman_sec_group_info = {
	.sec_groups_names = sx7_sec_groups_names,
	.nr_sec_groups = ARRAY_SIZE(sx7_sec_groups_names),
};

static struct pman_sec_group sx8_pman_sec_group_info = {
	.sec_groups_names = sx8_sec_groups_names,
	.nr_sec_groups = ARRAY_SIZE(sx8_sec_groups_names),
};

static struct pman_sec_group union_pman_sec_group_info = {
	.sec_groups_names = union_sec_groups_names,
	.nr_sec_groups = ARRAY_SIZE(union_sec_groups_names),
};

static const struct of_device_id trix_pman_ids[] = {
	/* SoC specific */
	{ .compatible = "trix,pman-sx7",	.data = &sx7_pman_sec_group_info},
	{ .compatible = "trix,pman-sx8",	.data = &sx8_pman_sec_group_info},
	{ .compatible = "trix,pman-union",	.data = &union_pman_sec_group_info},
	{ /* sentinel */ }
};

static u32 pman_reg_read(struct pman_hub_priv *pman, unsigned int offset)
{
	return ReadRegWord(pman->phys_base + offset);
}

static void pman_reg_write(struct pman_hub_priv *pman, unsigned int offset, u32 val)
{
	WriteRegWord(pman->phys_base + offset, val);
}

static void pman_interrupt_enable(struct pman_hub_priv *pman, u32 mask)
{
	pman_reg_write(pman, PMAN_INT_SET_ENABLE, mask);
}

static void pman_interrupt_disable(struct pman_hub_priv *pman, u32 mask)
{
	pman_reg_write(pman, PMAN_INT_CLR_ENABLE, mask);
}

static void pman_interrupt_clear(struct pman_hub_priv *pman, u32 mask)
{
	pman_reg_write(pman, PMAN_INT_CLR_STATUS, mask);
}

static void pman_get_region(struct pman_hub_priv *pman, int idx, struct pman_sec_region *region)
{
	if (region && idx < pman->nr_region) {
		region->addr_lo = pman_reg_read(pman, PMAN_REGION_ADDR_LOW(idx));
		region->addr_hi = pman_reg_read(pman, PMAN_REGION_ADDR_HIGH(idx));
		region->sec_access = pman_reg_read(pman, PMAN_REGION_SEC_ACCESS(idx));
		region->attr = pman_reg_read(pman, PMAN_REGION_ATTR(idx));
	}
}


#ifdef CONFIG_PROC_FS
static int pman_seq_show(struct seq_file* m, void* v)
{
	struct pman_hub_priv *pman =
		(struct pman_hub_priv *)m->private;

	struct pman_sec_region rgn = {0};
	unsigned long irq_flags;
	int i;

	seq_printf(m, "pman-hub regions:\n");

	spin_lock_irqsave(&pman->lock, irq_flags);

	/*
	 * show PMAN regions setting
	 */
	for(i = 0; i < pman->nr_region; i++) {
		pman_get_region(pman, i, &rgn);
		seq_printf(m, "%s (reg%02d): 0x%08x - 0x%08x (0x%08x, 0x%5x) (%4dM-%4dM)\n",	\
		pman->dev->of_node->name, i,							\
		rgn.addr_lo, rgn.addr_hi,							\
		rgn.sec_access, rgn.attr,							\
		rgn.addr_lo >> 20,								\
		(rgn.addr_hi + PMAN_REGION_GRANULARITY) >> 20);
	}


	spin_unlock_irqrestore(&pman->lock, irq_flags);

	/*
	 * show the security groups info
	 */
	seq_printf(m, "\n-----------------------------------------------------\n");
	for (i = 0; i < pman->sec_group->nr_sec_groups; i++) {
		seq_printf(m, "PManSecurityGroup (No#%2d): %s\n", i, pman->sec_group->sec_groups_names[i]);
	}

	return 0;
}

static int pman_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pman_seq_show, PDE_DATA(inode));
}

static const struct file_operations pman_proc_fops = {
	.open		= pman_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static irqreturn_t pman_irq(int irq, void *dev_id)
{
	struct pman_hub_priv *pman = (struct pman_hub_priv *)dev_id;
	int region_id, count;
	unsigned long irq_flags;
	const char *group_name = "Unknown agent";
	u32 cmd, cmd_addr;
	u32 cmd_id, cmd_bs;
	u32 cmd_lines, cmd_stride;
	u32 region, group;
	u32 violat, int_enb;

	spin_lock_irqsave(&pman->lock, irq_flags);

	cmd	= pman_reg_read(pman, PMAN_BLOCKED_CMD);
	cmd_addr = pman_reg_read(pman, PMAN_BLOCKED_CMD_ADDR);

	cmd_id = pman_reg_read(pman, PMAN_BLOCKED_CMD_ID);
	cmd_bs = pman_reg_read(pman, PMAN_BLOCKED_CMD_BS);
	cmd_lines = pman_reg_read(pman, PMAN_BLOCKED_CMD_LINES);
	cmd_stride = pman_reg_read(pman, PMAN_BLOCKED_CMD_STRIDE);

	region = pman_reg_read(pman, PMAN_BLOCKED_REGION);
	group = pman_reg_read(pman, PMAN_BLOCKED_CMD_GROUP);

	violat = pman_reg_read(pman, PMAN_INT_STATUS);
	int_enb = pman_reg_read(pman, PMAN_INT_ENABLE);

	region_id = cmd & PMAN_BLOCK_CMD_BIT_DEFAULT_REGION?(-1):region;

	if (group <= pman->sec_group->nr_sec_groups) {
		count = pman->sec_group->violat_stat[group]++;
		group_name = pman->sec_group->sec_groups_names[group];
	}else {
		count = pman->sec_group->violat_stat_unknown++;
	}

	if (count <= MAX_PRINT_LIMITS) {
		if (is_power_of_2(count)) {
			printk(KERN_INFO "\n==========pmandrv: Here comes PMan Hub(%s) interrupt.\n", pman->dev->of_node->name);
			printk(KERN_INFO "pman-hub: Group[%d] Invalid Access Count:%d\n", group, count);
			printk(KERN_INFO "pman-hub: BLOCKED_CMD 0x%x: (%s/%s cmd)\n", cmd,
					(cmd & PMAN_BLOCK_CMD_BIT_EXEC) ? "Executable" : "Data",
					(cmd & PMAN_BLOCK_CMD_BIT_READ) ? "Read" : "Write");
			printk(KERN_INFO "pman-hub: BLOCKED_CMD_ID 0x%x\n", cmd_id);
			printk(KERN_INFO "pman-hub: BLOCKED_CMD_BS 0x%x\n", cmd_bs);
			printk(KERN_INFO "pman-hub: BLOCKED_CMD_LINES 0x%x\n", cmd_lines);
			printk(KERN_INFO "pman-hub: BLOCKED_CMD_STRIDE 0x%x\n", cmd_stride);
			printk(KERN_INFO "pman-hub: BLOCKED_CMD_ADDR 0x%x\n", cmd_addr);
			printk(KERN_INFO "pman-hub: BLOCKED_SEC_GROUP 0x%x\n", group);
			printk(KERN_INFO "pman-hub: BLOCKED_REGION 0x%x\n", region);

			printk(KERN_INFO "pman-hub: INT_STATUS 0x%x\n", violat);
			printk(KERN_INFO "pman-hub: INT_ENABLE 0x%x\n", int_enb);

			printk(KERN_INFO
			       "%s: (!)PMAN error for group[%d: %s] tries to access address [0x%x]\n",
				pman->dev->of_node->name,
				group, group_name,
				cmd_addr);


			printk(KERN_INFO "pman-hub: (*)You can check regions from \"/proc/pman_hub\".\n\n");
		}
		/* clear all interrupt status */
		pman_interrupt_clear(pman, U32_MAX);
	} else {
		/*
		 * to prevent too many interrupt
		 * we just clear these blocked security groups
		 */
		pman_interrupt_disable(pman, violat);
	}

	spin_unlock_irqrestore(&pman->lock, irq_flags);

	return IRQ_HANDLED;
}

/*
 * pman_probe_config_dt - parse device-tree driver parameters
 * @pdev: platform_device structure
 * @pman: driver data platform structure
 * Description:
 * this function is to read the driver parameters from device-tree and
 * set some private fields that will be used by the main at runtime.
 */
static int pman_probe_config_dt(struct platform_device *pdev,
				struct pman_hub_priv *pman)
{
	const struct of_device_id *device;
	u32 sec_group_mask;
	u32 nr_region;

	device = of_match_device(trix_pman_ids, &pdev->dev);
	if (!device) {
		dev_err(&pdev->dev, "Error: No device match found\n");
		return -ENODEV;
	}

	if (!device->data) {
		dev_err(&pdev->dev, "Error: No device spec data found\n");
		return -ENODEV;
	}

	pman->sec_group = (struct pman_sec_group *)device->data;

	if(!of_property_read_u32(pdev->dev.of_node,"pman,region", &nr_region))
		pman->nr_region = nr_region;

	if(!of_property_read_u32(pdev->dev.of_node,"pman,sec_group_mask", &sec_group_mask))
		pman->sec_group->mask = sec_group_mask;

	return 0;
}

static int pman_probe(struct platform_device *pdev)
{
	struct pman_hub_priv *pman;
	struct resource *res;
	int ret;

	pman = devm_kzalloc(&pdev->dev, sizeof(struct pman_hub_priv), GFP_KERNEL);
	if (!pman) {
		dev_err(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	if (pdev->dev.of_node) {
		ret = pman_probe_config_dt(pdev, pman);
		if (ret) {
			pr_err("%s: main dt probe failed", __func__);
			return ret;
		}
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pman->phys_base = (phys_addr_t)res->start;

	pman->irq = platform_get_irq(pdev, 0);
	if (pman->irq < 0) {
		if (pman->irq != EPROBE_DEFER)
			dev_err (&pdev->dev, "PMAN IRQ configuration information not found\n");
		return pman->irq;
	}

	ret = devm_request_irq(&pdev->dev, pman->irq, pman_irq,
				IRQF_SHARED, pdev->name, pman);
	if (unlikely(ret < 0)) {
		pr_err("%s: ERROR: allocating the IRQ %d (error: %d)\n",
			__func__, pman->irq, ret);
		return ret;
	}

	pman->dev = &pdev->dev;
	platform_set_drvdata(pdev, pman);

	spin_lock_init(&pman->lock);

#ifdef CONFIG_PROC_FS
	proc_create_data(pdev->dev.of_node->name, S_IRUGO,
			NULL, &pman_proc_fops, (void *)pman);
#endif

	pman_interrupt_enable(pman, pman->sec_group->mask);

	return 0;
}

static int pman_remove(struct platform_device *pdev)
{
	struct pman_hub_priv *pman = platform_get_drvdata(pdev);

	free_irq(pman->irq, NULL);

	return 0;
}

static struct platform_driver pman_driver = {
	.probe = pman_probe,
	.remove = pman_remove,
	.driver = {
		.name = "trix-pman",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(trix_pman_ids),
	},
};

module_platform_driver(pman_driver);

MODULE_AUTHOR("Sigma Kernel team");
MODULE_DESCRIPTION("Sigma PMAN protection driver");
MODULE_LICENSE("GPL");

