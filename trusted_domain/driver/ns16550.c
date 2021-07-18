#include <stdint.h>
#include "ns16550.h"

/* register definitions */
#define REG_RBR		0x00 /* Receiver buffer reg. */
#define REG_THR		0x00 /* Transmitter holding reg. */
#define REG_IER		0x01 /* Interrupt enable reg. */
#define REG_IIR		0x02 /* Interrupt ID reg. */
#define REG_FCR		0x02 /* FIFO control reg. */
#define REG_LCR		0x03 /* Line control reg. */
#define REG_MCR		0x04 /* Modem control reg. */
#define REG_LSR		0x05 /* Line status reg. */
#define REG_MSR		0x06 /* Modem status reg. */
#define REG_SCR		0x07 /* Scratch reg. */
#define REG_BRDL	0x00 /* Divisor latch (LSB) */
#define REG_BRDH	0x01 /* Divisor latch (MSB) */

/* Line status */
#define LSR_DR			0x01 /* Data ready */
#define LSR_OE			0x02 /* Overrun error */
#define LSR_PE			0x04 /* Parity error */
#define LSR_FE			0x08 /* Framing error */
#define LSR_BI			0x10 /* Break interrupt */
#define LSR_THRE		0x20 /* Transmitter holding register empty */
#define LSR_TEMT		0x40 /* Transmitter empty */
#define LSR_EIRF		0x80 /* Error in RCVR FIFO */

static uint8_t readb( uintptr_t addr )
{
	return *( (uint8_t *) addr );
}

static void writeb( uint8_t b, uintptr_t addr )
{
	*( (uint8_t *) addr ) = b;
}

void vOutNS16550( uintptr_t addr, unsigned char c )
{
	while ( (readb( addr + REG_LSR ) & LSR_THRE) == 0 ) {
		/* busy wait */
	}

	writeb( c, addr + REG_THR );
}
