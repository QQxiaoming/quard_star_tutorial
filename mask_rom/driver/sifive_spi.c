#include <stdint.h>
#include "sifive_spi.h"
#include "debug_log.h"

typedef struct
{
	volatile uint32_t SIFIVE_SPI_REG_SCKDIV; /* 0x00 Serial clock divisor */
	volatile uint32_t SIFIVE_SPI_REG_SCKMODE;/* 0x04 Serial clock mode */
	volatile uint32_t resv0[2];
	volatile uint32_t SIFIVE_SPI_REG_CSID;   /* 0x10 Chip select ID */
	volatile uint32_t SIFIVE_SPI_REG_CSDEF;  /* 0x14 Chip select default */
	volatile uint32_t SIFIVE_SPI_REG_CSMODE; /* 0x18 Chip select mode */
	volatile uint32_t resv1[3];
	volatile uint32_t SIFIVE_SPI_REG_DELAY0; /* 0x28 Delay control 0 */
	volatile uint32_t SIFIVE_SPI_REG_DELAY1; /* 0x2c Delay control 1 */
	volatile uint32_t resv2[4];
	volatile uint32_t SIFIVE_SPI_REG_FMT;    /* 0x40 Frame format */
	volatile uint32_t resv3[1];
	volatile uint32_t SIFIVE_SPI_REG_TXDATA; /* 0x48 Tx FIFO data */
	volatile uint32_t SIFIVE_SPI_REG_RXDATA; /* 0x4c Rx FIFO data */
	volatile uint32_t SIFIVE_SPI_REG_TXMARK; /* 0x50 Tx FIFO watermark */
	volatile uint32_t SIFIVE_SPI_REG_RXMARK; /* 0x54 Rx FIFO watermark */
	volatile uint32_t resv4[2];
	volatile uint32_t SIFIVE_SPI_REG_FCTRL;  /* 0x60 SPI flash interface control */
	volatile uint32_t SIFIVE_SPI_REG_FFMT;   /* 0x64 SPI flash instruction format */
	volatile uint32_t resv5[2];
	volatile uint32_t SIFIVE_SPI_REG_IE;     /* 0x70 Interrupt Enable Register */
	volatile uint32_t SIFIVE_SPI_REG_IP;     /* 0x74 Interrupt Pendings Register */
} sifive_spi_t;

#define BIT(x)  (0x1 << x)

/* sckmode bits */
#define SIFIVE_SPI_SCKMODE_PHA           BIT(0)
#define SIFIVE_SPI_SCKMODE_POL           BIT(1)
#define SIFIVE_SPI_SCKMODE_MODE_MASK     (SIFIVE_SPI_SCKMODE_PHA | \
					  SIFIVE_SPI_SCKMODE_POL)

/* delay0 bits */
#define SIFIVE_SPI_DELAY0_CSSCK(x)       ((uint32_t)(x))
#define SIFIVE_SPI_DELAY0_CSSCK_MASK     0xffU
#define SIFIVE_SPI_DELAY0_SCKCS(x)       ((uint32_t)(x) << 16)
#define SIFIVE_SPI_DELAY0_SCKCS_MASK     (0xffU << 16)

/* delay1 bits */
#define SIFIVE_SPI_DELAY1_INTERCS(x)     ((uint32_t)(x))
#define SIFIVE_SPI_DELAY1_INTERCS_MASK   0xffU
#define SIFIVE_SPI_DELAY1_INTERXFR(x)    ((uint32_t)(x) << 16)
#define SIFIVE_SPI_DELAY1_INTERXFR_MASK  (0xffU << 16)

/* fmt bits */
#define SIFIVE_SPI_FMT_PROTO_SINGLE      0U
#define SIFIVE_SPI_FMT_PROTO_DUAL        1U
#define SIFIVE_SPI_FMT_PROTO_QUAD        2U
#define SIFIVE_SPI_FMT_PROTO_MASK        3U
#define SIFIVE_SPI_FMT_ENDIAN            BIT(2)
#define SIFIVE_SPI_FMT_DIR               BIT(3)
#define SIFIVE_SPI_FMT_LEN(x)            ((uint32_t)(x) << 16)
#define SIFIVE_SPI_FMT_LEN_MASK          (0xfU << 16)

void sifive_spi_init_hw(uintptr_t addr)
{
	sifive_spi_t *reg = (sifive_spi_t *)addr;
	uint32_t cs_bits;
    uint32_t cs_inactive;

	/* probe the number of CS lines */
	cs_inactive = reg->SIFIVE_SPI_REG_CSDEF;
	reg->SIFIVE_SPI_REG_CSDEF = 0xffffffffU;
	cs_bits = reg->SIFIVE_SPI_REG_CSDEF;
	reg->SIFIVE_SPI_REG_CSDEF = cs_inactive;
	if (!cs_bits) {
		debug_log("Could not auto probe CS lines\n");
		return;
	}

	/* Watermark interrupts are disabled by default */
	reg->SIFIVE_SPI_REG_IE = 0;

	/* Default watermark FIFO threshold values */
	reg->SIFIVE_SPI_REG_TXMARK = 1;
	reg->SIFIVE_SPI_REG_RXMARK = 0;

	/* Set CS/SCK Delays and Inactive Time to defaults */
	reg->SIFIVE_SPI_REG_DELAY0 = SIFIVE_SPI_DELAY0_CSSCK(1) | SIFIVE_SPI_DELAY0_SCKCS(1);
	reg->SIFIVE_SPI_REG_DELAY1 = SIFIVE_SPI_DELAY1_INTERCS(1) | SIFIVE_SPI_DELAY1_INTERXFR(0);

	/* Exit specialized memory-mapped SPI flash mode */
	reg->SIFIVE_SPI_REG_FCTRL = 0;
	
    debug_log("sifive_spi_init_hw done\n");
}
