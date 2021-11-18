#ifndef SPI_FLASH_H
#define SPI_FLASH_H

void spi_flash_init(void);
void spi_flash_load(uint64_t addr, uint32_t offset, uint32_t size);

#endif /* SPI_FLASH_H */