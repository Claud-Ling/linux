/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */

#ifndef __ARCH_SIGMA_DTV_MCOMM_H__
#define __ARCH_SIGMA_DTV_MCOMM_H__

/**
 * struct mcomm_ops - represents the various operations provided
 *                    by MCU through mcu_comm protocol
 * @sys_reboot:     system reboot
 * @sys_poweroff:   system power off
 *
 */
struct mcomm_ops {
	void (*sys_reboot)(void);
	void (*sys_poweroff)(void);
};

#if IS_REACHABLE(CONFIG_TRIX_MCU_COMM) && !defined(MODULE)
/**
 * it's not allowed to invoke mcomm operations out of built-in
 */
extern struct mcomm_ops *get_mcomm_ops(void);
#else
static inline struct mcomm_ops *get_mcomm_ops(void) { return NULL; }
#endif /* IS_REACHABLE(CONFIG_TRIX_MCU_COMM) && !defined(MODULE) */

#if IS_REACHABLE(CONFIG_TRIX_MCU_COMM)
/*
 * @brief it's a wrapper used to send frame data to MCU side, i.e. for mcu_comm driver use. This is a synchronous call. Note that standby response interrupt is servered inside linux kernel.
 * @param[in]	data	pointer of mcu frame data
 * @param[in]	len	bytes of data
 *
 * @return
 * 	0 	- on success
 * 	<0	- error code, could be one of
 * 		EINVAL	- invalid paramters
 * 		ETIME	- timeout
 * 		EINTR	- interrupted by signal
 */
extern int trix_mcomm_send_data(void *data, unsigned int len);

/*
 * @brief it's a wrapper used to send response to MCU side. For mcu_comm driver use only. SMP awared.
 * @param[in]	param	pointer to user param (N/A for now)
 *
 * @return
 * 	0 	- on success
 * 	<0	- error code
 */
extern int trix_mcomm_response_mcu(void *param);
#else
static inline int trix_mcomm_send_data(void *data, unsigned int len) { return 0;}

static inline int trix_mcomm_response_mcu(void *param) { return 0; }
#endif /* IS_REACHABLE(CONFIG_TRIX_MCU_COMM) */

#endif /*__ARCH_SIGMA_DTV_MCOMM_H__*/
