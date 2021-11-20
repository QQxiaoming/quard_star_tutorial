#ifndef UART_UPDATE_H
#define UART_UPDATE_H

bool uart_update_request(void);
bool uart_update_xmodem_recv(uint64_t addr, uint32_t size);

#endif /* UART_UPDATE_H */
