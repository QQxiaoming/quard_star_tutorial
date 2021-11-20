#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "spi_flash.h"
#include "sifive_spi.h"
#include "debug_log.h"
#include "quard_star.h"

typedef enum {
    NOP = 0,
    WRSR = 0x1,
    WRDI = 0x4,
    RDSR = 0x5,
    WREN = 0x6,
    BRRD = 0x16,
    BRWR = 0x17,
    JEDEC_READ = 0x9f,
    BULK_ERASE_60 = 0x60,
    BULK_ERASE = 0xc7,
    READ_FSR = 0x70,
    RDCR = 0x15,

    READ = 0x03,
    READ4 = 0x13,
    FAST_READ = 0x0b,
    FAST_READ4 = 0x0c,
    DOR = 0x3b,
    DOR4 = 0x3c,
    QOR = 0x6b,
    QOR4 = 0x6c,
    DIOR = 0xbb,
    DIOR4 = 0xbc,
    QIOR = 0xeb,
    QIOR4 = 0xec,

    PP = 0x02,
    PP4 = 0x12,
    PP4_4 = 0x3e,
    DPP = 0xa2,
    QPP = 0x32,
    QPP_4 = 0x34,
    RDID_90 = 0x90,
    RDID_AB = 0xab,
    AAI_WP = 0xad,

    ERASE_4K = 0x20,
    ERASE4_4K = 0x21,
    ERASE_32K = 0x52,
    ERASE4_32K = 0x5c,
    ERASE_SECTOR = 0xd8,
    ERASE4_SECTOR = 0xdc,

    EN_4BYTE_ADDR = 0xB7,
    EX_4BYTE_ADDR = 0xE9,

    EXTEND_ADDR_READ = 0xC8,
    EXTEND_ADDR_WRITE = 0xC5,

    RESET_ENABLE = 0x66,
    RESET_MEMORY = 0x99,

    /*
     * Micron: 0x35 - enable QPI
     * Spansion: 0x35 - read control register
     */
    RDCR_EQIO = 0x35,
    RSTQIO = 0xf5,

    RNVCR = 0xB5,
    WNVCR = 0xB1,

    RVCR = 0x85,
    WVCR = 0x81,

    REVCR = 0x65,
    WEVCR = 0x61,

    DIE_ERASE = 0xC4,
} FlashCMD;


void spi_flash_init(void)
{
	uint8_t cmd = JEDEC_READ;
	uint8_t jedec_date[6] = {0};

	sifive_spi_init_hw(SIFIVE_SPI_ADDR);

    sifive_spi_set_cs(SIFIVE_SPI_ADDR,true);
	sifive_spi_send(SIFIVE_SPI_ADDR,&cmd,1);
	sifive_spi_recv(SIFIVE_SPI_ADDR,jedec_date,6);
    sifive_spi_set_cs(SIFIVE_SPI_ADDR,false);
    //debug_log("spi flash jedec id %x_%x_%x_%x_%x_%x\n",
	//				jedec_date[0],jedec_date[1],jedec_date[2],
	//				jedec_date[3],jedec_date[4],jedec_date[5]);
}

/* 
 * 这个实现需要size 8字节对齐，
 * 可以正确读取，但是在qemu上运行实在太慢了，需要优化 
 */
void spi_flash_load(uint64_t addr, uint32_t offset, uint32_t size)
{
    uint8_t *buffer = (uint8_t *)addr;
	uint8_t cmd = READ;
	uint8_t offset_arry[3] = {  
                                (offset & 0xFF0000) >> 16,
                                (offset& 0xFF00) >> 8,
                                 offset & 0xFF   
                             };
    debug_log("Load address: 0x%lx\nLoading: ",addr);
    sifive_spi_set_cs(SIFIVE_SPI_ADDR,true);
	sifive_spi_send(SIFIVE_SPI_ADDR,&cmd,1);
	sifive_spi_send(SIFIVE_SPI_ADDR,offset_arry,3);
    for(int i=0;i < size; i+=8) {
        sifive_spi_recv(SIFIVE_SPI_ADDR,buffer+i,8);
        if((i%(size/64))==0){
            debug_log("#");
        }
    }
    sifive_spi_set_cs(SIFIVE_SPI_ADDR,false);
    if(size >= 1024*1024) {
        debug_log(" %dMiB\n",size/1024/1024);
    } else if(size >= 1024) {
        debug_log(" %dKiB\n",size/1024);
    } else {
        debug_log(" %dB\n",size);
    }
}
