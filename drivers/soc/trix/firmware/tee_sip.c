/**
  @file   tee_sip.c
  @brief
	This file decribes ree glue codes for sigma designs defined sip services, which is headed for ARMv8 projects.

  @author Tony He, tony_he@sigmadesigns.com
  @date   2017-04-25
  */

#define pr_fmt(fmt) "sip: " fmt

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/arm-smccc.h>
#include <linux/soc/sigma-dtv/security.h>
#include "sd_sip_svc.h"

#ifdef DEBUG
# define NOISE_ON_FAIL(sret) do {				\
	if ((sret) != SD_SIP_E_SUCCESS) {			\
		dev_warn("%s failed ret %d\n", __func__, sret);	\
	}							\
}while(0)
#else
# define NOISE_ON_FAIL(sret) do{}while(0)
#endif

#define call_invoke_fn(...)					\
	((ptee_drv && ptee_drv->invoke_fn) ? ptee_drv->invoke_fn(__VA_ARGS__) : (-ENOSYS))

typedef void (*sip_invoke_fn)(unsigned long, unsigned long, unsigned long,
			unsigned long, unsigned long, unsigned long,
			unsigned long, unsigned long,
			struct arm_smccc_res *);
/**
 * struct fw_sip
 */
struct fw_sip {
	struct device *dev;
	sip_invoke_fn invoke_fn;
	void *priv_data;
};

static struct fw_sip *ptee_drv = NULL;
static int security_state = EXEC_STATE_NORMAL;	/* non-secure always */

static inline void reg_pair_from_64(uint32_t *reg0, uint32_t *reg1, uint64_t val)
{
	*reg0 = val >> 32;
	*reg1 = val;
}

static int sip_to_linux_ret(const int sret)
{
	if (sret == SD_SIP_E_SUCCESS)
		return 0;
	else if (sret == SD_SIP_E_INVALID_PARAM)
		return -EINVAL;
	else if (sret == SD_SIP_E_NOT_SUPPORTED)
		return -EPERM;
	else if (sret == SD_SIP_E_INVALID_RANGE)
		return -ERANGE;
	else if (sret == SD_SIP_E_PERMISSION_DENY)
		return -EACCES;
	else if (sret == SD_SIP_E_LOCK_FAIL)
		return -EAGAIN;
	else if (sret == SD_SIP_E_SMALL_BUFFER)
		return -ENOMEM;
	else
		return -EIO;
}

static bool sip_api_uid_is_sd_api(sip_invoke_fn fn)
{
	struct arm_smccc_res res;
	fn(SIP_SVC_UID, 0, 0, 0, 0, 0, 0, 0, &res);

	pr_debug("SIP UID %lx %lx %lx %lx\n", res.a0, res.a1, res.a2, res.a3);
	if (res.a0 == SD_SIP_UID_0 && res.a1 == SD_SIP_UID_1 &&
	    res.a2 == SD_SIP_UID_2 && res.a3 == SD_SIP_UID_3)
		return true;
	return false;
}

static bool sip_api_revision_is_compatible(sip_invoke_fn fn)
{
	struct arm_smccc_res res;
	fn(SIP_SVC_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == SD_SIP_SVC_VERSION_MAJOR &&
	    res.a1 >= SD_SIP_SVC_VERSION_MINOR)
		return true;
	return false;
}

static int sd_sip_get_security_state(void)
{
	return security_state;
}

static int sd_sip_set_mem_protection(unsigned long va, unsigned long sz)
{
	struct arm_smccc_res res;
	uint32_t high, low;
	void *tmp = NULL;
	BUG_ON(va == 0);
	if ((tmp = kzalloc(sz, GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}
	memcpy(tmp, (void*)va, sz);
	reg_pair_from_64(&high, &low, virt_to_phys(tmp));
	call_invoke_fn(SD_SIP_FUNC_N_SET_PST, high, low, sz, 0, 0, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	kfree(tmp);
	return sip_to_linux_ret(res.a0);
}

static int sd_sip_mmio(unsigned long mode, unsigned long pa, unsigned long a2, unsigned long a3, unsigned int wnr)
{
	unsigned long op;
	struct arm_smccc_res res;
	uint32_t high, low;
	reg_pair_from_64(&high, &low, pa);
	op = ((mode & SEC_MMIO_MODE_MASK) << SEC_MMIO_MODE_SHIFT) |
		(wnr & SEC_MMIO_CMD_WNR);
	call_invoke_fn(SD_SIP_FUNC_N_MMIO, op, high, low, a2, a3, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0 &&
	    !(op & SEC_MMIO_CMD_WNR)) {
		BUG_ON(a2 == 0);
		*(uint32_t*)a2 = res.a1;
	}
	return sip_to_linux_ret(res.a0);
}

static int sd_sip_fuse_read(unsigned long ofs, unsigned long va, unsigned long len, unsigned int *pprot)
{
	struct arm_smccc_res res;
	uint32_t high, low;
	void *tmp = NULL;
	if (va != 0) {
		if ((tmp = kzalloc(len, GFP_KERNEL)) == NULL)
			return -ENOMEM;
	}
	reg_pair_from_64(&high, &low, (tmp != NULL) ? virt_to_phys(tmp) : 0);
	call_invoke_fn(SD_SIP_FUNC_C_OTP_READ, ofs, high, low, len, 0, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0) {
		if (tmp != NULL)
			memcpy((void*)va, tmp, len);
		if (pprot != NULL)
			*pprot = res.a1;
	}

	if (tmp != NULL) kfree(tmp);
	return sip_to_linux_ret(res.a0);
}

static int sd_sip_get_rsa_key(unsigned long va, unsigned long len)
{
	struct arm_smccc_res res;
	uint32_t high, low;
	void *tmp = NULL;
	BUG_ON(va == 0);
	if ((tmp = kzalloc(len, GFP_KERNEL)) == NULL)
		return -ENOMEM;
	reg_pair_from_64(&high, &low, virt_to_phys(tmp));
	call_invoke_fn(SD_SIP_FUNC_N_RSA_KEY, high, low, len, 0, 0, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0) {
		memcpy((void*)va, tmp, len);
	}
	kfree(tmp);
	return sip_to_linux_ret(res.a0);
}

static int sd_sip_get_mem_state(unsigned long pa, unsigned long len, unsigned int *pstate)
{
	struct arm_smccc_res res;
	uint32_t high, low;
	reg_pair_from_64(&high, &low, pa);
	call_invoke_fn(SD_SIP_FUNC_C_MEM_STATE, high, low, len, 0, 0, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0) {
		BUG_ON(pstate == NULL);
		*pstate = res.a1;
	}
	return sip_to_linux_ret(res.a0);
}

TEE_OPS("sd sip",
	sd_sip_get_security_state,
	sd_sip_set_mem_protection,
	NULL,
	sd_sip_mmio,
	sd_sip_fuse_read,
	sd_sip_get_rsa_key,
	sd_sip_get_mem_state)

static int get_invoke_func(struct device *dev, sip_invoke_fn *invoke_fn)
{
	struct device_node *np = dev->of_node;
	const char *method;

	dev_info(dev, "probing for conduit method from DT.\n");

	if (of_property_read_string(np, "method", &method)) {
		dev_warn(dev, "missing \"method\" property\n");
		return -ENXIO;
	}

	if (!strcmp("hvc", method)) {
		*invoke_fn = arm_smccc_hvc;
	} else if (!strcmp("smc", method)) {
		*invoke_fn = arm_smccc_smc;
	} else {
		dev_warn(dev, "invalid \"method\" property: %s\n", method);
		return -EINVAL;
	}
	return 0;
}

static int sd_sip_probe(struct platform_device *pdev)
{
	int rc;
	struct fw_sip *sip = NULL;
	sip_invoke_fn fn = NULL;

	rc = get_invoke_func(&pdev->dev, &fn);
	if (rc)
		return rc;

	if (!sip_api_uid_is_sd_api(fn)) {
		dev_warn(&pdev->dev, "api uid mismatch\n");
		return -EINVAL;
	}

	if (!sip_api_revision_is_compatible(fn)) {
		dev_warn(&pdev->dev, "api revision mismatch\n");
		return -EINVAL;
	}

	sip = devm_kzalloc(&pdev->dev, sizeof(*sip), GFP_KERNEL);
	if (!sip) {
		return -ENOMEM;
	}

	sip->dev = &pdev->dev;
	sip->invoke_fn = fn;
	/* register the ops*/
	sip->priv_data = secure_firmware_register(&tee_ops);

	platform_set_drvdata(pdev, sip);
	ptee_drv = sip;
	dev_info(&pdev->dev, "initialized driver\n");
	return 0;
}

static int sd_sip_remove(struct platform_device *pdev)
{
	struct fw_sip *sip = platform_get_drvdata(pdev);
	if (!sip)
		return -EINVAL;

	secure_firmware_unregister(sip->priv_data);
	ptee_drv = NULL;
	return 0;
}

static const struct of_device_id sip_of_match[] = {
	{ .compatible = "trix,arm-tf-sip", },
	{ },
};

static struct platform_driver sd_sip_driver = {
	.driver = {
		.name = "trix_sip",
		.of_match_table = of_match_ptr(sip_of_match),
	},
	.probe = sd_sip_probe,
	.remove = sd_sip_remove,
};

static int __init sd_sip_init(void)
{
	struct device_node *node;

	/*
	 * Preferred path is /firmware/sip, but it's the matching that
	 * matters.
	 */
	for_each_matching_node(node, sip_of_match)
		of_platform_device_create(node, NULL, NULL);

	return platform_driver_register(&sd_sip_driver);
}
arch_initcall(sd_sip_init);

static void __exit sd_sip_exit(void)
{
	platform_driver_unregister(&sd_sip_driver);
}
module_exit(sd_sip_exit);

MODULE_AUTHOR("SigmaDesigns");
MODULE_DESCRIPTION("SiP driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
