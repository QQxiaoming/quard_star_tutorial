#ifndef SIFIVE_SPI_H_
#define SIFIVE_SPI_H_

#include <stdint.h>
#include <stdbool.h>

void sifive_spi_init_hw(uintptr_t addr);
void sifive_spi_set_cs(uintptr_t addr, bool set);
void sifive_spi_send(uintptr_t addr, const uint8_t *tx_ptr, uint32_t size);
void sifive_spi_recv(uintptr_t addr, uint8_t *rx_ptr, uint32_t size);

#endif /* SIFIVE_SPI_H_ */
