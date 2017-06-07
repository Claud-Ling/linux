#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/soc/sigma-dtv/mi2c.h>

#define MI2C_DBG(dev, fmt, arg...) ({			\
	if (mi2c_dbg)					\
		dev_info(dev, fmt, ##arg);		\
	else						\
		do{}while(0);				\
})

/* Control debug log output */
static u32 mi2c_dbg = 0;

#if !defined(CONFIG_OF)
void sigma_mi2c_pinshare_init(struct sigma_mi2c_dev *dev)
{
	struct i2c_adapter *adap = &dev->adapter;

	switch(adap->nr) {
		case 0:
		#if defined(CONFIG_SIGMA_SOC_SX7)
			/* MI2C 0 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee1b, 0x0, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea48, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea49, 0x80, 0x80);
		#elif defined(CONFIG_SIGMA_SOC_SX8)
			/* MI2C 0 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee1b, 0x0, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea4c, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea4d, 0x80, 0x80);

		#endif
			break;

		case 1:
		#if defined(CONFIG_SIGMA_SOC_SX7)
			/* MI2C 1 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee1c, 0x0, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea4a, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea4b, 0x80, 0x80);
		#elif defined(CONFIG_SIGMA_SOC_SX8)
			/* MI2C 1 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee1c, 0x0, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea4e, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea4f, 0x80, 0x80);
		#endif
			break;
		case 2:
		#if defined(CONFIG_SIGMA_SOC_SX7)
			/* MI2C 2 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee1a, 0x0, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea46, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea47, 0x80, 0x80);
		#elif defined(CONFIG_SIGMA_SOC_SX8)
			/* MI2C 2 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee1a, 0x0, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea4a, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea4b, 0x80, 0x80);
		#endif
			break;
		case 3:
		#if defined(CONFIG_SIGMA_SOC_SX7)
			/* MI2C 2 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee19, 0x11, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea44, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea44, 0x80, 0x80);
		#elif defined(CONFIG_SIGMA_SOC_SX8)
			/* MI2C 2 */
			/* Set bit[6:4] 3b'000; bit[2:0] 3b'000 */
			MWriteRegByte((volatile void *)0x1500ee19, 0x11, 0x77);
			/* Set open-drain mode */
			MWriteRegByte((volatile void *)0x1500ea48, 0x80, 0x80);
			MWriteRegByte((volatile void *)0x1500ea49, 0x80, 0x80);
		#endif
			break;
		default:
			dev_err(dev->dev, "Unknow adapter nr = %d\n", adap->nr);
	}
	return;
}
#endif /* !CONFIG_OF */

static u8 mi2c_readb(struct sigma_mi2c_dev *dev, u32 ptr)
{
	return readb((const volatile void __iomem *)
			((unsigned long)(dev->base) + (unsigned int)ptr));
}

static void mi2c_writeb(struct sigma_mi2c_dev *dev, u8 val, u32 ptr)
{
	writeb(val, (volatile void __iomem *)
			((unsigned long)(dev->base) +(unsigned int)ptr));
	return;
}

/*
 * Calculate divide for particular speed with 50% duty cycle
 */
static u32 speed_to_divide(struct sigma_mi2c_dev *dev, u32 speed)
{
	u32 div = 0;

	/* split by 2 with 50% duty cycle */
	div = (dev->clkbase / speed) >> 1;

	return div;
}

static void sigma_mi2c_set_scl_timing(struct sigma_mi2c_dev *dev)
{
	mi2c_writeb(dev, dev->scl_low_time & 0xff, MI2C_TIMING_LLSB);
	mi2c_writeb(dev, (dev->scl_low_time >> 8) & 0xff, MI2C_TIMING_LMSB);
	mi2c_writeb(dev, dev->scl_high_time & 0xff, MI2C_TIMING_HLSB);
	mi2c_writeb(dev, (dev->scl_high_time >> 8) & 0xff, MI2C_TIMING_HMSB);
	return;
}

static ssize_t show_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned long flags;
	u32 speed = 0;
	struct i2c_adapter *adap = container_of(dev, struct i2c_adapter, dev);
	struct sigma_mi2c_dev *mdev = container_of(adap, struct sigma_mi2c_dev, adapter);

	spin_lock_irqsave(&mdev->lock, flags);
	speed = mdev->speed;
	spin_unlock_irqrestore(&mdev->lock, flags);

	return sprintf(buf, "%u\n", speed);
}

static ssize_t set_speed(struct device *dev, struct device_attribute *attr,
							const char *buf, size_t count)
{
	unsigned long flags, val;
	int err = 0;
	u32 speed = 0, div = 0;
	struct i2c_adapter *adap = container_of(dev, struct i2c_adapter, dev);
	struct sigma_mi2c_dev *mdev = container_of(adap, struct sigma_mi2c_dev, adapter);

	err = kstrtoul(buf, 10, &val);
	if (err)
		return err;

	speed = val;

	div = speed_to_divide(mdev, speed);

	spin_lock_irqsave(&mdev->lock, flags);
	mdev->speed = speed;
	mdev->scl_high_time = div;
	mdev->scl_low_time = div;
	sigma_mi2c_set_scl_timing(mdev);
	spin_unlock_irqrestore(&mdev->lock, flags);


	return count;

}

static ssize_t show_debug(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (mi2c_dbg)
		return sprintf(buf, "%s\n", "enable");
	else
		return sprintf(buf, "%s\n", "disable");
}

static ssize_t set_debug(struct device *dev, struct device_attribute *attr,
							const char *buf, size_t count)
{

	char str[] = "enable";

	if (!strncmp(buf, str, (sizeof(str)-1)))
		mi2c_dbg = 1;
	else
		mi2c_dbg = 0;

	return count;
}

static DEVICE_ATTR(speed, S_IRUGO | S_IWUSR, show_speed, set_speed);
static DEVICE_ATTR(debug, S_IRUGO | S_IWUSR, show_debug, set_debug);

static struct attribute *mi2c_attrs[] = {
	&dev_attr_speed.attr,
	&dev_attr_debug.attr,
	NULL
};

static const struct attribute_group mi2c_attrgp = {
	.attrs = mi2c_attrs,
};

static int sigma_mi2c_bus_recovery(struct sigma_mi2c_dev *dev) {

	u8 fsm = 0;


	fsm = mi2c_readb(dev, MI2C_FSM);

	/* Not Busy case, directly return */
	if (!(fsm & MI2C_FSM_BUSY))
		return -ETIMEDOUT;

	/* Disable adapter */
	mi2c_writeb(dev, 0x0, MI2C_CON);

	/* Directly clear busy bit first */
	mi2c_writeb(dev, (fsm & 0x7), MI2C_FSM);


	/* Busy bit still exist */
	if (mi2c_readb(dev, MI2C_FSM) & MI2C_FSM_BUSY) {

		mi2c_writeb(dev, 0x5, MI2C_SOFT);

		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();

		mi2c_writeb(dev, 0x1, MI2C_SOFT);

		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();

		mi2c_writeb(dev, 0x3, MI2C_SOFT);

		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();

		mi2c_writeb(dev, 0x7, MI2C_SOFT);

		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();

		mi2c_writeb(dev, 0x0, MI2C_SOFT);
	}

	/* Enable adapter */
	mi2c_writeb(dev, MI2C_C_ENABLE, MI2C_CON);

	return -EAGAIN;
}

irqreturn_t sigma_mi2c_irq_handler(int irq, void * data)
{

	u8 stat = 0, con = 0;
	u8 err_msk = MI2C_S_ERR | MI2C_S_ARBLOST;
	u32 size;
	u8 *p = NULL;
	u8 dump[32] = { 0 };

	int i;
	unsigned long flags;

	struct i2c_msg *msg = NULL;

	struct sigma_mi2c_dev *dev = (struct sigma_mi2c_dev *)data;
	struct mi2c_rq_ctx	*ctx = &dev->ctx;

	spin_lock_irqsave(&dev->lock, flags);


	stat = (mi2c_readb(dev, MI2C_STAT) & MI2C_S_MSK);

	/* Ack all of interrupts */
	mi2c_writeb(dev, MI2C_S_MSK, MI2C_STAT);

	MI2C_DBG(dev->dev, "Got MI2C(%d) interrupt, stat = %02x\n", dev->adapter.nr, stat);
	/* Error case */
	if (stat & err_msk) {
		dev_err(dev->dev, "Got error interrupt, stat = %02x\n", stat);
		dev->ctx.err = -ENODEV;
		goto done;
	} else if (dev->ctx.msg == NULL) {
		/* This cond should never reached !*/
		WARN(dev->ctx.msg == NULL, "MI2C(%d) got interrupt, but no request pending!\n", dev->adapter.nr);
		goto out;
	}


	msg = dev->ctx.msg;

	size = min(dev->fifo_size, (__u16)(msg->len - ctx->offs));
	p = (u8 *)(((unsigned long) msg->buf) + ctx->offs);

	if (!size) {
		/* Write case, last data be tranfered */
		goto done;
	}

	if (msg->flags & I2C_M_RD) {
		MI2C_DBG(dev->dev, "READ: Data dump, len = %d\n", size);
	} else {
		MI2C_DBG(dev->dev, "WRITE: Data dump, len = %d\n", size);
	}
	for (i=0; i<size; i++) {
		if (msg->flags & I2C_M_RD) {
			*p = mi2c_readb(dev, MI2C_DAT);
		} else {
			mi2c_writeb(dev, *p, MI2C_DAT);
		}

		sprintf(&dump[(i&0x7)*3], " %02x", *p);
		if (i != 0 && (i&0x7) == 0x7) {
			MI2C_DBG(dev->dev, "%s", &dump[0]);
			memset(&dump[0], 0, sizeof(dump));
		}

		p++;
		ctx->offs++;
	}
	MI2C_DBG(dev->dev, "%s", &dump[0]);
	size = min(dev->fifo_size, (__u16)(msg->len - ctx->offs));

	if (!size && msg->flags & I2C_M_RD) {
		/* In read case, Job should be done here*/
		goto done;
	}

	/* Set read size */
	if (msg->flags & I2C_M_RD) {

		mi2c_writeb(dev, (size&0xff), MI2C_READ_NUM);
	}

	/* Trigger next read/write */
	con = MI2C_C_ENABLE | MI2C_C_NEWDAT;
	mi2c_writeb(dev, con, MI2C_CON);

out:
	spin_unlock_irqrestore(&dev->lock, flags);
	return IRQ_HANDLED;

done:
	complete(&dev->rq_complete);
	spin_unlock_irqrestore(&dev->lock, flags);
	return IRQ_HANDLED;

}

static int mi2c_calculate_wait_time(struct i2c_adapter *adap, struct i2c_msg *msg)
{
	struct sigma_mi2c_dev *mdev = container_of(adap, struct sigma_mi2c_dev, adapter);
	/* Bus speed, theoretically max bits per seconds, omit START STOP ACK bit */
	u32 speed = mdev->speed;
	u32 transfer_bits = 0;
	u32 wait_millsec = 0;
	u32 bit_per_microsec = 1000000 / speed;
	u32 bonus = 0;

	/* data length from device, the MAX is 256 bytes */
	if (msg->flags & I2C_M_RECV_LEN) {
		/* 256 * 8 bits */
		transfer_bits = (0x100 << 3);
	} else {
		transfer_bits = (msg->len << 3);
	}

	if (msg->len > 1 || msg->flags & I2C_M_RECV_LEN) {
	/*
	 * message len greater than 1, should not be probe operation
	 * from i2c-core, so add more wait time here should not impact
	 * performance.
	 */
		bonus = (10*jiffies_to_msecs(HZ));
	}

	/* Bonus for START STOP ACK bit & with some redundance, totally 300% */
	transfer_bits += ((transfer_bits * 30) / 10);

	/* Convert to million seconds and with add-on 200ms for interrupt delay in system busy case */
	wait_millsec = ((transfer_bits * bit_per_microsec) / 1000) + 200;
	wait_millsec += bonus;

	MI2C_DBG(mdev->dev, "Calculate msg transfer wait time\n");
	MI2C_DBG(mdev->dev, "bit_per_microsec = %u\n", bit_per_microsec);
	MI2C_DBG(mdev->dev, "transfer_bits = %u\n", transfer_bits);
	MI2C_DBG(mdev->dev, "wait_millsec = %u\n", wait_millsec);

	return wait_millsec;
}

static int sigma_mi2c_xfer_msg(struct i2c_adapter *adap, struct i2c_msg *msg, int stop_bit)
{

	u8 con = 0, adr = 0;
	u32 size, timeout, wait_time;
	unsigned long flags;
	int i, err = 0;
	u8 *p = msg->buf;
	u8 dump[32] = { 0 };
	struct sigma_mi2c_dev *dev = container_of(adap, struct sigma_mi2c_dev, adapter);

	spin_lock_irqsave(&dev->lock, flags);
	dev->ctx.msg = msg;
	dev->ctx.err = 0;
	dev->ctx.offs = 0;


	con = MI2C_C_ENABLE | MI2C_C_FIFO_CLR;
	mi2c_writeb(dev, con, MI2C_CON);

	adr = (msg->addr<<1) & 0xfe;

	if (msg->flags & I2C_M_RD)
		adr |= MI2C_A_RD;

	if (msg->flags & I2C_M_STOP)
		stop_bit = 1;

	mi2c_writeb(dev, adr, MI2C_ADR);

	/* Fill FIFO or set read length */
	size = min(dev->fifo_size, msg->len);
	if (msg->flags & I2C_M_RD) {
		mi2c_writeb(dev, (size&0xff), MI2C_READ_NUM);
		MI2C_DBG(dev->dev, "READ: set read num %02x", size & 0xff);
	} else {
		MI2C_DBG(dev->dev, "WRITE: Data dump, len = %d\n", size);
		for (i=0; i<size; i++) {
			mi2c_writeb(dev, *p, MI2C_DAT);

			sprintf(&dump[(i&0x7)*3], " %02x", *p);
			if (i != 0 && (i&0x7) == 0x7) {
				MI2C_DBG(dev->dev, "%s", &dump[0]);
				memset(&dump[0], 0, sizeof(dump));
			}
			p++;
			dev->ctx.offs++;
		}
	}
	MI2C_DBG(dev->dev, "%s", &dump[0]);

	wait_time = mi2c_calculate_wait_time(adap, msg);
	/* Start transfer */
	con = MI2C_C_ENABLE | MI2C_C_START;
	mi2c_writeb(dev, con, MI2C_CON);

	spin_unlock_irqrestore(&dev->lock, flags);
	/* Wait request done */
	timeout = wait_for_completion_timeout(&dev->rq_complete, msecs_to_jiffies(wait_time));

	if (!timeout) {
		dev_err(dev->dev, "Timeout wait for msg transfer!\n");
		dev_err(dev->dev, "msg submit by %s(%d)!\n", current->comm, current->pid);
		dev_err(dev->dev, "\taddr:%04x len:%04x\n", msg->addr, msg->len);
		err = -EAGAIN;
	}

	if (!stop_bit)
		goto skip;

	spin_lock_irqsave(&dev->lock, flags);
	mi2c_writeb(dev, MI2C_C_ENABLE | MI2C_C_STOP, MI2C_CON);
	spin_unlock_irqrestore(&dev->lock, flags);

	/* Wait request done */
	timeout = wait_for_completion_timeout(&dev->rq_complete, msecs_to_jiffies(20));

	if (!timeout) {
		dev_err(dev->dev, "Timeout wait for stop transfer!\n");
		err = -EAGAIN;
	}


skip:
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->ctx.err)
		err = -ENODEV;

	/* Cleanup context */
	dev->ctx.msg = NULL;
	dev->ctx.err = 0;
	dev->ctx.offs = 0;

	/* Try to recovery adapter FSM */
	if (err == -EAGAIN) {
		/* May be bus stuck in busy status, check & recovery */
		err = sigma_mi2c_bus_recovery(dev);
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return err;
}

static u32 sigma_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | (I2C_FUNC_SMBUS_EMUL & ~I2C_FUNC_SMBUS_QUICK) |
	       I2C_FUNC_PROTOCOL_MANGLING;
}

static int sigma_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	int i, ret = -1;
	for (i=0; i<num; i++) {
		ret = sigma_mi2c_xfer_msg(adap, &msgs[i], i==(num-1));
		if (ret)
			break;
	}

	if (ret==0)
		ret = num;

	return ret;
}

static const struct i2c_algorithm sigma_mi2c_algo = {
	.master_xfer = sigma_i2c_xfer,
	.functionality = sigma_i2c_func,
};


static void sigma_mi2c_setup_platdata(struct device *dev,
				 struct sigma_mi2c_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	u32 val = 0;

	/* Assume base clock is 200MHz */
	pdata->clkbase = 200000000;

	if(!of_property_read_u32(np,"mi2c,fifosize", &val)) {
		pdata->fifo_size = val;
	} else {
		/* Assume not support FIFO */
		pdata->fifo_size = 1;
	}

	if (pdata->fifo_size > 1) {
		pdata->capacity |= MI2C_CAP_BULK;
	}

	if(!of_property_read_u32(np,"mi2c,default-speed", &val)) {
		pdata->default_speed = val;
	} else {
		/* Normal speed is 100kHz */
		pdata->default_speed = 100000;
	}

	/* Refer to I2C spec, normal speed is 100kHz, up to 400kHz */
	if (pdata->default_speed > 400000) {
		pdata->default_speed = 400000;
	}

	return;
}

static struct sigma_mi2c_platform_data *sigma_mi2c_get_platdata_by_of(struct device *dev)
{
	struct sigma_mi2c_platform_data *pdata = NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);

	if (pdata == NULL)
		return ERR_PTR(-ENOMEM);

	sigma_mi2c_setup_platdata(dev, pdata);

	return pdata;
}

static int sigma_mi2c_probe(struct platform_device *pdev)
{
	struct sigma_mi2c_dev	*mdev = NULL;
	struct i2c_adapter	*adap = NULL;
	struct resource		*mem = NULL;
	int irq, ret;
	void __iomem 		*regs = NULL;
	struct device 		*dev = &pdev->dev;

	struct sigma_mi2c_platform_data *pdata = pdev->dev.platform_data;

	if (pdata == NULL) {
		pdata = sigma_mi2c_get_platdata_by_of(dev);
	}

	if (IS_ERR(pdata))
		return PTR_ERR(pdata);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		dev_err(&pdev->dev, "no mem resource?\n");
		return -ENODEV;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq resource?\n");
		return irq;
	}

	mdev = devm_kzalloc(&pdev->dev, sizeof(struct sigma_mi2c_dev), GFP_KERNEL);
	if (!mdev) {
		dev_err(&pdev->dev, "Menory allocation failed\n");
		return -ENOMEM;
	}

	regs = devm_ioremap_resource(dev, mem);
	mdev->base = (void __iomem *)((unsigned long)regs + MI2C_REG_OFFS);
	mdev->irq = irq;
	mdev->dev = &pdev->dev;

	platform_set_drvdata(pdev, dev);
	init_completion(&mdev->rq_complete);

	/* Get device physical attribute */
	mdev->clkbase = pdata->clkbase;
	mdev->capacity = pdata->capacity;
	mdev->fifo_size = pdata->fifo_size;
	mdev->speed = pdata->default_speed;
	mdev->scl_high_time = speed_to_divide(mdev, mdev->speed);
	mdev->scl_low_time = mdev->scl_high_time;

	if (mdev->capacity & MI2C_CAP_BULK) {
		/* Enable BULK fifo */
		mi2c_writeb(mdev, MI2C_BULK_EN, MI2C_BULK_CTL);

	} else {
		/* Not support BULK, set fifo_size equ 1 */

		dev_err(mdev->dev, "MI2C(%d) not support MI2C_CAP_BULK, overwrite fifo size equ '1'\n", pdev->id);
		mdev->fifo_size = 1;
	}

	/* Setup SCL speed, default is 100k specified from platform data */
	sigma_mi2c_set_scl_timing(mdev);


	adap = &mdev->adapter;
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON;
	adap->nr = pdev->id;
	adap->algo = &sigma_mi2c_algo;
	snprintf(adap->name, sizeof(adap->name), "MI2C adapter %d", adap->nr);
	/* Msg transfer failed, retry one more time */
	adap->retries = 1;

#if !defined(CONFIG_OF)
	/* Setup pinshare */
	sigma_mi2c_pinshare_init(mdev);
#endif

	/* Make sure no interrupt pending here */
	ret = devm_request_irq(mdev->dev, irq, sigma_mi2c_irq_handler, IRQF_SHARED,
									pdev->name, mdev);
	if (ret) {

		dev_err(mdev->dev, "Unable request irq %d, ret = %d\n", irq, ret);
		goto err;
	}

	ret = i2c_add_numbered_adapter(adap);
	if (ret) {
		dev_err(mdev->dev, "Register I2C adapter %s err\n", adap->name);
		goto err;
	}

	ret = sysfs_create_group(&adap->dev.kobj, &mi2c_attrgp);

	if (ret) {
		dev_err(mdev->dev, "Create sysfs for  %s err\n", adap->name);
		goto err1;
	}

	dev_info(mdev->dev, "Register MI2C(%d) with speed %uHz, clk div %04x, irq %d\n", adap->nr, mdev->speed, mdev->scl_high_time, irq);
	return ret;

err1:
	i2c_del_adapter(&mdev->adapter);
err:
	return -ENODEV;
}


static int sigma_mi2c_remove(struct platform_device *pdev)
{
	struct sigma_mi2c_dev	*dev = platform_get_drvdata(pdev);
	struct i2c_adapter *adap = &dev->adapter;

	sysfs_remove_group(&adap->dev.kobj, &mi2c_attrgp);
	i2c_del_adapter(&dev->adapter);

	return 0;
}

#if defined(CONFIG_PM)
int sigma_mi2c_suspend(struct device *dev)
{
	/* Nothing need to do */
	MI2C_DBG(dev, "Enter %s\n", __func__);
	return 0;
}

int sigma_mi2c_resume(struct device *dev)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct sigma_mi2c_dev *mdev = (struct sigma_mi2c_dev *)platform_get_drvdata(pdev);

#if !defined(CONFIG_OF)
	/* Setup pinshare */
	sigma_mi2c_pinshare_init(mdev);
#endif

	/* Setup SCL timing */
	sigma_mi2c_set_scl_timing(mdev);

	if (mdev->capacity & MI2C_CAP_BULK) {
		/* Enable BULK fifo */
		mi2c_writeb(mdev, MI2C_BULK_EN, MI2C_BULK_CTL);
	}

	MI2C_DBG(dev, "Enter %s\n", __func__);
	return 0;
}

static struct dev_pm_ops sigma_mi2c_pm_ops = {
		.suspend = sigma_mi2c_suspend,
		.resume = sigma_mi2c_resume,
};

#define SIGMA_MI2C_PM_OPS	(&sigma_mi2c_pm_ops)
#else
#define SIGMA_MI2C_PM_OPS	(NULL)
#endif

static const struct of_device_id sigma_mi2c_dt_match[] = {
	{ .compatible = "sigma,trix-mi2c" },
	{/* sentinel */}
};

static struct platform_driver sigma_mi2c_driver = {
	.probe		= sigma_mi2c_probe,
	.remove		= sigma_mi2c_remove,
	.driver		= {
		.name	= "trix-mi2c",
		.owner	= THIS_MODULE,
		.pm	= SIGMA_MI2C_PM_OPS,
		.of_match_table = of_match_ptr(sigma_mi2c_dt_match),
	},
};

static int __init sigma_i2c_init(void)
{
	return platform_driver_register(&sigma_mi2c_driver);
}
subsys_initcall(sigma_i2c_init);

static void __exit sigma_i2c_exit(void)
{
	platform_driver_unregister(&sigma_mi2c_driver);
	return;
}
module_exit(sigma_i2c_exit);

MODULE_AUTHOR("Sigma Kernel team");
MODULE_DESCRIPTION("Sigma I2C bus adapter");
MODULE_LICENSE("GPL");
