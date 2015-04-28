/*
 * Direct MTD block device access, readonly with NAND bad block skipping
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/blktrans.h>
#include <linux/mutex.h>

#define DEV_BLOCKSIZE		(512)			/* fixed by MTD block driver infrastructure */
#define CACHE_SIZE		(4 * 1024)		/* must be a multiple of DEV_BLOCKSIZE and a fraction of NAND erase block size */
#define CACHE_BASE_INVALID	((unsigned long) -1)


/*
   Pick a major device number to 'borrow'... whichever one we pick, the
   driver which really owns that number MUST NOT ALSO BE INCLUDE IN THE KERNEL !!

   31 : The offical mtdblock major number. Picking this allows the driver to be
	tested as a drop in (readonly) replacement for the normal mtdblock driver.

   44 : The mtd 'ftl.c' major number. Note that the bootloader, fstab, etc,
	etc may need tweaking to use an unexpected device...
*/

#if 0
#define ROBBS_NAME	"mtdblock"
#define ROBBS_MAJOR	31
#else
#define ROBBS_NAME	"mtdblock_robbs"
#define ROBBS_MAJOR	44
#endif


struct mtdblk_dev {
	struct mtd_blktrans_dev mbd;
	int count;
	unsigned int bblist_count;
	unsigned long *bblist_data;
	unsigned long cache_base;
	struct mutex cache_mutex;
	unsigned char cache_buffer[CACHE_SIZE];
};

static DEFINE_MUTEX(mtdblks_lock);

static int mtdblock_robbs_cache_load (struct mtdblk_dev *mtdblk, unsigned long pos_phys, unsigned long pos_virt)
{
	struct mtd_info *mtd = mtdblk->mbd.mtd;
	size_t retlen;
	int ret;

	if (pos_phys & (CACHE_SIZE - 1)) {
		printk ("%s: pos_phys (0x%08lx) not CACHE_SIZE aligned ?!?\n", __FUNCTION__, pos_phys);
		return -EIO;
	}

	mutex_lock (&mtdblk->cache_mutex);

	/*
	   Fixme: is mtd->read() error handling correct ??
	*/
	ret = mtd_read(mtd, pos_phys, CACHE_SIZE, &retlen, mtdblk->cache_buffer);
	if (ret == 0 || ret == -EUCLEAN)
		mtdblk->cache_base = (retlen == CACHE_SIZE) ? pos_phys : CACHE_BASE_INVALID;

	mutex_unlock (&mtdblk->cache_mutex);

	if (ret && ret != -EUCLEAN)
		return ret;
	if (retlen != CACHE_SIZE)
		return -EIO;

	return 0;
}

static unsigned long robbs_offset(struct mtd_blktrans_dev *dev, unsigned long pos_base)
{
	struct mtdblk_dev *mtdblk = container_of(dev, struct mtdblk_dev, mbd);
	struct mtd_info *mtd = mtdblk->mbd.mtd;
	int i;

	if (mtdblk->bblist_count != 0) {
		for (i = 0; i < mtdblk->bblist_count; i++) {
			if (pos_base < mtdblk->bblist_data[i])
				break;
			pos_base += mtd->erasesize;
			if (pos_base >= mtd->size) {
				printk ("%s: %s 0x%08lx slipped out of range (0x%08llx)\n",
					__FUNCTION__, mtd->name, pos_base, mtd->size);
				return -1;
			}
		}
	}
	return pos_base;
}

static int mtdblock_robbs_readsect (struct mtd_blktrans_dev *dev, unsigned long lba, char *buf)
{
	struct mtdblk_dev *mtdblk = container_of(dev, struct mtdblk_dev, mbd);
	struct mtd_info *mtd = mtdblk->mbd.mtd;
	unsigned long pos_abs = lba * DEV_BLOCKSIZE;
	unsigned long pos_base = (pos_abs / CACHE_SIZE) * CACHE_SIZE;
	unsigned long pos_offset = pos_abs - pos_base;
	unsigned long pos_base_original = pos_base;
	int err = 0;

	pr_debug("%s: %s at 0x%08lx (0x%08lx + %4ld)\n",
			__FUNCTION__, mtd->name, pos_abs,
			 pos_base, pos_offset);

	pos_base = robbs_offset(dev, pos_base);
	if (pos_base == (unsigned long)-1)
	{
		err = -EIO;
		goto out;
	}

	pr_debug("robbs readsect %lx: base %lx (cached %lx, bad %d), orig %lx, offset %ld\n",
		lba, pos_base, mtdblk->cache_base,
		mtdblk->bblist_count, pos_base_original, pos_offset);

	if (pos_base != mtdblk->cache_base)
	{
		err = mtdblock_robbs_cache_load (mtdblk, pos_base, pos_base_original);
		if(err)
			goto out;
	}
	memcpy (buf, mtdblk->cache_buffer + pos_offset, DEV_BLOCKSIZE);
out:
	return err;
}

static int mtdblock_robbs_open (struct mtd_blktrans_dev *mbd)
{
	struct mtdblk_dev *mtdblk = container_of(mbd, struct mtdblk_dev, mbd);
	struct mtd_info *mtd = mbd->mtd;
	unsigned long pos;
	unsigned int count;
	int ret = 0;

	/*mutex for SMP or preempt..*/
	mutex_lock(&mtdblks_lock);
	if (mtdblk->count) {
		mtdblk->count++;
		printk ("mtdblock_open: %s, count: %d\n", mtd->name, mtdblk->count);
		mutex_unlock(&mtdblks_lock);
		return 0;
	}

	pr_debug ("%s: %s, erasesize: %d, sizeof(struct mtdblk_dev): %d\n",
		__FUNCTION__, mtd->name, mtd->erasesize, sizeof(struct mtdblk_dev));

	mtdblk->bblist_count = 0;
	for (pos = 0; pos < mtd->size; pos += mtd->erasesize) {
		if (mtd_block_isbad(mtd, pos)) {
			mtdblk->bblist_count++;
			printk ("%s: badpos: 0x%08lx (count %d)\n",
				__FUNCTION__, pos, mtdblk->bblist_count);
		}
	}

	if (mtdblk->bblist_count != 0) {
		mtdblk->bblist_data = kmalloc (mtdblk->bblist_count * sizeof (*mtdblk->bblist_data), GFP_KERNEL);
		if (! mtdblk->bblist_data) {
			kfree (mtdblk);
			ret = -ENOMEM;
			goto out;
		}
		count = 0;
		for (pos = 0; pos < mtd->size; pos += mtd->erasesize) {
			if ( mtd_block_isbad(mtd, pos)) {
				count++;
				if (count > mtdblk->bblist_count) {
					printk ("%s: increase in bad block count since first pass !?!\n", __FUNCTION__);
					break;
				}
				mtdblk->bblist_data[count - 1] = pos;
			}
		}
	}

	mtdblk->cache_base = CACHE_BASE_INVALID;
	mutex_init (&mtdblk->cache_mutex);


out:
	if (0 == ret) mtdblk->count = 1;
	mutex_unlock(&mtdblks_lock);
	return ret;
}

static void mtdblock_robbs_release (struct mtd_blktrans_dev *mbd)
{
	struct mtdblk_dev *mtdblk = container_of(mbd, struct mtdblk_dev, mbd);
	//struct mtd_info *mtd = mtdblk->mbd.mtd;

	mutex_lock(&mtdblks_lock);
	if (--mtdblk->count == 0) {
		kfree (mtdblk->bblist_data);
	}
	mutex_unlock(&mtdblks_lock);

	return;
}


static void mtdblock_robbs_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
	struct mtdblk_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev)
		return;

	dev->mbd.mtd = mtd;
	dev->mbd.devnum = mtd->index;

	dev->mbd.size = mtd->size >> 9;
	dev->mbd.tr = tr;

	dev->mbd.readonly = 1;

	if (add_mtd_blktrans_dev(&dev->mbd))
		kfree(dev);
}

static void mtdblock_robbs_remove_dev(struct mtd_blktrans_dev *dev)
{
	del_mtd_blktrans_dev(dev);
}

static struct mtd_blktrans_ops mtdblock_robbs_tr =
{
	.name		= ROBBS_NAME,
	.major		= ROBBS_MAJOR,
	.part_bits	= 0,
	.blksize 	= 512,
	.open		= mtdblock_robbs_open,
	.release	= mtdblock_robbs_release,
	.readsect	= mtdblock_robbs_readsect,
	.add_mtd	= mtdblock_robbs_add_mtd,
	.remove_dev	= mtdblock_robbs_remove_dev,
	.owner		= THIS_MODULE,
};

static int __init init_mtdblock_robbs (void)
{
	return register_mtd_blktrans (&mtdblock_robbs_tr);
}

static void __exit cleanup_mtdblock_robbs (void)
{
	deregister_mtd_blktrans (&mtdblock_robbs_tr);
}

module_init(init_mtdblock_robbs);
module_exit(cleanup_mtdblock_robbs);

MODULE_LICENSE("GPL");
