#ifndef __SX6_COMMON_H__
#define __SX6_COMMON_H__

#include <linux/kernel.h>

#ifdef CONFIG_SIGMA_SX6_SMC
#include "smc.h"
#endif

#ifndef __ASSEMBLY__

extern void sx6_map_io(void);
extern void mcu_send_rest(void);
extern void __init sx6_init_irq(void);
extern void __init sx6_timer_init(void);

#ifdef CONFIG_LOCAL_TIMERS
extern int __cpuinit sx6_local_timer_setup(void);
#endif

#ifdef CONFIG_HOTPLUG_CPU
extern void __ref sx6_cpu_die(unsigned int cpu);
#endif

#ifdef CONFIG_SMP
#include <asm/smp.h>
extern volatile int pen_release;
extern struct smp_operations sx6_smp_ops;

extern void sigma_secondary_startup(void);

#ifdef CONFIG_PM
extern void platform_smp_resume_cpus(void);
#endif /*CONFIG_PM*/

#endif /*CONFIG_SMP*/

#endif /*__ASSEMBLY__*/
#endif
