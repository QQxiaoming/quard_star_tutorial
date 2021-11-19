#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdhci.h"
#include "syscon.h"

typedef struct
{
	volatile uint32_t SDHCI_DMA_ADDRESS;      /* 0x00 */
	volatile uint16_t SDHCI_BLOCK_SIZE;       /* 0x04 */
	volatile uint16_t SDHCI_BLOCK_COUNT;      /* 0x06 */
	volatile uint32_t SDHCI_ARGUMENT;         /* 0x08 */
	volatile uint16_t SDHCI_TRANSFER_MODE;    /* 0x0c */
	volatile uint16_t SDHCI_COMMAND;          /* 0x0e */
	volatile uint32_t SDHCI_RESPONSE[4];      /* 0x10 */
	volatile uint32_t SDHCI_BUFFER;           /* 0x20 */
	volatile uint32_t SDHCI_PRESENT_STATE;    /* 0x24 */
	volatile uint8_t  SDHCI_HOST_CONTROL;     /* 0x28 */
	volatile uint8_t  SDHCI_POWER_CONTROL;    /* 0x29 */
	volatile uint8_t  SDHCI_BLOCK_GAP_CONTROL;/* 0x2a */
	volatile uint8_t  SDHCI_WAKE_UP_CONTROL;  /* 0x2b */
	volatile uint16_t SDHCI_CLOCK_CONTROL;    /* 0x2c */
	volatile uint8_t  SDHCI_TIMEOUT_CONTROL;  /* 0x2e */
	volatile uint8_t  SDHCI_SOFTWARE_RESET;   /* 0x2f */
	volatile uint32_t SDHCI_INT_STATUS;       /* 0x30 */
	volatile uint32_t SDHCI_INT_ENABLE;       /* 0x34 */
	volatile uint32_t SDHCI_SIGNAL_ENABLE;    /* 0x38 */
	volatile uint16_t SDHCI_ACMD12_ERR;       /* 0x3c */
	volatile uint16_t SDHCI_HOST_CONTROL2;    /* 0x3e */
	volatile uint32_t SDHCI_CAPABILITIES;     /* 0x40 */
	volatile uint32_t SDHCI_CAPABILITIES_1;   /* 0x44 */
	volatile uint32_t SDHCI_MAX_CURRENT;      /* 0x48 */
	volatile uint32_t resv1[1];
	volatile uint16_t SDHCI_SET_ACMD12_ERROR; /* 0x50 */
	volatile uint16_t SDHCI_SET_INT_ERROR;    /* 0x52 */
	volatile uint32_t SDHCI_ADMA_ERROR;       /* 0x54 */
	volatile uint32_t SDHCI_ADMA_ADDRESS;     /* 0x58 */
	volatile uint32_t SDHCI_ADMA_ADDRESS_HI;  /* 0x5c */
	volatile uint32_t resv2[40];
    volatile uint16_t SDHCI_SLOT_INT_STATUS;  /* 0xfc */
    volatile uint16_t SDHCI_HOST_VERSION;     /* 0xfe */
} sdhci_t;

#define  BIT(x)                    (0x1 << x)

#define  SDHCI_CMD_INHIBIT	       BIT(0)
#define  SDHCI_DATA_INHIBIT	       BIT(1)

#define  SDHCI_CMD_RESP_MASK	0x03
#define  SDHCI_CMD_CRC		0x08
#define  SDHCI_CMD_INDEX	0x10
#define  SDHCI_CMD_DATA		0x20
#define  SDHCI_CMD_ABORTCMD	0xC0

#define  SDHCI_MAKE_CMD(c, f)      ((((c) & 0xff) << 8) | ((f) & 0xff))
#define  SDHCI_GET_CMD(c)          (((c)>>8) & 0x3f)

#define  SDHCI_INT_RESPONSE	       BIT(0)
#define  SDHCI_INT_DATA_END	       BIT(1)
#define  SDHCI_INT_DMA_END	       BIT(3)
#define  SDHCI_INT_SPACE_AVAIL	   BIT(4)
#define  SDHCI_INT_DATA_AVAIL	   BIT(5)
#define  SDHCI_INT_CARD_INSERT	   BIT(6)
#define  SDHCI_INT_CARD_REMOVE	   BIT(7)
#define  SDHCI_INT_CARD_INT	       BIT(8)
#define  SDHCI_INT_ERROR	       BIT(15)
#define  SDHCI_INT_TIMEOUT	       BIT(16)
#define  SDHCI_INT_CRC		       BIT(17)
#define  SDHCI_INT_END_BIT	       BIT(18)
#define  SDHCI_INT_INDEX	       BIT(19)
#define  SDHCI_INT_DATA_TIMEOUT	   BIT(20)
#define  SDHCI_INT_DATA_CRC	       BIT(21)
#define  SDHCI_INT_DATA_END_BIT	   BIT(22)
#define  SDHCI_INT_BUS_POWER	   BIT(23)
#define  SDHCI_INT_ACMD12ERR	   BIT(24)
#define  SDHCI_INT_ADMA_ERROR	   BIT(25)

#define  SDHCI_INT_CMD_MASK	       (SDHCI_INT_RESPONSE | SDHCI_INT_TIMEOUT |         \
		                            SDHCI_INT_CRC | SDHCI_INT_END_BIT | SDHCI_INT_INDEX)
#define  SDHCI_INT_DATA_MASK	   (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END |         \
		                            SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL |   \
		                            SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC |    \
		                            SDHCI_INT_DATA_END_BIT | SDHCI_INT_ADMA_ERROR)
#define  SDHCI_INT_ALL_MASK	       (0xFFFFFFFF)

#define  SDHCI_PROG_CLOCK_MODE     BIT(5)
#define  SDHCI_CLOCK_CARD_EN	   BIT(2)
#define  SDHCI_CLOCK_INT_STABLE	   BIT(1)

#define  SDHCI_RESET_ALL	       0x01
#define  SDHCI_RESET_CMD	       0x02
#define  SDHCI_RESET_DATA	       0x04

#define  SDHCI_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))
#define  SDHCI_DEFAULT_BOUNDARY_SIZE	(512 * 1024)
#define  SDHCI_DEFAULT_BOUNDARY_ARG	(7)

#define  SDHCI_TRNS_DMA		BIT(0)
#define  SDHCI_TRNS_BLK_CNT_EN	BIT(1)
#define  SDHCI_TRNS_ACMD12	BIT(2)
#define  SDHCI_TRNS_READ	BIT(4)
#define  SDHCI_TRNS_MULTI	BIT(5)

static void sdhci_set_clk(sdhci_t *reg)
{
    uint16_t clk;

    while(reg->SDHCI_PRESENT_STATE&(SDHCI_CMD_INHIBIT|SDHCI_DATA_INHIBIT));
    reg->SDHCI_CLOCK_CONTROL = 0x8001;
    do {
        clk = reg->SDHCI_CLOCK_CONTROL;
    }while(!(clk&SDHCI_CLOCK_INT_STABLE));
    reg->SDHCI_CLOCK_CONTROL = clk|SDHCI_CLOCK_CARD_EN;
}

void sdhci_send_cmd(uintptr_t addr,uint32_t cmd,uint32_t arg,uint32_t resp_type,uint8_t *data_ptr)
{
	sdhci_t *reg = (sdhci_t *)addr;

    if(cmd != 12) {
        while(reg->SDHCI_PRESENT_STATE&(SDHCI_CMD_INHIBIT|SDHCI_DATA_INHIBIT));
    } else {
        while(reg->SDHCI_PRESENT_STATE&SDHCI_CMD_INHIBIT);
    }

	reg->SDHCI_INT_STATUS = SDHCI_INT_ALL_MASK;
	reg->SDHCI_TIMEOUT_CONTROL = 0xe;

	reg->SDHCI_ARGUMENT = arg;
	if(data_ptr) {
		uint32_t data;
		reg->SDHCI_BLOCK_SIZE = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 512);
		reg->SDHCI_BLOCK_COUNT = 0x1;
		reg->SDHCI_TRANSFER_MODE = SDHCI_TRNS_BLK_CNT_EN|SDHCI_TRNS_READ;
		reg->SDHCI_COMMAND = SDHCI_MAKE_CMD(cmd, resp_type|SDHCI_CMD_DATA);
    	while((reg->SDHCI_INT_STATUS&(SDHCI_INT_RESPONSE|SDHCI_INT_DATA_AVAIL)) != (SDHCI_INT_RESPONSE|SDHCI_INT_DATA_AVAIL)){

		}
		reg->SDHCI_INT_STATUS = SDHCI_INT_RESPONSE|SDHCI_INT_DATA_AVAIL;
		for(uint32_t i=0;i<512;i+=4) {
			data = reg->SDHCI_BUFFER;
			data_ptr[0+i] = data&0xFF;
			data_ptr[1+i] = (data>>8)&0xFF;
			data_ptr[2+i] = (data>>16)&0xFF;
			data_ptr[3+i] = (data>>24)&0xFF;
		}
		while((reg->SDHCI_INT_STATUS&(SDHCI_INT_DATA_END)) != SDHCI_INT_DATA_END){

		}
		reg->SDHCI_INT_STATUS = SDHCI_INT_DATA_END;
	} else {
		reg->SDHCI_COMMAND = SDHCI_MAKE_CMD(cmd, resp_type);
    	while((reg->SDHCI_INT_STATUS&SDHCI_INT_RESPONSE) != SDHCI_INT_RESPONSE) {

		}
		reg->SDHCI_INT_STATUS = SDHCI_INT_RESPONSE;
	}

    reg->SDHCI_SOFTWARE_RESET = SDHCI_RESET_CMD;
    reg->SDHCI_SOFTWARE_RESET = SDHCI_RESET_DATA;
}

void sdhci_read_resp(uintptr_t addr,uint32_t *ptr,uint32_t resp_type)
{
	sdhci_t *reg = (sdhci_t *)addr;
	
	if(!ptr) {
		return;
	}
	switch (resp_type)
	{
	case SDHCI_CMD_RESP_SHORT:
		*ptr = reg->SDHCI_RESPONSE[0];
		break;
	case SDHCI_CMD_RESP_LONG:
		*ptr = reg->SDHCI_RESPONSE[0];
		*(ptr+1) = reg->SDHCI_RESPONSE[1];
		*(ptr+2) = reg->SDHCI_RESPONSE[2];
		*(ptr+3) = reg->SDHCI_RESPONSE[3];
		break;
	default:
		break;
	}
}

void sdhci_init(uintptr_t addr)
{
	sdhci_t *reg = (sdhci_t *)addr;

    reg->SDHCI_SOFTWARE_RESET = SDHCI_RESET_ALL;
    reg->SDHCI_POWER_CONTROL = 0xf;
    reg->SDHCI_INT_ENABLE = SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK;
    reg->SDHCI_SIGNAL_ENABLE = 0;

    sdhci_set_clk(reg);
}
