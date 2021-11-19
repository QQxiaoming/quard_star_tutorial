#ifndef SDCARD_H
#define SDCARD_H

void sdcard_init(void);
void sdcard_load(uint64_t addr, uint32_t offset, uint32_t size);

#endif /* SDCARD_H */