/*
 *  linux/arch/arm/mach-trix/mcuctrl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SoC mcu comm driver
 * A9 -> MCU communication implemented
 *
 * Author: Tony He, 2016.
 */
#define pr_fmt(fmt) "mcuctrl: " fmt

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <mach/irqs.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/semaphore.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#define MIPS_COMM_BASEREG		0xf5000000 //0x15000000
#define CMD_MIPS_INTR_MCU_MASK	(1<<3)
#define CMD_MIPS_RES_MCU_MASK	(1<<4)

#define CMD_CTRL_TOTAL_MASK		(0xf<<4)
#define CMD_CTRL_STAMP_MASK		(0xf)
/*F,R,C bits are introduced since mcu_comm protocol 1.0.3 (mcu_comm extension)*/
#define CMD_CTRL_F_MASK			(1<<7)
#define CMD_CTRL_R_MASK			(1<<6)
#define CMD_CTRL_C_MASK			(1<<5)

#define CMD_CTRL_ERR_MASK		(1<<4)
#define CMD_CTRL_LEN_MASK		(0xf)

#define CTRLSEG_LEN				(2)
#define DATASEG_LEN				(16 - CTRLSEG_LEN)

#define MIPS_REGFILE_START 		0xf5000010
#define MIPS_CTRLSEG_START		MIPS_REGFILE_START
#define MIPS_DATASEG_START		(MIPS_REGFILE_START + CTRLSEG_LEN)

#define MCU_REGFILE_START		0xf5000030
#define MCU_CTRLSEG_START		MCU_REGFILE_START
#define MCU_DATASEG_START		(MCU_REGFILE_START + CTRLSEG_LEN)

#define MAX_PACKAGE_COUNT		(1<<4)	// 4 bit
#define MCU_COMM_BUFCOUNT_MAX		210	/* (15 x 14) */

typedef enum _tag_mcu_cmd
{
	MCU_SET_MIPS_POWER_OFF	= 0x01,
	MCU_SET_MIPS_RESET	= 0x27,
}mcu_cmd_t;

typedef struct _tag_mcu_comm_param{
	unsigned int buf_len;
	unsigned char buffer[MCU_COMM_BUFCOUNT_MAX];
}mcu_comm_param_t;

static struct mcu_comm_driver {
	int irq_resp;
#define MCOMM_FLAG_RUNNING	(1)
	unsigned flags;
	unsigned try, out;
#define MCOMM_RETRY_NUM		5
	unsigned retry;
	unsigned last_cmd;
#define MCOMM_STATE_NONE	(-1)
#define MCOMM_STATE_OK		0	/*succeed*/
#define MCOMM_STATE_TIMEOUT	1	/*timeout*/
#define MCOMM_STATE_INT		2	/*interrupted*/
	unsigned last_state;
	int users;
	struct semaphore lock;
	struct spinlock spin;		/*spinlock to handle mailbox concurrency*/
#define MCOMM_TIMEOUT_MS	1000	/*timout 1s*/
	struct completion finish;
	struct {
#define MCOMM_DEBUG_NCMDS	16
		int idx;
		unsigned total;
		unsigned cmds[MCOMM_DEBUG_NCMDS][2];
	} debug;
} *pmcomm_drv = NULL;

static int mcomm_drv_get(struct mcu_comm_driver *drv)
{
	int ret = down_interruptible(&drv->lock);
	if (0 == ret)
		drv->users++;
	return ret;
}

static int mcomm_drv_put(struct mcu_comm_driver *drv)
{
	up(&drv->lock);
	drv->users--;
	return 0;
}

static int mcomm_debug_push_cmd(struct mcu_comm_driver *drv, unsigned cmd, unsigned len)
{
	if (drv->debug.idx < 0 || drv->debug.idx > MCOMM_DEBUG_NCMDS - 1)
		drv->debug.idx = MCOMM_DEBUG_NCMDS - 1;

	drv->debug.total++;
	if (++drv->debug.idx > MCOMM_DEBUG_NCMDS - 1)
		drv->debug.idx = 0;
	drv->debug.cmds[drv->debug.idx][0] = cmd;
	drv->debug.cmds[drv->debug.idx][1] = len;
	return 0;
}

static void mips_request_mcu(struct spinlock *spin, unsigned char len)
{
	unsigned long flags;
	unsigned char temp;

	spin_lock_irqsave(spin, flags);
	/* fill M1 */
	temp = readb((volatile void*)(MIPS_CTRLSEG_START+1));
	temp &= ~(CMD_CTRL_C_MASK | CMD_CTRL_LEN_MASK);
	temp |= (len & CMD_CTRL_LEN_MASK);	/*set len*/
	/* M1.C = !E1.R */
	if (!(readb((volatile void*)(MCU_CTRLSEG_START+1)) & CMD_CTRL_R_MASK))
		temp |= CMD_CTRL_C_MASK;
	/* M1.F = 1 */
	temp |= CMD_CTRL_F_MASK;
	pr_debug("request_mcu, M1/E1 %x/%x\n", temp, readb((volatile void*)(MCU_CTRLSEG_START+1)));
	writeb( temp, (volatile void*)(MIPS_CTRLSEG_START+1));
	dmb();

	/* toggle INTR bit */
	temp = readb((volatile void*)MIPS_COMM_BASEREG);
	temp ^= CMD_MIPS_INTR_MCU_MASK;	//reverse INTR bit
	writeb( temp, (volatile void*)MIPS_COMM_BASEREG );
	spin_unlock_irqrestore(spin, flags);
}

static void mips_response_mcu(struct spinlock *spin)
{
	unsigned long flags;
	unsigned char temp;

	spin_lock_irqsave(spin, flags);
	/* deal with M1.R
	 * M1.R = E1.C
	 */
	temp = readb((volatile void*)(MIPS_CTRLSEG_START+1));
	if (readb((volatile void*)(MCU_CTRLSEG_START+1)) & CMD_CTRL_C_MASK)
		temp |= CMD_CTRL_R_MASK;
	else
		temp &= ~CMD_CTRL_R_MASK;
	temp |= CMD_CTRL_F_MASK;
	pr_debug("response_mcu, M1/E1 %x/%x\n", temp, readb((volatile void*)(MCU_CTRLSEG_START+1)));
	writeb(temp, (volatile void*)(MIPS_CTRLSEG_START+1));
	dmb();

	/* toggle REST bit */
	temp = readb((volatile void*)MIPS_COMM_BASEREG);
	temp ^= CMD_MIPS_RES_MCU_MASK;	//reverse RESP bit
	writeb( temp, (volatile void*)MIPS_COMM_BASEREG );
	spin_unlock_irqrestore(spin, flags);
}

static int protocol_wrap_puart(void *b1,void *b2,int len)
{
	unsigned char *buf1=b1;
	unsigned char *buf2=b2;
	int  i;

	buf1[0] = 0xff;
	buf1[len+2] = 0xfe;

	memcpy( buf1+1, buf2, len);

	buf1[len+1] = 0;

	for( i=0; i < len; i++ ) 
		buf1[len+1] += buf2[i];

	buf1[len+1] = (unsigned char)0 - buf1[len+1];

	return len+3;
}

/*
 * return value
 * 	0	- ok
 * 	-EINVAL	- invalid parameter
 * 	-ETIME	- timeout
 * 	-EINTR	- interrupted by signal
 */
static int send_one_frame(mcu_comm_param_t * param)
{
	int ret;
	unsigned char total, stamp, len, temp;
	unsigned int i;

	/*init value*/
	total = (param->buf_len + DATASEG_LEN - 1) / DATASEG_LEN;
	if ( total > MAX_PACKAGE_COUNT-1 ) {
		pr_err("frame data (%d) exceeds %d bytes!\n", param->buf_len, MCU_COMM_BUFCOUNT_MAX);
		return -EINVAL;
	}

	stamp = 1;

	BUG_ON(pmcomm_drv == NULL);
	if((ret = mcomm_drv_get(pmcomm_drv)) != 0) {
		pr_err("mcomm_drv_get failed ret %d\n", ret);
		return ret;
	}

	/*collect debug information*/
	pmcomm_drv->retry = MCOMM_RETRY_NUM;
	pmcomm_drv->try = pmcomm_drv->out = 0;
	pmcomm_drv->last_cmd = param->buffer[1];	/*cmd id*/
	pmcomm_drv->last_state = MCOMM_STATE_NONE;
	mcomm_debug_push_cmd(pmcomm_drv, param->buffer[1], param->buf_len);
	pr_debug("sending mcu cmd %x len %d\n", pmcomm_drv->last_cmd, param->buf_len);
	while( stamp <= total )
	{
		//fill in Control Segment except M1
		//M1 shall be filled inside mips_request_mcu for smp safe.
		temp = (total<<4) + stamp;

		writeb( temp, (volatile void*)MIPS_CTRLSEG_START );

		len = ( min((int)(param->buf_len - (stamp-1)*DATASEG_LEN), DATASEG_LEN)) & CMD_CTRL_LEN_MASK;

		//fill in Data Segment
		for( i=0; i < len; i++ )
		{
			temp = param->buffer[(stamp-1)*DATASEG_LEN + i];
			writeb( temp, (volatile void*)(MIPS_DATASEG_START+i) );
		}

		pmcomm_drv->flags |= MCOMM_FLAG_RUNNING;
		dmb();
		mips_request_mcu(&pmcomm_drv->spin, len);

		/*wait for mcu acknowledge*/
		pmcomm_drv->try = stamp * DATASEG_LEN;
		ret = wait_for_completion_timeout(&pmcomm_drv->finish, 
					msecs_to_jiffies(MCOMM_TIMEOUT_MS));
		if (ret == 0) {
			//timeout
			if (pmcomm_drv->retry-- > 0) {
				pr_warn("sending mcu cmd %x timeout -%d-\n", pmcomm_drv->last_cmd, pmcomm_drv->retry);
				continue;
			} else {
				pmcomm_drv->last_state = MCOMM_STATE_TIMEOUT;
				ret = -ETIME;
				break;
			}
		} else if (ret < 0) {
			//interrupted
			pr_warn("interrupt by signal, %d of %d bytes are sent for mcu cmd %x\n", pmcomm_drv->out, param->buf_len, pmcomm_drv->last_cmd);
			pmcomm_drv->last_state = MCOMM_STATE_INT;
			ret = -EINTR;
		}

		pmcomm_drv->out = pmcomm_drv->try;
		stamp ++;
	}

	if (stamp > total) {
		pmcomm_drv->last_state = MCOMM_STATE_OK;
		ret = 0;
	}

	pr_debug("finish mcu cmd %x, ret %d\n", pmcomm_drv->last_cmd, ret);
	mcomm_drv_put(pmcomm_drv);
	return ret;
}

static irqreturn_t mcomm_resp_irq(unsigned int irq, void *dev, struct pt_regs *reg)
{
	struct mcu_comm_driver *drv = (struct mcu_comm_driver*)dev;
	if (drv && (drv->flags & MCOMM_FLAG_RUNNING)){
		pr_debug("mcu reponses cmd %x\n", drv->last_cmd);
		drv->flags &= ~MCOMM_FLAG_RUNNING;
		dmb();
		complete(&drv->finish);
	} else {
		pr_warn("fake mcu resp interrut!\n");
	}
	return IRQ_HANDLED;
}

static int mcu_send_cmd( mcu_cmd_t action )
{
	int len;
	unsigned char msg[5];
	unsigned char tmpbuf[32];

	mcu_comm_param_t mcu_comm_param;

	memset(msg, 0, sizeof(msg));

	msg[0] = action;

	len = protocol_wrap_puart(tmpbuf, msg, sizeof(msg));

	mcu_comm_param.buf_len = len;

	memcpy(mcu_comm_param.buffer, tmpbuf, len);

	return send_one_frame( &mcu_comm_param );
}

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_mcomm = NULL;
static int mcomm_debug_show(struct seq_file *m, void *v)
{
	int i, j, start;
	struct mcu_comm_driver *drv = pmcomm_drv;
	BUG_ON(drv == NULL);
	seq_printf(m, "mcomm last cmd %x state %08x flow %d/%d retry %d users %d\n", 
		drv->last_cmd, drv->last_state, drv->try, drv->out, drv->retry, drv->users);
	seq_printf(m, "----------------------------------------------------\n");
	seq_printf(m, "mcomm cmd details: total %d\n|<-- ", drv->debug.total);
	for (i = 0; i < MCOMM_DEBUG_NCMDS; i++) {
		seq_printf(m, "%3d ", i + 1);
	}
	seq_printf(m, "\ncmd: ");
	start = drv->debug.idx;
	//cmd
	for (i = 0, j = start; i < MCOMM_DEBUG_NCMDS; i++) {
		seq_printf(m, "%3x ", drv->debug.cmds[j][0]);
		if (--j < 0)
			j = MCOMM_DEBUG_NCMDS - 1;
	}
	seq_printf(m, "\nlen: ");
	//len
	for (i = 0, j = start; i < MCOMM_DEBUG_NCMDS; i++) {
		seq_printf(m, "%3x ", drv->debug.cmds[j][1]);
		if (--j < 0)
			j = MCOMM_DEBUG_NCMDS - 1;
	}
	seq_printf(m, "\n");
	return 0;
}

static int mcomm_debug_open(struct inode *inode, struct file *file)
{
        return single_open(file, mcomm_debug_show, NULL);
}

static const struct file_operations mcomm_debug_ops = {
        .open           = mcomm_debug_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

void  trix_mcomm_send_reboot( void )
{
	int retry = 10;
	do {
		int ret = mcu_send_cmd(MCU_SET_MIPS_RESET);
		if (ret == 0) {
			break;
		} else {
			pr_warn("send mcu reboot fail %d, retry %d...\n", ret, retry);
		}
	} while(retry-- > 0);
}

void trix_mcomm_send_poweroff( void )
{
	int retry = 10;
	do {
		int ret = mcu_send_cmd(MCU_SET_MIPS_POWER_OFF);
		if (ret == 0) {
			break;
		} else {
			pr_warn("send mcu poweroff fail %d, retry %d...\n", ret, retry);
		}

	} while(retry-- > 0);
}

/*
 * @brief it's a wrapper used to send frame data to MCU side, i.e. for mcu_comm driver use. Note that standby response interrupt is servered inside linux kernel.
 * @param[in]	data	pointer of mcu frame data
 * @param[in]	len	bytes of data
 *
 * @return
 * 	0 	- on success
 * 	<0	- error code, could be one of
 * 		EINVAL	- invalid paramters
 * 		ETIME	- timeout
 * 		EINTR	- interrupted by signal
 */
int trix_mcomm_send_data(void *data, unsigned int len)
{
	int ret = 0;
	mcu_comm_param_t param;

	if (data == NULL)
		return -EINVAL;

	memset(&param, 0, sizeof(mcu_comm_param_t));
	param.buf_len = len;
	memcpy(param.buffer, data, min_t(int, len, MCU_COMM_BUFCOUNT_MAX));
	ret = send_one_frame(&param);
	pr_debug("mcomm_send_data for cmd %x %s, ret %d\n", 
		*(u8*)(data+1), (ret==0)?"OK":"FAIL", ret);
	return ret;
}
EXPORT_SYMBOL(trix_mcomm_send_data);

/*
 * @brief it's a wrapper used to send response to MCU side. For mcu_comm driver use only. SMP awared.
 * @param[in]	param	pointer to user param (N/A for now)
 *
 * @return
 * 	0 	- on success
 * 	<0	- error code
 */
int trix_mcomm_response_mcu(void *param)
{
	BUG_ON(pmcomm_drv == NULL);
	pr_debug("mcomm_response_mcu\n");
	mips_response_mcu(&pmcomm_drv->spin);
	return 0;
}
EXPORT_SYMBOL(trix_mcomm_response_mcu);

static int __init trix_mcomm_init(void)
{
	int retval = 0;

	if ((pmcomm_drv = kzalloc(sizeof(struct mcu_comm_driver), GFP_KERNEL)) == NULL) {
		pr_err("failed to malloc memory for mcomm driver!\n");
		return -ENOMEM;
	}

	pmcomm_drv->irq_resp = TRIHIDTV_STANDBY_CPU_RESPONSE_INTERRUPT;
	pmcomm_drv->users = 0;
	sema_init(&pmcomm_drv->lock, 1);
	spin_lock_init(&pmcomm_drv->spin);
	init_completion(&pmcomm_drv->finish);
	/*
	 * register IRQs for MCU reponse on A9
	 */
	retval = request_irq(pmcomm_drv->irq_resp,(void *)mcomm_resp_irq, IRQF_SHARED | IRQF_TRIGGER_RISING, "mcomm_resp", (void*)pmcomm_drv);
	pr_debug("<1> register standby response IRQ (%d).\n", retval);

#ifdef CONFIG_PROC_FS
	proc_mcomm = proc_mkdir("mcomm", NULL);
	BUG_ON(proc_mcomm == NULL);
        proc_create("debug", 0, proc_mcomm, &mcomm_debug_ops);
#endif
	return 0;
}

static void __exit trix_mcomm_exit(void)
{
	if (pmcomm_drv) {
		if (pmcomm_drv->users == 0) {
#ifdef CONFIG_PROC_FS
			proc_remove(proc_mcomm);
			proc_mcomm = NULL;
#endif
			free_irq(pmcomm_drv->irq_resp, NULL);
			kfree(pmcomm_drv);
			pmcomm_drv = NULL;
		} else {
			pr_warn("failed, mcomm is still hold by someone!\n");
		}
	}
}

MODULE_DESCRIPTION("MCU comm driver for "CONFIG_TRIX_SOC_NAME" SoCs."
"Note that only one direction communication is implemented here:\n"
"  A9 -> MCU");
MODULE_LICENSE("GPL");
module_init(trix_mcomm_init);
module_exit(trix_mcomm_exit);
