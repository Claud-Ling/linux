#ifndef __DRIVERS_MTD_NAND_TRIX_ECC_H__
#define __DRIVERS_MTD_NAND_TRIX_ECC_H__

#include <linux/mtd/mtd.h>

void bch_encode_start(struct nand_chip *chip);
int bch_encode_end(unsigned int ecc[],struct nand_chip *chip);
void bch_decode_start(unsigned int ecc[],struct nand_chip *chip);
int bch_decode_end_correct_old(struct mtd_info *mtd,unsigned char* pBuff,struct nand_chip *chip);
int bch_decode_end_correct_new(struct mtd_info *mtd,unsigned char* pBuff,struct nand_chip *chip);

#endif //__DRIVERS_MTD_NAND_TRIX_ECC_H__
