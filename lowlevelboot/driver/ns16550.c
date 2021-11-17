#include <stdint.h>
#include "ns16550.h"

typedef struct
{
	volatile uint8_t THR_REG; /* Transmitter holding reg. */
	volatile uint8_t IER_REG; /* Interrupt enable reg. */
	volatile uint8_t IIR_REG; /* Interrupt ID reg. */
	volatile uint8_t LCR_REG; /* Line control reg. */
	volatile uint8_t MCR_REG; /* Modem control reg. */
	volatile uint8_t LSR_REG; /* Line status reg. */
	volatile uint8_t MSR_REG; /* Modem status reg. */
	volatile uint8_t SCR_REG; /* Scratch reg. */
} ns16550_t;

/* Line status */
#define LSR_DR 0x01	  /* Data ready */
#define LSR_OE 0x02	  /* Overrun error */
#define LSR_PE 0x04	  /* Parity error */
#define LSR_FE 0x08	  /* Framing error */
#define LSR_BI 0x10	  /* Break interrupt */
#define LSR_THRE 0x20 /* Transmitter holding register empty */
#define LSR_TEMT 0x40 /* Transmitter empty */
#define LSR_EIRF 0x80 /* Error in RCVR FIFO */

void ns16550_tx(uintptr_t addr, unsigned char c)
{
	ns16550_t *reg = (ns16550_t *)addr;

	while ((reg->LSR_REG & LSR_THRE) == 0) ;
	reg->THR_REG = c;
}
