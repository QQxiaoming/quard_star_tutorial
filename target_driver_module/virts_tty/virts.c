// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/tty_flip.h>
#include <linux/tty.h>

#define PORT_VIRTS 188

struct virtual_uart_port {
	struct platform_device *pdev;
    struct uart_port port;
    struct work_struct work;
    spinlock_t lock;
	bool tx_enable_flag;
    bool rx_enable_flag;
};

static struct virtual_uart_port *vport[2];

static unsigned int tx_empty(struct uart_port *port)
{
    struct circ_buf *xmit = &port->state->xmit;

    return uart_circ_empty(xmit);
}

static void set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int get_mctrl(struct uart_port *port)
{
	/* This routine is used to get signals of: DCD, DSR, RI, and CTS */
	return TIOCM_CAR | TIOCM_DSR | TIOCM_CTS;
}

/* Transmit stop */
static void stop_tx(struct uart_port *port)
{
	struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);

    uart_port->tx_enable_flag = false;
}

/* There are probably characters waiting to be transmitted. */
static void start_tx(struct uart_port *port)
{
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;

    spin_lock_irqsave(&uart_port->lock, flags);
    uart_port->tx_enable_flag = true;
    spin_unlock_irqrestore(&uart_port->lock, flags);

    schedule_work(&uart_port->work);
}

/* Receive stop */
static void stop_rx(struct uart_port *port)
{
	struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;

    spin_lock_irqsave(&uart_port->lock, flags);
    uart_port->rx_enable_flag = false;
    spin_unlock_irqrestore(&uart_port->lock, flags);
}

/* Handle breaks - ignored by us */
static void break_ctl(struct uart_port *port, int break_state)
{
}

static int startup(struct uart_port *port)
{
	struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;

    spin_lock_irqsave(&uart_port->lock, flags);
    uart_port->rx_enable_flag = true;
    uart_port->tx_enable_flag = true;
    spin_unlock_irqrestore(&uart_port->lock, flags);

	return 0;
}

static void shutdown(struct uart_port *port)
{
	struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;

    spin_lock_irqsave(&uart_port->lock, flags);
    uart_port->rx_enable_flag = false;
    uart_port->tx_enable_flag = false;
    spin_unlock_irqrestore(&uart_port->lock, flags);
}

static void set_termios(struct uart_port *port, struct ktermios *termios,
			    struct ktermios *old)
{
	/* Just copy the old termios settings back */
	if (old)
		tty_termios_copy_hw(termios, old);
}

static const char *type(struct uart_port *port)
{
	return (port->type == PORT_VIRTS) ? "virt-usart" : NULL;
}

static void release_port(struct uart_port *port)
{
}

static int request_port(struct uart_port *port)
{
	return 0;
}

static void config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_VIRTS;
}

static int verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if (ser->type != PORT_UNKNOWN && ser->type != PORT_VIRTS)
		return -EINVAL;
	return 0;
}


static void transmission_data(struct work_struct *work)
{
    struct virtual_uart_port *uart_port = container_of(work, struct virtual_uart_port, work);
	struct virtual_uart_port *target_vport = platform_get_drvdata(uart_port->pdev);
	struct uart_port *port = &uart_port->port;
	struct uart_port *target_port = &target_vport->port;
	unsigned long flags = 0;

	if (port->x_char) {
		unsigned char ch = port->x_char;
		port->icount.tx++;
		port->x_char = 0;

    	spin_lock_irqsave(&target_vport->lock, flags);
		if(target_vport->rx_enable_flag)
		{
			target_port->icount.rx++;
			if (!uart_handle_sysrq_char(target_port, ch))
				uart_insert_char(target_port, 0, 0, ch, TTY_NORMAL);
			tty_flip_buffer_push(&target_port->state->port);
		}
    	spin_unlock_irqrestore(&target_vport->lock, flags);
		return;
	}

	if (uart_circ_empty(&port->state->xmit) || uart_tx_stopped(port)) {
		stop_tx(port);
		return;
	}

    spin_lock_irqsave(&target_vport->lock, flags);
	do {
		unsigned char ch = port->state->xmit.buf[port->state->xmit.tail];
		port->state->xmit.tail = (port->state->xmit.tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;

		if(target_vport->rx_enable_flag)
		{
			target_port->icount.rx++;
			if (!uart_handle_sysrq_char(target_port, ch))
				uart_insert_char(target_port, 0, 0, ch, TTY_NORMAL);
		}
		
		if (uart_circ_empty(&port->state->xmit))
			break;
	} while (1);

	if(target_vport->rx_enable_flag)
		tty_flip_buffer_push(&target_port->state->port);
    spin_unlock_irqrestore(&target_vport->lock, flags);
	
	if (uart_circ_chars_pending(&port->state->xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(&port->state->xmit))
		stop_tx(port);
}

static const struct uart_ops uart_ops = {
	.tx_empty	= tx_empty,
	.set_mctrl	= set_mctrl,
	.get_mctrl	= get_mctrl,
	.stop_tx	= stop_tx,
	.start_tx	= start_tx,
	.stop_rx	= stop_rx,
	.break_ctl	= break_ctl,
	.startup	= startup,
	.shutdown	= shutdown,
	.set_termios	= set_termios,
	.type		= type,
	.release_port	= release_port,
	.request_port	= request_port,
	.config_port	= config_port,
	.verify_port	= verify_port,
};


static struct virtual_uart_port *alloc_and_init_device(struct uart_driver *driver, unsigned int id)
{
	struct virtual_uart_port *vport;
	struct platform_device *pdev;
	
	pdev = platform_device_alloc("virt-usart_dev",id);
	platform_device_add(pdev);
    vport = devm_kzalloc(&pdev->dev, sizeof(struct virtual_uart_port), GFP_KERNEL);
	vport->pdev = pdev;
	vport->port.ops	     = &uart_ops;
	vport->port.dev	     = &vport->pdev->dev;
	vport->port.type	 = PORT_VIRTS;
	vport->port.fifosize = 512;
	vport->port.line     = id;

    spin_lock_init(&vport->lock);
   	INIT_WORK(&vport->work, transmission_data);

	uart_add_one_port(driver, &vport->port);
    platform_set_drvdata(vport->pdev, NULL);

	return vport;
}

static void link_dev_port(struct virtual_uart_port *vport0,struct virtual_uart_port *vport1)
{
    platform_set_drvdata(vport0->pdev, vport1);
    platform_set_drvdata(vport1->pdev, vport0);
}

static void destroy_and_deinit_port(struct virtual_uart_port *vport,struct uart_driver *driver)
{
    uart_remove_one_port(driver, &vport->port);
	platform_device_unregister(vport->pdev);
}

static struct uart_driver usart_driver = {
	.driver_name =  "virt-usart",
	.dev_name	 =  "ttyVIRT",
    .nr			 = 2,
};


static int __init usart_init(void)
{
	pr_info("virt serial port driver initialized\n");

	uart_register_driver(&usart_driver);
	vport[0] = alloc_and_init_device(&usart_driver,0);
	vport[1] = alloc_and_init_device(&usart_driver,1);
	link_dev_port(vport[0],vport[1]);

	return 0;
}

static void __exit usart_exit(void)
{
    destroy_and_deinit_port(vport[0], &usart_driver);
    destroy_and_deinit_port(vport[1], &usart_driver);
	uart_unregister_driver(&usart_driver);
}

module_init(usart_init);
module_exit(usart_exit);

MODULE_AUTHOR("qiaoqm@aliyun.com");
MODULE_DESCRIPTION("virt serial port driver");
MODULE_LICENSE("GPL v2");
