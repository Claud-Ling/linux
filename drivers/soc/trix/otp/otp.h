/*
 * otp.c
 * - interface for accessing OTP(One Time Programable) bits
 * - expose SoC's OTP configuration in procfs(read-only, under /proc/otp/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ARCH_SIGMA_DTV_TRIX_OTP_H__
#define __ARCH_SIGMA_DTV_TRIX_OTP_H__

#include <linux/types.h>
#include <linux/list.h>

struct trix_otp;

struct trix_otp_soc {
	struct trix_otp *host;

	void (*init)(struct trix_otp *host);

	/*
	 * SoC specific operations:
	 * @get_security_boot_state:  get security boot state from OTP
	 * @get_new_nand_sel_state:   get new NAND selection state from OTP
	 * @get_rsa_key_index:        get index value of current using RSA public key
	 * @get_rsa_key:              read RSA public key from OTP
	 */
	bool (*get_security_boot_state)(struct trix_otp *host);
	bool (*get_new_nand_sel_state)(struct trix_otp *host);
	int  (*get_rsa_key_index)(struct trix_otp *host);
	int (*get_rsa_key)(struct trix_otp *host, uint32_t *buf, uint32_t nbytes);
};

struct trix_otp {
	struct trix_otp_soc *soc;
	int (*read)(struct trix_otp *otp, const uint32_t offset, uint32_t *pval, uint32_t *pprot);
	int (*read_array)(struct trix_otp *otp, const uint32_t ofs, uint32_t *buf, const uint32_t nbytes, uint32_t *pprot);

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *proc_dir;
	struct list_head fuse_map_list;
#endif
};

extern const struct trix_otp_soc sxx_fuse_soc;

#endif /* __ARCH_SIGMA_DTV_TRIX_OTP_H__  */
