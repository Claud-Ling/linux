#ifndef __TRIX_COMMON_H__
#define __TRIX_COMMON_H__

#include <linux/kernel.h>
#include "tri_dbg.h"

#ifdef CONFIG_TRIX_SMC
#include "smc.h"
#endif

#ifdef CONFIG_PM
#include "pm.h"
#endif

#ifndef __ASSEMBLY__

struct trix_board_ops{
	int (*set_dbg_swen)(unsigned int cpu, bool state);
	int (*make_l2x_auxctrl)(u32* val, u32* mask);
};

struct trix_board_object{
	const char *name;
	struct trix_board_ops ops;
};

extern void trix_map_io(void);
extern void trix_mcomm_send_reboot(void);
extern void trix_mcomm_send_poweroff(void);
extern void __init trix_init_irq(void);
extern void __init trix_init_timer(void);

extern void __init trix_timer_init(void __iomem *base,
				   unsigned int src_id, unsigned int evt_id,
				   unsigned int rate, unsigned int irq);

#ifdef CONFIG_HOTPLUG_CPU
extern void __ref trix_cpu_die(unsigned int cpu);
#endif

#ifdef CONFIG_SMP
#include <asm/smp.h>
extern volatile int pen_release;
extern struct smp_operations trix_smp_ops;

extern void sigma_secondary_startup(void);
extern void __iomem *platform_get_scu_base(void);
#ifdef CONFIG_PM
extern void platform_smp_resume_cpus(void);
#endif /*CONFIG_PM*/

#endif /*CONFIG_SMP*/

//board specific methods
extern const char* trix_board_name(void);
extern int trix_set_dbg_swen(unsigned int cpu, bool state);
extern int trix_make_l2x_auxctrl(u32* val, u32* mask);

//power control
extern void cpu_enter_lowpower(void);
extern void cpu_leave_lowpower(void);

#ifdef CONFIG_PM
/*
 * pm specific implementation for cpu hotplug
 */
extern int trix_pm_suspend_core(unsigned int cpu);
#endif

#endif /*__ASSEMBLY__*/
#endif
