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

/*
 * inspired by smc-common.c from u-boot
 */
#define pr_fmt(fmt) "sip: " fmt

#include <linux/soc/sigma-dtv/security.h>


#define call_secure_fw_op(op, ...)					\
	((secure_fw_ops && secure_fw_ops->op) ? secure_fw_ops->op(__VA_ARGS__) : (-ENOSYS))

/**************************************************************************/
/* secure fw operations                                                   */
/**************************************************************************/
static struct secure_fw_ops *secure_fw_ops = NULL;

/*
 * @fn		int secure_get_security_state(void);
 * @brief	get the present security state of ARM core as reflects
		on ARM TrustZone security extensions.
 * @return	0 - non-secure, 1 - secure, others - reserved
 */
int secure_get_security_state(void)
{
	if (!secure_fw_ops)
		return EXEC_STATE_NORMAL;

	return call_secure_fw_op(get_secure_state);
}
EXPORT_SYMBOL(secure_get_security_state);

int secure_set_mem_protection(const uintptr_t va, const uint32_t sz)
{
	int ret;
	ret = call_secure_fw_op(set_mem_protection, va, sz);
	if (ret != 0) {
		pr_warn("%s failed, error %d\n", __func__, ret);
	}
	return ret;
}

/**************************************************************************/
/* l2x0 control wrapper						  */
/**************************************************************************/
int secure_l2x0_set_reg(const uint32_t ofs, const uint32_t val)
{
	return call_secure_fw_op(set_l2x_reg, ofs, val);
}

int secure_read_reg(const uint32_t mode, const uint32_t pa, uint32_t *pval)
{
	return call_secure_fw_op(secure_mmio, mode, pa, (unsigned long)pval, 0, 0);
}
EXPORT_SYMBOL(secure_read_reg);

int secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask)
{
	return call_secure_fw_op(secure_mmio, mode, pa, val, mask, 1);
}
EXPORT_SYMBOL(secure_write_reg);

int secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval, uint32_t *pprot)
{
	return secure_otp_get_fuse_array(offset, pval, 4, pprot);
}

int secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, const uint32_t nbytes, uint32_t *pprot)
{
	BUG_ON(secure_get_security_state() == EXEC_STATE_SECURE);
	return call_secure_fw_op(fuse_read, offset, (uintptr_t)buf, nbytes, pprot);
}

int secure_get_rsa_pub_key(uint32_t *buf, const uint32_t nbytes)
{
	BUG_ON(secure_get_security_state() == EXEC_STATE_SECURE);
	return call_secure_fw_op(get_rsa_key, (uintptr_t)buf, nbytes);
}

int secure_get_mem_state(const uintptr_t pa, const size_t len, uint32_t *pstate)
{
	BUG_ON(secure_get_security_state() == EXEC_STATE_SECURE);
	return call_secure_fw_op(get_mem_state, pa, len, pstate);
}

/*
 * secure_firmware_register(ops)
 *
 * A function to register platform secure_fw_ops struct.
 */
void* secure_firmware_register(struct secure_fw_ops *ops)
{
	BUG_ON(!ops);
	pr_debug("register secure fw: %s\n", secure_fw_ops->name);
	secure_fw_ops = ops;
	return secure_fw_ops;
}
EXPORT_SYMBOL(secure_firmware_register);

/*
 * secure_firmware_unregister(handle)
 * TODO: deal with possible race condition
 */
void secure_firmware_unregister(void *handle)
{
	BUG_ON(secure_fw_ops != handle);
	secure_fw_ops = NULL;
}
EXPORT_SYMBOL(secure_firmware_unregister);
