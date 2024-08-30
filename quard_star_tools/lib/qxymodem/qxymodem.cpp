/*
 * This file is part of the https://github.com/QQxiaoming/qxymodem.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 3 of the 
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qxymodem.h"

#include <cstring>
#include <cstdint>

#define XMODEM_BUFFER_SIZE		1024

uint16_t QXYmodem::crc_xmodem_update(uint16_t crc, uint8_t data)
{
	int i;

	crc = crc ^ ((uint16_t)data << 8);
	for (i=0; i<8; i++) {
		if(crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}

	return crc;
}

long QXYmodem::xmodemReceive(void)
{
	/*
	 * create xmodem buffer
	 * 1024b for Xmodem 1K 128 bytes for Xmodem std.
	 * + 5b header/crc + NULL
	 */
	unsigned char xmbuf[XMODEM_BUFFER_SIZE+6];
	unsigned char seqnum = 1;		/* xmodem sequence number starts at 1    */
	unsigned short pktsize = 128;	/* default packet size is 128 bytes      */
	unsigned char response = 'C';	/* solicit a connection with CRC enabled */
    char retry = m_retry_limit;
	unsigned char crcflag = 0;
	unsigned long totalbytes = 0;
	int i;
	unsigned char c;

	while(retry > 0) {
		/* solicit a connection/packet */
		xmodemOut(response);
		/* wait for start of packet */
        if( (xmodemInTime(&c, m_timeout)) >= 0) {
			switch(c) {
				case SOH: {
					pktsize = 128;
                    break;
				}
			#if(XMODEM_BUFFER_SIZE>=1024)
				case STX: {
					pktsize = 1024;
                    break;
				}
			#endif
				case EOT: {
					xmodemInFlush();
					xmodemOut(ACK);
					/* completed transmission normally */
					return totalbytes;
				}
				case CAN: {
                    if((xmodemInTime(&c, m_timeout)) == CAN) {
						xmodemInFlush();
						xmodemOut(ACK);
						/* transaction cancelled by remote node */
						return XMODEM_ERROR_REMOTECANCEL;
					}
					break;
				}
				default:
                    continue;
			}
		} else {
			/* 
			 * timed out, try again
			 * no need to flush because receive buffer is already empty
			 */
			retry--;
			continue;
		}

		/* check if CRC mode was accepted */
		if(response == 'C') crcflag = 1;
		/* got SOH/STX, add it to processing buffer */
		xmbuf[0] = c;
		/* try to get rest of packet */
		for(i=0; i<(pktsize+crcflag+4-1); i++) {
            if((xmodemInTime(&c,m_timeout)) >= 0) {
				xmbuf[1+i] = c;
			} else {
				/* timed out, try again */
				retry--;
				xmodemInFlush();
				response = NAK;
				break;
			}
		}
		/* packet was too small, retry */
		if(i<(pktsize+crcflag+4-1))
			continue;

		/* 
		 * got whole packet
		 * check validity of packet
		 */
		if(	(xmbuf[1] == (unsigned char)(~xmbuf[2])) &&		/* sequence number was transmitted w/o error */
			xmodemCrcCheck(crcflag, &xmbuf[3], pktsize) ) {	/* packet is not corrupt */
			/* is this the packet we were waiting for? */
			if(xmbuf[1] == seqnum) {
                /* write/deliver data */
                int w = writefile((const char*)&xmbuf[3], pktsize);
				if(w == -1) {
					xmodemOut(NAK);
					return XMODEM_ABORT;
				} else {
					totalbytes += pktsize;
					/* next sequence number */
					seqnum++;
					/* reset retries */
					retry = m_retry_limit;
					/* reply with ACK */
					response = ACK;
				}
				continue;
			} else if(xmbuf[1] == (unsigned char)(seqnum-1)) {
				/* 
				 * this is a retransmission of the last packet
				 * ACK and move on
				 */
				response = ACK;
				continue;
			} else {
				/* 
				 * we are completely out of sync
				 * cancel transmission
				 */
				xmodemInFlush();
				xmodemOut(CAN);
				xmodemOut(CAN);
				xmodemOut(CAN);
				return XMODEM_ERROR_OUTOFSYNC;
			}
		} else {
			/* 
			 * packet was corrupt
			 * NAK it and try again
			 */
			retry--;
			xmodemInFlush();
			response = NAK;
			continue;
		}
	}

	/* exceeded retry count */
	xmodemInFlush();
	xmodemOut(CAN);
	xmodemOut(CAN);
	xmodemOut(CAN);
	return XMODEM_ERROR_RETRYEXCEED;
}


long QXYmodem::xmodemTransmit(unsigned short pktsize)
{
	/* still to be written */
	unsigned char xmbuf[XMODEM_BUFFER_SIZE+6];
	unsigned char seqnum = 1;		/* xmodem sequence number starts at 1    */
	unsigned char crcflag = 0;
    char retry = m_retry_limit;
	unsigned char c;
	while(retry > 0) {
        if( (xmodemInTime(&c, m_timeout)) >= 0) {
			switch(c) {
				case 'C': {
                    crcflag = 1;
                    goto start;
                }
				case NAK: {
                    crcflag = 0;
                    goto start;
                }
				case CAN: {
                    /* TODO: */
                    goto no_start;
				}
				default:
                    goto no_start;
			}
        start:
            break;
        no_start:
            continue;
		} else {
			/* 
			 * timed out, try again
			 * no need to flush because receive buffer is already empty
			 */
			retry--;
			continue;
		}
	}

	if(retry == 0) {
		return XMODEM_ERROR_RETRYEXCEED;
	} else {
        retry = m_retry_limit;
	}

	while (retry > 0)
	{
		unsigned short crc = 0;
		memset(xmbuf,CTRLZ,pktsize);
        int read_size = readfile((char*)xmbuf,pktsize);
		if(read_size == 0) {
			xmodemOut(EOT);
            retry = m_retry_limit;
			while (retry > 0) {
                if( (xmodemInTime(&c, m_timeout)) >= 0) {
					switch(c) {
						case ACK: {
							return 0;
						}
						case NAK: {
							return XMODEM_ERROR_RETRYEXCEED;
						}
						default:
							break;
					}
				} else {
					/* 
					* timed out, try again
					* no need to flush because receive buffer is already empty
					*/
					retry--;
					continue;
				}
				if(retry == 0) {
					return XMODEM_ERROR_RETRYEXCEED;
				}
			}
		} else if(read_size == -1) {
			xmodemOut(CAN);
			xmodemOut(CAN);
			xmodemOut(CAN);
			return XMODEM_ABORT;
		}
		xmodemOut(SOH);
		xmodemOut(seqnum);
		xmodemOut(0xff - seqnum);
		if(crcflag == 1) {
			/* do CRC */
			for(int i=0;i < pktsize;i++) {
				xmodemOut(xmbuf[i]);
				crc = crc_xmodem_update(crc, xmbuf[i]);
			}
			xmodemOut((unsigned char)((crc>>8)&0xFF));
			xmodemOut((unsigned char)(crc&0xFF));
		} else {
			/* do checksum */
			for(int i=0;i < pktsize;i++) {
				xmodemOut(xmbuf[i]);
				crc += xmbuf[i];
			}
			xmodemOut((unsigned char)(crc&0xFF));
		}
		seqnum = (seqnum + 1) % 0x100;
        if( (xmodemInTime(&c, m_timeout)) >= 0) {
			switch(c) {
				case ACK: {
					break;
				}
				case NAK: {
					return XMODEM_ERROR_RETRYEXCEED;
				}
				default:
					break;
			}
		} else {
			/* 
			 * timed out, try again
			 * no need to flush because receive buffer is already empty
			 */
			retry--;
			continue;
		}
	}
	
	if(retry == 0) {
		return XMODEM_ERROR_RETRYEXCEED;
	}

	return 0;
}

long QXYmodem::ymodemTransmit(unsigned short pktsize)
{
	/* still to be written */
	unsigned char xmbuf[XMODEM_BUFFER_SIZE+6];
	unsigned char seqnum = 0;		/* ymodem sequence number starts at 0    */
	unsigned char crcflag = 0;
    char retry = m_retry_limit;
	unsigned char c;
	while(retry > 0) {
        if( (xmodemInTime(&c, m_timeout)) >= 0) {
			switch(c) {
				case 'C': {
                    crcflag = 1;
                    goto start;
                }
				case NAK: {
                    crcflag = 0;
                    goto start;
                }
				case CAN: {
                    /* TODO: */
                    goto no_start;
				}
				default:
                    goto no_start;
			}
        start:
            break;
        no_start:
            continue;
		} else {
			/* 
			 * timed out, try again
			 * no need to flush because receive buffer is already empty
			 */
			retry--;
			continue;
		}
	}

	if(retry == 0) {
		return XMODEM_ERROR_RETRYEXCEED;
	} else {
        retry = m_retry_limit;
	}

	while (retry > 0)
	{
		unsigned short crc = 0;
        int read_size = 0;
		if(seqnum == 0) {
			memset(xmbuf,0x0,pktsize);
			read_size = readfileInfo((char*)xmbuf,pktsize);
			if(read_size == 0) {
				xmodemOut(SOH);
				xmodemOut(seqnum);
				xmodemOut(0xff - seqnum);
				if(crcflag == 1) {
					/* do CRC */
					for(int i=0;i < pktsize;i++) {
						xmodemOut(xmbuf[i]);
						crc = crc_xmodem_update(crc, xmbuf[i]);
					}
					xmodemOut((unsigned char)((crc>>8)&0xFF));
					xmodemOut((unsigned char)(crc&0xFF));
				} else {
					/* do checksum */
					for(int i=0;i < pktsize;i++) {
						xmodemOut(xmbuf[i]);
						crc += xmbuf[i];
					}
					xmodemOut((unsigned char)(crc&0xFF));
				}
				xmodemInTime(&c, m_timeout);
				return XMODEM_END;
			}
		} else {
			memset(xmbuf,CTRLZ,pktsize);
        	read_size = readfile((char*)xmbuf,pktsize);
		}
		if(read_size == 0) {
			xmodemOut(EOT);
            retry = m_retry_limit;
			while (retry > 0) {
                if( (xmodemInTime(&c, m_timeout)) >= 0) {
					switch(c) {
						case ACK: {
							return 0;
						}
						case NAK: {
							return XMODEM_ERROR_RETRYEXCEED;
						}
						default:
							break;
					}
				} else {
					/* 
					* timed out, try again
					* no need to flush because receive buffer is already empty
					*/
					retry--;
					continue;
				}
				if(retry == 0) {
					return XMODEM_ERROR_RETRYEXCEED;
				}
			}
		} else if(read_size == -1) {
			xmodemOut(CAN);
			xmodemOut(CAN);
			xmodemOut(CAN);
			return XMODEM_ABORT;
		}
		if(pktsize == 1024)
			xmodemOut(STX);
		else
			xmodemOut(SOH);
		xmodemOut(seqnum);
		xmodemOut(0xff - seqnum);
		if(crcflag == 1) {
			/* do CRC */
			for(int i=0;i < pktsize;i++) {
				xmodemOut(xmbuf[i]);
				crc = crc_xmodem_update(crc, xmbuf[i]);
			}
			xmodemOut((unsigned char)((crc>>8)&0xFF));
			xmodemOut((unsigned char)(crc&0xFF));
		} else {
			/* do checksum */
			for(int i=0;i < pktsize;i++) {
				xmodemOut(xmbuf[i]);
				crc += xmbuf[i];
			}
			xmodemOut((unsigned char)(crc&0xFF));
		}
		seqnum = (seqnum + 1) % 0x100;
        if( (xmodemInTime(&c, m_timeout)) >= 0) {
			switch(c) {
				case ACK: {
					break;
				}
				case NAK: {
					return XMODEM_ERROR_RETRYEXCEED;
				}
				default:
					break;
			}
		} else {
			/* 
			 * timed out, try again
			 * no need to flush because receive buffer is already empty
			 */
			retry--;
			continue;
		}
	}
	
	if(retry == 0) {
		return XMODEM_ERROR_RETRYEXCEED;
	}

	return 0;
}

long QXYmodem::ymodemReceive(void)
{
	/*
	 * create xmodem buffer
	 * 1024b for Xmodem 1K 128 bytes for Xmodem std.
	 * + 5b header/crc + NULL
	 */
	unsigned char xmbuf[XMODEM_BUFFER_SIZE+6];
	unsigned char seqnum = 0;		/* xmodem sequence number starts at 1    */
	unsigned short pktsize = 128;	/* default packet size is 128 bytes      */
	unsigned char response = 'C';	/* solicit a connection with CRC enabled */
    char retry = m_retry_limit;
	unsigned char crcflag = 0;
	unsigned long totalbytes = 0;
	int i;
	unsigned char c;

    Q_UNUSED(totalbytes);
	while(retry > 0) {
		/* solicit a connection/packet */
		xmodemOut(response);
		/* wait for start of packet */
        if( (xmodemInTime(&c, m_timeout)) >= 0) {
			switch(c) {
				case SOH: {
					pktsize = 128;
                    break;
				}
			#if(XMODEM_BUFFER_SIZE>=1024)
				case STX: {
					pktsize = 1024;
                    break;
				}
			#endif
				case EOT: {
					xmodemInFlush();
					xmodemOut(ACK);
					/* completed transmission normally */
					return 0;
				}
				case CAN: {
                    if((xmodemInTime(&c, m_timeout)) == CAN) {
						xmodemInFlush();
						xmodemOut(ACK);
						/* transaction cancelled by remote node */
						return XMODEM_ERROR_REMOTECANCEL;
					}
					break;
				}
				default:
                    continue;
			}
		} else {
			/* 
			 * timed out, try again
			 * no need to flush because receive buffer is already empty
			 */
			retry--;
			continue;
		}

		/* check if CRC mode was accepted */
		if(response == 'C') crcflag = 1;
		/* got SOH/STX, add it to processing buffer */
		xmbuf[0] = c;
		/* try to get rest of packet */
		for(i=0; i<(pktsize+crcflag+4-1); i++) {
            if((xmodemInTime(&c,m_timeout)) >= 0) {
				xmbuf[1+i] = c;
			} else {
				/* timed out, try again */
				retry--;
				xmodemInFlush();
				response = NAK;
				break;
			}
		}
		/* packet was too small, retry */
		if(i<(pktsize+crcflag+4-1))
			continue;

		/* 
		 * got whole packet
		 * check validity of packet
		 */
		if(	(xmbuf[1] == (unsigned char)(~xmbuf[2])) &&		/* sequence number was transmitted w/o error */
			xmodemCrcCheck(crcflag, &xmbuf[3], pktsize) ) {	/* packet is not corrupt */
			/* is this the packet we were waiting for? */
			if(xmbuf[1] == seqnum) {
				if(seqnum == 0) {
					if(xmbuf[3] != 0x0){
                		writefileInfo((const char*)&xmbuf[3], pktsize);
					} else {
						xmodemOut(ACK);
						return XMODEM_END;
					}
				} else {
                	/* write/deliver data */
					int w = writefile((const char*)&xmbuf[3], pktsize);
					if(w == -1) {
						xmodemOut(NAK);
						return XMODEM_ABORT;
					}
				}
				totalbytes += pktsize;
				/* next sequence number */
				seqnum++;
				/* reset retries */
                retry = m_retry_limit;
				/* reply with ACK */
				response = ACK;
				continue;
			} else if(xmbuf[1] == (unsigned char)(seqnum-1)) {
				/* 
				 * this is a retransmission of the last packet
				 * ACK and move on
				 */
				response = ACK;
				continue;
			} else {
				/* 
				 * we are completely out of sync
				 * cancel transmission
				 */
				xmodemInFlush();
				xmodemOut(CAN);
				xmodemOut(CAN);
				xmodemOut(CAN);
				return XMODEM_ERROR_OUTOFSYNC;
			}
		} else {
			/* 
			 * packet was corrupt
			 * NAK it and try again
			 */
			retry--;
			xmodemInFlush();
			response = NAK;
			continue;
		}
	}

	/* exceeded retry count */
	xmodemInFlush();
	xmodemOut(CAN);
	xmodemOut(CAN);
	xmodemOut(CAN);
	return XMODEM_ERROR_RETRYEXCEED;
}

int QXYmodem::xmodemCrcCheck(int crcflag, const unsigned char *buffer, int size)
{
	/* 
	 * crcflag=0 - do regular checksum
	 * crcflag=1 - do CRC checksum 
	 */
	if(crcflag) {
		unsigned short crc = 0;
		unsigned short pktcrc = (buffer[size]<<8)+buffer[size+1];
		/* do CRC checksum */
		while(size--)
			crc = crc_xmodem_update(crc, *buffer++);
		/* check checksum against packet */
		if(crc == pktcrc)
			return 1;
	} else {
		int i;
		unsigned char cksum = 0;
		/* do regular checksum */
		for(i=0; i<size; ++i) {
			cksum += buffer[i];
		}
		/* check checksum against packet */
		if(cksum == buffer[size])
			return 1;
	}

	return 0;
}

int QXYmodem::xmodemInTime(unsigned char *c, unsigned short timeout)
{
	int ret=-1;

retry:
	while( (timeout--) && ((ret = xmodemIn(c)) <= 0) ) {
        if(m_abort) break;
		timerPause(1);
	}

    if(m_abort)
		return -1;

	if(ret <= 0) {
        if(m_no_timeout) {
			timeout = 0xffff;
			goto retry;
		} else {
			return -1;
		}
	}

	return ret;
}

void QXYmodem::xmodemInFlush(void)
{
	flushfile();
}
