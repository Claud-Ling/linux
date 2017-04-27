/**
  @file   tee_armor.c
  @brief
	This file decribes ree glue codes for sigma designs defined TOS - armor.
	This is for legacy A9 projects only.

  @author Tony He, tony_he@sigmadesigns.com
  @date   2017-04-25
  */

#define pr_fmt(fmt) "armor: " fmt

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/cacheflush.h>
#include <linux/soc/sigma-dtv/security.h>
#include "armor_smc.h"

#define RM_OK 0x6	/*check armor code*/

#define armor_call_body(dbg, nm, args...)	({		\
	volatile uint64_t _ret;					\
	_ret = armor_smc_##nm(args);				\
	if (dbg)						\
	  pr_debug("armor_call_%s result: %llx\n", #nm, _ret);	\
	_ret;							\
})

#define armor_call(nm, args...)	armor_call_body(1, nm, args)
#define armor2linuxret(ret)	((ret) == RM_OK ? 0 : -EIO)

/**
 * struct fw_armor
 */
struct fw_armor {
	struct device *dev;
	size_t rsa_key_ofs;
	void *priv_data;
};

/**************************************************************************/
/* cortex-a9 security state control					  */
/**************************************************************************/
static int security_state = EXEC_STATE_NORMAL;	/*0 - NS, 1 - Secure*/
static struct fw_armor *ptee_drv = NULL;

/*
 * otp_access_code
 * otp access code definition
 * this enum is inherited from armor/include/otp.h
 */
enum otp_access_code{
	OTP_ACCESS_CODE_FUSE_MIRROR = 0,	/*read fuse mirror, arg0 - fuse offset, return fuse value on success*/
	OTP_ACCESS_CODE_FUSE_ARRAY,		/*read fuse array, arg0 - fuse offset, arg1 - phy addr of buf, arg2 - buf length, return RM_OK on success*/
};

/*
 * probe core executing state
 * return value
 *   0 in case of non-secure
 *   1 in case of secure
 *
 */
static int is_execute_in_secure_state(void)
{
	#define MTESTREG 0xf2101080	/*GICDISR0*/
	#define MTESTBIT (1 << 27)	/*Global Timer Interrupt*/
	volatile unsigned long *addr = (volatile unsigned long*)MTESTREG;
	unsigned long bak = 0, val = 0;

	/*
	 * GICDISR0
	 *  r/w in secure state
	 *  RAZ/WI in non-secure state
	 */
	bak = *addr;
	*addr = (bak | MTESTBIT);
	val = *addr;

	if (val != 0) {
		*addr = bak;	/*recover*/
		return EXEC_STATE_SECURE;
	} else {
		return EXEC_STATE_NORMAL;
	}
}

static int armor_get_security_state(void)
{
	security_state = is_execute_in_secure_state();
	pr_debug("security state: %d\n", security_state);
	return security_state;
}

static int armor_set_mem_protection(unsigned long va, unsigned long sz)
{
	uint32_t ret = 0;
	void *tmp = NULL;
	BUG_ON(va == 0);
	if ((tmp = kzalloc(sz, GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}
	memcpy(tmp, (void*)va, sz);
	ret = armor_call(set_mem_protection, virt_to_phys(tmp), sz);
	kfree(tmp);
	return armor2linuxret(ret);
}

/**************************************************************************/
/* l2x0 control wrapper						  */
/**************************************************************************/
static int armor_set_l2x0_reg(unsigned long ofs, unsigned long val)
{
	/*
	 * Program PL310 Secure R/W registers
	 * So far secure monitor only supports l2x0 regs
	 * L2X0_CTRL
	 * L2X0_AUX_CTRL
	 * L2X0_DEBUG_CTRL
	 * L310_PREFETCH_CTRL
	 */
	uint32_t ret = 0;
	pr_debug("program l2x0 regs (ofs %#lx) -> %#lx\n", ofs, val);
	ret = armor_call(set_l2_reg, ofs, val);
	return armor2linuxret(ret);
}

static int armor_read_reg(uint32_t mode, unsigned long pa, uint32_t *pval)
{
	union {
		uint64_t data;
		struct {
			uint32_t val;	/* [31:0] */
			uint32_t ret;	/* [63:32] */
		};
	} tmp;

	BUG_ON(pval == NULL);
	tmp.data = armor_call(read_reg, mode, pa);
	if (RM_OK == tmp.ret) {
		*pval = tmp.val;
		return 0;
	} else {
		pr_debug("read_reg_uint%d(0x%08lx) failed!\n", 8 << mode, pa);
		*pval = 0;	/*fill with 0 whatever*/
		return -EACCES;
	}
}

static int armor_write_reg(uint32_t mode, unsigned long pa, uint32_t val, uint32_t mask)
{
	uint32_t ret = armor_call(write_reg, mode, pa, val, mask);
	if (ret != RM_OK) {
		pr_debug("write_reg_uint%d(0x%08lx, 0x%08x, 0x%08x) failed!\n", 8 << mode, pa, val, mask);
		return -EACCES;
	}
	return 0;
}

static int armor_mmio(unsigned long mode, unsigned long pa, unsigned long a2, unsigned long a3, unsigned int wnr)
{
	if (wnr) {
		return armor_write_reg(mode, pa, a2, a3);
	} else {
		return armor_read_reg(mode, pa, (uint32_t*)a2);
	}
}

static int armor_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval)
{
	union {
		uint64_t data;
		struct {
			uint32_t val;	/* [31:0] */
			uint32_t ret;	/* [63:32] */
		};
	} tmp;

	tmp.data = armor_call(otp_access, OTP_ACCESS_CODE_FUSE_MIRROR, offset, 0, 0);
	BUG_ON(pval == NULL);
	if (RM_OK == tmp.ret) {
		*pval = tmp.val;
		return 0;
	} else {
		*pval = 0;
		return -EIO;
	}
}

static int armor_otp_get_fuse_array(const uint32_t offset, uintptr_t buf, uint32_t nbytes)
{
	uint32_t ret;
	uintptr_t pa;
	void *tmp = NULL;
	if ((tmp = kzalloc(nbytes, GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}
	pa = virt_to_phys(tmp);
	__cpuc_flush_dcache_area(tmp, nbytes);
	outer_inv_range(pa, pa + nbytes);
	ret = (uint32_t)armor_call(otp_access, OTP_ACCESS_CODE_FUSE_ARRAY,
					offset, pa, nbytes);
	if (RM_OK == ret) {
		BUG_ON(buf == 0);
		memcpy((void*)buf, tmp, nbytes);
		ret = 0;
	} else {
		ret = -EIO;
	}
	kfree(tmp);
	return ret;
}

static int armor_fuse_read(unsigned long ofs, unsigned long va, unsigned long len, unsigned int *pprot)
{
	(void)pprot;
	if (len == 4) {
		return armor_otp_get_fuse_mirror(ofs, (uint32_t*)va);
	} else {
		return armor_otp_get_fuse_array(ofs, (uintptr_t)va, len);
	}
}

static int armor_get_rsa_key(unsigned long va, unsigned long len)
{
	BUG_ON(ptee_drv == NULL);
	/*fallback to load key from OTP*/
	return armor_otp_get_fuse_array(ptee_drv->rsa_key_ofs, (uintptr_t)va, len);
}

TEE_OPS("armor",
	armor_get_security_state,
	armor_set_mem_protection,
	armor_set_l2x0_reg,
	armor_mmio,
	armor_fuse_read,
	armor_get_rsa_key,
	NULL)

static int sd_armor_probe(struct platform_device *pdev)
{
	struct fw_armor *tee = NULL;
	struct device_node *np = pdev->dev.of_node;
	u32 tmp;

	if (of_property_read_u32(np, "armor,rsa-key-ofs", &tmp)) {
		return -EINVAL;
	}

	tee = devm_kzalloc(&pdev->dev, sizeof(*tee), GFP_KERNEL);
	if (!tee) {
		return -ENOMEM;
	}

	tee->dev = &pdev->dev;
	tee->rsa_key_ofs = tmp;
	/* register the ops*/
	tee->priv_data = secure_firmware_register(&tee_ops);

	platform_set_drvdata(pdev, tee);
	ptee_drv = tee;
	dev_info(&pdev->dev, "initialized driver\n");
	return 0;
}

static int sd_armor_remove(struct platform_device *pdev)
{
	struct fw_armor *tee = platform_get_drvdata(pdev);
	if (!tee)
		return -EINVAL;

	secure_firmware_unregister(tee->priv_data);
	ptee_drv = NULL;
	return 0;
}

static const struct of_device_id armor_of_match[] = {
	{ .compatible = "trix,armor", },
	{ },
};

static struct platform_driver sd_armor_driver = {
	.driver = {
		.name = "trix_armor",
		.of_match_table = of_match_ptr(armor_of_match),
	},
	.probe = sd_armor_probe,
	.remove = sd_armor_remove,
};

static int __init sd_armor_init(void)
{
	struct device_node *node;

	/*
	 * Preferred path is /firmware/armor, but it's the matching that
	 * matters.
	 */
	for_each_matching_node(node, armor_of_match)
		of_platform_device_create(node, NULL, NULL);

	return platform_driver_register(&sd_armor_driver);
}
arch_initcall(sd_armor_init);

static void __exit sd_armor_exit(void)
{
	platform_driver_unregister(&sd_armor_driver);
}
module_exit(sd_armor_exit);

MODULE_AUTHOR("SigmaDesigns");
MODULE_DESCRIPTION("armor driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
