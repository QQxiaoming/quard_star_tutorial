#ifndef SDHCI_H
#define SDHCI_H

#define  SDHCI_CMD_RESP_NONE	   0x00
#define  SDHCI_CMD_RESP_LONG	   0x01
#define  SDHCI_CMD_RESP_SHORT	   0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY 0x03

void sdhci_init(uintptr_t addr);
void sdhci_read_resp(uintptr_t addr,uint32_t *ptr,uint32_t resp_type);
void sdhci_send_cmd(uintptr_t addr,uint32_t cmd,uint32_t arg,uint32_t resp_type,uint8_t *data_ptr);

#endif /* SDHCI_H */