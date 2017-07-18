#include <linux/init.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <asm/delay.h>
#include <linux/soc/sigma-dtv/io.h>
#include "sist.h"

#define PROC_MON_SEL_REG			(0x19b00070)
#define PROC_MON_SEL_CTR_MASK			(0x00000001)
#define PROC_MON_SEL_CTR_SHIFT			(0x0)
#define PROC_MON_SEL_CTR_ENABLE			(1UL)
#define PROC_MON_SEL_CTR_DISABLE		(0UL)

#define PROC_MON_PM_COUNT0			(0x19b00080)
#define PROC_MON_PM_COUNT1			(0x19b00084)
#define PROC_MON_PM_SEL				(0x19b00088)
#define PROC_MON_PM_COUNT_MASK			(0x00ffffff)

#define PROC_MON_TOUT_REG			(0x19b0008c)
#define PROC_MON_TOUT_MASK			(0x00000001)

#define GLB_SIST_CTL_REG			(0x19b0007c)
#define GLB_SIST_MODE_MASK			(0xffffff)

#define GLB_TEMP_CTR_REG			(0x19b00074)
#define GLB_TEMP_CTR_SHIFT			16
#define GLB_TEMP_CTR_MASK			(0x003f0000)

struct soc_sist_regs {
	volatile uint32_t mon_sel_reg;
	volatile uint32_t temp_ctl;
	volatile uint32_t noise_ctl;
	volatile uint32_t sensor_ctl;
	volatile uint32_t pm_count0;
	volatile uint32_t pm_count1;
	volatile uint32_t pm_sel;
	volatile uint32_t pm_tout;
};

static int sist_soc_set_mode(struct sist_sensor *sensor, int mode)
{
	int shift = sensor->ctl_shift;
	struct soc_sist_regs *sist_regs = (struct soc_sist_regs *)sensor->base;

	/* Switch to request mode */
	sist_regs->sensor_ctl &= (~(SIST_MODE_MASK<<shift));
	sist_regs->sensor_ctl |= (mode<<shift);

	return 0;

}

static int sist_soc_get_value(struct sist_sensor *sensor, int request)
{
	unsigned int cur_tout, pre_tout;
	int val = 0, valid = 0;
	int mode = REQ_TO_SIST_MODE(request);
	struct soc_sist_regs *sist_regs = (struct soc_sist_regs *)sensor->base;

	int shift = sensor->ctl_shift;

	/* Disable ringo counters */
	sist_regs->mon_sel_reg &= (~PROC_MON_SEL_CTR_MASK);
	sist_regs->mon_sel_reg |= PROC_MON_SEL_CTR_DISABLE;


	/* Switch sensor to request mode */
	sist_regs->sensor_ctl &= (~(SIST_MODE_MASK<<shift));
	sist_regs->sensor_ctl |= (mode<<shift);

	/* load previous temperature index */
	if (request == SIST_RQ_TOUT) {
		udelay(100);	
		/* Start try from middle of temp index*/
		val = 25;
		sist_regs->temp_ctl &= (~GLB_TEMP_CTR_MASK);
		sist_regs->temp_ctl |= (val<<GLB_TEMP_CTR_SHIFT);

	} else {
	/*
	 *  Read Ringo
	 *  TODO: Support read out other values!!
	 */
		/* Enable ringo counters */
		sist_regs->mon_sel_reg &= (~PROC_MON_SEL_CTR_MASK);
		sist_regs->mon_sel_reg |= PROC_MON_SEL_CTR_ENABLE;

		udelay(100);	
		valid = (sist_regs->pm_sel & 0x1);
		if (valid == 0) {
			val = (sist_regs->pm_count1 & PROC_MON_PM_COUNT_MASK);
		} else {
			val = (sist_regs->pm_count0 & PROC_MON_PM_COUNT_MASK);
		}

		goto exit;
	}

	cur_tout = (sist_regs->pm_tout & PROC_MON_TOUT_MASK); 
	pre_tout = cur_tout;

try_loop:
	if (cur_tout == 0) {
	/* Decrease the temp index */
		val--;
	} else {
	/* Increase index */
		val++;
	}

	if (val< T_MIN_IDX || val > T_MAX_IDX) {
		val = -EINVAL;
		goto exit;
	}
	/* load new temp index*/	
	sist_regs->temp_ctl &= (~GLB_TEMP_CTR_MASK);
	sist_regs->temp_ctl |= (val<<GLB_TEMP_CTR_SHIFT);

	cur_tout = (sist_regs->pm_tout & PROC_MON_TOUT_MASK); 

	if (cur_tout == pre_tout) {
	/* Try again*/
		goto try_loop;
	}

	if (cur_tout == 1) {
		val++;
	}
	
exit:

	return val;

}

static struct sensor_ops sist_soc_ops = {
	.get_value = sist_soc_get_value,
	.set_mode = sist_soc_set_mode,
};

static int sist_soc_sensor_init(struct device_node *np, void *base)
{
	int ret = 0;
	struct sist_sensor *psensor = NULL;

	printk(KERN_EMERG"In %s\n", __func__);
	psensor = sist_alloc_sensor();

	if (IS_ERR(psensor)) {
		return -ENOMEM;
	}

	psensor->name = np->name;
	psensor->base = (volatile void *)base;
	ret = of_property_read_u32(np, "bit-shift", &psensor->ctl_shift); 
	psensor->ops = &sist_soc_ops;

	ret = sist_register_sensor(psensor);
	if (IS_ERR_VALUE(ret)) {
		pr_err("Failed to register SIST sensor: %s, err = %d\n", psensor->name, ret);
			return ret;
	}
	printk("Register SIST sensor: %s, ID: %d\n", psensor->name, psensor->id);

	return ret;
}

int of_sist_soc_sensor_probe(struct device_node *np)
{
	int ret = -1;
	void *base = NULL;
	struct device_node *child;

	base = of_iomap(np, 0);

	for_each_child_of_node(np, child) {
		ret = sist_soc_sensor_init(child, base);
		if (ret)
			break;
	}

	return ret;
}
EXPORT_SYMBOL(of_sist_soc_sensor_probe);
