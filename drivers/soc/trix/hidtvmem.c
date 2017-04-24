/*
 * mapping MMIO and physical memory to user process
 *
 * Copyright (c) 2017 SigmaDesigns, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#define pr_fmt(fmt)	"DRV_NAME: " fmt

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>

#include <linux/uaccess.h>

#define DRV_NAME	"hidtvmem"
#define SIGMA_DTV_MMIO_MASK	(0xF0000000UL)

static int major;
static struct class *hidtvmem_class;

static inline int is_mmio_pfn(unsigned long pfn)
{
	u64 addr = ((u64)pfn) << PAGE_SHIFT;

	if ((addr & SIGMA_DTV_MMIO_MASK) == SIGMA_DTV_MMIO_MASK)
		return 1;
	else
		return 0;
}

static inline int range_is_allowed(unsigned long pfn, unsigned long size)
{
	u64 from = ((u64)pfn) << PAGE_SHIFT;
	u64 to = from + size;
	u64 cursor = from;
	int last = is_mmio_pfn(pfn);

	/*
	 * reject the mapping request in case:
	 * 1. attempt to kernel memory domain
	 * 2. the mapping across boundary of ddr and mmio
	 */
	while (cursor < to) {
		if (pfn_valid(pfn))
			return 0;

		if (is_mmio_pfn(pfn) != last)
			return 0;

		cursor += PAGE_SIZE;
		pfn++;
	}

	return 1;
}

/*
 * We have two types of memory type in system:
 * - normal memory
 * - device memory(MMIO)
 * maps one of two different memory areas depending
 * on the pgoff of the mmap
 */
static int hidtvmem_mmap(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;

	pr_debug("%s: mapping 0x%x bytes at address 0x%lx prot:0x%llx flag: 0x%lx\n", __func__,
					(unsigned)size, vma->vm_pgoff,
					(u64)pgprot_val(vma->vm_page_prot), vma->vm_flags);

	if (!range_is_allowed(vma->vm_pgoff, size))
		return -EPERM;

	if (is_mmio_pfn(vma->vm_pgoff)) {
		/* Device memory*/
		vma->vm_page_prot = pgprot_device(vma->vm_page_prot);
	} else if (file->f_flags & O_SYNC) {
		/* Normal uncacheable memory */
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	} else {
		/* Normal cacheable memory */
	}

	/* Remap-pfn-range will mark the range VM_IO */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

static const struct file_operations hidtvmem_fops = {
	.mmap = hidtvmem_mmap,
};

static int __init hidtvmem_cdev_init(void)
{
	struct device *dev;

	/* Create device class needed by udev */
	hidtvmem_class = class_create(THIS_MODULE, DRV_NAME);
	if (IS_ERR(hidtvmem_class)) {
		return PTR_ERR(hidtvmem_class);
	}

	major = register_chrdev(0, DRV_NAME, &hidtvmem_fops);
	if (major < 0) {
		printk(KERN_WARNING DRV_NAME
			 ": could not get major number\n");
		class_destroy(hidtvmem_class);
		return major;
	}

	dev = device_create(hidtvmem_class, NULL, MKDEV(major, 0), NULL, DRV_NAME);
	if (IS_ERR(dev)) {
		unregister_chrdev(major, DRV_NAME);
		class_destroy(hidtvmem_class);
		return PTR_ERR(dev);
	}

	return 0;
}

static void __exit hidtvmem_cdev_exit(void)
{
	device_destroy(hidtvmem_class, MKDEV(major, 0));
	class_destroy(hidtvmem_class);
	unregister_chrdev(major, DRV_NAME);
}

module_init(hidtvmem_cdev_init);
module_exit(hidtvmem_cdev_exit);

MODULE_DESCRIPTION("HiDTV memory Access Driver");
MODULE_LICENSE("Dual BSD/GPL");
