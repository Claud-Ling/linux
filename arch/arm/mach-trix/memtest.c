/* test memory in veloce*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <asm/tlb.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <asm/mach/map.h>

#define TEST_MEMORY_START	0xc1000000
#define TEST_MEMORY_SIZE	0x600000
#define TEST_MEMORY_COPYS	0x3

#define TEST_LOOPS		100


int sigma_mem_test(void)
{

	unsigned int src, dst, size;
	int ret;
	int loop = TEST_LOOPS;

	while(loop-- > 0)
	{
		int copy = TEST_MEMORY_COPYS;

		src = TEST_MEMORY_START;
		size = TEST_MEMORY_SIZE;
		dst = src + size;

		printk("%d:  %08x %08x %08x\n",loop, dst,src,ret);
		while(copy-- >0)
		{
			memcpy((void*)dst, (void*)src, size);
			if((ret = memcmp((const void *)dst, (const void *)src, size)))
			{
				printk("Error %d %d: %08x %08x %08x\n",loop,copy, dst,src,ret);
				while(1);
			}
	
			src += size;
			dst += size;
		}

	}

	printk("OK: %d %08x %08x %08x\n",loop, dst,src,ret);
	while(1);

}
