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

static int chip_id = CHIP_UNKNOWN;

int dtv_get_chip_id(void)
{
	if (CHIP_UNKNOWN == chip_id)
	{
		unsigned int val;
		val = ReadRegWord((volatile void*)CHIP_ID_REG);
		if (GETBITS(val, 8, 8) == SX6_CHIP_ID)
		{
			if (GETBITS(val, 0, 3) == 0)
				chip_id = CHIP_SX6_REVA;
			else if(GETBITS(val, 0, 3) == 1)
				chip_id = CHIP_SX6_REVB;
			else
				chip_id = CHIP_SX6_NEW;
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
		case(CHIP_SX6_REVA): nm = ID2NAME(CHIP_SX6_REVA); break;
		case(CHIP_SX6_REVB): nm = ID2NAME(CHIP_SX6_REVB); break;
		case(CHIP_SX6_NEW): nm = ID2NAME(CHIP_SX6_NEW); break;
		default: nm = ID2NAME(CHIP_UNKNOWN);break;
		#undef ID2NAME
	}
	return nm;
}

