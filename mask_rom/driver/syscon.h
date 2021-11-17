#ifndef SYSCON_H
#define SYSCON_H

#include <stdbool.h>
#include <stdint.h>
#include "quard_star.h"

#define SYSCON_PFLASH_BOOT (0x1<<0)
#define SYSCON_SPI_BOOT    (0x1<<1)
#define SYSCON_SD_BOOT     (0x1<<2)
#define SYSCON_UART_UPDATE (0x1<<16)

uint32_t syscon_get_boot_source(void);
bool syscon_get_update(void);

#endif /* SYSCON_H */
