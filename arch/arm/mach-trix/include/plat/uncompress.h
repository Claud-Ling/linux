/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */

#ifndef __ASM_ARCH_TRIX_PLAT_UNCOMPRESS_H__
#define __ASM_ARCH_TRIX_PLAT_UNCOMPRESS_H__

#include <linux/types.h>
#include <linux/serial_reg.h>

#include <asm/memory.h>
#include <asm/mach-types.h>

//serial registers
#define SERIAL_BASE   0xfb005100	 
#define SER_CMD       5
#define SER_DATA      0x00
#define RX_READY      0x01
#define TX_BUSY       0x20

/* uart data register */
#define UART_UTBR	0	/*DLAB = 0*/

/* uart control register */
#define UART_UIER	1	/*DLAB = 0*/
#define UART_UFCR	2
#define UART_ULCR	3
#define UART_UMCR	4

#define UART_UDLL	0	/*DLAB = 1*/
#define UART_UDLM	1	/*DLAB = 2*/

/*uart status register */
#define UART_ULSR	5
#define UART_UMSR	6
#define UART_USCR	7

#define TIMEOUT       	0xfffff	

/*offset of virt to phy io*/
unsigned int sigma_io_offset = 0;

void sigma_serial_init(void);

static inline unsigned char serial_read(int offset)
{
	volatile unsigned char * addr = (volatile unsigned char *) (sigma_io_offset + SERIAL_BASE + offset);
	return *addr;	
}

static inline void serial_write(unsigned char val, int offset)
{
	volatile unsigned char * addr = (volatile unsigned char *) (sigma_io_offset + SERIAL_BASE + offset);
	 *addr = val;	
}

void sigma_serial_init(void)
{
	unsigned char val;

	
	serial_write(0x3, UART_ULCR); 	//each character, 8 bits, stop bit 1 bit
	serial_write(0xc7,UART_UFCR);	//FIFO trigger level: 14 bytes, enable FIFO and clear FIFO

	val = serial_read(UART_ULCR);	
	serial_write(val | 0x80, UART_ULCR);	//set DLAB=1

	serial_write(1, UART_UDLL);		//set baudrate=115200
	serial_write(0, UART_UDLM);	
	serial_write(0x06, UART_UMCR);

	val = serial_read(UART_ULCR);	
	serial_write(val & (~0x80), UART_ULCR);	//set DLAB=1
}

void putc(const unsigned char c)
{
	unsigned char ch;

	do {
		/*read UART line status register*/
		//use MAG's macro, endian issues
		ch = serial_read(SER_CMD);   //MAG
	} while (0 == (ch & TX_BUSY));
	
	//use MAG's macro, endian issues
	if(c=='\n')
		serial_write('\r', SER_DATA);	//MAG

	serial_write(c, SER_DATA);		//MAG
	
}

static inline void flush(void)
{
}



#define arch_decomp_setup()	sigma_serial_init()

/*
 * nothing to do
 */
#define arch_decomp_wdog()

#endif /*__ASM_ARCH_TRIX_PLAT_UNCOMPRESS_H__*/
