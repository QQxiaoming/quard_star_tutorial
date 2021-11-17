#include <stdbool.h>
#include <stdint.h>
#include "debug_log.h"
#include "sifive_spi.h"
#include "syscon.h"

#define MASKROM_VERSION "v0.1"

static volatile bool copy_finsh = false;
static volatile int secondary_start_flag = 0x0;
static volatile uint64_t boot_addr = 0x20000000;
static void _delay(int cont)
{
    while (cont--)
    {
		__asm volatile("nop");
    }
}

static void jump(uint64_t addr)
{
	__asm volatile("fence");
	__asm volatile("fence.i");
	__asm volatile("jr a0":"=r"(addr));
}

static bool uart_update_request(void)
{
	/* TODO:等待来自串口的升级请求*/
	return false;
}

int main(void)
{
	while(secondary_start_flag != 0xFE);
	debug_log("Quard Star MaskRom %s\n",MASKROM_VERSION);
	switch (syscon_get_boot_source())
	{
	case SYSCON_PFLASH_BOOT:
		/* 无需加载，直接跳转0x20000000 boot */
		boot_addr = 0x20000000;
		break;
	case SYSCON_SPI_BOOT:
		/* TODO:加载256K到sram 0x20000，跳转0x20000 boot */
		sifive_spi_init_hw(SIFIVE_SPI_ADDR);
		boot_addr = 0x20000;
		debug_log("TODO: SPI_BOOT\n");
		while(1);
		break;
	case SYSCON_SD_BOOT:
		/* TODO:加载256K到sram 0x20000，跳转0x20000 boot */
		boot_addr = 0x20000;
		debug_log("TODO: SD_BOOT\n");
		while(1);
		break;
	default:
		break;
	}
	if(syscon_get_update()) {
		debug_log("U\nP\nD\nA\nT\nE\n.\n.\n.\n");
		if(uart_update_request()) {
			/* TODO:通过串口将fw写入sram 0x20000，跳转0x20000 boot */
			boot_addr = 0x20000;
			debug_log("TODO: UART_UPDATE\n");
			while(1);
		} else {
			debug_log("Timeout, we will boot...\n");
		}
	}
	copy_finsh = true;
	jump(boot_addr);
	return 0;
}

int secondary_main(void)
{
	int mhartid = 0;
	_delay(10000);
	__asm volatile("csrr %0, mhartid":"=r"(mhartid));
	secondary_start_flag |= 0x1 << mhartid;
	while(!copy_finsh);
	jump(boot_addr);
	return 0;
}

