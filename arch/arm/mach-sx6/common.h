#ifndef __TRIX_COMMON_H__
#define __TRIX_COMMON_H__

#include <linux/kernel.h>

#ifdef CONFIG_SIGMA_SMC
#include "smc.h"
#endif

#ifndef __ASSEMBLY__

struct trix_board_ops{
	int (*set_dbg_swen)(unsigned int cpu, bool state);
	u32 (*make_l2x_auxctrl)(void);
};

struct trix_board_object{
	const char *name;
	struct trix_board_ops ops;
};

extern void trix_map_io(void);
extern void mcu_send_rest(void);
extern void __init trix_init_irq(void);
extern void __init trix_timer_init(void);

#ifdef CONFIG_LOCAL_TIMERS
extern int __cpuinit trix_local_timer_setup(void);
#endif

#ifdef CONFIG_HOTPLUG_CPU
extern void __ref trix_cpu_die(unsigned int cpu);
#endif

#ifdef CONFIG_SMP
#include <asm/smp.h>
extern volatile int pen_release;
extern struct smp_operations trix_smp_ops;

extern void sigma_secondary_startup(void);

#ifdef CONFIG_PM
extern void platform_smp_resume_cpus(void);
#endif /*CONFIG_PM*/

#endif /*CONFIG_SMP*/

//board specific methods
extern const char* trix_board_name(void);
extern int trix_set_dbg_swen(unsigned int cpu, bool state);
extern u32 trix_make_l2x_auxctrl(void);

#endif /*__ASSEMBLY__*/
#endif
