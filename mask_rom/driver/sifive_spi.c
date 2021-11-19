#include <stdlib.h>
#include <stdint.h>
#include "sifive_spi.h"

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

#define BIT(x)                           (0x1 << x)

/* csmode bits */
#define SIFIVE_SPI_CSMODE_MODE_AUTO      0U
#define SIFIVE_SPI_CSMODE_MODE_HOLD      2U
#define SIFIVE_SPI_CSMODE_MODE_OFF       3U

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

/* txdata bits */
#define SIFIVE_SPI_TXDATA_DATA_MASK      0xffU
#define SIFIVE_SPI_TXDATA_FULL           BIT(31)

/* rxdata bits */
#define SIFIVE_SPI_RXDATA_DATA_MASK      0xffU
#define SIFIVE_SPI_RXDATA_EMPTY          BIT(31)

/* ie and ip bits */
#define SIFIVE_SPI_IP_TXWM               BIT(0)
#define SIFIVE_SPI_IP_RXWM               BIT(1)


static void sifive_spi_rx(sifive_spi_t *reg, uint8_t *rx_ptr)
{
	uint32_t data;
	do {
		data = reg->SIFIVE_SPI_REG_RXDATA;
	} while (data & SIFIVE_SPI_RXDATA_EMPTY);

	if (rx_ptr)
		*rx_ptr = data & SIFIVE_SPI_RXDATA_DATA_MASK;
}

static void sifive_spi_tx(sifive_spi_t *reg, const uint8_t *tx_ptr)
{
	uint32_t data;
	uint8_t tx_data = (tx_ptr) ? *tx_ptr & SIFIVE_SPI_TXDATA_DATA_MASK :
				SIFIVE_SPI_TXDATA_DATA_MASK;

	do {
		data = reg->SIFIVE_SPI_REG_TXDATA;
	} while (data & SIFIVE_SPI_TXDATA_FULL);

	reg->SIFIVE_SPI_REG_TXDATA = tx_data;
}

void sifive_spi_send(uintptr_t addr, const uint8_t *tx_ptr, uint32_t size)
{
	sifive_spi_t *reg = (sifive_spi_t *)addr;
	reg->SIFIVE_SPI_REG_FMT = SIFIVE_SPI_FMT_DIR | SIFIVE_SPI_FMT_LEN(8);

	for(int i=0;i<size;i++) {
		sifive_spi_tx(reg,&tx_ptr[i]);
	}
	while(!(reg->SIFIVE_SPI_REG_IP&SIFIVE_SPI_IP_TXWM));
}

void sifive_spi_recv(uintptr_t addr, uint8_t *rx_ptr, uint32_t size)
{
	sifive_spi_t *reg = (sifive_spi_t *)addr;
	reg->SIFIVE_SPI_REG_FMT = SIFIVE_SPI_FMT_LEN(8);

	for(int i=0;i<size;i++) {
		sifive_spi_tx(reg,NULL);
	}
	while(!(reg->SIFIVE_SPI_REG_IP&SIFIVE_SPI_IP_RXWM));

	for(int i=0;i<size;i++) {
		sifive_spi_rx(reg,&rx_ptr[i]);
	}
}

void sifive_spi_set_cs(uintptr_t addr, bool set)
{
	sifive_spi_t *reg = (sifive_spi_t *)addr;
	if(set) {
		reg->SIFIVE_SPI_REG_CSMODE = SIFIVE_SPI_CSMODE_MODE_HOLD;
		reg->SIFIVE_SPI_REG_CSDEF = 1;
		reg->SIFIVE_SPI_REG_CSID = 0;
	} else {
		reg->SIFIVE_SPI_REG_CSMODE = SIFIVE_SPI_CSMODE_MODE_AUTO;
		reg->SIFIVE_SPI_REG_CSDEF = 1;
		reg->SIFIVE_SPI_REG_CSID = 0;
	}
}

void sifive_spi_init_hw(uintptr_t addr)
{
	sifive_spi_t *reg = (sifive_spi_t *)addr;

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
}
