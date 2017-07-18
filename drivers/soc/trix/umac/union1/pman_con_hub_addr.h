/*
 * Auto generated from drivers/soc/trix/umac/union1/bitfields/pman_con_hub_addr.rd,
 * DO NOT EDIT!!
 *
 * Must include stdint.h.
 * The most important part of the file is the union describing the register
 * content.
 *
 */

#ifndef __PMAN_CON_HUB_ADDR_H__
#define __PMAN_CON_HUB_ADDR_H__

#define PMAN_CON_HUB_ADDR_rank_SHIFT 0 /* #[3:0] -> physicall [31:28] */
#define PMAN_CON_HUB_ADDR_rank_WIDTH 4

#ifndef __ASSEMBLY__

union PMAN_CON_HUB_ADDRReg {
        struct { __extension__ uint32_t // lsbs...
                rank: 4  /* #[3:0] -> physicall [31:28] */,
                                                hole0: 28; // ... to msbs
        } bits;

        uint32_t val;
};

#endif /* !__ASSEMBLY__ */
#endif /* __PMAN_CON_HUB_ADDR_H__ */
