#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "debug_log.h"
#include "syscon.h"
#include "xmodem.h"
#include "ns16550.h"
#include "debug_log.h"

static uint64_t target_addr = 0;
static uint32_t target_size = 0;

static void _delay(int cont)
{
    while (cont--)
    {
		__asm volatile("nop");
    }
}

static int getbyte(unsigned char *c)
{
	return ns16550_rx(NS16550_ADDR,c,2500);
}

static void sendbyte(unsigned char c)
{
	ns16550_tx(NS16550_ADDR, c);
}

static int write(unsigned char* buffer, int size)
{
    uint8_t *target_buffer = (uint8_t *)target_addr;
	if(target_size >= size) {
		memcpy(target_buffer,buffer,size);
		target_addr += size;
		target_size -= size;
	}
	return size;
}

static void timerPause(int s) {
	_delay(s);
}

bool uart_update_request(void)
{
	unsigned char c = 0;
	/* 等待来自串口的升级请求*/
	ns16550_rx(NS16550_ADDR, &c,10000);
	if(c == 'Q') {
		return true;
	} else {
		return false;
	}
}

bool uart_update_xmodem_recv(uint64_t addr, uint32_t size)
{
	int ret = -1;
	target_addr=addr;
	target_size=size;
	xmodemInit(sendbyte,getbyte,timerPause);
	ret = xmodemReceive(write);
	if(ret >= 0){
		return true;
	} else {
		return false;
	}
}

