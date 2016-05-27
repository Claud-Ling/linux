/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/err.h>
#include "umac.h"

#ifdef CONFIG_PROC_FS
static int umac_states_open(struct inode *node, struct file *filp)
{
	return 0;
}

static ssize_t umac_states_read(struct file *filp, 
			char __user *buffer, size_t count, loff_t *offp)
{
#define MAX_STATE_STR_LEN 32
	int id, len;
	unsigned long cp_sz;
	char states_str[MAX_STATE_STR_LEN * CONFIG_SIGMA_NR_UMACS];
	char work_str[MAX_STATE_STR_LEN];

	states_str[0] = '\0';
	for (id = 0; id < CONFIG_SIGMA_NR_UMACS; id++) {
		snprintf(work_str, MAX_STATE_STR_LEN, "umac%d: %s\n",
			id, umac_is_activated(id) ? "on" : "off");
		strcat(states_str, work_str);
	}

	len = strlen(states_str);
	if (*offp) {
		*offp = 0;
		return 0;
	}

	if (count > len) {
		cp_sz = copy_to_user(buffer, states_str, len);
	} else {
		cp_sz = copy_to_user(buffer, states_str, count);
		len = count;
	}

	*offp += len;
	return len;
}

static ssize_t umac_states_write(struct file *filp, 
			const char __user *buffer, size_t count, loff_t *offp)
{
	unsigned long id, cp_sz = 0;
	char umac_str[32];

	if (count > sizeof(umac_str) - 1)
		return -EINVAL;

	cp_sz = copy_from_user(umac_str, buffer, count);
	id = simple_strtoul(umac_str, NULL, 0);
	if (id < CONFIG_SIGMA_NR_UMACS)
		pr_info("umac%ld: %s\n", id, umac_is_activated(id)?"on":"off");
	else
		pr_err("invalid umac id %ld\n", id);

	return count;
}

static int umac_states_release(struct inode *node, struct file *filp)
{
	return 0;
}

static struct file_operations umac_states_ops = {
	.open = umac_states_open,
	.read = umac_states_read,
	.write = umac_states_write,
	.release = umac_states_release,
};

struct umac_proc_dir {
	struct proc_dir_entry *root;
	struct proc_dir_entry *states_entry;
};

static struct umac_proc_dir umac_proc = { NULL };

static int __init trix_umac_proc_init(void)
{
	umac_proc.root = proc_mkdir("umac", NULL);
	if (!umac_proc.root) {
		pr_err("failed to create /proc/umac\n");
		return -1;
	}

	umac_proc.states_entry = \
		proc_create("states",0666,umac_proc.root,&umac_states_ops);
	if (!umac_proc.states_entry) {
		pr_err("failed to create /proc/umac/states\n");
	}
	return 0;
}

static void __exit trix_umac_proc_exit(void)
{
	if (umac_proc.states_entry != NULL) {
		proc_remove(umac_proc.states_entry);
		umac_proc.states_entry = NULL;
	}
	if (umac_proc.root != NULL) {
		proc_remove(umac_proc.root);
		umac_proc.root = NULL;
	}
	return;
}

MODULE_LICENSE("GPL");
late_initcall(trix_umac_proc_init);
module_exit(trix_umac_proc_exit);
#endif	/* CONFIG_PROC_FS */
