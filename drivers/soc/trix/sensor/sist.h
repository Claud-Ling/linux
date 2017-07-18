#ifndef __ASM_ARCH_TRIX_MACH_SIST_H__
#define __ASM_ARCH_TRIX_MACH_SIST_H__



#define T_MIN_IDX	(0)
#define T_MAX_IDX	(41)



#define REQ_TO_SIST_MODE(rq) ({			\
	int __mode__ = 0;			\
	__mode__ = SIST_BYPASS_MODE + (rq);	\
	__mode__;				\
})

#define IS_VALID_SIST_RQ(rq) ({			\
	((rq >= SIST_RQ_BYPASS)			\
		&& (rq < SIST_RQ_INVALID));	\
})

enum {
	SIST_RQ_BYPASS = 0,
	SIST_RQ_NOISE,
	SIST_RQ_TOUT,
	SIST_RQ_BRINGO,
	SIST_RQ_PNMONITOR,
	SIST_RQ_FRINGOLL,
	SIST_RQ_FRINGOLS,
	SIST_RQ_SPSENSOR,
	SIST_RQ_INVALID
};

#define SIST_MODE_MASK				(0x7)
#define SIST_BYPASS_MODE			(0x0)
#define SIST_NOISE_OUT_MODE			(0x1)
#define SIST_TOUT_MODE				(0x2)
#define SIST_BRINGO_MODE			(0x3)
#define SIST_PNMONITOR_MODE			(0x4)
#define SIST_FRINGOLL_MODE			(0x5)
#define SIST_FRINGOLS_MODE			(0x6)
#define SIST_SPSENSOR_MODE			(0x7)


struct sist_bus {
	struct list_head sensors;
	spinlock_t lock;
	int max_sensors;
	struct proc_dir_entry *proc_sist_entry;
};

struct sist_sensor {
	struct list_head list;
	int id;
	const char *name;
	unsigned int ctl_shift;
	volatile void *base;
	struct sensor_ops *ops;
	struct sist_bus *sist;
};
#define SENSOR_TO_SIST(s) ({		\
	(struct sist_bus *)((s)->sist);	\
})

struct sensor_ops {
	int(*get_value)(struct sist_sensor *sensor, int request);
	int(*set_mode)(struct sist_sensor *sensor, int mode);
};


extern int send_sist_request(int sensor_id, int request);
extern int sist_get_sensor_count(void);
extern const char *sist_get_sensor_name(int sensor_id);
extern struct sist_sensor *sist_alloc_sensor(void);
extern int sist_register_sensor(struct sist_sensor *new);


#endif /*__ASM_ARCH_TRIX_MACH_SIST_H__*/
