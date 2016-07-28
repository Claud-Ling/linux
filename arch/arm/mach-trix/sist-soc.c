#include <linux/init.h>
#include <linux/module.h>
#include <plat/io.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <mach/sist.h>
#include <asm/delay.h>


static int sist_soc_set_mode(struct sist_sensor *sensor, int mode)
{
	int shift = sensor->ctl_shift;
	void *ctl_reg = sensor->ctl_reg;

	/* Switch to request mode */
	MWriteRegWord(ctl_reg, (mode<<shift), (SIST_MODE_MASK<<shift));

	return 0;

}

static int sist_soc_get_value(struct sist_sensor *sensor, int request)
{
	unsigned int cur_tout, pre_tout;
	int val = 0, valid = 0;
	void *count_reg = NULL;
	int mode = REQ_TO_SIST_MODE(request);
	void *ctl_reg = sensor->ctl_reg;
	void *temp_reg = sensor->temp_reg;
	int shift = sensor->ctl_shift;

	/* Disable ringo counters */
	MWriteRegWord((void *)PROC_MON_SEL_REG, 
		PROC_MON_SEL_CTR_DISABLE, PROC_MON_SEL_CTR_MASK);


	/* Switch sensor to request mode */
	MWriteRegWord(ctl_reg, (mode<<shift), (SIST_MODE_MASK<<shift));

	/* load previous temperature index */
	if (request == SIST_RQ_TOUT) {
		udelay(100);	
		/* Start try from middle of temp index*/
		val = 25;
		MWriteRegWord(temp_reg, 
				((val)<<GLB_TEMP_CTR_SHIFT),
						GLB_TEMP_CTR_MASK);

	} else {
	/*
	 *  Read Ringo
	 *  TODO: Support read out other values!!
	 */
		/* Enable ringo counters */
		MWriteRegWord((void *)PROC_MON_SEL_REG, 
			PROC_MON_SEL_CTR_ENABLE, PROC_MON_SEL_CTR_MASK);
		udelay(100);	
		valid = (ReadRegWord((void *)PROC_MON_PM_SEL) & 0x1);
		if (valid == 0) {
			count_reg = ((void *)PROC_MON_PM_COUNT1);
		} else {
			count_reg = ((void *)PROC_MON_PM_COUNT0);
		}

		val = (ReadRegWord(count_reg) & PROC_MON_PM_COUNT_MASK);
		goto exit;
	}

	cur_tout = (ReadRegWord((void *)PROC_MON_TOUT_REG) & 
					PROC_MON_TOUT_MASK);
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
	MWriteRegWord(temp_reg, 
		((val)<<GLB_TEMP_CTR_SHIFT), GLB_TEMP_CTR_MASK);

	cur_tout = (ReadRegWord((void *)PROC_MON_TOUT_REG) & 
					PROC_MON_TOUT_MASK);

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

#if defined(CONFIG_SIGMA_SOC_SX8)
static struct sist_node soc_sist_node_list[] = {
	SIST_NODE(UMAC0, GLB_SIST_CTL_REG, 0, GLB_TEMP_CTR_REG),
	SIST_NODE(UMAC1, GLB_SIST_CTL_REG, 3, GLB_TEMP_CTR_REG),
	SIST_NODE(UMAC2, GLB_SIST_CTL_REG, 6, GLB_TEMP_CTR_REG),
	SIST_NODE(PMAN, GLB_SIST_CTL_REG, 9, GLB_TEMP_CTR_REG),
	SIST_NODE(GFX, GLB_SIST_CTL_REG, 12, GLB_TEMP_CTR_REG),
	SIST_NODE(FRCB_A, GLB_SIST_CTL_REG, 15, GLB_TEMP_CTR_REG),
	SIST_NODE(FRCB_B, GLB_SIST_CTL_REG, 18, GLB_TEMP_CTR_REG),
	SIST_NODE(ARM, GLB_SIST_CTL_REG, 21, GLB_TEMP_CTR_REG),
};

#elif defined(CONFIG_SIGMA_SOC_SX7)
static struct sist_node soc_sist_node_list[] = {
	SIST_NODE(UMAC0, GLB_SIST_CTL_REG, 0, GLB_TEMP_CTR_REG),
	SIST_NODE(GFX, GLB_SIST_CTL_REG, 4, GLB_TEMP_CTR_REG),
	SIST_NODE(UMAC2, GLB_SIST_CTL_REG, 8, GLB_TEMP_CTR_REG),
	SIST_NODE(VEDR, GLB_SIST_CTL_REG, 12, GLB_SIST_CTL_REG),
	SIST_NODE(VDETN, GLB_SIST_CTL_REG, 16, GLB_SIST_CTL_REG),
};
#else
static struct sist_node soc_sist_node_list[] = {

};
#endif

static int sist_soc_sensor_init(void)
{
	int i, ret = 0;
	struct sist_sensor *psensor = NULL;

	for (i=0; i<ARRAY_SIZE(soc_sist_node_list); i++) {

		psensor = sist_alloc_sensor();

		if (IS_ERR(psensor)) {
			return -ENOMEM;
		}
		
		psensor->id = soc_sist_node_list[i].id;
		psensor->name = soc_sist_node_list[i].name;
		psensor->ctl_reg = soc_sist_node_list[i].ctl_reg;
		psensor->ctl_shift = soc_sist_node_list[i].ctl_shift;
		psensor->temp_reg = soc_sist_node_list[i].temp_reg;
		psensor->ops = &sist_soc_ops;

		ret = sist_register_sensor(psensor);

		if (IS_ERR_VALUE(ret)) {
			pr_err("Failed to register SIST sensor: %s, err = %d\n", psensor->name, ret);
			return ret;
		}
		printk("Register SIST sensor: %s, ID: %d\n", psensor->name, psensor->id);
	}

	return 0;
}
device_initcall(sist_soc_sensor_init);

