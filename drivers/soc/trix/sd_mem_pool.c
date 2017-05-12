/**
  @file   sd_mem_pool.c
  @brief
	simple memory pool
	Allocate static buffer from BSS, this can guarantee buffer located
at low address.
  */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#define MAX_BUFF_NODES 	(32)
#define BUFF_SZ		(2048)
typedef struct {
	char guard1[16];
	char data[BUFF_SZ];
	char guard2[16];
}__packed __aligned(32) buff_node_t;

struct sd_mem_desc {
	struct list_head list;
	buff_node_t *n;
};

/*
 * Alloacte static buffer in BSS section
 */

static buff_node_t sd_node_pool[MAX_BUFF_NODES] __aligned(32) = { 0 };

static struct list_head free_list = LIST_HEAD_INIT(free_list);
static struct list_head inuse_list = LIST_HEAD_INIT(inuse_list);
static spinlock_t pool_lock;


void sd_pool_free(void *ptr)
{
	unsigned long flags;
	struct sd_mem_desc *desc = NULL;
	struct list_head *pos;

	BUG_ON(ptr == NULL);

	spin_lock_irqsave(&pool_lock, flags);
	WARN_ON(list_empty(&inuse_list));

	list_for_each(pos, &inuse_list) {
		desc = list_entry(pos, struct sd_mem_desc, list);
		BUG_ON((desc == NULL));
		if ((void *)&desc->n->data[0] != ptr) {
			desc = NULL;
			continue;
		} else {
			break;
		}
	}

	/*
	 * ptr not belong any descriptor ?
	 */
	if (desc == NULL) {
		WARN((desc==NULL), "%s: ptr not belong any descriptor(Double free?)\n"
			"ptr(%p)\n"
			"pool_start(%p)\n"
			"poll_end(%p)\n", __func__,
				ptr, &sd_node_pool[0], &sd_node_pool[MAX_BUFF_NODES]);

		goto out;
	}

	list_del(&desc->list);
	list_add_tail(&desc->list, &free_list);

out:
	spin_unlock_irqrestore(&pool_lock, flags);

	return;
}
EXPORT_SYMBOL(sd_pool_free);

void *sd_pool_alloc(int sz)
{
	void *ptr = NULL;
	unsigned long flags;
	struct sd_mem_desc *desc = NULL;

	BUG_ON(sz>BUFF_SZ);

	spin_lock_irqsave(&pool_lock, flags);
	if (list_empty(&free_list)) {
		goto out;
	}

	desc = list_first_entry(&free_list, struct sd_mem_desc, list);
	if (desc == NULL) {
		goto out;
	}
	list_del(&desc->list);

	list_add_tail(&desc->list, &inuse_list);
	ptr = (void *)&desc->n->data[0];

	memset(ptr, 0, sizeof(desc->n->data));

out:
	spin_unlock_irqrestore(&pool_lock, flags);
	return ptr;
}
EXPORT_SYMBOL(sd_pool_alloc);

static int __init sd_pool_init(void)
{
	int i;
	struct sd_mem_desc *desc = NULL;

	for (i=0; i<ARRAY_SIZE(sd_node_pool); i++) {
		desc = kzalloc(sizeof(struct sd_mem_desc), GFP_KERNEL);
		BUG_ON((desc == NULL));
		desc->n = &sd_node_pool[i];
		list_add_tail(&desc->list, &free_list);
	}

	spin_lock_init(&pool_lock);

	return 0;
}
arch_initcall(sd_pool_init);
