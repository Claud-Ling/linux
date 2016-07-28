#ifndef __ASM_ARCH_TRIX_MACH_SIST_H__
#define __ASM_ARCH_TRIX_MACH_SIST_H__

#define PROC_MON_SEL_REG			(0x19b00070)
#define PROC_MON_SEL_CTR_MASK			(0x00000001)
#define PROC_MON_SEL_CTR_SHIFT			(0x0)
#define PROC_MON_SEL_CTR_ENABLE			(1UL)
#define PROC_MON_SEL_CTR_DISABLE		(0UL)

#define PROC_MON_PM_COUNT0			(0x19b00080)
#define PROC_MON_PM_COUNT1			(0x19b00084)
#define PROC_MON_PM_SEL				(0x19b00088)
#define PROC_MON_PM_COUNT_MASK			(0x00ffffff)

#define PROC_MON_TOUT_REG			(0x19b0008c)
#define PROC_MON_TOUT_MASK			(0x00000001)

#define GLB_SIST_CTL_REG			(0x19b0007c)
#define GLB_SIST_MODE_MASK			(0xffffff)

#define GLB_TEMP_CTR_REG			(0x19b00074)
#define GLB_TEMP_CTR_SHIFT			16
#define GLB_TEMP_CTR_MASK			(0x003f0000)

#define A9_SIST_CTL_REG				(0x1500ef17)
#define A9_SIST_MODE_MASK			(0x07)
#define A9_SIST_MODE_SHIFT			(0)

#define A9_TEMP_CTL_REG				(0x1500ef16)
#define A9_TEMP_CTL_MASK			(0x7f)
#define A9_TEMP_CTL_SHIFT			(0)

#define T_MIN_IDX	(0)
#define T_MAX_IDX	(41)

enum {	
	SIST_ID_UMAC0 = 0,
	SIST_ID_GFX,
	SIST_ID_UMAC2,
#if defined(CONFIG_SIGMA_SOC_SX8)
	SIST_ID_UMAC1,
	SIST_ID_PMAN,
	SIST_ID_FRCB_B,
	SIST_ID_FRCB_A,
#else
	SIST_ID_VEDR,
	SIST_ID_VDETN,
#endif
	SIST_ID_ARM,
	SIST_ID_MAX
};

#define IS_VALID_SIST_ID(id) ({		\
	(((id) >= SIST_ID_UMAC0) &&	\
		((id) < SIST_ID_MAX));	\
})

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
	char *name;
	void *ctl_reg;
	int ctl_shift;
	void *temp_reg;
	struct sensor_ops *ops;
	struct sist_bus *sist;
};
#define SENSOR_TO_SIST(s) ({		\
	(struct sist_bus *)((s)->sist);	\
})

#define SIST_NODE(name, ctl, shift, temp)		\
	{SIST_ID_##name, #name, (void *)ctl, shift, (void *)temp}

struct sist_node {
	int id;
	char *name;
	void *ctl_reg;
	int ctl_shift;
	void *temp_reg;
};


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
