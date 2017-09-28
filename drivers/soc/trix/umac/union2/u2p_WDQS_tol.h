/*
 * Auto generated from /HAL_TEAM/shown/umac_driver/union2/bitfields/u2p_WDQS_tol.rd,
 * DO NOT EDIT!!
 *
 * Must include stdint.h.
 * The most important part of the file is the union describing the register
 * content.
 *
 */

#ifndef __U2P_WDQS_TOL_H__
#define __U2P_WDQS_TOL_H__

#define U2P_WDQS_TOL_max_pos_drift_SHIFT 16 /* # The threshold of positive DQS drift(more delay) direction */
#define U2P_WDQS_TOL_max_pos_drift_WIDTH 16
#define U2P_WDQS_TOL_max_neg_drift_SHIFT 0 /* # The threshold of negative DQS drift(less delay) direction */
#define U2P_WDQS_TOL_max_neg_drift_WIDTH 16

#ifndef __ASSEMBLY__

union U2P_WDQS_TOLReg {
        struct { __extension__ uint32_t // lsbs...
                max_neg_drift: 16  /* # The threshold of negative DQS drift(less delay) direction */,
                max_pos_drift: 16  /* # The threshold of positive DQS drift(more delay) direction */; // ... to msbs
        } bits;

        uint32_t val;
};

#endif /* !__ASSEMBLY__ */
#endif /* __U2P_WDQS_TOL_H__ */
