/*
 * Auto generated from /HAL_TEAM/shown/umac_driver/union2/bitfields/u2p_DQSDQCR.rd,
 * DO NOT EDIT!!
 *
 * Must include stdint.h.
 * The most important part of the file is the union describing the register
 * content.
 *
 */

#ifndef __U2P_DQSDQCR_H__
#define __U2P_DQSDQCR_H__

#define U2P_DQSDQCR_dir_SHIFT 28 /* # In manual update, setting this bit to 1 would add 'dlyoffs' to WDQ delays from u2p_PTSR_XX */
#define U2P_DQSDQCR_dir_WIDTH 1
#define U2P_DQSDQCR_dlymax_SHIFT 20 /* # Maximum scan steps for DQS2DQ training session. */
#define U2P_DQSDQCR_dlymax_WIDTH 8
#define U2P_DQSDQCR_mpcrpt_SHIFT 16 /* # The number of consecutive MPC FIFO write/read before checking the data during retraining. */
#define U2P_DQSDQCR_mpcrpt_WIDTH 3
#define U2P_DQSDQCR_mupd_SHIFT 14 /* # Manual update enable. Shall be set to 0 for retraining. */
#define U2P_DQSDQCR_mupd_WIDTH 1
#define U2P_DQSDQCR_ranksel_SHIFT 12 /* # 01:rank0, 10:rank1, 11:both */
#define U2P_DQSDQCR_ranksel_WIDTH 2
#define U2P_DQSDQCR_dqsel_SHIFT 8 /* # Each bit update corresponding data module, e.g. 0011: data module 0,1 */
#define U2P_DQSDQCR_dqsel_WIDTH 4
#define U2P_DQSDQCR_dlyoffs_SHIFT 0 /* # tDQSDQ drift amount in unit of DRAM clock phase */
#define U2P_DQSDQCR_dlyoffs_WIDTH 8

#ifndef __ASSEMBLY__

union U2P_DQSDQCRReg {
        struct { __extension__ uint32_t // lsbs...
                dlyoffs: 8  /* # tDQSDQ drift amount in unit of DRAM clock phase */,
                dqsel: 4  /* # Each bit update corresponding data module, e.g. 0011: data module 0,1 */,
                ranksel: 2  /* # 01:rank0, 10:rank1, 11:both */,
                mupd: 1  /* # Manual update enable. Shall be set to 0 for retraining. */,
                                                hole16: 1,
                mpcrpt: 3  /* # The number of consecutive MPC FIFO write/read before checking the data during retraining. */,
                                                hole20: 1,
                dlymax: 8  /* # Maximum scan steps for DQS2DQ training session. */,
                dir: 1  /* # In manual update, setting this bit to 1 would add 'dlyoffs' to WDQ delays from u2p_PTSR_XX */,
                                                hole0: 3; // ... to msbs
        } bits;

        uint32_t val;
};

#endif /* !__ASSEMBLY__ */
#endif /* __U2P_DQSDQCR_H__ */
