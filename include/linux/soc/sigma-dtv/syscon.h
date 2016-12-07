/*
 * System Control Driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LINUX_SIGMA_DTV_SYSCON_H__
#define __LINUX_SIGMA_DTV_SYSCON_H__

#include <linux/err.h>

struct device_node;

#ifdef CONFIG_SYSCON_TRIX
extern void __iomem *syscon_node_to_iomap(struct device_node *np);
extern void __iomem *syscon_iomap_lookup_by_compatible(const char *s);
extern void __iomem *syscon_iomap_lookup_by_pdevname(const char *s);
extern void __iomem *syscon_iomap_lookup_by_phandle(
					struct device_node *np,
					const char *property);
#else
static inline void __iomem *syscon_node_to_regmap(struct device_node *np)
{
	return ERR_PTR(-ENOSYS);
}

static inline void __iomem *syscon_iomap_lookup_by_compatible(const char *s)
{
	return ERR_PTR(-ENOSYS);
}

static inline void __iomem *syscon_iomap_lookup_by_pdevname(const char *s)
{
	return ERR_PTR(-ENOSYS);
}

static inline void __iomem *syscon_iomap_lookup_by_phandle(
					struct device_node *np,
					const char *property)
{
	return ERR_PTR(-ENOSYS);
}
#endif

#endif /* __LINUX_SIGMA_DTV_SYSCON_H__ */
