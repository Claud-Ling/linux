/*
 * DOC:
 * Interface for registering and calling firmware-specific operations
 *
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
 */

#include <linux/soc/sigma-dtv/security.h>


#define call_secure_fw_op(op, ...)					\
	((secure_fw_ops->op) ? secure_fw_ops->op(__VA_ARGS__) : (-ENOSYS))

/* Global pointer for current secure_fw_ops structure */
struct secure_fw_ops *secure_fw_ops = NULL;


/*
 * @fn		int secure_get_security_state(void);
 * @brief	get the present security state of ARM core as reflects
		on ARM TrustZone security extensions.
 * @return	0 - non-secure, 1 - secure, others - reserved
 */
int secure_get_security_state(void)
{
	if (!secure_fw_ops)
		return 0;

	return call_secure_fw_op(get_security_state);	
}
EXPORT_SYMBOL(secure_get_security_state);

/*
 * secure_get_ops() -
 *
 * Checks if firmware operation is present then calls it,
 * otherwise returns -ENOSYS
 */
struct secure_fw_ops *secure_get_ops(void)
{
	return secure_fw_ops? secure_fw_ops : NULL;
}
EXPORT_SYMBOL(secure_get_ops);

/*
 * trix_secure_firmware_register(ops)
 *
 * A function to register platform secure_fw_ops struct.
 */
void trix_secure_firmware_register(struct secure_fw_ops *ops)
{
	BUG_ON(!ops);

	secure_fw_ops = ops;
}
EXPORT_SYMBOL(trix_secure_firmware_register);

