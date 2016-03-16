
#include <config.h>
#include <system.h>
#include <assembler.h>

/*
 * prototype: void init_serial(void);
 */
LEAF(initserial_asm)
	//serial base address
	ldr 	r0, =0xfb005100
	
	//each character, 8 bits, stop bit 1 bit
	mov 	r1, #0x03
	strb  	r1, [r0, #3]

	//FIFO trigger level: 14 bytes, enable FIFO and clear FIFO
	mov 	r1, #0xc7
	strb  	r1, [r0, #2]

	//set DLAB=1
	ldrb 	r1, [r0, #3]
	orr	r1, #0x80	
	strb  	r1, [r0, #3]

	//set baudrate=115200
	mov 	r1, #0x01
	strb  	r1, [r0]

	// 0x00 -> 0xb4011801
	mov 	r1, #0x0
	strb  	r1, [r0, #1]

	// 0x06 -> 0xb4011804
	mov 	r1, #0x06
	strb  	r1, [r0, #4]

	//set DLAB=0
	ldrb 	r1, [r0, #3]
	and 	r1, #0x7f
	strb  	r1, [r0, #3]

	 bx 	lr
END(init_serial)


/*prototype: put_s(char *)*/
LEAF(puts_asm)

	ldr 	r4, =0xfb005100

1:      
	ldrb 	r1, [r0], #1		@post index
	strb 	r1, [r4]

	/*check it tx ready*/
2:
	ldrb 	r2, [r4, #5]
	tst	r2, #0x20	
	bne 	2b

	/*check end of string*/
	teq	r1, #0
    	bne 	1b

	bx 	lr
END(put_asm)

