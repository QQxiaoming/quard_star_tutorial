#include <stdbool.h>
#include <stdint.h>
#include "debug_log.h"
#include "syscon.h"
#include "spi_flash.h"
#include "sdcard.h"

void spi_flash_boot(void)
{
	spi_flash_init();

	debug_log("Load opensbi_fw.bin\n");
	//[0x200000:0x400000] --> [0xBFF80000:0xC0000000]
	spi_flash_load(0xBFF80000,0x200000,512*1024);

	debug_log("Load qemu_sbi.dtb\n");
	//[0x80000:0x100000] --> [0xBFF00000:0xBFF80000]
	spi_flash_load(0xBFF00000,0x80000,512*1024);

	debug_log("Load trusted_fw.bin\n");
	//[0x400000:0x800000] --> [0xBF800000:0xBFC00000]
	spi_flash_load(0xBF800000,0x400000,4*1024*1024);

	debug_log("Load qemu_uboot.dtb\n");
	//[0x100000:0x180000] --> [0xB0000000:0xB0080000]
	spi_flash_load(0xB0000000,0x100000,512*1024);

	debug_log("Load u-boot.bin\n");
	//[0x800000:0xC00000] --> [0xB0200000:0xB0600000]
	spi_flash_load(0xB0200000,0x800000,4*1024*1024);
}

void sd_boot(void)
{
	sdcard_init();

	debug_log("Load opensbi_fw.bin\n");
	//[0x200000:0x400000] --> [0xBFF80000:0xC0000000]
	sdcard_load(0xBFF80000,0x200000,512*1024);

	debug_log("Load qemu_sbi.dtb\n");
	//[0x80000:0x100000] --> [0xBFF00000:0xBFF80000]
	sdcard_load(0xBFF00000,0x80000,512*1024);

	debug_log("Load trusted_fw.bin\n");
	//[0x400000:0x800000] --> [0xBF800000:0xBFC00000]
	sdcard_load(0xBF800000,0x400000,4*1024*1024);

	debug_log("Load qemu_uboot.dtb\n");
	//[0x100000:0x180000] --> [0xB0000000:0xB0080000]
	sdcard_load(0xB0000000,0x100000,512*1024);

	debug_log("Load u-boot.bin\n");
	//[0x800000:0xC00000] --> [0xB0200000:0xB0600000]
	sdcard_load(0xB0200000,0x800000,4*1024*1024);
}

int main(void)
{
	debug_log("Quard Star LowLevelBoot\n");

	/* in lowlevel_init ddr already init*/
	
	switch (syscon_get_boot_source())
	{
	case SYSCON_SPI_BOOT:
		debug_log("In SPI BOOT...\n");
		spi_flash_boot();
		break;
	case SYSCON_SD_BOOT:
		debug_log("In SD BOOT...\n");
		sd_boot();
		break;
	default:
		debug_log("BOOT err...\n");
		while(1);
		break;
	}

	return 0;
}

