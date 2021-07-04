#include <stdint.h>

uint32_t mbox_server_total_size(void);
uint32_t mbox_server_erase_granule(void);
int mbox_server_version(void);
void mbox_server_memset(int c);
int mbox_server_memcmp(int off, const void *buf, size_t len);
int mbox_server_reset(unsigned int version, uint8_t block_shift);
int mbox_server_init(void);
void mbox_server_destroy(void);
