/*
 *  drivers/cpufreq/trix-pll-stub.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * stub pll implementation for SigmaDesigns SXx serials SoCs
 *
 * Author: Tony He, JUL 2016.
 */
/********************************************************/

/*
 * the stub implementation of trix_get_pll_ops
 */
static struct pll_operations* trix_get_pll_ops(void)
{
	return NULL;
}
