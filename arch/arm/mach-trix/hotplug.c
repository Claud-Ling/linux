/*
 *  linux/arch/arm/trix/hotplug.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SoC hotplug Support
 * Author: Tony He, 2014.
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>

#include <asm/smp_scu.h>
#include <asm/cacheflush.h>
#include <asm/cp15.h>
#include <asm/smp_plat.h>
#ifdef CONFIG_PM
#include <asm/suspend.h>
#endif
#include "common.h"

#ifdef CONFIG_PM
static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
	for (;;) {
		/*Zzzzzz*/
		trix_pm_suspend_core(cpu);
		/*Oooooo*/
		if (pen_release == cpu_logical_map(cpu)) {
		/*OK, we're done*/
			break;
		}

		/*
		 * Getting here, means that we have come out of WFI without
		 * having been woken up - this shouldn't happen
		 *
		 * Just note it happening - when we're woken, we can report
		 * its occurrence.
		 */
		(*spurious)++;

		//pr_info("CPU%d: spurious call!\n", smp_processor_id());
	}
}

#else
static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
	/*
	 * we're ready for shutdown now, so do it
	 */
	cpu_enter_lowpower();

	/*
	 * there is no power-control hardware on this platform, so all
	 * we can do is put the core into WFI; this is safe as the calling
	 * code will have already disabled interrupts
	 */
	for (;;) {
		/*
		 * here's the WFI
		 */
		asm(".word	0xe320f003\n"
		    :
		    :
		    : "memory", "cc");

		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}

		/*
		 * Getting here, means that we have come out of WFI without
		 * having been woken up - this shouldn't happen
		 *
		 * Just note it happening - when we're woken, we can report
		 * its occurrence.
		 */
		(*spurious)++;
	}

	/*
	 * bring this CPU back into the world of cache
	 * coherency, and then restore interrupts
	 */
	cpu_leave_lowpower();
}
#endif

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void __ref trix_cpu_die(unsigned int cpu)
{
	int spurious = 0;

	platform_do_lowpower(cpu, &spurious);
	if (spurious)
		pr_warn("CPU%u: %u spurious wakeup calls\n", cpu, spurious);
}

