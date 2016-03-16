/******************************************
 * Copyright 2015
 * Sigma Designs, Inc. All Rights Reserved
 * Proprietary and Confidential
 ******************************************/

/**
 * @file	proc_fuse.h
 * @brief	it declares fuse interfaces over proc file system (eFuse)
 *
 * 	prototypes:
 * 	int fuse_read_data_NAME(void *buf, int buflen);
 * 	int fuse_read_field_NAME(int *ptr);
 *
 * 	for instance:
 * 	int fuse_read_data_RSA_PUB_KEY(void *buf, int buflen);
 * 	int fuse_read_data_FC_0(void *buf, int buflen);
 * 	int fuse_read_field_EDR_EN(int *ptr);
 *
 * 	while user program could make use of the interface as shown below:
 * 	fuse_read_data_RSA_PUB_KEY(buf, len) OR fuse_read_data(RSA_PUB_KEY, buf, len);
 * 	fuse_read_data_FC_0(buf, 4) OR fuse_read_data(FC_0, buf, 4) OR fuse_read_entry(FC_0, buf);
 * 	fuse_read_field_EDR_EN(ptr) OR fuse_read_field(EDR_EN, ptr);
 *
 * @author	Tony He
 * @date	2015-10-30
 */

#ifndef __ASM_ARCH_TRIX_TEST_PROC_FUSE_H__
#define __ASM_ARCH_TRIX_TEST_PROC_FUSE_H__

/*
 * @fn	int proc_fuse_read_generic(const char* name, const char* field, void *buf, const int len);
 * @brief	read fuse data from proc file system. Rarely used outside the scope of this file.
 * @param[in]	name	specify fuse data name
 * @param[in]	field	specify field name of the fuse, given NULL to return whole fuse data
 * @param[in]	buf	pointer of buffer
 * @param[in]	len	number of bytes data to read
 *
 * @return	the number of bytes data read, or <0 for error
 */
extern int proc_fuse_read_generic(const char* name, const char* field, void *buf, const int len);

/*
 * @fn	int fuse_read_data_NAME(void* buf, const int buflen);
 * @brief	read fuse data specified by #NAME from proc file system,
 * 		and fill data in <buf> and set length in <buflen> on success.
 * @param[in]	buf	pointer of buffer
 * @param[in]	buflen	length of buffer pointed by <buf>
 * @return	the number of bytes data read, or <0 for error
 *
 */
#define OTP_FUSE_DATA(nm, sz)						\
static inline int fuse_read_data_##nm (void *buf, const int buflen)	\
{									\
	if (buflen >= (sz))						\
		return proc_fuse_read_generic(#nm, NULL, buf, (sz));	\
	else								\
		return -1;						\
}									\

#define OTP_FUSE_ENTRY(nm) OTP_FUSE_DATA(nm, 4)

/*
 * @fn	int fuse_read_field_NAME(int *ptr);
 * @brief	read fuse field value specified by #NAME from proc file system,
 * 		and set value to integer pointer by <ptr> on success.
 * @param[in]	ptr	pointer of integer
 * @return	non-negative value on success, <0 for error
 *
 */
#define OTP_FUSE_FIELD(entry, fn)					\
static inline int fuse_read_field_##fn (int *ptr)			\
{									\
	return proc_fuse_read_generic(#entry, #fn, ptr, 4);		\
}

#include "autofuse.h"

#undef OTP_FUSE_DATA
#undef OTP_FUSE_ENTRY
#undef OTP_FUSE_FIELD

/*
 * variants for user to choose
 */
/* read generic fuse array #nm */
#define fuse_read_data(nm, buf, len) fuse_read_data_##nm (buf, len)
/* read 4-bytes fuse array #nm */
#define fuse_read_entry(nm, buf) fuse_read_data_##nm (buf, 4)
/* read fuse field #nm */
#define fuse_read_field(nm, ptr) fuse_read_field_##nm (ptr)

#endif
