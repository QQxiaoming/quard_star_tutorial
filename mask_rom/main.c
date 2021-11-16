#include <stdbool.h>
#include <stdint.h>
#include "debug_log.h"
#include "quard_star.h"

#define MASKROM_VERSION "v0.1"

typedef struct
{
	volatile uint32_t CONTROL_REG;
    volatile uint32_t BOOT_REG;
    volatile uint32_t VERSION_REG;
}syscon_t;

syscon_t *syscon = (syscon_t *)(SYSCON_ADDR);

#define SYSCON_PFLASH_BOOT (0x1<<0)
#define SYSCON_SPI_BOOT    (0x1<<1)
#define SYSCON_SD_BOOT     (0x1<<2)
#define SYSCON_UART_UPDATE (0x1<<16)

static volatile bool copy_finsh = false;
static volatile int secondary_start_flag = 0x0;
static volatile uint32_t boot_addr = 0x20000000;
static void _delay(int cont)
{
    while (cont--)
    {
		__asm volatile("nop");
    }
}

void jump(int addr)
{
	__asm volatile( "jr a0");
}

int main(void)
{
	while(secondary_start_flag != 0xFE);
	debug_log("Quard Star MaskRom %s\n",MASKROM_VERSION);
	switch (syscon->BOOT_REG&0xffff)
	{
	case SYSCON_PFLASH_BOOT:
		boot_addr = 0x20000000;
		break;
	case SYSCON_SPI_BOOT:
		//TODO:
		break;
	case SYSCON_SD_BOOT:
		//TODO:
		break;
	default:
		break;
	}
	if(syscon->BOOT_REG&0x10000) {
		debug_log("U\nP\nD\nA\nT\nE\n.\n.\n.\n");
		//TODO:
	}
	copy_finsh = true;
	jump(boot_addr);
	return 0;
}

int secondary_main(void)
{
	int mhartid = 0;
	_delay(10000);
	__asm volatile( "csrr %0, mhartid" : "=r"( mhartid ) );
	secondary_start_flag |= 0x1 << mhartid;
	while(!copy_finsh);
	jump(boot_addr);
	return 0;
}

