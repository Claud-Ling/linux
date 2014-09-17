
/*********************************************************************
 Copyright (C) 2001-2007
 Sigma Designs, Inc. 

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.
 *********************************************************************/

#ifndef __DTV_SMC_H__
#define __DTV_SMC_H__

#ifndef __ASSEMBLY__
/* armor api */
extern void sigma_dtv_smc1( int code, int arg);

/* smc wrapper api */
void sx6_smc_l2x0_enable(void);
void sx6_smc_l2x0_disable(void);
void sx6_smc_l2x0_set_auxctrl(unsigned long val);
void sx6_smc_l2x0_set_debug(unsigned long val);
void sx6_smc_l2x0_set_prefetchctrl(unsigned long val);
void sx6_smc_set_actlr(unsigned long val);
#endif

#define ARMOR_SMCCALL_WRITE            2
#define ARMOR_SMCCALL_READ             3
#define ARMOR_SMCCALL_SHELL            4
#define ARMOR_SMCCALL_SET_L2_CONTROL    0x102
#define ARMOR_SMCCALL_GET_AUX_CORE_BOOT 0x103
#define ARMOR_SMCCALL_SET_AUX_CORE_BOOT0 0x104
#define ARMOR_SMCCALL_SET_AUX_CORE_BOOT_ADDR 0x105
#define ARMOR_SMCCALL_SET_L2_AUX_DEBUG 0x106
#define ARMOR_SMCCALL_SET_L2_PREFETCH_CTRL  0x107
#define ARMOR_SMCCALL_SET_L2_AUX_CONTROL    0x109

#endif // __DTV_SMC_H__

