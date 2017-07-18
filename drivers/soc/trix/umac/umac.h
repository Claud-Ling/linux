/*
 * Copyright  2016
 * Sigma Designs, Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * This file is to define register files to describe
 * UMAC_IP_2034 and UMAC_IP_2037 of SX6/SX7/UXLB/SX8 onwards
 * DTV SoC chipsets.
 *
 * The host can override PMAN_IP_1820_CON base address by
 * defining CONFIG_REG_BASE_PMAN_CON.
 *
 * Author: Tony He <tony_he@sigmadesigns.com>
 * Date:   05/16/2016
 */

#include <linux/kernel.h>
#include <umac_dev.h>
#ifndef __ASM_ARCH_TRIX_MACH_UMAC_H__
#define __ASM_ARCH_TRIX_MACH_UMAC_H__

/*
 * PCTL CMD/STATE
 */
#define PCTL_CMD_INIT			0x0
#define PCTL_CMD_CFG			0x1
#define PCTL_CMD_GO			0x2
#define PCTL_CMD_SLEEP			0x3
#define PCTL_CMD_WAKEUP			0x4

#define PCTL_STAT_INIT			0x0
#define PCTL_STAT_CONFIG		0x1
#define PCTL_STAT_CONFIG_REQ		0x2
#define PCTL_STAT_ACCESS		0x3
#define PCTL_STAT_ACCESS_REQ		0x4
#define PCTL_STAT_LOWPOWER		0x5
#define PCTL_STAT_LOWPOWER_ENTRY_REQ	0x6
#define PCTL_STAT_LOWPOWER_EXIT_REQ	0x7

/*
 * UMAC width
 */
#define UMAC_DATA_WIDTH16		16
#define UMAC_DATA_WIDTH32		32

#ifndef __ASSEMBLY__

#ifdef NW
#undef NW
#endif
#define NW(s,e) (((e)-(s))>>2)	/*Number of Words*/

#ifndef NULL
# define NULL ((void *)0)
#endif


#define MAX_NR_UMACS	(3)
#define SIGMA_NR_UMACS	(get_nr_umacs())


/*umac device structure*/
struct umac_device {
	struct umac_ctl		*ctl;		/*UMAC controller include PHY controller */
	struct pman_con		*pman;		/*pman base*/
	u32			width;		/*data width (number of bits)*/
	u32			granule;	/* Granule of 'msize' value, msize be passed by kernel parameter 'msize=XXX'
						 * This value can be specified via DTB, default granule is 256 unit is 'MB';
						 */
	struct device		*dev;
	u32			id;
};

extern struct umac_device *trix_udev_tbl[MAX_NR_UMACS];

/**
 * @brief	get umac_device object for the specified umac id.
 * @fn		const struct umac_device* umac_get_dev_by_id(int uid);
 * @param[in]	uid	umac id (0,1,...)
 * @retval	object	on success
 * @retval	NULL	fail
 */
extern struct umac_device* umac_get_dev_by_id(int uid);

/**
 * @brief	check if specified umac is activated or not.
 * @fn		int umac_is_activated(int uid);
 * @param[in]	uid	umac id (0,1,...)
 * @retval	TRUE	activated
 * @retval	FALSE	inactivated
 */
extern int umac_is_activated(int uid);

/**
 * @brief	get the base address of specified umac.
 * @fn		const U32 umac_get_addr(int uid);
 * @param[in]	uid	umac id (0,1,...)
 * @retval	addr	base address
 */
extern const u32 umac_get_addr(int uid);

/**
 * @brief	get number of UMACS avaiable
 * @fn		int get_nr_umacs(void);
 * @retval	nr	number of UMACS
 */
extern int get_nr_umacs(void);



#endif /*__ASSEMBLY__*/

#endif /*__ASM_ARCH_TRIX_MACH_UMAC_H__*/
