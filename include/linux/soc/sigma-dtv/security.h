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

#define EXEC_STATE_NORMAL	0
#define EXEC_STATE_SECURE	(!EXEC_STATE_NORMAL)

#ifndef __ASSEMBLY__

/*
 * derived from tee_service.h from u-boot
 *
 * struct secure_fw_ops -
 *    represents the various operations provided by secure world
 *
 *    A filled up structure can be registered with trix_secure_firmware_register().
 */
struct secure_fw_ops {
	/*
	 * debug only
	 */
	const char* name;
	/*
	 * the present security state of ARM core, to be deprecated
	 */
	int (*get_secure_state)(void);
	/*
	 * update pman protection from normal world
	 */
	int (*set_mem_protection)(unsigned long tva, unsigned long sz);
	/*
	 * set LC310 registers, deprecated in arm64
	 */
	int (*set_l2x_reg)(unsigned long ofs, unsigned long val);
	/*
	 * access to allowable secure registers
	 */
	int (*secure_mmio)(unsigned long op, unsigned long pa, unsigned long a2, unsigned long a3, unsigned int wnr);
	/*
	 * read allowable otp
	 */
	int (*fuse_read)(unsigned long ofs, unsigned long va, unsigned long len, unsigned int *pprot);
	/*
	 * read present selected RSA public key
	 */
	int (*get_rsa_key)(unsigned long va, unsigned long len);
	/*
	 * check memory access state
	 */
	int (*get_mem_state)(unsigned long pa, unsigned long len, unsigned int *pstate);
};

#define TEE_OPS(tag, secure_state_fn,		\
		set_pst_fn, set_l2x_fn,		\
		mmio_fn, fusread_fn,		\
		rsa_key_fn, mem_state_fn)	\
static struct secure_fw_ops tee_ops = {		\
	.name = tag,				\
	.get_secure_state = secure_state_fn,	\
	.set_mem_protection = set_pst_fn,	\
	.set_l2x_reg = set_l2x_fn,		\
	.secure_mmio = mmio_fn,			\
	.fuse_read = fusread_fn,		\
	.get_rsa_key = rsa_key_fn,		\
	.get_mem_state = mem_state_fn,		\
};

/*
 * @fn		void* secure_firmware_register(struct secure_fw_ops *ops);
 * @brief	register secure firmware operations.
 * @return	handler
 */
void* secure_firmware_register(struct secure_fw_ops *ops);

/*
 * @fn		void secure_firmware_unregister(void* handle);
 * @brief	unregister secure firmware operations.
 * @return	void
 */
void secure_firmware_unregister(void* handle);

/*
 * @fn		int secure_get_security_state(void);
 * @brief	get the present security state of ARM core as reflects
		on ARM TrustZone security extensions.
 * @return	0 - non-secure, 1 - secure, others - reserved
 */
int secure_get_security_state(void);

/*
 * @fn		int secure_l2x0_set_reg(const uint32_t ofs, const uint32_t val);
 * @brief	request tee to set L2 cache registers (A9 only, deprecated)
 * @param[in]	<ofs>  - register offset offset
 * @param[in]	<val>  - value to set with
 * @return	0 on success. Otherwise error code
 */
int secure_l2x0_set_reg(const uint32_t ofs, const uint32_t val);

/*
 * @fn		int secure_set_mem_protection(const uintptr_t va, const uint32_t sz);
 * @brief	request tee to setup pman protection
 * @param[in]	<va>   - pman table buffer address
 * @param[in]	<sz>   - table buffer size
 * @return	0 on success. Otherwise error code
 */
int secure_set_mem_protection(const uintptr_t va, const uint32_t sz);

/*
 * @fn		int secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval, uint32_t *pprot);
 * @brief	request tee to read fuse mirror, must from NS world
 * @param[in]	<offset> - fuse offset
 * @param[out]	<pval>   - buffer pointer
 * @param[out]	<pprot>  - pointer of integar to load protection value on return, or NULL to ignore
 * @return	0 on success. Otherwise error code
 */
int secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval, uint32_t *pprot);

/*
 * @fn		int secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, const uint32_t nbytes, uint32_t *pprot);
 * @brief	request tee to read fuse array, must from NS world
 * @param[in]	<offset> - fuse offset
 * @param[out]	<buf>    - buffer pointer, or NULL to read protection only
 * @param[in]	<nbytes> - buffer length
 * @param[out]	<pprot>  - pointer of integar to load protection value on return, or NULL to ignore
 * @return	return 0 and fuse value filled in buf on success. Otherwise error code
 */
int secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, const uint32_t nbytes, uint32_t *pprot);


/*
 * @fn		int secure_read_reg(const uint32_t mode, const uint32_t pa, uint32_t *pval);
 * @brief	secure read register
 * 		user shall rarely use this API, go with secure_read_uintx for instead.
 * @param[in]	<mode> - specifies access mode
 * 		0 - byte
 * 		1 - halfword
 * 		2 - word
 * 		others - reserved
 * @param[in]	<pa>   - specifies reg physical addr
 * @param[out]	<pval> - pointer of buffer
 * @return	0 on success. Otherwise error code
 */
int secure_read_reg(const uint32_t mode, const uint32_t pa, uint32_t *pval);

/*
 * @fn		int secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
 * @brief	secure write register, supports mask
 * 		user shall rarely use this API, go with secure_write_uintx for instead.
 * @param[in]	<mode> - specifies access mode
 * 		0 - byte
 * 		1 - halfword
 * 		2 - word
 * 		others - reserved
 * @param[in]	<pa>   - specifies reg physical addr
 * @param[in]	<val>  - value to write
 * @param[in]	<mask> - mask for write
 * @return	0 on success. Otherwise error code
 */
int secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask);
#define secure_read_generic(pa, m, t) ({					\
				uint32_t _tmp;					\
				secure_read_reg(m, (uint32_t)pa, &_tmp);	\
				(t)_tmp;					\
})
#define secure_read_uint8(pa)	secure_read_generic(pa, 0, uint8_t)
#define secure_read_uint16(pa)	secure_read_generic(pa, 1, uint16_t)
#define secure_read_uint32(pa)	secure_read_generic(pa, 2, uint32_t)
#define secure_write_uint8(pa, v, m)	secure_write_reg(0, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define secure_write_uint16(pa, v, m)	secure_write_reg(1, (uint32_t)pa, (uint32_t)v, (uint32_t)m)
#define secure_write_uint32(pa, v, m)	secure_write_reg(2, (uint32_t)pa, (uint32_t)v, (uint32_t)m)

/*
 * @fn		int secure_get_rsa_pub_key(uint32_t *buf, const uint32_t nbytes);
 * @brief	request tee to load the selected rsa public key, must from NS world
 * @param[out]	<buf>    - buffer pointer
 * @param[in]	<nbytes> - buffer length
 * @return	return 0 and rsa public key filled in buf on success. Otherwise error code
 */
int secure_get_rsa_pub_key(uint32_t *buf, const uint32_t nbytes);

/*
 * @fn		int secure_get_mem_state(const uintptr_t pa, const size_t len, uint32_t *pstate);
 * @brief	check access state of specified memory block
 * @param[in]	<pa>     - physical address of memory block
 * @param[in]	<len>    - length of memory block in bytes
 * @param[out]  <pstate> - pointer of buffer to load access state on success
 *		bit[0]	- 1: secure accessible,   0: secure non-accessible
 *		bit[1]	- 1: non-secure readable, 0: non-secure non-readable
 *		bit[2]	- 1: non-secure writable, 0: non-secure non-writable
 *		bit[3]	- 1: secure executable,   0: secure non-executable
 *		bit[4]	- 1: ns executable,       0: non-secure non-executable
 *		others	- N/A
 * @return	return 0 and access state filled in buffer pointed by <pstate> on success. Otherwise error code
 */
int secure_get_mem_state(const uintptr_t pa, const size_t len, uint32_t *pstate);

#endif /*__ASSEMBLY__*/

#endif
