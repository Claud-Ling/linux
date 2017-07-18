#ifndef MMC_QUEUE_H
#define MMC_QUEUE_H

struct request;
struct task_struct;

struct mmc_blk_request {
	struct mmc_request	mrq;
	struct mmc_command	sbc;
	struct mmc_command	cmd;
	struct mmc_command	stop;
	struct mmc_data		data;
};

struct mmc_queue_req {
	struct request		*req;
	struct mmc_blk_request	brq;
	struct scatterlist	*sg;
	char			*bounce_buf;
	struct scatterlist	*bounce_sg;
	unsigned int		bounce_sg_len;
	struct mmc_async_req	mmc_active;
};

struct mmc_queue {
	struct mmc_card		*card;
	struct task_struct	*thread;
	struct semaphore	thread_sem;
	unsigned int		flags;
	int			(*issue_fn)(struct mmc_queue *, struct request *);
	void			*data;
	struct request_queue	*queue;
	struct mmc_queue_req	mqrq[2];
	struct mmc_queue_req	*mqrq_cur;
	struct mmc_queue_req	*mqrq_prev;
};

/*
 *  *  * There is one mmc_blk_data per slot.
 *   *   */
struct mmc_blk_data {
        spinlock_t      lock;
        struct gendisk  *disk;
        struct mmc_queue queue;
        struct list_head part;

        unsigned int    flags;
#define MMC_BLK_CMD23   (1 << 0)        /* Can do SET_BLOCK_COUNT for multiblock */
#define MMC_BLK_REL_WR  (1 << 1)        /* MMC Reliable write support */

        unsigned int    usage;
        unsigned int    read_only;
        unsigned int    part_type;
        unsigned int    name_idx;
        unsigned int    reset_done;
#define MMC_BLK_READ            BIT(0)
#define MMC_BLK_WRITE           BIT(1)
#define MMC_BLK_DISCARD         BIT(2)
#define MMC_BLK_SECDISCARD      BIT(3)

        /*
	 * Only set in main mmc_blk_data associated
         * with mmc_card with mmc_set_drvdata, and keeps
         * track of the current selected device partition.
         */
        unsigned int    part_curr;
        struct device_attribute force_ro;
        struct device_attribute power_ro_lock;
        int     area_type;
};

#endif
