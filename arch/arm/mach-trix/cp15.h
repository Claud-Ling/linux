#ifndef __ASM_ARM_MACH_TRIX_CP15_H__
#define __ASM_ARM_MACH_TRIX_CP15_H__

#include <linux/bitops.h>
#include <asm/cp15.h>

/*ACTLR bits*/
#define AUXCR_SMP	BIT(6)

/*NSACR bits*/
#define NSACR_NS_SMP	BIT(18)

#ifndef __ASSEMBLY__

static inline unsigned int get_nsacr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c1, 2	@ get NSACR" : "=r" (val));
	return val;
}

#endif /*__ASSEMBLY__*/

#endif
