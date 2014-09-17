/*
 *  linux/arch/arm/mach-sx6/chipver.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SigmaDesigns SX6 soc chip version
 *
 * Author: Tony He, 2014.
 */

#define pr_fmt(fmt) "chipver: " fmt

#include <linux/kernel.h>
#include <asm/io.h>
#include "chipver.h"

#define GETBITS(RegValue, StartBit, Bits) (((RegValue) >> (StartBit)) & ((0x1<<(Bits))-1))

#define CHIP_ID_REG 0x1500e000

#define SX6_CHIP_ID 0x96
#define SX7_CHIP_ID 0x97

static int chip_id = CHIP_UNKNOWN;

int dtv_get_chip_id(void)
{
	if (CHIP_UNKNOWN == chip_id)
	{
		unsigned int val;
		val = ReadRegWord((volatile void*)CHIP_ID_REG);
		switch (GETBITS(val, 8, 8)) {
			#define CID_CASE(n)				\
			case n##_CHIP_ID:				\
				if (GETBITS(val, 0, 3) == 0)		\
					chip_id = CHIP_##n##_REVA;	\
				else if(GETBITS(val, 0, 3) == 1)	\
					chip_id = CHIP_##n##_REVB;	\
				else if(GETBITS(val, 0, 3) == 2)	\
					chip_id = CHIP_##n##_REVC;	\
				else					\
					chip_id = CHIP_##n##_NEW;	\
				break

			CID_CASE(SX6);
			CID_CASE(SX7);

			default: chip_id = CHIP_UNKNOWN; break;
			#undef CID_CASE
		}
		pr_debug("%s\n", dtv_chip_name(chip_id));
	}
	return chip_id;
}

const char* dtv_chip_name(int chip_id)
{
	const char* nm = NULL;
	switch(chip_id)
	{
		#define ID2NAME(id) #id
		#define CNM_CASE(n)						\
		case(CHIP_##n##_REVA): nm = ID2NAME(CHIP_##n##_REVA); break;	\
		case(CHIP_##n##_REVB): nm = ID2NAME(CHIP_##n##_REVB); break;	\
		case(CHIP_##n##_REVC): nm = ID2NAME(CHIP_##n##_REVC); break;	\
		case(CHIP_##n##_NEW): nm = ID2NAME(CHIP_##n##_NEW); break

		CNM_CASE(SX6);
		CNM_CASE(SX7);

		default: nm = ID2NAME(CHIP_UNKNOWN);break;
		#undef ID2NAME
		#undef CNM_CASE
	}
	return nm;
}

