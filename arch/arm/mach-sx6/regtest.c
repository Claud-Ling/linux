/*
 *  linux/arch/arm/mach-trix/regtest.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file contains a generic register access interface for debug purpose.
 *
 * Author: Tony He, 2015.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#ifdef CONFIG_PROC_FS

static void *local_ioremap(uint32_t pa)
{
	if (!SIGMA_IO_TO_VIRT_BETWEEN(pa, SIGMA_IO_0_BASE_PHYS, SIGMA_IO_0_SIZE) &&
	    !SIGMA_IO_TO_VIRT_BETWEEN(pa, SIGMA_IO_ARM_BASE_PHYS, SIGMA_IO_ARM_SIZE))
		return ioremap(pa, SZ_4K);
	else
		return (void*)pa;
}

static void local_iounmap(void* va)
{
	if (!SIGMA_IO_TO_VIRT_BETWEEN((uint32_t)va, SIGMA_IO_0_BASE_PHYS, SIGMA_IO_0_SIZE) &&
	    !SIGMA_IO_TO_VIRT_BETWEEN((uint32_t)va, SIGMA_IO_ARM_BASE_PHYS, SIGMA_IO_ARM_SIZE))
		iounmap(va);
}

static ssize_t proc_regtest_set(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
#define BUFFER_LENGTH 64
	char cmd[BUFFER_LENGTH] = {0}, *p, *q;
	uint32_t addr, val;
	int op = -1, mode = -1;
	void *vaddr = NULL;

	if(count > BUFFER_LENGTH - 1)
		count = BUFFER_LENGTH - 1;

	if(copy_from_user(cmd, buffer, count))
		goto out;

	p = strim(cmd);
	if ((q = strpbrk(p, " \t")) == NULL)
		goto out;

	*q = '\0';
	if (strcmp(p, "regtest")) {
		goto out;
	}

	p = skip_spaces(q + 1);
	if ((q = strpbrk(p, " \t")) == NULL)
		goto out;

	*q = '\0';
	if (0 == strcasecmp(p, "w")) {
		op = 1;	/*write*/
		p++;
	} else if (0 == strcasecmp(p, "r")) {
		op = 0; /*read*/
		p++;
	} else {
		/*unknown op code*/
		goto out;
	}

	p = skip_spaces(q + 1);
	if ((q = strpbrk(p, " \t")) == NULL)
		goto out;

	*q = '\0';
	if (0 == strcasecmp(p, "w")) {
		mode = 2;	/*word*/
	} else if (0 == strcasecmp(p, "hw")) {
		mode = 1;	/*halfword*/
	} else if (0 == strcasecmp(p, "b")) {
		mode = 0;	/*byte*/
	} else {
		/*unknown mode code*/
		goto out;
	}

	p = skip_spaces(q + 1);
	addr = simple_strtoul(p, &q, 16);
	vaddr = local_ioremap(addr);

	if (1 == op) {
		uint32_t mask = -1u;
		if ('\0' == *q) {
			local_iounmap(vaddr);
			vaddr = NULL;
			goto out;
		}

		p = skip_spaces(q);
		val = simple_strtoul(p, &q, 16);
		if (p != q && *skip_spaces(q) != '\0') {
			p = skip_spaces(q);
			mask = simple_strtoul(p, &q, 16);
		}

		printk(KERN_INFO "proc_write_uint%d(%08x,%08x,%08x)\n", 8<<mode, addr, val, mask);
		switch(mode) {
		case 0:	MWriteRegByte(vaddr, val, mask); break;
		case 1:	MWriteRegHWord(vaddr, val, mask); break;
		case 2:	MWriteRegWord(vaddr, val, mask); break;
		default:
			break;
		}
	} else {
		printk(KERN_INFO "proc_read_uint%d(%08x)\n", 8<<mode, addr);
		switch(mode) {
		case 0: val = ReadRegByte(vaddr); break;
		case 1: val = ReadRegHWord(vaddr); break;
		case 2: val = ReadRegWord(vaddr); break;
		default:
			break;
		}
		printk("0x%08x\n", val);
	}

	local_iounmap(vaddr);
	vaddr = NULL;
	return count;
out:
	return -EFAULT;
}

static int proc_regtest_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Generic register read/write interface\n");
	seq_printf(m, "Usage: echo \"regtest <r,w> <b,hw,w> <addr> [val] [mask]\" > /proc/regtest \n");
	return 0;
}
static int proc_regtest_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_regtest_show, NULL);
}

static const struct file_operations regtest_ops = {
	.open           = proc_regtest_open,
	.read           = seq_read,
	.write		= proc_regtest_set,
	.llseek         = seq_lseek,
	.release        = seq_release,
};
#endif

static int __init proc_regtest_init(void)
{
#ifdef CONFIG_PROC_FS
	proc_create("regtest", 0, NULL, &regtest_ops);
#endif
	return 0;
}

late_initcall(proc_regtest_init);
