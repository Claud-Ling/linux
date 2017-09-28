/*
 * Auto generated from /HAL_TEAM/shown/umac_driver/union2/bitfields/u2p_MR18_19_ref.rd,
 * DO NOT EDIT!!
 *
 * Must include stdint.h.
 * The most important part of the file is the union describing the register
 * content.
 *
 */

#ifndef __U2P_MR18_19_REF_H__
#define __U2P_MR18_19_REF_H__

#define U2P_MR18_19_REF_rank1_SHIFT 16 /* #[31:16] -> MR19_rank1_ref[31:24] + MR18_rank1_ref[23:16] */
#define U2P_MR18_19_REF_rank1_WIDTH 16
#define U2P_MR18_19_REF_rank0_SHIFT 0 /* #[15:0] -> MR19_rank0_ref[15:8] + MR18_rank0_ref[7:0] */
#define U2P_MR18_19_REF_rank0_WIDTH 16

#ifndef __ASSEMBLY__

union U2P_MR18_19_REFReg {
        struct { __extension__ uint32_t // lsbs...
                rank0: 16  /* #[15:0] -> MR19_rank0_ref[15:8] + MR18_rank0_ref[7:0] */,
                rank1: 16  /* #[31:16] -> MR19_rank1_ref[31:24] + MR18_rank1_ref[23:16] */; // ... to msbs
        } bits;

        uint32_t val;
};

#endif /* !__ASSEMBLY__ */
#endif /* __U2P_MR18_19_REF_H__ */
