#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <plat/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>


/* Process Monitor Select and Control Register */
#define PROC_MON_SEL_REG			(0x19b00070)
#define PROC_MON_SEL_CTR_MASK			(0x00000001)
#define PROC_MON_SEL_CTR_SHIFT			(0x0)
#define PROC_MON_SEL_CTR_ENABLE			(1UL)
#define PROC_MON_SEL_CTR_DISABLE		(0UL)

#define PROC_MON_TOUT_REG			(0x19b0008c)
#define PROC_MON_TOUT_MASK			(0x00000001)


#define GLB_SIST_CTL_REG			(0x19b0007c)
#define GLB_SIST_MODE_MASK			(0xffffff)
#define SIST_BYPASS_MODE			(0x0)
#define SIST_NOISE_OUT_MODE			(0x1)
#define SIST_TOUT_MODE				(0x2)


#define GLB_TEMP_CTR_REG			(0x19b00074)
#define GLB_TEMP_CTR_SHIFT			16
#define GLB_TEMP_CTR_MASK			(0x003f0000)

#define A9_SIST_CTL_REG				(0x1500ef17)
#define A9_SIST_MODE_MASK			(0x07)
#define A9_SIST_MODE_SHIFT			(0)

#define A9_TEMP_CTL_REG				(0x1500ef16)
#define A9_TEMP_CTL_MASK			(0x7f)
#define A9_TEMP_CTL_SHIFT			(0)


enum {
	TYPE_GLOBAL,
	TYPE_ARM,
	TYPE_UNKNOW
};

enum {
	THERMAL_ENABLE =1,
	THERMAL_DISABLE,
};

struct sigma_thermal_device {
	struct semaphore  mutex_lock;
	struct thermal_mon_node *node_list;
	int node_count;
	char temp_str[4096];
	int status;
	struct proc_dir_entry *thermal_entry;
	struct proc_dir_entry *temperature_entry;
	struct proc_dir_entry *status_entry;
};

struct thermal_mon_node {
	char *name;
	int type;
	int sist_shift;
	int temp_idx;
	char *temperature;
	void *private_data;
};

#define NODE_TO_DEVICE(node)	({				\
	(struct sigma_thermal_device *)(node->private_data);	\
})

#define T_MIN_IDX	(0)
#define T_MAX_IDX	(41)

static char * temperature_array[] = {
  "-45.7",  "-45.7",  "-41.7",  "-37.5",   "-33",
  "-28.4",  "-23.5",  "-18.3",  "-11.4",  "-6.1",
   "-2.1",    "2.1",    "6.5",     "11",  "15.7",
   "20.6",   "25.6",   "30.9",   "36.4",    "42",
   "46.1",   "50.2",   "54.5",   "58.8",  "63.3",
   "67.9",   "72.6",   "77.4",   "82.4",  "87.5",
   "92.8",   "98.2",  "102.5",  "106.9", "111.4",
  "116.0",  "120.7",  "125.5",  "130.5", "135.5",
  "135.5",
};


static struct sigma_thermal_device sigma_thermal_device;

struct thermal_mon_node mon_node_list[] = {
	{
		.name = "UMAC0",
		.type = TYPE_GLOBAL,
		.sist_shift =0,
		.temp_idx = 25,
		.temperature = "0",
		.private_data = (void *)&sigma_thermal_device,
	},
	{
		.name = "GFX",
		.type = TYPE_GLOBAL,
		.sist_shift =4,
		.temp_idx = 25,
		.temperature = "0",
		.private_data = (void *)&sigma_thermal_device,
	},
	{
		.name = "UMAC2",
		.type = TYPE_GLOBAL,
		.sist_shift =8,
		.temp_idx = 25,
		.temperature = "0",
		.private_data = (void *)&sigma_thermal_device,
	},
	{
		.name = "VEDR",
		.type = TYPE_GLOBAL,
		.sist_shift =12,
		.temp_idx = 25,
		.temperature = "0",
		.private_data = (void *)&sigma_thermal_device,
	},
	{
		.name = "VDETN",
		.type = TYPE_GLOBAL,
		.sist_shift =16,
		.temp_idx = 25,
		.temperature = "0",
		.private_data = (void *)&sigma_thermal_device,
	},
	{
		.name = "ARM",
		.type = TYPE_ARM,
		.sist_shift =A9_SIST_MODE_SHIFT,
		.temp_idx = 25,
		.temperature = "0",
		.private_data = (void *)&sigma_thermal_device,
	},

};


static void get_glb_sensor_temp(struct thermal_mon_node *node)
{
	unsigned int cur_tout, pre_tout;

	/* Disable ringo counters */
	MWriteRegWord((void *)PROC_MON_SEL_REG, 
		PROC_MON_SEL_CTR_DISABLE, PROC_MON_SEL_CTR_MASK);

	/* Switch A9 node to BYPASS mode */
	MWriteRegByte((void *)A9_SIST_CTL_REG, 
		(SIST_BYPASS_MODE<<A9_SIST_MODE_SHIFT),
						A9_SIST_MODE_MASK);

	/* Switch node to TOUT mode */
	MWriteRegWord((void *)GLB_SIST_CTL_REG, 
		(SIST_TOUT_MODE<<(node->sist_shift)), GLB_SIST_MODE_MASK);

	/* load previous temperature index */
	MWriteRegWord((void *)GLB_TEMP_CTR_REG, 
		((node->temp_idx)<<GLB_TEMP_CTR_SHIFT), 
						GLB_TEMP_CTR_MASK);

	cur_tout = (ReadRegWord((void *)PROC_MON_TOUT_REG) & 
					PROC_MON_TOUT_MASK);
	pre_tout = cur_tout;

try_loop:
	if (cur_tout == 0) {
	/* Decrease the temp index */
		node->temp_idx--;
	} else {
	/* Increase index */
		node->temp_idx++;
	}

	if (node->temp_idx < T_MIN_IDX || node->temp_idx > T_MAX_IDX) {
		printk("sigma-thermal-dev: Temperature index error!\n");
		goto exit;
	}
	/* load new temp index*/	
	MWriteRegWord((void *)GLB_TEMP_CTR_REG, 
		((node->temp_idx)<<GLB_TEMP_CTR_SHIFT), 
						GLB_TEMP_CTR_MASK);

	cur_tout = (ReadRegWord((void *)PROC_MON_TOUT_REG) & 
					PROC_MON_TOUT_MASK);

	if (cur_tout == pre_tout) {
	/* Try again*/
		goto try_loop;
	}

	if (cur_tout == 1) {
		node->temp_idx++;
	}
	
	node->temperature = temperature_array[node->temp_idx];
exit:

	return;
}

static void get_arm_sensor_temp(struct thermal_mon_node *node)
{
	unsigned int cur_tout, pre_tout;

	/* Switch other sensor node to BYPASS mode */
	MWriteRegWord((void *)GLB_SIST_CTL_REG, 
		SIST_BYPASS_MODE, GLB_SIST_MODE_MASK);

	/* Switch A9 node to TOUT mode */
	MWriteRegByte((void *)A9_SIST_CTL_REG, 
		(SIST_TOUT_MODE<<(node->sist_shift)), A9_SIST_MODE_MASK);

	/* load previous A9 temperature index */
	MWriteRegByte((void *)A9_TEMP_CTL_REG, 
		((node->temp_idx)<<A9_TEMP_CTL_SHIFT), 
						A9_TEMP_CTL_MASK);

	cur_tout = (ReadRegWord((void *)PROC_MON_TOUT_REG) & 
					PROC_MON_TOUT_MASK);
	pre_tout = cur_tout;

try_loop:
	if (cur_tout == 0) {
	/* Decrease the temp index */
		node->temp_idx--;
	} else {
	/* Increase index */
		node->temp_idx++;
	}

	if (node->temp_idx < T_MIN_IDX || node->temp_idx > T_MAX_IDX) {
		printk("sigma-thermal-dev: Temperature index error!\n");
		goto exit;
	}
	/* load new temp index*/	
	MWriteRegByte((void *)A9_TEMP_CTL_REG, 
		((node->temp_idx)<<A9_TEMP_CTL_SHIFT), 
						A9_TEMP_CTL_MASK);

	cur_tout = (ReadRegWord((void *)PROC_MON_TOUT_REG) & 
					PROC_MON_TOUT_MASK);

	if (cur_tout == pre_tout) {
	/* Try again*/
		goto try_loop;
	}

	if (cur_tout == 1) {
		node->temp_idx++;
	}
	
	node->temperature = temperature_array[node->temp_idx];
exit:

	return;
}

static int sigma_thermal_get_temp(void)
{
	int i;
	struct thermal_mon_node *pnode = sigma_thermal_device.node_list;


	for(i=0; i<sigma_thermal_device.node_count; i++) {

		if (pnode->type == TYPE_GLOBAL) {

			get_glb_sensor_temp(pnode);

		} else if (pnode->type == TYPE_ARM) {

			get_arm_sensor_temp(pnode);

		} else {
			printk("sigma-thermal-dev: Unknow thermal device type!\n");
			return -1;
		}
		pnode++;
	}

	return 0;
}
static void prepare_thermal_value_buff(void)
{
	int i, idx_sum = 0, avg_temp;
	struct thermal_mon_node *pnode = sigma_thermal_device.node_list;
	char *buff = sigma_thermal_device.temp_str;
	char tmp[64] = { 0 };

	memset(buff, 0, 4096);

	for(i=0; i<sigma_thermal_device.node_count; i++) {
		
		sprintf(tmp, "%s %s\n", pnode->name, pnode->temperature);
		strcat(buff, tmp);
		memset(tmp, 0, 64);
		idx_sum += pnode->temp_idx;
		pnode++;
	}

	avg_temp = idx_sum / (sigma_thermal_device.node_count);
	sprintf(tmp, "%s %s\n", "Average", temperature_array[avg_temp]);
	strcat(buff, tmp);

	return;
}

static int thermal_generic_open(struct inode *node, struct file *filp)
{
	filp->private_data = (void *)&sigma_thermal_device;
	return 0;
}

static ssize_t thermal_temp_read(struct file *filp, 
			char __user *buffer, size_t count, loff_t *offp)
{
	int len = 0;
	unsigned long cp_sz = 0;
	struct sigma_thermal_device *pdev = 
			(struct sigma_thermal_device *)filp->private_data;

	if (pdev->status == THERMAL_DISABLE) {
		return 0;
	}
	if (down_interruptible(&pdev->mutex_lock)) {
		return -ERESTARTSYS;
	}

	sigma_thermal_get_temp();
	prepare_thermal_value_buff();

	up(&pdev->mutex_lock);
	
	len = strlen(sigma_thermal_device.temp_str);

	if (*offp) {
		*offp = 0;
		return 0;
	}


	if (count > len) {
		cp_sz = copy_to_user(buffer, sigma_thermal_device.temp_str, len);
	} else {
		cp_sz = copy_to_user(buffer, sigma_thermal_device.temp_str, count);
		len = count;
	}
	
	*offp += len;

	return len;
}

static ssize_t thermal_temp_write(struct file *filp,
			const char __user *buffer, size_t count, loff_t *offp)
{
	/* Don't allow write operation */
	return -EPERM;
}

static ssize_t thermal_status_read(struct file *filp, 
			char __user *buffer, size_t count, loff_t *offp)
{
	char status_str[32] = { 0 };
	char *p;
	int len;
	unsigned long cp_sz = 0;	
	struct sigma_thermal_device *pdev = 
			(struct sigma_thermal_device *)filp->private_data;

	if (down_interruptible(&pdev->mutex_lock)) {
		return -ERESTARTSYS;
	}

	if (pdev->status == THERMAL_ENABLE) {
		p = "enable";
	} else {
		p = "disable";
	}

	up(&pdev->mutex_lock);

	len = strlen(p);
	sprintf(status_str, "%s", p);

	if (*offp) {
		*offp = 0;
		return 0;
	}


	if (count > len) {
		cp_sz = copy_to_user(buffer, status_str, len);
	} else {
		cp_sz = copy_to_user(buffer, status_str, count);
		len = count;
	}

	*offp += len;

	return len;
}

static ssize_t thermal_status_write(struct file *filp, 
			const char __user *buffer, size_t count, loff_t *offp)
{
	char status_str[32] = {0};
	int len;
	unsigned long cp_sz = 0;
	struct sigma_thermal_device *pdev = 
			(struct sigma_thermal_device *)filp->private_data;

	if (count > strlen("disable\n")) {
		return -EINVAL;
	}

	cp_sz = copy_from_user(status_str, buffer, count);

	len = strlen(status_str);

	if (down_interruptible(&pdev->mutex_lock)) {
		return -ERESTARTSYS;
	}

	if (!(strncmp("disable", status_str, strlen("disable")))) {
		if (pdev->status == THERMAL_ENABLE)
			pdev->status = THERMAL_DISABLE;

	} else if (!(strncmp("enable", status_str, strlen("enable")))) {
		if (pdev->status == THERMAL_DISABLE)
			pdev->status = THERMAL_ENABLE;

	} else {
		count = -EINVAL;
	}

	up(&pdev->mutex_lock);

	return count;
}

static int thermal_generic_release(struct inode *node, struct file *filp)
{
	return 0;
}

static struct file_operations thermal_temp_ops = {
	.open = thermal_generic_open,
	.read = thermal_temp_read,
	.write = thermal_temp_write,
	.release = thermal_generic_release
};

static struct file_operations thermal_status_ops = {
	.open = thermal_generic_open,
	.read = thermal_status_read,
	.write = thermal_status_write,
	.release = thermal_generic_release
};

static int __init sigma_thermal_init(void)
{
	
	sema_init(&sigma_thermal_device.mutex_lock, 1);
	sigma_thermal_device.node_list = &mon_node_list[0];
	sigma_thermal_device.node_count = ARRAY_SIZE(mon_node_list);
	
	sigma_thermal_device.status = THERMAL_ENABLE;
	
	sigma_thermal_device.thermal_entry = proc_mkdir("thermal", NULL);

	if (!sigma_thermal_device.thermal_entry) {
		printk("sigma-thermal-dev create proc file failed!\n");
		return -1;
	}

	sigma_thermal_device.temperature_entry = \
			proc_create("thermal/temperature",0444,NULL,&thermal_temp_ops);

	sigma_thermal_device.status_entry = \
		proc_create("thermal/status",0666,NULL,&thermal_status_ops);

	return 0;
}

static void __exit sigma_thermal_exit(void)
{
	proc_remove(sigma_thermal_device.status_entry);
	proc_remove(sigma_thermal_device.temperature_entry);
	proc_remove(sigma_thermal_device.thermal_entry);

	return;
}
MODULE_LICENSE("GPL");
module_init(sigma_thermal_init);
module_exit(sigma_thermal_exit);
