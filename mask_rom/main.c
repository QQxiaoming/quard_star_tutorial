#include <stdbool.h>
#include <stdint.h>
#include "debug_log.h"
#include "spi_flash.h"
#include "sdcard.h"
#include "syscon.h"
#include "uart_update.h"

#define MASKROM_VERSION "v0.1"

__attribute__((section(".data.startup"))) volatile bool copy_finsh = false;
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

int main(void)
{
	debug_log("Quard Star MaskRom %s\n",MASKROM_VERSION);
	if(syscon_get_update()) {
		debug_log("UPDATE...\n");
		if(uart_update_request()) {
			/* 通过串口将fw写入sram 0x20000，跳转0x20000 boot */
			boot_addr = 0x20000;
			if(uart_update_xmodem_recv(0x20000,256*1024)) {
				/* 数据正确则jump */
				debug_log("UART_BOOT\n");
				copy_finsh = true;
				jump(boot_addr);
			}
		}
		debug_log("Timeout, we will boot...\n");
	}
	switch (syscon_get_boot_source())
	{
	case SYSCON_PFLASH_BOOT:
		/* 无需加载，直接跳转0x20000000 boot */
		debug_log("PFLASH_BOOT\n");
		boot_addr = 0x20000000;
		break;
	case SYSCON_SPI_BOOT:
		/* 加载256K到sram 0x20000，跳转0x20000 boot */
		debug_log("SPI_BOOT\n");
		spi_flash_init();
		debug_log("Load lowlevelboot.bin\n");
		spi_flash_load(0x20000,0x0,256*1024);
		boot_addr = 0x20000;
		break;
	case SYSCON_SD_BOOT:
		/* 加载256K到sram 0x20000，跳转0x20000 boot */
		debug_log("SD_BOOT\n");
		sdcard_init();
		debug_log("Load lowlevelboot.bin\n");
		sdcard_load(0x20000,0x0,256*1024);
		boot_addr = 0x20000;
		break;
	default:
		break;
	}
	copy_finsh = true;
	jump(boot_addr);
	return 0;
}

int secondary_main(void)
{
	_delay(10000);
	while(!copy_finsh);
	jump(boot_addr);
	return 0;
}

