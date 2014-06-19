#ifndef __MONZA_NAND_H__
#define __MONZA_NAND_H__

#define BASE                 (CONFIG_MTD_NAND_PHYSMAP_START + 0x2000)  /* here goes nand reg base from top config */

#define NAND_BOOTROM_R_TIMG  (BASE + 0x00000000) /* boot rom read timing control register */
#define NAND_FCR             (BASE + 0x00000004) /* flash rom control register */
#define NAND_DCR             (BASE + 0x00000008) /* data control register */
#define NAND_TCR             (BASE + 0x0000000c) /* timing control register */
#define NAND_SYN_MODE0       (BASE + 0x00000010) /* sync mode control 0 register */
#define NAND_SYN_MODE1	     (BASE + 0x00000014) /* sync mode control 1 register */
#define NAND_SPI_MODE_CMD    (BASE + 0x00000018) /* sync mode control 2 register */
#define NAND_OP_MODES        (BASE + 0x0000001c) /* */

#define SHIFT_WORK_MODE(a)   ((a) & 0x0000000f) << 26)
#define SHIFT_RD_FIN_SW(a)   ((a) & 0x00000001) << 25)
#define SHIFT_WR_FIN_SW(a)   ((a) & 0x00000001) << 24)
#define SHIFT_RD_BACK_SW(a)  ((a) & 0x000000ff) << 15)
#define SHIFT_CS_HI_TIME(a)  ((a) & 0x000000ff) << 8 )
#define SHIFT_SPI_TX_SW(a)   ((a) & 0x000000ff)      )

#define NAND_SPI_DLY_CTRL    (BASE + 0x0000020) /* SPI mode clock delay control register */
#define NAND_SPI_TIMG_CTRL0  (BASE + 0x0000024) /* SPI mode timing control register 0 */
#define NAND_SPI_TIMG_CTRL1  (BASE + 0x0000028) /* SPI mode timing control register 1 */
#define NAND_BASE_ADDR0      (BASE + 0x000002c) /* base address register 0 */
#define NAND_BASE_ADDR1      (BASE + 0x0000030) /* base address register 1 */
#define NAND_BURST_TIMG      (BASE + 0x0000034) /* flashrom burst timing control register */
#define NAND_BASE_ADDR2      (BASE + 0x0000038) /* base address register 2 */
#define NAND_BASE_ADDR3      (BASE + 0x000003c) /* base address register 3 */
#define NAND_BASE_ADDR4      (BASE + 0x0000040) /* base address register 4 */

#define NAND_ONE_DATA_ECC0   (BASE + 0x00000044) /* onenand flash data spcae ecc registe 0 */
#define NAND_ONE_DATA_ECC1   (BASE + 0x00000048) /* onenand flash data spcae ecc registe 1 */
#define NAND_ONE_DATA_ECC2   (BASE + 0x0000004c) /* onenand flash data spcae ecc registe 2 */
#define NAND_ONE_SPR_ECC0    (BASE + 0x00000050) /* onenand flash spare space ecc register 0 */
#define NAND_ONE_SPR_ECC1    (BASE + 0x00000054) /* onenand flash spare space ecc register 1 */

#define NAND_CTRL            (BASE + 0x00000058) /* nand flash control register */
#define NAND_TIMG            (BASE + 0x0000005c) /* nand flash timing register */

/* ---------------------------------------------------------------------
   address hole in the middle
   ---------------------------------------------------------------------*/

#define NAND_MLC_ECC_CTRL            (BASE + 0x00000070) /* MLC ECC control register */
#define NAND_MLC_ESS_STATUS          (BASE + 0x00000074) /* MLC ECC status register */
#define NAND_MLC_ECC_DATA            (BASE + 0x00000078) /* MLC ECC data register */
#define NAND_MLC_ECC_ENC_DATA1       (BASE + 0x0000007c) /* MLC ECC encoder data register 1*/
#define NAND_MLC_ECC_DEC_ERR_POS0    (BASE + 0x00000080) /* MLC ECC decoder error position egister 0*/
#define NAND_MLC_ECC_DEC_ERR_POS1    (BASE + 0x00000084) /* MLC ECC decoder error position egister 1*/

#define NAND_ECC_CTRL                (BASE + 0x00000200) /* ECC control register */



#define NAND_SW_MODE	0
#define NAND_HW_MODE	0x44 

#endif
