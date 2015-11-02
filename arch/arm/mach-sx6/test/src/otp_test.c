#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "proc_fuse.h"

#define fuse_data(nm, buf, len) fuse_read_data_##nm(buf, len)
#define fuse_field(nm, p)	fuse_read_field_##nm(p)


int main(int argc, char* argv[])
{
	int i, field = 0, ret;
	char *nm = NULL;
	unsigned char buf[300];
	uint32_t val;

	for (i = 0; i < argc; i++) {
		if (!strcmp("-f", argv[i])) {
			field = 1;
			argc--;
			argv++;
		}
	}

	if (field) {
		ret = fuse_field(EMMC_BIT_WIDTH, &val);
		printf("ret = %d, val = %08x\n", ret, val);
	} else {
		ret = fuse_data(FC_2, &val, 4);
		printf("FC_2, ret = %d, val = %08x\n", ret, val);
		ret = fuse_data(RSA_PUB_KEY, buf, 300);
		printf("RSA_PUB_KEY, ret = %d:\n", ret);
		for (i = 0; i < ret && i < sizeof(buf); i++)
			printf("%02x", buf[i]);
		printf("\n");
	}
	return 0;

}
