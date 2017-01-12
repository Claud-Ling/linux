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

#ifndef __ARCH_SIGMA_DTV_SECURITY_H__
#define __ARCH_SIGMA_DTV_SECURITY_H__

#include <linux/types.h>
#include <linux/bug.h>

#ifndef __ASSEMBLY__

/*
 * struct secure_fw_ops -
 *    represents the various operations provided by secure world
 *
 *    A filled up structure can be registered with trix_secure_firmware_register().
 */
struct secure_fw_ops {
	/*
	 * the present security state of ARM core
	 */
	int (*get_security_state)(void);
	/*
	 * secure read one register
	 */
	int (*read_reg)(uint32_t, uint32_t, uint32_t *);
	/*
	 *  secure write one register, supports mask
	 */
	int (*write_reg)(uint32_t, uint32_t, uint32_t, uint32_t);

	//TODO: add more operations
};

#if IS_REACHABLE(CONFIG_TRIX_SECURE_FIRMWARE)
/*
 * @fn		int secure_get_security_state(void);
 * @brief	get the present security state of ARM core as reflects
		on ARM TrustZone security extensions.
 * @return	0 - non-secure, 1 - secure, others - reserved
 */
int secure_get_security_state(void);

struct secure_fw_ops *secure_get_ops(void);

void trix_secure_firmware_register(struct secure_fw_ops *ops);

#else
static inline int secure_get_security_state(void) { return 0; }

static inline void trix_secure_firmware_register(struct secure_fw_ops *ops) { }

static inline struct secure_fw_ops *secure_get_ops(void) { return NULL;}
#endif

#endif /*__ASSEMBLY__*/

#endif
