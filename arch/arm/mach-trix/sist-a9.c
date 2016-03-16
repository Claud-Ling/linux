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


static int sist_a9_set_mode(struct sist_sensor *sensor, int mode)
{
	int id = sensor->id;
	int shift = ID_TO_SIST_SHIFT(id);

	/* Switch to request mode */
	MWriteRegByte((void *)A9_SIST_CTL_REG, 
			(mode<<shift), (SIST_MODE_MASK<<shift));

	return 0;

}
static int sist_a9_get_value(struct sist_sensor *sensor, int request)
{
	unsigned int cur_tout, pre_tout;
	int val = 0, valid = 0;
	void *count_reg = NULL;
	int mode = REQ_TO_SIST_MODE(request);
	int id = sensor->id;
	int shift = ID_TO_SIST_SHIFT(id);

	/* Disable ringo counters */
	MWriteRegWord((void *)PROC_MON_SEL_REG, 
		PROC_MON_SEL_CTR_DISABLE, PROC_MON_SEL_CTR_MASK);


	/* Switch A9 node to request mode */
	MWriteRegByte((void *)A9_SIST_CTL_REG, 
		(mode<<shift), (SIST_MODE_MASK<<shift));

	/* load previous temperature index */
	if (request == SIST_RQ_TOUT) {
		udelay(100);	
		/* Start try from middle of temp index*/
		val = 25;
		/* load previous A9 temperature index */
		MWriteRegByte((void *)A9_TEMP_CTL_REG, 
				(val<<A9_TEMP_CTL_SHIFT), 
						A9_TEMP_CTL_MASK);

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
	/* load previous A9 temperature index */
	MWriteRegByte((void *)A9_TEMP_CTL_REG, 
			(val<<A9_TEMP_CTL_SHIFT), 
					A9_TEMP_CTL_MASK);

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


static struct sensor_ops sist_a9_ops = {
	.get_value = sist_a9_get_value,
	.set_mode = sist_a9_set_mode,
};

static struct sist_node a9_sist_node_list[] = {
	{ SIST_ID_ARM, "ARM" }
};

static int sist_a9_sensor_init(void)
{
	int i, ret = 0;
	struct sist_sensor *psensor = NULL;

	for (i=0; i<ARRAY_SIZE(a9_sist_node_list); i++) {

		psensor = sist_alloc_sensor();

		if (IS_ERR(psensor)) {
			return -ENOMEM;
		}
		
		psensor->id = a9_sist_node_list[i].id;
		psensor->name = a9_sist_node_list[i].name;
		psensor->ops = &sist_a9_ops;

		ret = sist_register_sensor(psensor);

		if (IS_ERR_VALUE(ret)) {
			pr_err("Failed to register SIST sensor: %s, err = %d\n", psensor->name, ret);
			return ret;
		}
		printk("Register SIST sensor: %s, ID: %d\n", psensor->name, psensor->id);
	}

	return 0;
}
device_initcall(sist_a9_sensor_init);

