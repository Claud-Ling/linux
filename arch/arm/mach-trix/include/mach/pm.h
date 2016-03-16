#ifndef __ARCH_ARM_TRIX_MACH_PM_H__
#define __ARCH_ARM_TRIX_MACH_PM_H__

#ifdef CONFIG_PM
#include <linux/suspend.h>

/*
 * get pm suspend state (during suspend callback)
 * return value mighe be one of:
 * PM_SUSPEND_ON	- ON state
 * PM_SUSPEND_FREEZE	- Low-Power Idle
 * PM_SUSPEND_STANDBY	- Power-On Suspend
 * PM_SUSPEND_MEM	- Suspend-to-RAM
 */
suspend_state_t trix_get_suspend_state(void);

#endif

#endif
