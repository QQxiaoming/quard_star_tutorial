/*! \file xmodem.h \brief XModem Transmit/Receive Implementation with CRC and 1K support. */
//*****************************************************************************
//
// File Name	: 'xmodem.h'
// Title		: XModem Transmit/Receive Implementation with CRC and 1K support
// Author		: Pascal Stang - Copyright (C) 2006
// Created		: 4/22/2006
// Revised		: 7/22/2006
// Version		: 0.1
// Target MCU	: AVR processors
// Editor Tabs	: 4
//
///	\ingroup general
/// \defgroup xmodem XModem Transmit/Receive Implementation with CRC and 1K support (xmodem.c)
/// \code #include "xmodem.h" \endcode
/// \par Overview
///		This XModem implementation supports both 128b and 1K packets with or
///		without CRC checking.  The xmodem library must be initialized to use
///		a particular I/O stream by passing appropriate getbyte() and sendbyte()
///		functions to xmodemInit().  The xmodem transfer routines also expect
///		function pointers to read and write data blocks on the local system.
///		While this use of function pointers increases code size, it has great
///		adaptability.  The generalized read/write data functions mean that it
///		is easy to pipe data to/from any storage device like EEPROMs or flash
///		cards, rather than being limited to just processor RAM.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************
//@{

#ifndef XMODEM_H
#define XMODEM_H

// xmodem control characters
#define SOH			0x01
#define STX			0x02
#define EOT			0x04
#define ACK			0x06
#define NAK			0x15
#define CAN			0x18
#define CTRLZ		0x1A

// xmodem timeout/retry parameters
#define XMODEM_TIMEOUT_DELAY	1000
#define XMODEM_RETRY_LIMIT		16

// error return codes
#define XMODEM_ERROR_REMOTECANCEL	-1
#define XMODEM_ERROR_OUTOFSYNC		-2
#define XMODEM_ERROR_RETRYEXCEED	-3


//! initialize xmodem stream I/O routines
void xmodemInit(void (*sendbyte_func)(unsigned char c), int (*getbyte_func)(unsigned char *c), void (*timerpause_func)(int t));

//! xmodem receive
long xmodemReceive( int (*write)(unsigned char* buffer, int size) );

//! xmodem transmit
long xmodemTransmit( int (*read)(unsigned char* buffer, int size) );

//! xmodem CRC/checksum error checking
int xmodemCrcCheck(int crcflag, const unsigned char *buffer, int size);

// extra stream I/O functions
//! get incoming character (wait for timeout)
int xmodemInTime(unsigned char *c, unsigned short timeout);

//! flush incoming character stream
void xmodemInFlush(void);

#endif

//@}