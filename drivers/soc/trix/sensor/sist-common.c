#include <linux/init.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <asm/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/soc/sigma-dtv/io.h>
#include "sist.h"

#define SIST_ID_MIN	(0)
#define SIST_ID_MAX	((sist) ? (sist->max_sensors) : (0))

#define IS_VALID_SIST_ID(id) ({		\
	(((id) >= SIST_ID_MIN) &&	\
		((id) < SIST_ID_MAX));	\
})

extern int of_sist_soc_sensor_probe(struct device_node *np);
static struct sist_bus *sist = NULL;

static inline struct sist_sensor * sist_id_to_sensor(int id)
{
	unsigned long flags;
	struct sist_sensor *psensor = NULL, *sensor = NULL;
	if (!sist) {
		return NULL;
	}

	spin_lock_irqsave(&sist->lock, flags);

	list_for_each_entry(psensor, &sist->sensors, list) {
		if (psensor->id == id) {
			sensor = psensor;
			break;
		}
	}

	spin_unlock_irqrestore(&sist->lock, flags);

	return sensor;
}

int send_sist_request(int sensor_id, int request)
{
	unsigned long flags;
	struct sist_bus *psist = NULL;
	struct sist_sensor *sensor = NULL, *psensor = NULL;
	int value = -EINVAL;

	if (!IS_VALID_SIST_ID(sensor_id)) {
		pr_err("SIST invalid sensor ID, sensor_id = %d\n", sensor_id);
		return -EINVAL;
	}
	
	if (!IS_VALID_SIST_RQ(request)) {
		pr_err("SIST invalid request, request = %d\n", request);
		return -EINVAL;
	}
	
	sensor = sist_id_to_sensor(sensor_id);
	if (sensor == NULL) {
		return -ENODEV;
	}

	psist = SENSOR_TO_SIST(sensor);

	spin_lock_irqsave(&psist->lock, flags);

	list_for_each_entry(psensor, &psist->sensors, list) {
		if (psensor->id == sensor->id)
			continue;

		psensor->ops->set_mode(psensor, SIST_BYPASS_MODE);
	}

	value = sensor->ops->get_value(sensor, request);

	spin_unlock_irqrestore(&psist->lock, flags);

	return value;
}
EXPORT_SYMBOL(send_sist_request);


int sist_get_sensor_count(void)
{
	return sist->max_sensors;
}
EXPORT_SYMBOL(sist_get_sensor_count);

const char *sist_get_sensor_name(int sensor_id)
{
	struct sist_sensor *sensor = NULL;
	if (!IS_VALID_SIST_ID(sensor_id)) {
		pr_err("SIST invalid sensor ID, sensor_id = %d\n", sensor_id);
		return ERR_PTR(-EINVAL);
	}

	sensor = sist_id_to_sensor(sensor_id);
	if (sensor == NULL) {
		return ERR_PTR(-ENODEV);
	}

	return (sensor->name);

}
EXPORT_SYMBOL(sist_get_sensor_name);

struct sist_sensor *sist_alloc_sensor(void)
{	
	struct sist_sensor *psensor = NULL;
	
	psensor = (struct sist_sensor *) \
		kmalloc(sizeof(struct sist_sensor), GFP_KERNEL);
	
	if (psensor == NULL) {
		pr_err("SIST alloc sensor failed!\n");
		return ERR_PTR(-ENOMEM);
	}
	
	if (sist == NULL) {
		pr_err("SIST not initialized!\n");
		return ERR_PTR(-ENODEV);
	}

	psensor->sist = sist;
	return psensor;
}
EXPORT_SYMBOL(sist_alloc_sensor);

int sist_register_sensor(struct sist_sensor *new)
{
	int ret = 0;
	unsigned long flags;
	struct sist_bus *psist = SENSOR_TO_SIST(new);
	struct sist_sensor *psensor = NULL;

	spin_lock_irqsave(&psist->lock, flags);

	list_for_each_entry(psensor, &psist->sensors, list) {
		if (!strncmp(psensor->name, new->name, strlen(psensor->name))) {
			WARN(1, "sensor (%s) already registered!\n", psensor->name);
			ret = -EEXIST;
			goto exit;
		}
	}

	new->id = psist->max_sensors;
	list_add(&new->list, &psist->sensors);
	psist->max_sensors++;


exit:
	spin_unlock_irqrestore(&psist->lock, flags);

	return ret;
}
EXPORT_SYMBOL(sist_register_sensor);


static int sist_init(void)
{
	struct sist_bus *psist = NULL;
	int ret = 0;

	if (sist) {
		goto out;
	} 

	psist = (struct sist_bus *)kmalloc(sizeof(struct sist_bus), GFP_KERNEL);
	if (!psist) {
		pr_err("%s failed!\n", __func__);
		ret = -ENOMEM;
		goto out;
	}
	memset(psist, 0, sizeof(struct sist_bus));
	INIT_LIST_HEAD(&psist->sensors);
	spin_lock_init(&psist->lock);
	psist->proc_sist_entry = proc_mkdir("SIST", NULL);

	sist = psist;
out:
	return ret;

}
subsys_initcall(sist_init);


static const struct of_device_id sist_sensor_of_match[] = {
	{ .compatible = "trix,sist-soc-sensor", .data=of_sist_soc_sensor_probe},
	{ }
};
MODULE_DEVICE_TABLE(of, sist_sensor_of_match);

static int sist_sensor_probe(struct platform_device *pdev)
{
	const struct of_device_id*	match = NULL;
	struct device 		*dev = &pdev->dev;


	match = of_match_node(sist_sensor_of_match, dev->of_node);

	if (match->data == of_sist_soc_sensor_probe)
		of_sist_soc_sensor_probe(dev->of_node);

	return 0;
}

static int sist_sensor_remove(struct platform_device *pdev)
{
	return 0;
}

static __maybe_unused int sist_pm_suspend(struct device *dev)
{
	return 0;
}

static __maybe_unused int sist_pm_resume(struct device *dev)
{
	return 0;
}

SIMPLE_DEV_PM_OPS(sist_sensor_pm_ops, sist_pm_suspend, sist_pm_resume);
static struct platform_driver sist_sensor_driver = {
	.probe = sist_sensor_probe,
	.remove = sist_sensor_remove,
	.driver = {
		.name = "trix-sist-sensor",
		.owner	= THIS_MODULE,
		.pm	= &sist_sensor_pm_ops,
		.of_match_table = of_match_ptr(sist_sensor_of_match),
	},
};

static int __init sist_drv_init(void)
{
	return platform_driver_register(&sist_sensor_driver);
}
module_init(sist_drv_init);

static void __exit sist_drv_exit(void)
{
	platform_driver_unregister(&sist_sensor_driver);
}
module_exit(sist_drv_exit);
MODULE_AUTHOR("Sigma Kernel team");
MODULE_DESCRIPTION("Sigma SIST sensor");
MODULE_LICENSE("GPL");
