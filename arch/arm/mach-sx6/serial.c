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
 *  Author: Qipeng Zha, 2012.
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/serial_reg.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/serial_core.h>
#include <linux/irq.h>

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



void sigma_serial_init(void);

static inline unsigned char serial_read(int offset)
{
	unsigned int addr;

	addr = SERIAL_BASE + offset;

	return readb((volatile void*)addr);
	
}

static inline void serial_write(unsigned char val, int offset)
{
	unsigned int addr;

	addr = SERIAL_BASE + offset;
	writeb(val, (volatile void*)addr);
}

/*init serial, io_offset: offset of virt to phy serial registers*/
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

/*delay*/
static inline void simple_delay(int loop)
{
        volatile int i = 0;

	while(i++ < loop);
}

static void printch(const unsigned char c)
{
	unsigned char ch;

#ifdef CONFIG_SIGMA_DBG
	int timeout = 0;
#define DBG_TIMEOUT 0x20
#endif
	do {
		/*read UART line status register*/
		//use MAG's macro, endian issues
		ch = serial_read(SER_CMD);   //MAG
#ifdef CONFIG_SIGMA_DBG
		simple_delay(0x10);
		if(timeout ++ > DBG_TIMEOUT)
			break;
#endif
		
	} while (0 == (ch & TX_BUSY));
	
	//use MAG's macro, endian issues
	if(c=='\n')
		serial_write('\r', SER_DATA);	//MAG

	serial_write(c, SER_DATA);		//MAG
	
}

const char digits[16] = "0123456789abcdef";
void put32(unsigned int u)
{
	int cnt;
	unsigned ch;

	cnt = 8;		/* 8 nibbles in a 32 bit long */

	printch('0');
	printch('x');
	do {
		cnt--;
		ch = (unsigned char) (u >> cnt * 4) & 0x0F;
		printch(digits[ch]);
	} while (cnt > 0);
	printch(' ');

}

void serial_puts(const char *cp)
{
	unsigned char ch;
	int i = 0;

	while (*cp) {
		do {
			/*read UART line status register*/
			ch = serial_read(SER_CMD);  	//MAG
			i++;
			if (i > TIMEOUT) {
				break;
			}
		} while (0 == (ch & TX_BUSY));
		
		if(*cp=='\n')
		{
			serial_write('\r',SER_DATA);		//MAG
			serial_write('\n',SER_DATA);	//MAG
		}
		else
		{
			serial_write(*cp, SER_DATA);	//MAG
		}
		cp++;
	}
	
	printch('\n');
	return ;
}
