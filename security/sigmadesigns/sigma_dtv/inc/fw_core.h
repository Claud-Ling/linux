#ifndef __SIGMA_FW_CORE_H__
#define __SIGMA_FW_CORE_H__
#ifndef SEEK_SET
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include "flash_op.h"
#include "klib.h"

//#define DEBUG
#ifdef DEBUG
#define fw_debug(fmt,args...)	printk(fmt,##args)
#else
#define fw_debug(fmt,args...) do{}while(0)
#endif
#define fw_err(fmt,args...)	printk(fmt,##args)
#define fw_msg(fmt,args...)	printk(fmt,##args)

#define malloc(x) vmalloc(x)
#define free(x) vfree(x)
#define local_set_bit(nr, addr)		set_bit(nr, addr)
#define local_clear_bit(nr, addr)	clear_bit(nr, addr)
#define local_test_bit(nr, addr)	test_bit(nr, addr)

#define FW_INFO_VER(x, y)	({		\
	('F'<<24)+('H'<<16)+((x)<<8)+(y);	\
})

#define VOL_HEAD_VER(x, y)	({		\
	('V'<<24)+('H'<<16)+((x)<<8)+(y);	\
})

#define VOL_INFO_VER(x, y)	({		\
	('V'<<24)+('I'<<16)+((x)<<8)+(y);	\
})

#define PART_INFO_VER(x, y)	({		\
	('P'<<24)+('I'<<16)+((x)<<8)+(y);	\
})


#define INFO_TO_VALID_NUM(__inf)			({	\
	struct fw_head *__h = (struct fw_head *)__inf;		\
	__h->valid;						\
})

#define EMMC_FW_INFO_BASE	0x0		/*@0M*/
#define NAND_FW_INFO_BASE	0x1b00000	/*@27MB*/
#define INFO_PART_SZ		0x100000	/* 1MB */
//#define INFO_PART_SZ		0x1000	/* 1MB */
#define INFO_MAX_VALID_NUM	0xff		/*255*/


#define INC_VALID_COUNT(_cnt_)			({	\
	if((_cnt_) < INFO_MAX_VALID_NUM) {		\
		(_cnt_) = ((_cnt_) + 1);		\
	} else {					\
		(_cnt_) = 0;				\
	}						\
})

enum {
	FW_VOL_CONTENT_INUSE = 0,
	FW_VOL_CONTENT_FREE = 1,
};

struct fw_head {
	uint32_t magic;
	uint32_t crc;
	uint32_t len;
	uint32_t valid;
	uint32_t n_bdy;
	uint32_t bdy[0];
}__attribute__ ((packed));

struct fw_ctx {
	struct list_head vols;
	uint32_t max_vol_idx;
	uint64_t free_space_addr;
	uint32_t valid;
	uint32_t idx;
	void *fop;
};

struct vol_head {
	uint32_t magic;
	uint32_t n_entry;
}__attribute__ ((packed));

struct fw_vol {
	struct list_head list;
	struct list_head parts;
	struct vol_info *info;
};


struct vol_info {
	uint32_t magic;
	char name[64];
	char fs[64];
	uint32_t index;
	uint64_t dep_vol;
	uint32_t ablities;
	uint32_t active_ablities;
	uint64_t start;
	uint64_t sz_perpart;
	uint32_t nr_parts;
	uint32_t active_part;
	uint8_t p_info[0];
}__attribute__ ((packed));


struct fw_part {
	struct list_head list;
	struct part_info *info;
};

struct part_info {
	uint32_t magic;
	char name[64];
	uint32_t index;
	uint64_t start;
	uint64_t size;
/* Read-only content start, protected by info_sig */
	uint32_t need_sig;
	uint64_t valid_sz;
	uint8_t part_sig[256];
	uint8_t info_sig[256];
}__attribute__ ((packed));


typedef enum {
	FW_AB_BOOT = 0,
	FW_AB_MBOOT = 1
}fw_ability;

typedef enum {
	FW_BOOT_ACT = 0,
	FW_MBOOT_ACT = 1,
}fw_abl_active;
extern int32_t fw_vol_set_abilities(struct fw_vol *vol, fw_ability ability);
extern int32_t fw_vol_clr_abilities(struct fw_vol *vol, fw_ability ability);
extern int32_t fw_vol_tst_abilities(struct fw_vol *vol, fw_abl_active active);
extern int32_t fw_vol_set_abl_active(struct fw_vol *vol, fw_abl_active active);
extern int32_t fw_vol_clr_abl_active(struct fw_vol *vol, fw_abl_active active);
extern int32_t fw_vol_tst_abl_active(struct fw_vol *vol, fw_abl_active active);

extern void fw_vol_set_fs(struct fw_vol *vol, const char *fs);
extern void fw_vol_set_signature_en(struct fw_vol *vol);
extern int32_t fw_vol_set_depdent(struct fw_vol *vol, struct fw_vol *dep);

extern struct fw_vol *fw_get_vol_by_name(struct fw_ctx *ctx, const char *name);
extern struct fw_vol *fw_get_vol_by_ability_enabled(struct fw_ctx *ctx,
						fw_ability ab, fw_abl_active act);

extern struct fw_vol *fw_get_dep_vol(struct fw_ctx *ctx, struct fw_vol *vol);

extern struct fw_part * fw_vol_get_active_part(struct fw_vol *vol);
extern struct fw_part *fw_vol_get_inactive_part(struct fw_vol *vol);
extern struct fw_part * fw_vol_get_part_by_id(struct fw_vol *vol, uint32_t id);
extern void fw_vol_set_active_part(struct fw_vol *vol, struct fw_part *part);

extern int32_t fw_open_volume(struct fw_ctx *ctx, const char *name, uint32_t flags);
extern int64_t fw_write_volume(struct fw_ctx  *ctx, void *buff, uint64_t len);
extern int64_t fw_read_volume(struct fw_ctx *ctx, void *buff, uint64_t len);
extern void fw_close_volume(struct fw_ctx  *ctx);
extern struct fw_vol * fw_alloc_vol(struct fw_ctx *ctx, const char *name,  uint64_t sz, uint32_t nr_parts);
extern int32_t fw_add_volume(struct fw_ctx *ctx, struct fw_vol *vol);

extern struct fw_ctx *fw_ctx_alloc(void);
extern void fw_ctx_free(struct fw_ctx *ctx);
extern struct fw_ctx *fw_get_board_ctx(void);
extern int32_t fw_ctx_save_to_flash(struct fw_ctx *ctx);
extern struct fw_ctx *fw_board_ctx_reinit(void);
extern void fw_deinit(struct fw_ctx *ctx);

extern void fw_dump(struct fw_ctx *ctx);

extern char *volume_name_to_dev(struct fw_ctx *ctx, const char *name, uint32_t flags);
extern char *fw_get_part_list(struct fw_ctx *ctx);

extern int32_t fdetecter_main_storage_typ(void);
extern int32_t fdetecter_boot_storage_typ(void);


#if !defined(__KERNEL__)
extern int32_t fw_update_bootloader(struct fw_ctx *ctx, char *image);
extern struct fw_ctx *fw_init(const char *file);
extern struct fw_ctx *fw_ctx_init_from_file(const char *file);
extern int32_t fw_ctx_save_to_file(struct fw_ctx *ctx, const char *file);

extern int32_t fw_updater_ctx_restore(const char *file);
extern int32_t fw_updater_ctx_save(const char *file);

#endif /* !__KERNEL__ */

#endif /* __SIGMA_FW_CORE_H__ */
