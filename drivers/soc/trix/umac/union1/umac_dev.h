/*
 * Auto generated from drivers/soc/trix/umac/union1/umac_dev.md
 * DO NOT EDIT!
 *
 * Must include stdint.h.
 * The most important part of the file is the struct describing the registers
 * file content.
 *
 */

#ifndef __UMAC_DEV_H__
#define __UMAC_DEV_H__

#define MCTL_IF_ENABLE_OFS 0x0000
#define MCTL_MODULE_ID_OFS 0x0ffc
#define PCTL_SCFG_OFS 0x1000
#define PCTL_SCTL_OFS 0x1004
#define PCTL_STAT_OFS 0x1008
#define PCTL_TREFI_OFS 0x10d0
#define PUB_PIDR_OFS 0x1400
#define PUB_PIR_OFS 0x1404
#define PUB_ACIOCR_OFS 0x1438
#define PUB_TR_ADDR0_OFS 0x146c
#define PUB_TR_ADDR1_OFS 0x146c
#define PUB_TR_ADDR2_OFS 0x146c
#define PUB_TR_ADDR3_OFS 0x146c
#define PUB_DX0LCDCLR1_OFS 0x15e4
#define PUB_DX1LCDCLR1_OFS 0x1624
#define PUB_DX2LCDCLR1_OFS 0x1664
#define PUB_DX3LCDCLR1_OFS 0x16a4
#define PUB_AACR_OFS 0x1574

#ifndef __ASSEMBLY__
struct umac_ctl {
	volatile uint32_t	mctl_if_enable;	/* +0x0000*/
	volatile uint8_t 	hole0[4088];
	volatile uint32_t	mctl_module_id;	/* +0x0ffc*/
	volatile uint32_t	pctl_scfg;	/* +0x1000*/
	volatile uint32_t	pctl_sctl;	/* +0x1004*/
	volatile uint32_t	pctl_stat;	/* +0x1008*/
	volatile uint8_t 	hole1[196];
	volatile uint32_t	pctl_trefi;	/* +0x10d0*/
	volatile uint8_t 	hole2[812];
	volatile uint32_t	pub_pidr;	/* +0x1400*/
	volatile uint32_t	pub_pir;	/* +0x1404*/
	volatile uint8_t 	hole3[48];
	volatile uint32_t	pub_aciocr;	/* +0x1438*/
	volatile uint8_t 	hole4[48];
	volatile uint32_t	pub_tr_addr0;	/* +0x146c*/
	volatile uint32_t	pub_tr_addr1;	/* +0x146c*/
	volatile uint32_t	pub_tr_addr2;	/* +0x146c*/
	volatile uint32_t	pub_tr_addr3;	/* +0x146c*/
	volatile uint8_t 	hole5[360];
	volatile uint32_t	pub_dx0lcdclr1;	/* +0x15e4*/
	volatile uint8_t 	hole6[60];
	volatile uint32_t	pub_dx1lcdclr1;	/* +0x1624*/
	volatile uint8_t 	hole7[60];
	volatile uint32_t	pub_dx2lcdclr1;	/* +0x1664*/
	volatile uint8_t 	hole8[60];
	volatile uint32_t	pub_dx3lcdclr1;	/* +0x16a4*/
	volatile uint32_t	pub_aacr;	/* +0x1574*/
};
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__

#define FN_UMAC_IS_ACTIVE
static inline uint32_t umac_is_active(struct umac_ctl *p)
{
	return p->mctl_if_enable;
}
#endif /* !__ASSEMBLY__ */
#include <pman_con_hub_addr.h>
#define HUB_RESET_OFS 0x0000
#define HUB0_SADDR0_OFS 0x0034
#define HUB0_EADDR0_OFS 0x0038

#ifndef __ASSEMBLY__
struct pman_con {
	volatile uint32_t	hub_reset;	/* +0x0000*/
	volatile uint8_t 	hole0[48];
	volatile union PMAN_CON_HUB_ADDRReg	hub0_saddr0;	/* +0x0034*/
	volatile union PMAN_CON_HUB_ADDRReg	hub0_eaddr0;	/* +0x0038*/
};
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__

#define FN_UMAC0_START_ADDR
static inline uint32_t umac0_start_addr(struct pman_con *p)
{
	return p->hub0_saddr0.bits.rank;
}
#endif /* !__ASSEMBLY__ */
#endif
