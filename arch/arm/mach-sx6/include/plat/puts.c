
#include "trihidtv.h"

//serial
#define SERIAL_BASE   0xBB005100	 
#define SER_CMD       		5
#define SER_DATA      0x00
#define RX_READY      0x01
#define TX_BUSY       	0x20

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
#define KSEG1 		0xa0000000
void Init_Serial();

void Init_Serial()
{
	BYTE tmp;

	WriteRegByte(0x03, (SERIAL_BASE+UART_ULCR)|KSEG1);	//each character, 8 bits, stop bit 1 bit
	WriteRegByte(0xc7, (SERIAL_BASE+UART_UFCR)|KSEG1);	//FIFO trigger level: 14 bytes, enable FIFO and clear FIFO
	tmp=ReadRegByte((SERIAL_BASE+UART_ULCR)|KSEG1);	
	WriteRegByte(tmp|0x80, (SERIAL_BASE+UART_ULCR)|KSEG1);	//set DLAB=1
	WriteRegByte(1, (SERIAL_BASE+UART_UDLL)|KSEG1);		//set baudrate=115200
	WriteRegByte(0, (SERIAL_BASE+UART_UDLM)|KSEG1);	
	WriteRegByte(0x06, (SERIAL_BASE+UART_UMCR)|KSEG1);

	tmp=ReadRegByte((SERIAL_BASE+UART_ULCR)|KSEG1);	
	WriteRegByte(tmp&~0x80, (SERIAL_BASE+UART_ULCR)|KSEG1);	//set DLAB=0
}

void putch(const unsigned char c)
{
	unsigned char ch;

	do {
		/*read UART line status register*/
		//use MAG's macro, endian issues
		ch = ReadRegByte(SERIAL_BASE+SER_CMD);   //MAG
	} while (0 == (ch & TX_BUSY));
	
	//use MAG's macro, endian issues
	if(c=='\n')
		WriteRegByte('\r', SERIAL_BASE+SER_DATA);	//MAG

	WriteRegByte(c, SERIAL_BASE+SER_DATA);		//MAG
	
}
const char digits[16] = "0123456789abcdef";
void put32(unsigned int u)
{
	int cnt;
	unsigned ch;

	cnt = 8;		/* 8 nibbles in a 32 bit long */

	putch('0');
	putch('x');
	do {
		cnt--;
		ch = (unsigned char) (u >> cnt * 4) & 0x0F;
		putch(digits[ch]);
	} while (cnt > 0);
	putch(' ');

}

void puts(const char *cp)
{
	unsigned char ch;
	int i = 0;

	while (*cp) {
		do {
			/*read UART line status register*/
			ch=ReadRegByte(SERIAL_BASE+SER_CMD);  	//MAG
			i++;
			if (i > TIMEOUT) {
				break;
			}
		} while (0 == (ch & TX_BUSY));
		
		if(*cp=='\n')
		{
			WriteRegByte('\r', SERIAL_BASE+SER_DATA);		//MAG
			WriteRegByte('\n', SERIAL_BASE+SER_DATA);	//MAG
		}
		else
		{
			WriteRegByte(*cp, SERIAL_BASE+SER_DATA);	//MAG
		}
		cp++;
	}
	
	putch('\n');
	return ;
}


static void udelay(int us)
{
	int time;
	while(us-- > 0)
	{
		time = 0x1000;
		while(time--);
	}
}

