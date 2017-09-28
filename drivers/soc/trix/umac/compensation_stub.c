#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <umac.h>

/*
 * Stub implementation
 */
int __weak umac_compensation_init(struct umac_device *udev)
{
	return 0;
}

