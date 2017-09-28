/*
 * Auto generated from /HAL_TEAM/shown/umac_driver/union2/umac_dev.md
 * DO NOT EDIT!
 *
 * Must include stdint.h.
 * The most important part of the file is the struct describing the registers
 * file content.
 *
 */

#ifndef __UMAC_DEV_H__
#define __UMAC_DEV_H__

#include <u2p_MR18_19_shad.h>
#include <u2p_MR18_19_ref.h>
#include <u2p_WDQS_ctl.h>
#include <u2p_WDQS_tol.h>
#include <u2p_DQSDQCR.h>
#define MCTL_IF_ENABLE_OFS 0x0000
#define MCTL_MODULE_ID_OFS 0x0ffc
#define U2P_CONFIG_REG_0_OFS 0x1000
#define U2P_LPDDR_TRN_CTL_OFS 0x1014
#define U2P_MR23_SHAD_OFS 0x1060
#define U2P_MR4_SHAD_OFS 0x1064
#define U2P_MR18_19_SHAD_CHA_OFS 0x1068
#define U2P_MR18_19_SHAD_CHB_OFS 0x106c
#define U2P_MR18_19_REF_CHA_OFS 0x1070
#define U2P_MR18_19_REF_CHB_OFS 0x1074
#define U2P_WDQS_CTL_OFS 0x1078
#define U2P_WDQS_TOL_A0_OFS 0x107c
#define U2P_WDQS_TOL_B0_OFS 0x1080
#define U2P_WDQS_TOL_A1_OFS 0x1084
#define U2P_WDQS_TOL_B1_OFS 0x1088
#define U2P_DQSDQCR_SHAD_OFS 0x108c
#define U2P_DERATE_CTL_OFS 0x1090
#define U2P_PRDTRN_CTL0_OFS 0x1094
#define U2P_PRDTRN_CTL1_OFS 0x1098
#define U2P_PHY_OPER_MODE_OFS 0x1200
#define U2P_PHY_OPER_STAT_OFS 0x1204
#define U2P_DQSDQCR_OFS 0x138c

#ifndef __ASSEMBLY__
struct umac_ctl {
	volatile uint32_t	mctl_if_enable;	/* +0x0000*/
	volatile uint8_t 	hole0[4088];
	volatile uint32_t	mctl_module_id;	/* +0x0ffc*/
	volatile uint32_t	u2p_CONFIG_REG_0;	/* +0x1000*/
	volatile uint8_t 	hole1[16];
	volatile uint32_t	u2p_LPDDR_TRN_CTL;	/* +0x1014*/
	volatile uint8_t 	hole2[72];
	volatile uint32_t	u2p_MR23_shad;	/* +0x1060*/
	volatile uint32_t	u2p_MR4_shad;	/* +0x1064*/
	volatile union U2P_MR18_19_SHADReg	u2p_MR18_19_shad_CHA;	/* +0x1068*/
	volatile union U2P_MR18_19_SHADReg	u2p_MR18_19_shad_CHB;	/* +0x106c*/
	volatile union U2P_MR18_19_REFReg	u2p_MR18_19_ref_CHA;	/* +0x1070*/
	volatile union U2P_MR18_19_REFReg	u2p_MR18_19_ref_CHB;	/* +0x1074*/
	volatile union U2P_WDQS_CTLReg	u2p_WDQS_CTL;	/* +0x1078*/
	volatile union U2P_WDQS_TOLReg	u2p_WDQS_tol_A0;	/* +0x107c*/
	volatile union U2P_WDQS_TOLReg	u2p_WDQS_tol_B0;	/* +0x1080*/
	volatile union U2P_WDQS_TOLReg	u2p_WDQS_tol_A1;	/* +0x1084*/
	volatile union U2P_WDQS_TOLReg	u2p_WDQS_tol_B1;	/* +0x1088*/
	volatile union U2P_DQSDQCRReg	u2p_DQSDQCR_shad;	/* +0x108c*/
	volatile uint32_t	u2p_DERATE_CTL;	/* +0x1090*/
	volatile uint32_t	u2p_PRDTRN_CTL0;	/* +0x1094*/
	volatile uint32_t	u2p_PRDTRN_CTL1;	/* +0x1098*/
	volatile uint8_t 	hole3[356];
	volatile uint32_t	u2p_PHY_OPER_MODE;	/* +0x1200*/
	volatile uint32_t	u2p_PHY_OPER_STAT;	/* +0x1204*/
	volatile uint8_t 	hole4[388];
	volatile union U2P_DQSDQCRReg	u2p_DQSDQCR;	/* +0x138c*/
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

#define FN_UMAC0_SET_ADDR
static inline void umac0_set_addr(struct pman_con *p, uint32_t val)
{
	p->hub0_saddr0.bits.rank = val; return;
}
#endif /* !__ASSEMBLY__ */
#endif
