#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <plat/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/err.h>
#include <mach/sist.h>



enum {
	THERMAL_ENABLE =1,
	THERMAL_DISABLE,
};


struct sigma_thermal_device {
	struct semaphore  mutex_lock;
	char temp_str[4096];
	int status;
	struct proc_dir_entry *thermal_entry;
	struct proc_dir_entry *temperature_entry;
	struct proc_dir_entry *status_entry;
};

#if defined(CONFIG_SIGMA_SOC_SX8)
static char * temperature_array[] = {
	"19",	"22",	"25",	"28",	"31",
	"34",	"37",	"40",	"43",	"46",
	"49",	"52",	"55",	"58",	"61",
	"64",	"67",	"70",	"73",	"76",
	"79",	"82",	"85",	"88",	"91",
	"94",	"97",	"100",	"103",	"106",
	"109",	"112",	"115",	"118",	"121",
	"124",	"127",	"130",	"133",	"136",
	"139",
};
#else
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
#endif



static struct sigma_thermal_device sigma_thermal_device;


static int sigma_thermal_get_temp(void)
{
	int i, max_sensors, value, idx_sum = 0, avg_temp = 0;
	char *buff = &sigma_thermal_device.temp_str[0];
	char tmp[64] = { 0 };

	max_sensors = sist_get_sensor_count();

	memset(buff, 0, 4096);

	for (i=0; i<max_sensors; i++) {
		value = send_sist_request(i, SIST_RQ_TOUT);
		if (IS_ERR_VALUE(value)) {
			pr_err("sigma-thermal-dev: SIST request failed, err = %d\n", value);
			continue;
		}

		sprintf(tmp, "%s %s\n", sist_get_sensor_name(i), temperature_array[min(value, ARRAY_SIZE(temperature_array))]);
		strcat(buff, tmp);
		memset(tmp, 0, 64);
		idx_sum += value;
	}

	avg_temp = idx_sum / i;
	sprintf(tmp, "%s %s\n", "Average", temperature_array[min(avg_temp, ARRAY_SIZE(temperature_array))]);
	strcat(buff, tmp);

	return 0;
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
late_initcall(sigma_thermal_init);
module_exit(sigma_thermal_exit);
