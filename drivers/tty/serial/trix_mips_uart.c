/*
 * sx7_uart.c -- SX7 UART driver
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>

#define DRV_NAME "sx7_uart"
#define SERIAL_SX7_MAJOR 204
#define SERIAL_SX7_MINOR  66
#define SX7_UART_SIZE 32

#define SX7_UART_MAXPORTS 1
#define SX7_UART_CONTROL_REG 0
#define SX7_UART_DATA_REG 4
#define SX7_UART_RX_LEVEL 8
#define SX7_UART_TX_LEVEL 12

#define SX7_TX_READY 0x04000000
#define SX7_RX_READY 0x01000000
#define SX7_TX_EMPTY 0x00400000

#define PORT_SX7_UART	0x7
#define LOOPBACK_BUF_SIZE       (16*1024)

static int mips_console_enable = 0;
/*
 * Local per-uart structure.
 */
struct sx7_uart {
	struct uart_port port;
	struct timer_list tmr;
	unsigned int sigs;	/* Local copy of line sigs */
	unsigned char loopback_buf[LOOPBACK_BUF_SIZE];
	unsigned loopback_written;
};

static int __init sigma_console_config(char *str)
{
	if (!strcmp(str, "ttyMS0,115200n8")) {
		mips_console_enable = 1;
	} else {
		mips_console_enable = 0;
	}

	return 1;
}
early_param("console", sigma_console_config);

static u32 sx7_uart_readl(struct uart_port *port, int reg)
{
	return readl(port->membase + (reg << port->regshift));
}

static u8 sx7_uart_readb(struct uart_port *port, int reg)
{
	return readb(port->membase + (reg << port->regshift));
}

static void sx7_uart_writel(struct uart_port *port, u32 dat, int reg)
{
	writel(dat, port->membase + (reg << port->regshift));
}

static unsigned int sx7_uart_tx_empty(struct uart_port *port)
{
	return (sx7_uart_readl(port, SX7_UART_CONTROL_REG) & SX7_TX_EMPTY) ? TIOCSER_TEMT : 0;
}

static unsigned int sx7_uart_get_mctrl(struct uart_port *port)
{
	return 0;
}

static void sx7_uart_set_mctrl(struct uart_port *port, unsigned int sigs)
{
}

static void sx7_uart_start_tx(struct uart_port *port)
{
}

static void sx7_uart_stop_tx(struct uart_port *port)
{
}

static void sx7_uart_stop_rx(struct uart_port *port)
{
}

static void sx7_uart_break_ctl(struct uart_port *port, int break_state)
{
}

static void sx7_uart_enable_ms(struct uart_port *port)
{
}

static void sx7_uart_set_termios(struct uart_port *port,
				    struct ktermios *termios,
				    struct ktermios *old)
{
	if (old)
		tty_termios_copy_hw(termios, old);
	tty_termios_encode_baud_rate(termios, 115200, 115200);

	sx7_uart_writel(port, 0x363, SX7_UART_CONTROL_REG);
}

static void sx7_uart_rx_chars(struct sx7_uart *pp)
{
	struct uart_port *port = &pp->port;
	unsigned char ch, flag;
	int cnt = 0;

	while (sx7_uart_readl(port, SX7_UART_CONTROL_REG) & SX7_RX_READY) {
		++cnt;
		ch = sx7_uart_readb(port, SX7_UART_DATA_REG);
		flag = TTY_NORMAL;
		port->icount.rx++;

		if (uart_handle_sysrq_char(port, ch))
			continue;
		uart_insert_char(port, 0, 0, ch, flag);
	}

	if (cnt) tty_flip_buffer_push(&port->state->port);
}

static void sx7_loopback_write(struct uart_port *port, int ch)
{
	struct sx7_uart *up = container_of(port, struct sx7_uart, port);
	unsigned windex = up->loopback_written % LOOPBACK_BUF_SIZE;
	up->loopback_buf[windex] = ch;
	up->loopback_written++;
}

static void sx7_tx_char(struct uart_port *port, char ch)
{
	sx7_uart_writel(port, ch, SX7_UART_DATA_REG);
	sx7_loopback_write(port, ch);
}

static void sx7_uart_tx_chars(struct sx7_uart *pp)
{
	struct uart_port *port = &pp->port;
	struct circ_buf *xmit = &port->state->xmit;

	if (port->x_char) {
		/* Send special char - probably flow control */
		sx7_tx_char(port, port->x_char);
		port->x_char = 0;
		port->icount.tx++;
		return;
	}

	while (sx7_uart_readl(port, SX7_UART_TX_LEVEL) < 200) {
		if (xmit->head == xmit->tail)
			break;
		sx7_tx_char(port, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);
}

static void sx7_uart_interrupt(void *data)
{
	struct uart_port *port = data;
	struct sx7_uart *pp = container_of(port, struct sx7_uart, port);

	spin_lock(&port->lock);
	sx7_uart_rx_chars(pp);
	sx7_uart_tx_chars(pp);
	spin_unlock(&port->lock);
}

static void sx7_uart_timer(unsigned long data)
{
	struct uart_port *port = (void *)data;
	struct sx7_uart *pp = container_of(port, struct sx7_uart, port);

	sx7_uart_interrupt(port);
	mod_timer(&pp->tmr, jiffies + 1);
}

static void sx7_uart_config_port(struct uart_port *port, int flags)
{
	port->type = PORT_SX7_UART;

	sx7_uart_writel(port, 0x363, SX7_UART_CONTROL_REG);
}

static int sx7_uart_startup(struct uart_port *port)
{
	struct sx7_uart *pp = container_of(port, struct sx7_uart, port);

	pp->loopback_written = 0;
	setup_timer(&pp->tmr, sx7_uart_timer, (unsigned long)port);
	mod_timer(&pp->tmr, jiffies + 1);

	return 0;
}

static void sx7_uart_shutdown(struct uart_port *port)
{
	struct sx7_uart *pp = container_of(port, struct sx7_uart, port);

	del_timer_sync(&pp->tmr);
}

static const char *sx7_uart_type(struct uart_port *port)
{
	return (port->type == PORT_SX7_UART) ? "SX7 UART" : NULL;
}

static int sx7_uart_request_port(struct uart_port *port)
{
	/* UARTs always present */
	return 0;
}

static void sx7_uart_release_port(struct uart_port *port)
{
	/* Nothing to release... */
}

static int sx7_uart_verify_port(struct uart_port *port,
				   struct serial_struct *ser)
{
	if ((ser->type != PORT_UNKNOWN) && (ser->type != PORT_SX7_UART))
		return -EINVAL;
	return 0;
}

#ifdef CONFIG_CONSOLE_POLL
static int sx7_uart_poll_get_char(struct uart_port *port)
{
	int c;

	while (1) {
		while (!(sx7_uart_readl(port, SX7_UART_CONTROL_REG) & SX7_RX_READY))
			cpu_relax();

		c = sx7_uart_readb(port, SX7_UART_DATA_REG);
		return c;
	}
}

static void sx7_uart_poll_put_char(struct uart_port *port, unsigned char c)
{
	while (!(sx7_uart_readl(port, SX7_UART_CONTROL_REG) & SX7_TX_READY))
		cpu_relax();

	sx7_tx_char(port, c);
}
#endif

/*
 *	Define the basic serial functions we support.
 */
static struct uart_ops sx7_uart_ops = {
	.tx_empty	= sx7_uart_tx_empty,
	.get_mctrl	= sx7_uart_get_mctrl,
	.set_mctrl	= sx7_uart_set_mctrl,
	.start_tx	= sx7_uart_start_tx,
	.stop_tx	= sx7_uart_stop_tx,
	.stop_rx	= sx7_uart_stop_rx,
	.enable_ms	= sx7_uart_enable_ms,
	.break_ctl	= sx7_uart_break_ctl,
	.startup	= sx7_uart_startup,
	.shutdown	= sx7_uart_shutdown,
	.set_termios	= sx7_uart_set_termios,
	.type		= sx7_uart_type,
	.request_port	= sx7_uart_request_port,
	.release_port	= sx7_uart_release_port,
	.config_port	= sx7_uart_config_port,
	.verify_port	= sx7_uart_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_get_char	= sx7_uart_poll_get_char,
	.poll_put_char	= sx7_uart_poll_put_char,
#endif
};

static struct sx7_uart sx7_uart_ports[SX7_UART_MAXPORTS];


static void sx7_uart_console_putc(struct uart_port *port, const char c)
{
	while (!(sx7_uart_readl(port, SX7_UART_CONTROL_REG) & SX7_TX_READY))
		cpu_relax();

	writel(c, port->membase + SX7_UART_DATA_REG);
}

static void sx7_uart_console_write(struct console *co, const char *s,
				      unsigned int count)
{
	struct uart_port *port = &(sx7_uart_ports + co->index)->port;

	for (; count; count--, s++) {
		sx7_uart_console_putc(port, *s);
		if (*s == '\n')
			sx7_uart_console_putc(port, '\r');
	}
}

static int __init sx7_uart_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (co->index < 0 || co->index >= SX7_UART_MAXPORTS)
		return -EINVAL;
	port = &sx7_uart_ports[co->index].port;
	if (!port->membase)
		return -ENODEV;

	if (options) {
		MWriteRegByte((void *)0xf500ee1d, 0x11, 0xff);
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	}

	if (baud == 0) {
		printk("baud == 0: no input allowed\n");
		baud = 115200;
	}

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static struct uart_driver sx7_uart_driver;

static struct console sx7_uart_console = {
	.name	= "ttyMS",
	.write	= sx7_uart_console_write,
	.device	= uart_console_device,
	.setup	= sx7_uart_console_setup,
	.flags	= CON_PRINTBUFFER,
	.index	= -1,
	.data	= &sx7_uart_driver,
};

static void init_sx7_port(void)
{
	struct uart_port *port = &sx7_uart_ports[0].port;
	port->mapbase = 0xf0012000;
	port->membase = (void*)0xf0012000;

	port->regshift = 0;

	port->line = 0;
	port->type = PORT_SX7_UART;
	port->iotype = SERIAL_IO_MEM;
	port->ops = &sx7_uart_ops;
	port->flags = UPF_BOOT_AUTOCONF;
}

static int __init sx7_uart_console_init(void)
{
	if (mips_console_enable) {
		init_sx7_port();
		register_console(&sx7_uart_console);
	}
	return 0;
}

console_initcall(sx7_uart_console_init);


#define	SX7_UART_CONSOLE	(&sx7_uart_console)

/*
 *	Define the sx7_uart UART driver structure.
 */
static struct uart_driver sx7_uart_driver = {
	.owner		= THIS_MODULE,
	.driver_name	= DRV_NAME,
	.dev_name	= "ttyMS",
	.major		= SERIAL_SX7_MAJOR,
	.minor		= SERIAL_SX7_MINOR,
	.nr		= SX7_UART_MAXPORTS,
	.cons		= SX7_UART_CONSOLE,
};

static struct platform_device* serial_sx7_plat_devs;

static int sx7_uart_probe(struct platform_device *pdev)
{
	return 0;
}

static int sx7_uart_remove(struct platform_device *pdev)
{
	struct uart_port *port = &sx7_uart_ports[0].port;

	if (port->dev == &pdev->dev) {
		uart_remove_one_port(&sx7_uart_driver, port);
		port->mapbase = 0;
		port->dev = 0;
	}

	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id sx7_uart_match[] = {
	{ .compatible = "SX7,uart-1.0", },
	{},
};
MODULE_DEVICE_TABLE(of, sx7_uart_match);
#endif /* CONFIG_OF */

static struct platform_driver sx7_uart_platform_driver = {
	.probe	= sx7_uart_probe,
	.remove	= sx7_uart_remove,
	.driver	= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= of_match_ptr(sx7_uart_match),
	},
};

static int __init sx7_uart_init(void)
{
	int rc;

	if (!mips_console_enable) {
		return 0;
	}

	rc = uart_register_driver(&sx7_uart_driver);
	if (rc)
		return rc;

	serial_sx7_plat_devs = platform_device_alloc("serial_sx7", -1);
	platform_device_add(serial_sx7_plat_devs);

	sx7_uart_ports[0].port.dev = &serial_sx7_plat_devs->dev;
	uart_add_one_port(&sx7_uart_driver, &sx7_uart_ports[0].port);

	rc = platform_driver_register(&sx7_uart_platform_driver);
	if (rc)
		uart_unregister_driver(&sx7_uart_driver);
	return rc;
}

static void __exit sx7_uart_exit(void)
{
	if (!mips_console_enable) {
		return;
	}

	platform_driver_unregister(&sx7_uart_platform_driver);
	uart_unregister_driver(&sx7_uart_driver);
}

module_init(sx7_uart_init);
module_exit(sx7_uart_exit);

MODULE_DESCRIPTION("SIGMA mips UART driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sx7_uart");
MODULE_ALIAS_CHARDEV_MAJOR(SERIAL_SX7_MAJOR);
