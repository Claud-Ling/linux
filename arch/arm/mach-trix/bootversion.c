#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

extern char boot_revision_info[];
static int bootversion_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", boot_revision_info);
	return 0;
}

static int bootversion_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, bootversion_proc_show, NULL);
}

static const struct file_operations bootversion_proc_fops = {
	.open		= bootversion_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_bootversion_init(void)
{
	proc_create("bootversion", 0, NULL, &bootversion_proc_fops);
	return 0;
}
module_init(proc_bootversion_init);
