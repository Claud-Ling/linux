/************************************************************************/
/* ECC module define                                                    */
/************************************************************************/
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>

#define	ETIME		62	/* Timer expired */
#define	EIO		 5	/* I/O error */

#define MLC_ECC_REG_CONTROL     0xFB002070
#define MLC_ECC_REG_STATUS      0xFB002074

#define MLC_ECC_CONTROL_DECODER_CLEAR    0x08
#define MLC_ECC_CONTROL_DECODER_START    0x04
#define MLC_ECC_CONTROL_ENCODER_CLEAR    0x02
#define MLC_ECC_CONTROL_ENCODER_START    0x01


#define REG_BCH_ECC_BASE                0xFB002200
#define REG_BCH_ECC_MAP(x)		        (REG_BCH_ECC_BASE + x)
            
#define REG_BCH_ECC_CONTROL             REG_BCH_ECC_MAP(0x00)
#define REG_BCH_ECC_STATUS              REG_BCH_ECC_MAP(0x04)
                                            
#define REG_BCH_ECC_DATA_0              REG_BCH_ECC_MAP(0x10)
#define REG_BCH_ECC_DATA_1              REG_BCH_ECC_MAP(0x14)
#define REG_BCH_ECC_DATA_2              REG_BCH_ECC_MAP(0x18)
#define REG_BCH_ECC_DATA_3              REG_BCH_ECC_MAP(0x1C)
#define REG_BCH_ECC_DATA_4              REG_BCH_ECC_MAP(0x20)
#define REG_BCH_ECC_DATA_5              REG_BCH_ECC_MAP(0x24)
#define REG_BCH_ECC_DATA_6              REG_BCH_ECC_MAP(0x28)
#define REG_BCH_ECC_DATA_7              REG_BCH_ECC_MAP(0x2C)
#define REG_BCH_ECC_DATA_8              REG_BCH_ECC_MAP(0x30)
#define REG_BCH_ECC_DATA_9              REG_BCH_ECC_MAP(0x34)
#define REG_BCH_ECC_DATA_a              REG_BCH_ECC_MAP(0x38)
#define REG_BCH_ECC_DATA_b              REG_BCH_ECC_MAP(0x3C)

#define REG_BCH_ECC_CORRECTION_0        REG_BCH_ECC_MAP(0x60)
/*
#define REG_BCH_ECC_CORRECTION_1        REG_BCH_ECC_MAP(0x54)
#define REG_BCH_ECC_CORRECTION_2        REG_BCH_ECC_MAP(0x58)
#define REG_BCH_ECC_CORRECTION_3        REG_BCH_ECC_MAP(0x5C)
#define REG_BCH_ECC_CORRECTION_4        REG_BCH_ECC_MAP(0x60)
#define REG_BCH_ECC_CORRECTION_5        REG_BCH_ECC_MAP(0x64)
#define REG_BCH_ECC_CORRECTION_6        REG_BCH_ECC_MAP(0x68)
#define REG_BCH_ECC_CORRECTION_7        REG_BCH_ECC_MAP(0x6C)
*/
#define REG_BCH_ECC_INT_SET_ENABLE      REG_BCH_ECC_MAP(0xDC)

/*
 * ECC_CONTROL
 */
#define ECC_CONTROL_CLEAR               (1<<8)
#define ECC_CONTROL_PARITY_MODE         (1<<6)      // O:DECODE, 1:ENCODE
#define ECC_CONTROL_MODE_OPERATION      (3<<4)      // 00:4 BITS ECC, 01:8 BITS ECC
#define ECC_CONTROL_MODE_OPERATION_4BITS      (3<<4)      // 00:4 BITS ECC, 01:8 BITS ECC
#define ECC_CONTROL_MODE_OPERATION_8BITS      (1<<4)      // 00:4 BITS ECC, 01:8 BITS ECC
#define ECC_CONTROL_PAYLOAD_SIZE        (3<<2)      // 01:512 BYTES
#define ECC_CONTROL_PAYLOAD_SIZE_512B   (1<<2)      // 01:512 BYTES
#define ECC_CONTROL_ENABLE_NFC          (1<<1)
#define ECC_CONTROL_ENABLE_CALC         (1<<0)

/*
 * ECC_STATUS
 */
#define ECC_STATUS_NUM_CORRECTIONS      (0x3f<<20)
#define ECC_STATUS_ERASED_PAGE          (1<<19)
#define ECC_STATUS_UNCORRECTABLE_ERR    (1<<18)
#define ECC_STATUS_CORRECTION_VALID     (1<<17)
#define ECC_STATUS_PARITY_VALID         (1<<16)
#define ECC_STATUS_WORD_COUNT           (0xffff<<0)

#define MONZA_ECC_BUSY_WAIT_TIMEOUT (1 * HZ) 

void bch_encode_start(struct nand_chip *chip)
{
	unsigned int ecc_unit  = (chip->ecc.size >> 9) & 0x3; /*[3:2] 01-512bytes 10-1024bytes*/
	unsigned int ecc_level = chip->ecc.strength & 0x1F;   /*[9:4] ecc level*/	

	// enable the MLC encoder
	WriteRegWord((void*)MLC_ECC_REG_CONTROL, 0x101);
	
	WriteRegWord((void*)REG_BCH_ECC_CONTROL, 0x403 + (ecc_unit<<2) + (ecc_level<<4));
}

int bch_encode_end(unsigned int ecc[],struct nand_chip *chip)
{
	int err = 0;
	int i;

	unsigned int times = chip->ecc.bytes >> 2;
	unsigned short ecc_remain;
	unsigned long deadline = jiffies + MONZA_ECC_BUSY_WAIT_TIMEOUT;	
	
	do{
		if(ReadRegWord((void*)REG_BCH_ECC_STATUS) & ECC_STATUS_PARITY_VALID)
			break;
		else
			cond_resched();
	}while (!time_after_eq(jiffies, deadline));    

	if (time_after_eq(jiffies, deadline))
	{
		printk("ecc encode timed out\n");
		err = -ETIMEDOUT;
		goto exit;
	}

	for(i = 0; i < times; i ++)
		ecc[i] = ReadRegWord( (void*)REG_BCH_ECC_DATA_0 + (i<<2) );

	ecc_remain = (unsigned short)ReadRegWord((void*)(REG_BCH_ECC_DATA_0 + (i<<2)))&0x0000ffff;
	*(unsigned short *)(ecc+i) = ecc_remain;	
	 
exit:
    // clear MCL encoder
    WriteRegWord((void*)MLC_ECC_REG_CONTROL, 0x102);
    
    return err;
}

static int get_bitflip_count(unsigned int value)
{
	int count;

	for (count = 0; value; value >>= 1)
		 count += value & 1;

	return count;	
}

/* Count the number of 0's in buff upto a max of max_bits */
static int count_written_bits(uint8_t *buff, int size, int max_bits)
{
        int k, written_bits = 0;

        for (k = 0; k < size; k++) {
                written_bits += hweight8(~buff[k]);
                if (written_bits > max_bits)
                        break;
        }

        return written_bits;
}

void bch_decode_start(unsigned int ecc[],struct nand_chip * chip)
{
    // enable the MLC decoder
	unsigned int i;
	unsigned int ecc_unit  = (chip->ecc.size >> 9) & 0x3; /*[3:2] 01-512bytes 10-1024bytes*/
	unsigned int ecc_level = chip->ecc.strength & 0x1F;   /*[9:4] ecc level*/	
	unsigned int times     = chip->ecc.bytes >> 2;
	unsigned short ecc_remain;

	WriteRegWord((void*)REG_BCH_ECC_CONTROL, 0x3 + (ecc_unit<<2) + (ecc_level<<4));
	
	WriteRegWord((void*)MLC_ECC_REG_CONTROL, 0x108);
	WriteRegWord((void*)MLC_ECC_REG_CONTROL, 0x104);
	
	for(i = 0; i < times; i ++)
		WriteRegWord((void*)(REG_BCH_ECC_DATA_0 + (i<<2)), ecc[i]);

	ecc_remain = *((unsigned short *)(ecc+i));
	WriteRegWord((void*)(REG_BCH_ECC_DATA_0 + (i<<2)), ecc_remain);
}


int bch_decode_end_correct(struct mtd_info *mtd,unsigned char* pBuff,struct nand_chip * chip)
{
	int err = 0;
	unsigned int status;
	unsigned int bitnum;
	
	uint8_t *read_ecc = chip->buffers->ecccode;  
	unsigned short* pwBuff;
	unsigned int dwCorrection;
	unsigned int correction;
	unsigned long deadline = jiffies + MONZA_ECC_BUSY_WAIT_TIMEOUT;
	
	do{
		if(ReadRegWord((void*)REG_BCH_ECC_STATUS) & ECC_STATUS_CORRECTION_VALID)
			break;
		else
			cond_resched();
	}while (!time_after_eq(jiffies, deadline));    

	if (time_after_eq(jiffies, deadline))
	{
		printk("ecc decode timed out\n");
		err = -ETIMEDOUT;
		goto exit;
	}
	
	//ecc check 
	status = ReadRegWord((void*)REG_BCH_ECC_STATUS);
	if (unlikely(status & ECC_STATUS_UNCORRECTABLE_ERR))
	{
                /*
                 * This is a temporary erase check. A newly erased page read
                 * would result in an ecc error because the oob data is also
                 * erased to FF and the calculated ecc for an FF data is not
                 * FF..FF.
                 * This is a workaround to skip performing correction in case
                 * data is FF..FF
                 *
                 * Logic:
                 * For every page, each bit written as 0 is counted until these
                 * number of bits are greater than ecc level(8bits,24bits) (the maximum correction
                 * capability of ECC for each page + oob bytes)
                 */
		int bits_ecc = count_written_bits(read_ecc, chip->ecc.bytes, chip->ecc.strength);
		int bits_data = count_written_bits(pBuff, chip->ecc.size, chip->ecc.strength);
		if ((bits_ecc + bits_data) <= chip->ecc.strength) 
		{
			if (bits_data){
				memset(pBuff, 0xFF, chip->ecc.size);
				mtd->ecc_stats.corrected += bits_ecc;
			}
			goto exit;
		}	
		
		//err = -EIO;  
		mtd->ecc_stats.failed++;
		printk("BCH: uncorrectable error,status 0x%x \n",status);
		goto exit;
	}

	bitnum = (status & ECC_STATUS_NUM_CORRECTIONS)>>20; 
	if (bitnum != 0)
	{
		pwBuff = (unsigned short*)pBuff;
		dwCorrection = REG_BCH_ECC_CORRECTION_0;
		do
		{
			correction = ReadRegWord((void*)dwCorrection);
			if (correction == 0)
			{	
				break;
			}
			pwBuff[(correction>>16)&0x3ff] ^= (correction & 0xffff);
			dwCorrection += 4;
		/*
		 *bitflips occur,record the num of bitflips.
		 *register REG_BCH_ECC_CORRECTION_x[15:0] is the mask of the bit to correct.
		 *This is intended to be used as an XOR of the data read from nand device.
		 */
		 mtd->ecc_stats.corrected += get_bitflip_count(correction & 0xffff);
		} while(1);
	}
    
exit:
    // clear MCL decode
    WriteRegWord((void*)MLC_ECC_REG_CONTROL, 0x108);
    
    return err;
}

