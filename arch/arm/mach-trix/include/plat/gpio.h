/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */


#ifndef __ASM_ARCH_TRIX_PLAT_GPIO_H__
#define __ASM_ARCH_TRIX_PLAT_GPIO_H__

#include <linux/io.h>
#include <linux/platform_device.h>
#include <mach/irqs.h>


/* Wrappers for "new style" GPIO calls, using the new infrastructure
 * which lets us plug in FPGA, I2C, and other implementations.
 * *
 * The original OMAP-specific calls should eventually be removed.
 */

#include <linux/errno.h>
#include <asm-generic/gpio.h>

static inline int gpio_get_value(unsigned gpio)
{
	return __gpio_get_value(gpio);
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	__gpio_set_value(gpio, value);
}

static inline int gpio_cansleep(unsigned gpio)
{
	return __gpio_cansleep(gpio);
}

static inline int gpio_to_irq(unsigned gpio)
{
	return __gpio_to_irq(gpio);
}

static inline int irq_to_gpio(unsigned irq)
{
	return irq;
}

#endif /*__ASM_ARCH_TRIX_PLAT_GPIO_H__*/
