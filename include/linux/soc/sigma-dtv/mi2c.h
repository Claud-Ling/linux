#include <linux/i2c.h>
#ifndef __ASM_ARCH_TRIX_MACH_MI2C__H__
#define __ASM_ARCH_TRIX_MACH_MI2C__H__

#define MI2C_CON		(0x00)
#define MI2C_C_ENABLE		(0x01)
#define MI2C_C_START		(0x02) /* Send START bit, self clear after done */
#define MI2C_C_STOP		(0x04) /* Send STOP bit, self clear after done */
#define MI2C_C_NEWDAT		(0x08) /* Begin read/write data, self clear */
#define MI2C_C_FIFO_CLR		(0x10) /* Clear FIFO, self clear */


#define MI2C_ADR		(0x04)
#define MI2C_A_RD		(0x01) /* I2C read operation */

#define MI2C_DAT		(0x08)

#define MI2C_STAT		(0x0c)
#define MI2C_S_INT		(0x01)
#define MI2C_S_FINISH		(0x02)
#define MI2C_S_ERR		(0x04)
#define MI2C_S_ARBLOST		(0x08)
#define MI2C_S_NDAT		(0x10)
#define MI2C_S_SINT		(0x20)
#define MI2C_S_MSK		(0x3f)

#define MI2C_TIMING_LLSB	(0x10)
#define MI2C_TIMING_LMSB	(0x11)
#define MI2C_TIMING_HLSB	(0x12)
#define MI2C_TIMING_HMSB	(0x13)

#define MI2C_SOFT		(0x14)

#define MI2C_FSM		(0x18)
#define MI2C_FSM_BUSY		(0x08)

#define MI2C_FUNC_SEL		(0x1c)

#define MI2C_BULK_CTL		(0x20)
#define MI2C_BULK_EN		(0x80)

#define MI2C_DAT_HOLD_LSB	(0x24)
#define MI2C_DAT_HOLD_MSB	(0x25)

#define MI2C_TIMEOUT_LSB	(0x28)
#define MI2C_TIMEOUT_MSB	(0x29)

#define MI2C_READ_NUM		(0x2c)


/* MI2C capcity definition */
#define MI2C_CAP_BULK		(0x01)		/* Have FIFO */

struct mi2c_rq_ctx {
	struct i2c_msg	*msg;
	s32		err;	/* Non zero value indicate error*/
	u32		offs;	/* Offset of transcation buf */
};

struct sigma_mi2c_dev {
	spinlock_t		lock;
	struct device		*dev;
#define MI2C_REG_OFFS		(0x80)
	void __iomem		*base;
	int			irq;
	u32			clkbase;
	u32			speed;
	u32			scl_high_time;
	u32			scl_low_time;
	u32			capacity;
	__u16			fifo_size;
	u32			status;
	struct completion	rq_complete;
	struct mi2c_rq_ctx	ctx;
	struct i2c_adapter	adapter;
};


struct sigma_mi2c_platform_data {
	u32		clkbase;
	u32		default_speed;
	u32		capacity;
	u32		fifo_size;
};

#endif /*__ASM_ARCH_TRIX_MACH_MI2C__H__*/
