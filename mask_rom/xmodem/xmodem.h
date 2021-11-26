#ifndef XMODEM_H
#define XMODEM_H

/* xmodem control characters */
#define SOH			0x01
#define STX			0x02
#define EOT			0x04
#define ACK			0x06
#define NAK			0x15
#define CAN			0x18
#define CTRLZ		0x1A

/* xmodem timeout/retry parameters */
#define XMODEM_TIMEOUT_DELAY	1000
#define XMODEM_RETRY_LIMIT		16

/* error return codes */
#define XMODEM_ERROR_REMOTECANCEL	-1
#define XMODEM_ERROR_OUTOFSYNC		-2
#define XMODEM_ERROR_RETRYEXCEED	-3

/**
 * @brief initialize xmodem stream I/O routines
 */
void xmodemInit(void (*sendbyte_func)(unsigned char c), 
                int (*getbyte_func)(unsigned char *c), 
                void (*timerpause_func)(int t));

/**
 * @brief xmodem receive
 */
long xmodemReceive( int (*write)(unsigned char* buffer, int size) );

/**
 * @brief xmodem transmit
 */
long xmodemTransmit( int (*read)(unsigned char* buffer, int size) );

/**
 * @brief xmodem CRC/checksum error checking
 */
int xmodemCrcCheck(int crcflag, const unsigned char *buffer, int size);

/**
 * @brief get incoming character (wait for timeout)
 */
int xmodemInTime(unsigned char *c, unsigned short timeout);

/**
 * @brief flush incoming character stream
 */
void xmodemInFlush(void);

#endif
