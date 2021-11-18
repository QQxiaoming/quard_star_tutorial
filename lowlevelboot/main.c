#include <stdbool.h>
#include <stdint.h>
#include "debug_log.h"
#include "syscon.h"
#include "spi_flash.h"

int main(void)
{
	debug_log("Quard Star LowLevelBoot\n");
	debug_log("In SPI BOOT...\n");

	/* in lowlevel_init ddr already init*/
	spi_flash_init();

	//load opensbi_fw.bin 
	//[0x200000:0x400000] --> [0xBFF80000:0xC0000000]
	debug_log("load opensbi_fw.bin\n");
	spi_flash_load(0xBFF80000,0x200000,512*1024);

	//load qemu_sbi.dtb
	//[0x80000:0x100000] --> [0xBFF00000:0xBFF80000]
	debug_log("load qemu_sbi.dtb\n");
	spi_flash_load(0xBFF00000,0x80000,512*1024);

	//load trusted_fw.bin
	//[0x400000:0x800000] --> [0xBF800000:0xBFC00000]
	debug_log("load trusted_fw.bin\n");
	spi_flash_load(0xBF800000,0x400000,4*1024*1024);

	//load qemu_uboot.dtb
	//[0x100000:0x180000] --> [0xB0000000:0xB0080000]
	debug_log("load qemu_uboot.dtb\n");
	spi_flash_load(0xB0000000,0x100000,512*1024);

	//load u-boot.bin
	//[0x800000:0xC00000] --> [0xB0200000:0xB0600000]
	debug_log("load u-boot.bin\n");
	spi_flash_load(0xB0200000,0x800000,4*1024*1024);

	return 0;
}

