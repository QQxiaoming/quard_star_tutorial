#include "syscon.h"

typedef struct
{
	volatile uint32_t CONTROL_REG;
    volatile uint32_t BOOT_REG;
    volatile uint32_t VERSION_REG;
}syscon_t;

syscon_t *syscon = (syscon_t *)(SYSCON_ADDR);

uint32_t syscon_get_boot_source(void)
{
    return (syscon->BOOT_REG&0xffff);
}

bool syscon_get_update(void)
{
	if(syscon->BOOT_REG&0x10000) {
        return true;
    } else {
        return false;
    }
}