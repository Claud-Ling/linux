
#ifndef __ASM_ARCH_TRIX_MACH_SERIAL_H__
#define __ASM_ARCH_TRIX_MACH_SERIAL_H__

#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)

/*generic uart criver, Note: ttyS0 and ttyS3 share same uart port base[0xfb005100]*/
#define TRIHIDTV_SERIAL_PORT_DEFNS                     \
        { baud_base:BASE_BAUD,/* port: (0x1b005100),*/ irq: TRIHIDTV_PLF_UART_0_INTERRUPT, flags: STD_COM_FLAGS, io_type: SERIAL_IO_PORT | UPIO_MEM, iomem_base:((void*)0xfb005100) },\
        { baud_base:BASE_BAUD,/* port: (0x1b005200),*/ irq: TRIHIDTV_PLF_UART_1_INTERRUPT, flags: STD_COM_FLAGS, io_type: SERIAL_IO_PORT | UPIO_MEM, iomem_base:((void*)0xfb005200) },\
        { baud_base:BASE_BAUD,/* port: (0x1b005300),*/ irq: TRIHIDTV_PLF_UART_2_INTERRUPT, flags: STD_COM_FLAGS, io_type: SERIAL_IO_PORT | UPIO_MEM, iomem_base:((void*)0xfb005300) },\
        { baud_base:BASE_BAUD,/* port: (0x1b005100),*/ irq: TRIHIDTV_PLF_UART_0_INTERRUPT, flags: STD_COM_FLAGS, io_type: SERIAL_IO_PORT | UPIO_MEM, iomem_base:((void*)0xfb005100) },

#define SERIAL_PORT_DFNS                                \
        TRIHIDTV_SERIAL_PORT_DEFNS                      

/*generic uart criver*/

#endif
