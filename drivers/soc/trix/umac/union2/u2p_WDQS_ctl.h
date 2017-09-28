/*
 * Auto generated from /HAL_TEAM/shown/umac_driver/union2/bitfields/u2p_WDQS_ctl.rd,
 * DO NOT EDIT!!
 *
 * Must include stdint.h.
 * The most important part of the file is the union describing the register
 * content.
 *
 */

#ifndef __U2P_WDQS_CTL_H__
#define __U2P_WDQS_CTL_H__

#define U2P_WDQS_CTL_int_derate_mask_SHIFT 28 /* # Interrupt masks for derate_req_XX default 0x0(disabled) */
#define U2P_WDQS_CTL_int_derate_mask_WIDTH 4
#define U2P_WDQS_CTL_int_dqs_upd_mask_SHIFT 24 /* # Interrupt masks for dqs_upd_req_XX default 0xf(enabled) */
#define U2P_WDQS_CTL_int_dqs_upd_mask_WIDTH 4
#define U2P_WDQS_CTL_derate_req_11_SHIFT 15 /* # CHB RANK1 TUF is set */
#define U2P_WDQS_CTL_derate_req_11_WIDTH 1
#define U2P_WDQS_CTL_derate_req_10_SHIFT 14 /* # CHA RANK1 TUF is set */
#define U2P_WDQS_CTL_derate_req_10_WIDTH 1
#define U2P_WDQS_CTL_derate_req_01_SHIFT 13 /* # CHB RANK0 TUF is set */
#define U2P_WDQS_CTL_derate_req_01_WIDTH 1
#define U2P_WDQS_CTL_derate_req_00_SHIFT 12 /* # CHA RANK0 TUF is set */
#define U2P_WDQS_CTL_derate_req_00_WIDTH 1
#define U2P_WDQS_CTL_dqs_upd_req_11_SHIFT 11 /* # CHB RANK1 drift count exceed threshold of WDQS_tol */
#define U2P_WDQS_CTL_dqs_upd_req_11_WIDTH 1
#define U2P_WDQS_CTL_dqs_upd_req_10_SHIFT 10 /* # CHA RANK1 drift count exceed threshold of WDQS_tol */
#define U2P_WDQS_CTL_dqs_upd_req_10_WIDTH 1
#define U2P_WDQS_CTL_dqs_upd_req_01_SHIFT 9 /* # CHB RANK0 drift count exceed threshold of WDQS_tol */
#define U2P_WDQS_CTL_dqs_upd_req_01_WIDTH 1
#define U2P_WDQS_CTL_dqs_upd_req_00_SHIFT 8 /* # CHA RANK0 drift count exceed threshold of WDQS_tol */
#define U2P_WDQS_CTL_dqs_upd_req_00_WIDTH 1
#define U2P_WDQS_CTL_pctl_bg_task_en_SHIFT 4 /* # Enable PCTL to collect MR information */
#define U2P_WDQS_CTL_pctl_bg_task_en_WIDTH 1
#define U2P_WDQS_CTL_int_en_SHIFT 1 /* # Interrupt enable to UMAC */
#define U2P_WDQS_CTL_int_en_WIDTH 1
#define U2P_WDQS_CTL_restart_SHIFT 0 /* # Retrigger programming of LPDDR4 MR23 and refersh u2p_MR18_19_ref */
#define U2P_WDQS_CTL_restart_WIDTH 1

#ifndef __ASSEMBLY__

union U2P_WDQS_CTLReg {
        struct { __extension__ uint32_t // lsbs...
                restart: 1  /* # Retrigger programming of LPDDR4 MR23 and refersh u2p_MR18_19_ref */,
                int_en: 1  /* # Interrupt enable to UMAC */,
                                                hole4: 2,
                pctl_bg_task_en: 1  /* # Enable PCTL to collect MR information */,
                                                hole8: 3,
                dqs_upd_req_00: 1  /* # CHA RANK0 drift count exceed threshold of WDQS_tol */,
                dqs_upd_req_01: 1  /* # CHB RANK0 drift count exceed threshold of WDQS_tol */,
                dqs_upd_req_10: 1  /* # CHA RANK1 drift count exceed threshold of WDQS_tol */,
                dqs_upd_req_11: 1  /* # CHB RANK1 drift count exceed threshold of WDQS_tol */,
                derate_req_00: 1  /* # CHA RANK0 TUF is set */,
                derate_req_01: 1  /* # CHB RANK0 TUF is set */,
                derate_req_10: 1  /* # CHA RANK1 TUF is set */,
                derate_req_11: 1  /* # CHB RANK1 TUF is set */,
                                                hole24: 8,
                int_dqs_upd_mask: 4  /* # Interrupt masks for dqs_upd_req_XX default 0xf(enabled) */,
                int_derate_mask: 4  /* # Interrupt masks for derate_req_XX default 0x0(disabled) */; // ... to msbs
        } bits;

        uint32_t val;
};

#endif /* !__ASSEMBLY__ */
#endif /* __U2P_WDQS_CTL_H__ */
