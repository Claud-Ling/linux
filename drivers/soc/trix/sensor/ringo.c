#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/err.h>
#include "sist.h"



struct sigma_ringo_device {
	struct semaphore  mutex_lock;
	char ringo_str[4096];
	int status;
	struct proc_dir_entry *ringo_entry;
};

static struct sigma_ringo_device sigma_ringo_device;


static int sigma_get_ringo(void)
{
	int i, max_sensors, value;
	char *buff = &sigma_ringo_device.ringo_str[0];
	char tmp[64] = { 0 };

	max_sensors = sist_get_sensor_count();

	memset(buff, 0, 4096);

	for (i=0; i<max_sensors; i++) {
		value = send_sist_request(i, SIST_RQ_FRINGOLL);
		if (IS_ERR_VALUE(value)) {
			pr_err("sigma-ringo-dev: SIST request failed, err = %d\n", value);
			continue;
		}

		sprintf(tmp, "%s %d", sist_get_sensor_name(i), value);
		strcat(buff, tmp);
		memset(tmp, 0, 64);

		value = send_sist_request(i, SIST_RQ_FRINGOLS);
		if (IS_ERR_VALUE(value)) {
			pr_err("sigma-ringo-dev: SIST request failed, err = %d\n", value);
			goto next;
		}

		sprintf(tmp, " %d",value);
		strcat(buff, tmp);
		memset(tmp, 0, 64);

		value = send_sist_request(i, SIST_RQ_BRINGO);
		if (IS_ERR_VALUE(value)) {
			pr_err("sigma-ringo-dev: SIST request failed, err = %d\n", value);
			goto next;
		}

		sprintf(tmp, " %d",value);
		strcat(buff, tmp);
		memset(tmp, 0, 64);
next:
		sprintf(tmp, "\n");
		strcat(buff, tmp);
		
	}

	return 0;
}

static int ringo_generic_open(struct inode *node, struct file *filp)
{
	filp->private_data = (void *)&sigma_ringo_device;
	return 0;
}

static ssize_t ringo_generic_read(struct file *filp, 
			char __user *buffer, size_t count, loff_t *offp)
{
	int len = 0;
	unsigned long cp_sz = 0;
	struct sigma_ringo_device *pdev = 
			(struct sigma_ringo_device *)filp->private_data;

	if (down_interruptible(&pdev->mutex_lock)) {
		return -ERESTARTSYS;
	}

	sigma_get_ringo();

	up(&pdev->mutex_lock);
	
	len = strlen(sigma_ringo_device.ringo_str);

	if (*offp) {
		*offp = 0;
		return 0;
	}


	if (count > len) {
		cp_sz = copy_to_user(buffer, sigma_ringo_device.ringo_str, len);
	} else {
		cp_sz = copy_to_user(buffer, sigma_ringo_device.ringo_str, count);
		len = count;
	}
	
	*offp += len;

	return len;
}

static ssize_t ringo_generic_write(struct file *filp,
			const char __user *buffer, size_t count, loff_t *offp)
{
	/* Don't allow write operation */
	return -EPERM;
}


static int ringo_generic_release(struct inode *node, struct file *filp)
{
	return 0;
}

static struct file_operations ringo_generic_ops = {
	.open = ringo_generic_open,
	.read = ringo_generic_read,
	.write = ringo_generic_write,
	.release = ringo_generic_release,
};


static int __init sigma_ringo_init(void)
{

	sema_init(&sigma_ringo_device.mutex_lock, 1);
	
	sigma_ringo_device.ringo_entry = \
			proc_create("SIST/ringo",0444,NULL, &ringo_generic_ops);
	if (!sigma_ringo_device.ringo_entry) {
		printk("sigma-ringo-dev create proc file failed!\n");
		return -1;
	}

	return 0;
}

static void __exit sigma_ringo_exit(void)
{
	proc_remove(sigma_ringo_device.ringo_entry);
	return;
}
MODULE_LICENSE("GPL");
late_initcall(sigma_ringo_init);
module_exit(sigma_ringo_exit);
