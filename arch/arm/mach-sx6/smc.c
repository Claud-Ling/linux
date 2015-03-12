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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/hardware.h>

#include "smc.h"

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

# define SMC_DEBUG(fmt...) do{printk(KERN_DEBUG fmt);}while(0)
#else
# define SMC_DEBUG(fmt...) do{}while(0)
#endif

#define armor_call_body(dbg, nm, args...)	({		\
	volatile uint64_t _ret;					\
	int _count = 1;						\
	_ret = armor_smc_##nm(args);				\
	while (_ret == -1ll) { /*failed*/			\
		_count++; schedule();				\
		_ret = armor_smc_##nm(args); /*retry*/		\
	}							\
	if (dbg)						\
	  SMC_DEBUG("armor_call_%s succeeded in %d tries\n", 	\
		#nm, _count);					\
	_ret;							\
})

#define armor_call(nm, args...)	armor_call_body(1, nm, args)
#define armor2linuxret(ret)	((ret) == 0x6 ? 0 : 1)

/*
 * otp_access_code
 * otp access code definition
 * this enum is inherited from armor/include/otp.h
 */
enum otp_access_code{
	OTP_ACCESS_CODE_FUSE = 0,	/*read generic fuse, arg0 - fuse offset, return fuse value on success*/
	OTP_ACCESS_CODE_RSA_KEY,	/*read rsa pub_key, arg0 - phy addr of buf, return RM_OK on success*/
};

/**************************************************************************/
/* sx6 secure monitor configuration					  */
/* Give "have_armor" in cmdline to fit with secure monitor (armor), and   */
/* linux runs in non-secure then					  */
/**************************************************************************/
static int security_state = 1;	/*0 - NS, 1 - Secure*/
static int __init do_with_secure_monitor(char *str)
{
	printk(KERN_INFO "##linux start with secure monitor##\n");
	security_state = 0;
	return 0;
}
early_param("with_armor", do_with_secure_monitor);

/*
 * Return current security state
 * 0 - Non-secure, 1 - secure
 */
int get_security_state(void)
{
	return security_state;
}
EXPORT_SYMBOL(get_security_state);

/**************************************************************************/
/* sx6 l2x0 control wrapper						  */
/**************************************************************************/
#ifdef CONFIG_SIGMA_CACHE_L2X0
extern void __iomem *sx6_get_l2cache_base(void);
static void __iomem *l2c_base = NULL;
#define GET_L2C_BASE ((NULL==l2c_base) ? (l2c_base=sx6_get_l2cache_base()) : l2c_base)

void secure_l2x0_enable(void)
{
	/* Disable PL310 L2 Cache controller */
	if ( security_state ) {
		writel_relaxed(1, GET_L2C_BASE + L2X0_CTRL);
	} else {
		armor_call(set_l2_control, 1);
	}
}
void secure_l2x0_disable(void)
{
	/* Disable PL310 L2 Cache controller */
	if ( security_state ) {
		writel_relaxed(0, GET_L2C_BASE + L2X0_CTRL);
	} else {
		armor_call(set_l2_control, 0);
	}
}

void secure_l2x0_set_auxctrl(uint32_t val)
{
	/* Set L2 Cache auxiliary control register */
	if ( security_state ) {
		writel_relaxed(val, GET_L2C_BASE + L2X0_AUX_CTRL);
	} else {
		armor_call(set_l2_aux_control, val);
	}
}

void secure_l2x0_set_debug(uint32_t val)
{
	/* Program PL310 L2 Cache controller debug register */
	if ( security_state ) {
		writel_relaxed(val, GET_L2C_BASE + L2X0_DEBUG_CTRL);
	} else {
		armor_call_body(0, set_l2_debug, val); /*set_debug couldn't be interrupted by logging?!*/
	}
}

void secure_l2x0_set_prefetchctrl(uint32_t val)
{
	/* Program PL310 L2 Cache controller prefetch control register */
	if ( security_state ) {
		writel_relaxed(val, GET_L2C_BASE + L2X0_PREFETCH_CTRL);
	} else {
		armor_call(set_l2_reg, L2X0_PREFETCH_CTRL, val);
	}
}
#endif

/*
 * 1. ACTLR R/W in secure state;
 * 2. ACTLR RO in Non-secure state if NSACR.NS_SMP=0;
 * 3. ACTLR R/W in Non-secure state if NSACR.NS_SMP=1. In this case all bits are write ignored except for the SMP bit
 */
void secure_set_actlr(uint32_t val)
{
	/* Program PL310 L2 Cache controller debug register */
	if ( security_state )
		__asm__ __volatile__("mcr	p15, 0, %0, c1, c0, 1": : "r" (val):);
	else
		;				/*armor N/A yet*/
}

uint32_t secure_read_reg(uint32_t mode, uint32_t pa)
{
	SMC_DEBUG("read_reg_%s(%#x)\n", reg_access_mode(mode), pa);
	if ( security_state ) {
		if (mode == 0)
			return ReadRegByte((void*)pa);
		else if (mode == 1)
			return ReadRegHWord((void*)pa);
		else if (mode == 2)
			return ReadRegWord((void*)pa);
		else
			return 0;
	} else {
		return (uint32_t)armor_call(read_reg, mode, pa);
	}
}

void secure_write_reg(uint32_t mode, uint32_t pa, uint32_t val, uint32_t mask)
{
	SMC_DEBUG("write_reg_%s(%#x, %#x, %#x)\n", reg_access_mode(mode), pa, val, mask);
	if ( security_state ) {
		if (mode == 0) {
			if (mask != 0xff)
				return MWriteRegByte((void*)pa, (uint8_t)val, (uint8_t)mask);
			else
				return WriteRegByte((void*)pa, (uint8_t)val);
		} else if (mode == 1) {
			if (mask != 0xffff)
				return MWriteRegHWord((void*)pa, (uint16_t)val, (uint16_t)mask);
			else
				return WriteRegHWord((void*)pa, (uint16_t)val);
		} else if (mode == 2) {
			if (mask != 0xffffffff)
				return MWriteRegWord((void*)pa, val, mask);
			else
				return WriteRegWord((void*)pa, val);
		} else {
			return;
		}
	} else {
		armor_call(write_reg, mode, pa, val, mask);
	}
}

uint32_t secure_otp_read_fuse(const uint32_t offset)
{
	if ( security_state ) {
		printk(KERN_ERR "error: call to %s in Secure world!\n", __func__);
		return 0;
	}

	return (uint32_t)armor_call(otp_access, OTP_ACCESS_CODE_FUSE, offset, 0, 0);
}

uint32_t secure_otp_read_rsa_key(uint32_t *buf, uint32_t nbytes)
{
	if ( security_state ) {
		printk(KERN_ERR "error: call to %s in Secure world!\n", __func__);
		return 1;
	} else {
		uint32_t ret, a0;
		BUG_ON(buf == NULL);
		a0 = virt_to_phys((void*)buf); /*addr must in low memory*/
		outer_inv_range(a0, a0 + nbytes);
		ret = (uint32_t)armor_call(otp_access, OTP_ACCESS_CODE_RSA_KEY, a0, nbytes, 0);
		ret = armor2linuxret(ret);
		return ret;
	}
}

