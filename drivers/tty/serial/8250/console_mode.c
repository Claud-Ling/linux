
#include <linux/module.h>     
#include <linux/kernel.h>     
#include <linux/kthread.h>    
#include <linux/module.h>     
#include <linux/stat.h>       
#include <linux/slab.h>       
#include <linux/capability.h> 
#include <linux/uaccess.h>    
#include <linux/compat.h>     
#include <linux/math64.h>     
#include <linux/err.h>        
#include <linux/delay.h>      
#include <linux/poll.h>       
#include <linux/vmalloc.h>    
#include <linux/fcntl.h>      
#include <linux/signal.h>     
#include <linux/types.h>      
#include <linux/workqueue.h>  
#include <asm/div64.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

#include <linux/suspend.h>
/*
 *          ttyS0     ttyS3
 *             \       /       tty layer
 *          ----------------       
 *               \   /         serial core
 *               uart0
 *
 *	ttyS0 ttyS3 share same uart hardware
 */

#define CONSOLE_SYS	0	//ttyS0 for kernel printk and shell I\O
#define CONSOLE_USR	1	//ttyS3 for App-shell

static int console_mode = CONSOLE_SYS;

#ifdef CONFIG_SERIAL_CORE_CONSOLE
#define uart_console(port)      ((port)->cons && (port)->cons->index == (port)->line)
#else
#define uart_console(port)      (0)
#endif 

/*
 * disable console
 */
static int detach_console(struct uart_8250_port *serial_port)
{
	struct uart_port *uport = &serial_port->port;
	if(!uart_console(uport)){
		printk("Choose right console,refer to bootargs<console=ttySX>.. \n");
		return -1;
	}
	pm_prepare_console();		//redirect kernel msg
	suspend_console();		//suspend the console subsystem

	console_stop(uport->cons);

	serial8250_shutdown(uport);	

	return 0;	
}

static int attach_console(struct uart_8250_port *serial_port)
{
	struct ktermios termios;
	struct uart_port *uport = &serial_port->port;

	struct uart_state *state = serial8250_reg.state + uport->line;
	struct tty_port *port = &state->port;

	if(!uart_console(uport)){
		printk("Choose right console,refer to bootargs<console=ttySX>.. \n");
		return -1;
	}
	serial8250_startup(uport);
	/*
 	 * First try to use the console cflag setting. 	
	 */
	memset(&termios, 0, sizeof(struct ktermios)); 
	termios.c_cflag = uport->cons->cflag; 
	/*
	 * If that's unset, use the tty termios setting.
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)   
	if (port->tty && port->tty->termios && termios.c_cflag == 0)
		termios = *(port->tty->termios);
#else
	if (port->tty && termios.c_cflag == 0)
		termios = port->tty->termios;
#endif
	uport->ops->set_termios(uport, &termios, NULL); 	
	
	console_start(uport->cons);
	resume_console();
	pm_restore_console();
	
	printk("#resume console for kernel printk#\n");
		
	return 0;
}


static ssize_t console_mode_set(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	char mode[4] = {0};
	int org_mode = console_mode;

	struct uart_8250_port *up = &serial8250_ports[0];
	struct uart_port *port = &up->port;
	struct tty_struct *tty = port->state->port.tty;
	
	if(count > 3)
		count = 3;
	if(copy_from_user(mode, buffer,count))
		goto out;	
	
	if(0 == strncmp(mode,"sys",3))
	{
		console_mode = CONSOLE_SYS;
		if(console_mode == org_mode)
			goto out;
		attach_console(&serial8250_ports[0]);	//attach uart0 to ttyS0
		
		tty_unregister_device(serial8250_reg.tty_driver , \
				      serial8250_ports[3].port.line);
		tty_register_device(serial8250_reg.tty_driver , 		\
				    serial8250_ports[0].port.line ,		\
				    serial8250_ports[0].port.dev);
		__clear_bit(TTY_IO_ERROR, &tty->flags);
	}
	else if(0 == strncmp(mode,"usr",3))
	{
		console_mode = CONSOLE_USR;
		if(console_mode == org_mode)
			goto out;
		detach_console(&serial8250_ports[0]);	//detach uart0 from ttyS0
		/*
 	 	 * register /dev/ttyS3 for user mode
 	 	 */
		tty_register_device(serial8250_reg.tty_driver , 		\
				    serial8250_ports[3].port.line ,		\
				    serial8250_ports[3].port.dev);
		/*
 	 	 * unregister /dev/ttyS0 for user mode
 	 	 */
		tty_unregister_device(serial8250_reg.tty_driver , \
				      serial8250_ports[0].port.line);
		__set_bit(TTY_IO_ERROR, &tty->flags);
	}
	return count;

out:
	return -EFAULT;		
}

static int console_mode_show(struct seq_file *m, void *v)
{
	if(CONSOLE_SYS == console_mode)
		seq_printf(m, "sys\n");
	else if(CONSOLE_USR == console_mode)
		seq_printf(m, "usr\n");

	return 0;
}
static int console_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, console_mode_show, NULL);
}

static const struct file_operations console_mode_ops = { 
        .open           = console_proc_open,
        .read           = seq_read,
	.write		= console_mode_set,    
        .llseek         = seq_lseek,   
        .release        = seq_release,
};

static void proc_console_init(void)
{

        proc_create("console_mode",0,NULL,&console_mode_ops);
	/*
 	 * unregister /dev/ttyS3 in debug mode by default
 	 */
	tty_unregister_device(serial8250_reg.tty_driver , serial8250_ports[3].port.line);
}

