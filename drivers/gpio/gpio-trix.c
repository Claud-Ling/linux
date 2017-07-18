/*
 * Copyright (c) 2017 SigmaDesigns, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/pm.h>

#define DRIVER_NAME "GPIO"

/**
 * struct trix_gpio_reg_offs - describing the register offset inside the bank
 * @directoin:		configure the gpio direction
 * @data:		[15:8]datain   [7:0]dataout
 * @irqconf:		[15:8]trigger [7:0]irq enable/disable
 * @irqstatus:		indicates the irq status
 * @irqstatus_upper_byte:	true if the irq status reg resides in the upper-byte
 *				of irqstatus register
 */
struct trix_gpio_reg_offs {
	u16 direction;
	u16 data;
	u16 irqconf;
	u16 irqstatus;
	bool irqstatus_upper_byte;
};

struct trix_gpio_soc_info {
	int nbank;
	struct trix_gpio_reg_offs *regs;
};

/**
 * struct trix_gpio - gpio device private data structure
 * @base:	base address of the GPIO device
 * @irq:	interrupt for the GPIO device
 * @lock:	spin lock
 * @chip:	instance of the gpio_chip
 * @soc:	pointer to the soc specific data
 */
struct trix_gpio {
	void __iomem	*base;
	int		irq;
	spinlock_t	lock;

	struct gpio_chip	chip;

	struct trix_gpio_soc_info *soc;
};

#define NGPIO_PER_BANK		(8)
#define GPIO_BANK(pin)		((pin) >> 3)
#define GPIO_BIT(pin)		((pin) & (NGPIO_PER_BANK - 1))
#define GPIO_COMPOSE(bank,offset) ((bank << 3) + offset)

#define GPIO_DIR_IN	0x0
#define GPIO_DIR_OUT	0x1
#define GPIO_DIR_PWM_OUT	0x2
#define GPIO_DIR_MASK	0x3

#define GPIO_DATAIN_SHIFT      8

#define GPIO_INT_ENABLE_MASK   0x00ff
#define GPIO_INT_STATUS_MASK   0x00ff

#define GPIO_INT_EDGE_FALLING  0
#define GPIO_INT_EDGE_RISING   1

static bool trix_gpio_is_output(struct trix_gpio *gpio, unsigned int pin)
{
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].direction;
	u16 dir;

	dir = readw_relaxed(reg);
	dir = (dir >> (offs << 1)) & GPIO_DIR_MASK;

	if (dir == GPIO_DIR_OUT || dir == GPIO_DIR_PWM_OUT)
		return true;
	else
		return false;
}

static void trix_gpio_set_direction(struct trix_gpio *gpio, unsigned int pin,
				    int is_output)
{
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].direction;
	u16 val;

	val = readw_relaxed(reg);
	val &= ~(GPIO_DIR_MASK << (offs << 1));
	if (is_output)
		val |= (GPIO_DIR_OUT << (offs << 1));
	else
		val |= (GPIO_DIR_IN << (offs << 1));

	writew_relaxed(val, reg);
}

static void trix_gpio_set_irqmask(struct trix_gpio *gpio, unsigned int pin)
{
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].irqconf;
	u16 val;

	val = readw_relaxed(reg);
	val &= ~(BIT(offs));
	writew_relaxed(val, reg);
}

static void trix_gpio_clr_irqmask(struct trix_gpio *gpio, unsigned int pin)
{
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].irqconf;
	u16 val;

	val = readw_relaxed(reg);
	val |= BIT(offs);
	writew_relaxed(val, reg);
}

static void trix_gpio_clr_irqstatus(struct trix_gpio *gpio, unsigned int pin)
{
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].irqstatus;
	u16 val;

	val = BIT(offs);
	if (gpio->soc->regs[bank].irqstatus_upper_byte)
		val <<= 8;
	/*
	 * clear the interrupt by
	 * writting "1" to corresponding interrupt status bit
	 */
	writew_relaxed(val, reg);
}

static u16 trix_gpio_get_irq_status_bank(struct trix_gpio *gpio, unsigned int bank)
{
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].irqstatus;
	u16 int_sts;

	int_sts = readw_relaxed(reg);

	if (gpio->soc->regs[bank].irqstatus_upper_byte)
		int_sts >>= 8;

	int_sts &= GPIO_INT_ENABLE_MASK;

	return int_sts;
}

static u16 trix_gpio_get_irq_enable_bank(struct trix_gpio *gpio, unsigned int bank)
{
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].irqconf;
	u16 int_enb;

	int_enb = readw_relaxed(reg) & GPIO_INT_ENABLE_MASK;

	return int_enb;
}

static void trix_gpio_set_trigger(struct trix_gpio *gpio, unsigned int pin, int edge)
{
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].irqconf;
	u16 val;

	val = readw_relaxed(reg);

	if (edge == GPIO_INT_EDGE_FALLING)
		val &= ~(BIT(offs + 8));
	else if (edge == GPIO_INT_EDGE_RISING)
		val |= BIT(offs + 8);
	else
		return;

	writew_relaxed(val, reg);
}

static int trix_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	int pin = chip->base + offset;

	pinctrl_request_gpio(pin);

	return 0;
}

static void trix_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	int pin = chip->base + offset;

	pinctrl_free_gpio(pin);
}

/**
 * trix_gpio_get_value - Get the state of the specified pin
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 *
 * This function reads the state of the specified pin of the GPIO device.
 * Return: 0 if the pin is low, 1 if pin is high.
 */
static int trix_gpio_get_value(struct gpio_chip *chip, unsigned int pin)
{
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].data;
	u16 val;

	val = readw_relaxed(reg);

	if(trix_gpio_is_output(gpio,pin))
		return (val >> offs) & 1;
	else
		return (val >> (GPIO_DATAIN_SHIFT + offs)) & 0x1;
}

/**
 * trix_gpio_set_value - Modify the state of the pin with specified value
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 * @state:	value used to modify the state of the specified pin
 *
 * This function uses "read-modify-write" sequence to sets the state of a gpio pin
 * to the specified value.
 * The state is either 0 or non-zero.
 */
static void trix_gpio_set_value(struct gpio_chip *chip, unsigned int pin,
			       int state)
{
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned int bank = GPIO_BANK(pin);
	unsigned int offs = GPIO_BIT(pin);
	void __iomem *reg = gpio->base + gpio->soc->regs[bank].data;
	unsigned long flags;
	u16 val;

	state = !!state;

	spin_lock_irqsave(&gpio->lock, flags);

	val = readw_relaxed(reg);
	if (state)
		val |= BIT(offs);
	else
		val &= ~(BIT(offs));
	writew_relaxed(val, reg);

	spin_unlock_irqrestore(&gpio->lock, flags);
}

/**
 * trix_gpio_dir_in - Set the direction of the specified GPIO pin as input
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 *
 * This function uses the read-modify-write sequence to set the direction of
 * the gpio pin as input.
 *
 * Return: 0 always
 */
static int trix_gpio_dir_in(struct gpio_chip *chip, unsigned int pin)
{
	unsigned long flags;
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);

	spin_lock_irqsave(&gpio->lock, flags);

	trix_gpio_set_direction(gpio, pin, 0);

	spin_unlock_irqrestore(&gpio->lock, flags);

	return 0;
}

/**
 * trix_gpio_dir_out - Set the direction of the specified GPIO pin as output
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 * @state:	value to be written to specified pin
 *
 * This function uses the read-modify-write sequence to set the direction of
 * the gpio pin as output.
 *
 * Return: 0 always
 */
static int trix_gpio_dir_out(struct gpio_chip *chip, unsigned int pin,
			     int state)
{
	unsigned long flags;
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);

	spin_lock_irqsave(&gpio->lock, flags);
	trix_gpio_set_direction(gpio, pin, 1);
	spin_unlock_irqrestore(&gpio->lock, flags);

	/* set the state of the pin */
	trix_gpio_set_value(chip, pin, state);


	return 0;
}

/*
 * trix_gpio_irq_ack - Acknowledge the interrupt of a gpio pin
 */
static void trix_gpio_irq_ack(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_handler_data(d);
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned long flags;
	unsigned int pin = irqd_to_hwirq(d);

	spin_lock_irqsave(&gpio->lock, flags);
	trix_gpio_clr_irqstatus(gpio, pin);
	spin_unlock_irqrestore(&gpio->lock, flags);
}

/*
 * trix_gpio_irq_mask - Disable the interrupts for a gpio pin
 */
static void trix_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_handler_data(d);
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned long flags;
	unsigned int pin = irqd_to_hwirq(d);

	spin_lock_irqsave(&gpio->lock, flags);
	trix_gpio_set_irqmask(gpio, pin);
	spin_unlock_irqrestore(&gpio->lock, flags);
}

/*
 * trix_gpio_irq_unmask - Enable the interrupts for a gpio pin
 */
static void trix_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_handler_data(d);
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned long flags;
	unsigned int pin = irqd_to_hwirq(d);

	spin_lock_irqsave(&gpio->lock, flags);
	trix_gpio_clr_irqmask(gpio, pin);
	spin_unlock_irqrestore(&gpio->lock, flags);
}

/**
 * trix_gpio_irq_enable - Enable the interrupts for a gpio pin
 *
 * Clears the INTSTS bit and unmasks the given interrrupt
 */
static void trix_gpio_irq_enable(struct irq_data *d)
{
	struct gpio_chip *chip = irq_data_get_irq_chip_data(d);
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned long flags;
	unsigned int pin = irqd_to_hwirq(d);

	spin_lock_irqsave(&gpio->lock, flags);
	trix_gpio_set_direction(gpio, pin, 0);
	trix_gpio_clr_irqstatus(gpio, pin);
	trix_gpio_clr_irqmask(gpio, pin);
	spin_unlock_irqrestore(&gpio->lock, flags);
}
/*
 * trix_gpio_irq_set_type - Set the irq type for a gpio pin
 *
 * Return: 0, negative error otherwise.
 */
static int trix_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *chip = irq_data_get_irq_chip_data(d);
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	unsigned long flags;
	unsigned int pin = irqd_to_hwirq(d);
	int edge;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_FALLING:
		edge = GPIO_INT_EDGE_FALLING;
		break;
	case IRQ_TYPE_EDGE_RISING:
		edge = GPIO_INT_EDGE_RISING;
		break;
	default:
		return -EINVAL;
	}

	spin_lock_irqsave(&gpio->lock, flags);
	trix_gpio_set_trigger(gpio, pin, edge);
	spin_unlock_irqrestore(&gpio->lock, flags);

	return 0;
}

static void trix_gpio_handle_bank_irq(struct trix_gpio *gpio,
					unsigned int bank,
					unsigned long pending)
{
	int offset, pin;

	if (!pending)
		return;

	for_each_set_bit(offset, &pending, NGPIO_PER_BANK) {

		pin = GPIO_COMPOSE(bank, offset);
		trix_gpio_clr_irqstatus(gpio, pin);
		generic_handle_irq(irq_find_mapping(gpio->chip.irqdomain,
						    pin));
	}
}

/**
 * trix_gpio_irq_handler - IRQ handler for the gpio device
 * @irq:	irq number of the gpio device where interrupt has occurred
 * @desc:	irq descriptor instance of the 'irq'
 *
 * This function reads the Interrupt Status Register of each bank to get the
 * gpio pin number which has trigger an interrupt. It then acks the triggered
 * interrupt and calls the pin specific handler set by the higher layer
 * application for that pin.
 */
static void trix_gpio_irq_handler(unsigned irq, struct irq_desc *desc)
{
	struct gpio_chip *chip = irq_desc_get_handler_data(desc);
	struct trix_gpio *gpio = container_of(chip, struct trix_gpio, chip);
	struct irq_chip *irqchip = irq_desc_get_chip(desc);
	u16 int_sts, int_enb;
	unsigned int bank;

	/*
	 * note that the "irqchip" is pointing to gic_chip
	 * which is the interrupt parent of GPIO device
	 */
	chained_irq_enter(irqchip, desc);

	for (bank = 0; bank < gpio->soc->nbank; bank++) {
		int_sts = trix_gpio_get_irq_status_bank(gpio, bank);
		int_enb = trix_gpio_get_irq_enable_bank(gpio, bank);

		trix_gpio_handle_bank_irq(gpio, bank, int_sts & int_enb);
	}

	chained_irq_exit(irqchip, desc);
	/* now it may re-trigger */
}

/* irq chip descriptor */
static struct irq_chip trix_gpio_irqchip = {
	.name		= DRIVER_NAME,
	.irq_enable	= trix_gpio_irq_enable,
	.irq_ack	= trix_gpio_irq_ack,
	.irq_mask	= trix_gpio_irq_mask,
	.irq_unmask	= trix_gpio_irq_unmask,
	.irq_set_type	= trix_gpio_irq_set_type,
};

static struct of_device_id trix_gpio_of_match[];

static int trix_gpio_probe(struct platform_device *pdev)
{
	int ret;
	struct trix_gpio *gpio;
	struct gpio_chip *chip;
	struct resource *res;
	const struct of_device_id *match;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	platform_set_drvdata(pdev, gpio);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gpio->base))
		return PTR_ERR(gpio->base);

	gpio->irq = platform_get_irq(pdev, 0);
	if (gpio->irq < 0) {
		dev_err(&pdev->dev, "invalid IRQ\n");
		return gpio->irq;
	}

	match = of_match_device(of_match_ptr(trix_gpio_of_match), &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "Error: No device match found\n");
		return -ENODEV;
	}

	gpio->soc = (struct trix_gpio_soc_info *)match->data;

	/* configure the gpio chip */
	chip = &gpio->chip;
	chip->dev = &pdev->dev;
	chip->label = DRIVER_NAME;
	chip->owner = THIS_MODULE;

	chip->request = trix_gpio_request;
	chip->free = trix_gpio_free;

	chip->get = trix_gpio_get_value;
	chip->set = trix_gpio_set_value;

	chip->direction_input = trix_gpio_dir_in;
	chip->direction_output = trix_gpio_dir_out;

	/*
	 * identifies the first GPIO number as 0
	 * alternatively a negative value for dynamic ID allocation
	 */
	chip->base = 0;
	chip->ngpio = gpio->soc->nbank * NGPIO_PER_BANK;

	ret = gpiochip_add(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add gpio chip\n");
		return ret;
	}

	/*
	 * Let the generic code handle this edge IRQ, the the chained
	 * handler will perform the actual work of handling the parent
	 * interrupt.
	 */
	ret = gpiochip_irqchip_add(chip, &trix_gpio_irqchip,
				   0, handle_simple_irq,
				   IRQ_TYPE_NONE);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add irq chip\n");
		goto err_rm_gpiochip;
	}

	gpiochip_set_chained_irqchip(chip, &trix_gpio_irqchip, gpio->irq,
				     trix_gpio_irq_handler);

	spin_lock_init(&gpio->lock);

	return 0;

err_rm_gpiochip:
	gpiochip_remove(chip);

	return ret;
}

static struct trix_gpio_reg_offs sxx_gpio_regs[] = {
	/* direction    data    irqconf   irqstatus  irqstatus_upper_byte */
	{  0x00,        0x02,   0x16,     0x18,       false           },  //bank0
	{  0x20,        0x22,   0x36,     0x18,       true            },  //bank1
	{  0x40,        0x42,   0x56,     0x58,       false           },  //bank2
	{  0x60,        0x62,   0x76,     0x58,       true            },  //bank3
	{  0x80,        0x82,   0x96,     0x98,       false           },  //bank4
	{  0xa0,        0xa2,   0xb6,     0x98,       true            },  //bank5
	{  0xc0,        0xc2,   0xd6,     0xd8,       false           },  //bank6
	{  0xe0,        0xe2,   0xf6,     0xd8,       true            },  //bank7
	{  0x26,        0x28,   0x2a,     0x2c,       false           },  //bank8
	{  0x2e,        0x30,   0x32,     0x2c,       true            },  //bank9
	{  0x46,        0x48,   0x4a,     0x4c,       false           },  //bank10
	{  0x4e,        0x50,   0x52,     0x4c,       true            },  //bank11
};

struct trix_gpio_soc_info sxx_gpio_device = {
	.nbank = ARRAY_SIZE(sxx_gpio_regs),
	.regs = sxx_gpio_regs,
};

static struct of_device_id trix_gpio_of_match[] = {
	{ .compatible = "sigma,trix-gpio",   .data = &sxx_gpio_device},
	{ /* end of table */ },
};
MODULE_DEVICE_TABLE(of, trix_gpio_of_match);

static struct platform_driver trix_gpio_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(trix_gpio_of_match),
	},
	.probe = trix_gpio_probe,
};

static int __init trix_gpio_init(void)
{
	return platform_driver_register(&trix_gpio_driver);
}
postcore_initcall(trix_gpio_init);

MODULE_DESCRIPTION("GPIO driver for Sigmadesigns DTV platform");
MODULE_LICENSE("GPL");
