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
 *  Author: Tony He, 2014.
 */

#define pr_fmt(fmt) "smc: " fmt

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#ifdef CONFIG_TRIX_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif
#include <mach/hardware.h>
#include <asm/cacheflush.h>

#ifdef CONFIG_SMP
#include <mach/smp.h>
#endif
#include "smc.h"
#include "cp15.h"	/*set_auxcr,get_nsacr*/

#ifdef DEBUG
static const char* reg_access_mode(int mode)
{
	char *str = "unknown";
	switch(mode) {
	case 0: str = "byte"; break;
	case 1: str = "hword"; break;
	case 2: str = "word"; break;
	default: break;
	}
	return str;
}

# define SMC_DEBUG(fmt...) do{pr_debug(fmt);}while(0)
#else
# define SMC_DEBUG(fmt...) do{}while(0)
#endif

#define RM_OK 0x6	/*check armor code*/

#define armor_call_body(dbg, nm, args...)	({		\
	volatile uint64_t _ret;					\
	_ret = armor_smc_##nm(args);				\
	if (dbg)						\
	  SMC_DEBUG("armor_call_%s result: %llx\n", #nm, _ret);	\
	_ret;							\
})

#define armor_call(nm, args...)	armor_call_body(1, nm, args)
#define armor2linuxret(ret)	((ret) == RM_OK ? 0 : 1)

/*
 * otp_access_code
 * otp access code definition
 * this enum is inherited from armor/include/otp.h
 */
enum otp_access_code{
	OTP_ACCESS_CODE_FUSE_MIRROR = 0,	/*read fuse mirror, arg0 - fuse offset, return fuse value on success*/
	OTP_ACCESS_CODE_FUSE_ARRAY,		/*read fuse array, arg0 - fuse offset, arg1 - phy addr of buf, arg2 - buf length, return RM_OK on success*/
};

struct secure_reg_access {
	uint32_t addr;	/* address */
	uint32_t amask;	/* address mask */
#define OP_MODE_MASK	0x3
#define OP_MODE_BYTE	(0 << 0)
#define OP_MODE_HWORD	(1 << 0)
#define OP_MODE_WORD	(2 << 0)
#define OP_ACCESS_MASK	(0x3 << 2)
#define OP_ACCESS_RD	(1 << 2)
#define OP_ACCESS_WR	(1 << 3)
	uint32_t op;	/* operation value*/
};

/*
 * secure register access table. All NS access to those regiters will be routed to armor.
 * it shall be an superset of reg_access_table in armor/flow/reg_access.c
 */
static struct secure_reg_access reg_access_tbl[] =
{
#if defined(CONFIG_SIGMA_SOC_SX6) || defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX8)
	{0xf5005000, 0xfffff000, OP_ACCESS_RD | OP_ACCESS_WR},	/* PMAN_SEC0 (4k) */
	{0xf5008000, 0xfffff000, OP_ACCESS_RD | OP_ACCESS_WR},	/* PMAN_SEC1 (4k) */
# ifndef CONFIG_SIGMA_SOC_SX6
	{0xf5036000, 0xfffff000, OP_ACCESS_RD | OP_ACCESS_WR},	/* PMAN_SEC2 (4k) */
# endif
	{0xf5002000, 0xfffff000, OP_ACCESS_RD},			/* PLF_MMIO_Security (4k) */
	{0xf5003000, 0xfffff000, OP_ACCESS_RD | OP_ACCESS_WR},	/* PLF_MMIO_Configure (4k) */
	{0xf0016000, 0xfffff000, OP_ACCESS_RD},			/* AV_MMIO_Security (4k) */
	{0xf0017000, 0xfffff000, OP_ACCESS_RD | OP_ACCESS_WR},	/* AV_MMIO_Configure (4k) */
	{0xfa000000, 0xfffff000, OP_ACCESS_RD},			/* DISP_MMIO_Security (4k) */
	{0xfa001000, 0xfffff000, OP_ACCESS_RD | OP_ACCESS_WR},	/* DISP_MMIO_Configure (4k) */
# ifdef CONFIG_SIGMA_SOC_SX6
	{0xf5100000, 0xfffe0000, OP_ACCESS_RD | OP_ACCESS_WR},	/* Turing (128k) */
# elif defined(CONFIG_SIGMA_SOC_SX7) || defined(CONFIG_SIGMA_SOC_SX8)
	{0xf1040000, 0xfffe0000, OP_ACCESS_RD | OP_ACCESS_WR},	/* Turing (128k) */
# endif
#endif
	{-1, -1,}	/* the end */
};

/**************************************************************************/
/* sx6 secure monitor configuration					  */
/* Give "have_armor" in cmdline to fit with secure monitor (armor), and   */
/* linux runs in non-secure then					  */
/**************************************************************************/
static int security_state = 1;	/*0 - NS, 1 - Secure*/
static int __init do_with_secure_monitor(char *str)
{
	pr_info("##linux start with secure monitor##\n");
	security_state = 0;
	return 0;
}
early_param("with_armor", do_with_secure_monitor);

/*
 * Return current security state
 * 0 - Non-secure, 1 - secure
 */
int secure_get_security_state(void)
{
	return security_state;
}
EXPORT_SYMBOL(secure_get_security_state);

/**************************************************************************/
/* l2x0 control wrapper						  */
/**************************************************************************/
#ifdef CONFIG_TRIX_CACHE_L2X0
void secure_l2x0_set_reg(void* base, uint32_t ofs, uint32_t val)
{
	/*
	 * Program PL310 Secure R/W registers
	 * So far secure monitor only supports l2x0 regs
	 * L2X0_CTRL
	 * L2X0_AUX_CTRL
	 * L2X0_DEBUG_CTRL
	 * L310_PREFETCH_CTRL
	 */
	pr_debug("program l2x0 regs (ofs %#x) -> %#x\n", ofs, val);
	if ( security_state ) {
		writel_relaxed(val, base + ofs);
	} else {
		armor_call(set_l2_reg, ofs, val);
	}
}
#endif /*CONFIG_TRIX_CACHE_L2X0*/

/*
 * 1. ACTLR R/W in secure state;
 * 2. ACTLR RO in Non-secure state if NSACR.NS_SMP=0;
 * 3. ACTLR R/W in Non-secure state if NSACR.NS_SMP=1. In this case all bits are write ignored except for the SMP bit
 */
void secure_set_actlr(uint32_t val)
{
	/* Program PL310 L2 Cache controller debug register */
	if ( security_state ) {
		set_auxcr(val);
	} else {
		if (get_nsacr() & NSACR_NS_SMP) {
			unsigned int tmp = get_auxcr();
			/*check if only change to SMP bit*/
			if ((tmp & ~AUXCR_SMP) == (val & ~AUXCR_SMP)) {
				set_auxcr(val);
				return;
			}
		}
		;	/*armor N/A yet*/
	}
}

#ifdef CONFIG_TRIX_CPUFREQ_FORWARD
void secure_scale_cpufreq(uint32_t target)
{
	if ( !security_state )
		armor_call(scale_cpufreq, target);
}
#endif

#define is_secure_accessible(a, t) ({					\
	int _ret = false;						\
	struct secure_reg_access *_e;					\
	for (_e=&reg_access_tbl[0]; _e->addr != -1; _e++) {		\
		if (!(((a) ^ _e->addr) & _e->amask) &&			\
		((t) & _e->op)) {					\
			pr_debug("hit entry: %08x %08x %08x, %08x:%x\n", _e->addr, _e->amask, _e->op, (a), (t));						\
			_ret = true;					\
			break;						\
		}							\
	}								\
	_ret;								\
})

int secure_read_reg(uint32_t mode, uint32_t pa, uint32_t *pval)
{
	//SMC_DEBUG("read_reg_%s(%#x)\n", reg_access_mode(mode), pa);
	BUG_ON(pval == NULL);
	if ( !security_state && is_secure_accessible(pa, OP_ACCESS_RD)) {
		union {
			uint64_t data;
			struct {
				uint32_t val;	/* [31:0] */
				uint32_t ret;	/* [63:32] */
			};
		} tmp;

		tmp.data = armor_call(read_reg, mode, pa);
		if (RM_OK == tmp.ret) {
			*pval = tmp.val;
		} else {
			WARN(1, "read_reg_uint%d(0x%08x) failed!\n", 8 << mode, pa);
			*pval = 0;	/*fill with 0 whatever*/
			return -EACCES;
		}
	} else {
		if (mode == 0)
			*pval = __raw_readb((void*)pa);
		else if (mode == 1)
			*pval = __raw_readw((void*)pa);
		else if (mode == 2)
			*pval = __raw_readl((void*)pa);
		else
			*pval = 0;
	}
	return 0;
}
EXPORT_SYMBOL(secure_read_reg);

int secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask)
{
	//SMC_DEBUG("write_reg_%s(%#x, %#x, %#x)\n", reg_access_mode(mode), pa, val, mask);
	if ( !security_state && is_secure_accessible(pa, OP_ACCESS_WR)) {
		uint32_t ret = armor_call(write_reg, mode, pa, val, mask);
		if (ret != RM_OK) {
			WARN(1, "write_reg_uint%d(0x%08x, 0x%08x, 0x%08x) failed!\n", 8 << mode, pa, val, mask);
			return -EACCES;
		}
	} else {
		if (mode == 0) {
			if (mask != 0xff)
				MaskWriteReg(b, pa, val, mask);
			else
				__raw_writeb(val, (void*)pa);
		} else if (mode == 1) {
			if (mask != 0xffff)
				MaskWriteReg(w, pa, val, mask);
			else
				__raw_writew(val, (void*)pa);
		} else if (mode == 2) {
			if (mask != 0xffffffff)
				MaskWriteReg(l, pa, val, mask);
			else
				__raw_writel(val, (void*)pa);
		} else {
			pr_warning("unknown access mode value (%d)\n", mode);
		}
	}
	return 0;
}
EXPORT_SYMBOL(secure_write_reg);

int secure_otp_get_fuse_mirror(const uint32_t offset, uint32_t *pval)
{
	union {
		uint64_t data;
		struct {
			uint32_t val;	/* [31:0] */
			uint32_t ret;	/* [63:32] */
		};
	} tmp;

	if ( security_state ) {
		pr_err("error: call to %s in Secure world!\n", __func__);
		return -EAGAIN;
	}

	tmp.data = armor_call(otp_access, OTP_ACCESS_CODE_FUSE_MIRROR, offset, 0, 0);
	BUG_ON(pval == NULL);
	if (RM_OK == tmp.ret) {
		*pval = tmp.val;
		return 0;
	} else {
		*pval = 0;
		return -EACCES;
	}
}

int secure_otp_get_fuse_array(const uint32_t offset, uint32_t *buf, uint32_t nbytes)
{
	if ( security_state ) {
		pr_err("error: call to %s in Secure world!\n", __func__);
		return -EAGAIN;
	} else {
		uint32_t ret, addr;
		void *tmp = NULL;
		if ((tmp = kzalloc(nbytes, GFP_KERNEL)) != NULL) {
			addr = virt_to_phys(tmp);
			__cpuc_flush_dcache_area(tmp, nbytes);
			outer_inv_range(addr, addr + nbytes);
			ret = (uint32_t)armor_call(otp_access, OTP_ACCESS_CODE_FUSE_ARRAY,
							offset, addr, nbytes);
			if (RM_OK == ret) {
				BUG_ON(buf == NULL);
				memcpy(buf, tmp, nbytes);
				ret = 0;
			} else {
				ret = -EACCES;
			}
			kfree(tmp);
		} else {
			ret = -ENOMEM;
		}
		return ret;
	}
}

#ifdef CONFIG_SMP
int secure_set_aux_core_addr(const void* reg, const uint32_t pa)
{
	int ret = 0;
	uint32_t ta = (pa & AUX_BOOT_ADDR_MASK);
	if (!security_state && ((u32)reg != AUX_BOOT_ID_REG)) {
		ret = (uint32_t)armor_call(set_aux_core_boot_addr, ta);
		ret = armor2linuxret(ret);
	} else {
		BUG_ON(reg == NULL);
		writel(ta, (void*)reg);	/*reset boot id as well*/
	}

	return ret;
}

int secure_boot_aux_core(const void* reg, const uint32_t cpu)
{
	int ret = 0;
	if (!security_state && ((u32)reg != AUX_BOOT_ID_REG)) {
		ret = (uint32_t)armor_call(set_aux_core_boot0, cpu);
		ret = armor2linuxret(ret);
	} else {
		u32 tmp;
		BUG_ON(reg == NULL);
		tmp = readl(reg);
		tmp &= ~AUX_BOOT_ID_MASK;
		tmp |= (cpu & AUX_BOOT_ID_MASK);
		writel(tmp, (void*)reg);
	}

	return ret;
}

int secure_get_aux_core_boot(const void* reg, uint32_t *pval)
{
	if (!security_state && ((u32)reg != AUX_BOOT_ID_REG)) {
		union {
			uint64_t data;
			struct {
				uint32_t val;	/* [31:0] */
				uint32_t ret;	/* [63:32] */
			};
		} tmp;

		tmp.data = armor_call(get_aux_core_boot);
		if (tmp.ret == RM_OK) {
			BUG_ON(pval == NULL);
			*pval = tmp.val;
			return 0;
		} else {
			return -EACCES;
		}
	} else {
		BUG_ON(reg == NULL);
		BUG_ON(pval == NULL);
		*pval = readl((void*)reg);
		return 0;
	}
}
#endif
