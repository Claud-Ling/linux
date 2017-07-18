/*
 * inspired by smc-stub.c in u-boot
 */


#if !IS_REACHABLE(CONFIG_TRIX_SECURE_FIRMWARE)

#include <linux/kernel.h>
#include <linux/soc/sigma-dtv/security.h>

/*
 * Weak symbol definition for smc
 */
int __weak secure_get_security_state(void)
{
	pr_debug("In weak symbol %s, always indicate in normal status\n", __func__);
	return EXEC_STATE_NORMAL;
}

int __weak secure_set_mem_protection(const uintptr_t va, const uint32_t sz)
{
	(void)va;
	(void)sz;
	return -ENODEV;
}

int __weak secure_l2x0_set_reg(const uint32_t ofs, const uint32_t val)
{
	(void)ofs;
	(void)val;
	return -ENODEV;
}

int __weak secure_read_reg(const uint32_t mode, const uint32_t pa, uint32_t *pval)
{
	(void)mode;
	(void)pa;
	(void)pval;
	return -ENODEV;
}

int __weak secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask)
{
	(void)mode;
	(void)pa;
	(void)val;
	(void)mask;
	return -ENODEV;
}

int __weak secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval, uint32_t *pprot)
{
	(void)offset;
	(void)pval;
	(void)pprot;
	return -ENODEV;
}

int __weak secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, const uint32_t nbytes, uint32_t *pprot)
{
	(void)offset;
	(void)buf;
	(void)nbytes;
	(void)pprot;
	return -ENODEV;
}

int __weak secure_get_rsa_pub_key(uint32_t *buf, const uint32_t nbytes)
{
	(void)buf;
	(void)nbytes;
	return -ENODEV;
}

void* __weak secure_firmware_register(struct secure_fw_ops *ops)
{
	(void)ops;
	return NULL;
}

void __weak secure_firmware_unregister(void* handle)
{
	(void)handle;
}

#endif /* !CONFIG_TRIX_SECURE_FIRMWARE */
