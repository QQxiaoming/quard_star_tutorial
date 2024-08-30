/*
 * This file is part of the https://github.com/QQxiaoming/qzmodem.git
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
#include "lowlevelstuff.h"
#include "crctab.h"
#include "zglobal.h"
#include <QFileInfo>

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ <= 4)
#undef DEBUG_BLOCKSIZE
#endif

#ifdef DEBUG_BLOCKSIZE
struct debug_blocksize {
  int size;
  long count;
};
struct debug_blocksize blocksizes[] = {
    {32, 0},   {64, 0},   {128, 0},  {256, 0},  {512, 0},
    {1024, 0}, {2048, 0}, {4096, 0}, {8192, 0}, {0, 0}};
static inline void count_blk(int size) {
  for (int i = 0; blocksizes[i].size; i++) {
    if (blocksizes[i].size == size) {
      blocksizes[i].count++;
      return;
    }
  }
  blocksizes[i].count++;
}

#define COUNT_BLK(x) count_blk(x)
#else
#define COUNT_BLK(x)
#endif

#define badcrc "Bad CRC"
#define ISPRINT(x) ((unsigned)(x) & 0x60u)
static const char *frametypes[] = {
    "Carrier Lost", /* -3 */
    "TIMEOUT",      /* -2 */
    "ERROR",        /* -1 */
#define FTOFFSET 3
    "ZRQINIT",      "ZRINIT",  "ZSINIT",     "ZACK",   "ZFILE", "ZSKIP",
    "ZNAK",         "ZABORT",  "ZFIN",       "ZRPOS",  "ZDATA", "ZEOF",
    "ZFERR",        "ZCRC",    "ZCHALLENGE", "ZCOMPL", "ZCAN",  "ZFREECNT",
    "ZCOMMAND",     "ZSTDERR", "xxxxx"
#define FRTYPES 22 /* Total number of frame types in this array */
                   /*  not including psuedo negative entries */
};

const char *LowLevelStuff::Zendnames[4] = {"ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW"};

LowLevelStuff::LowLevelStuff(int no_timeout, int rxtimeout, int znulls, int eflag, int baudrate,
                             int zctlesc, int zrwindow, QObject *parent)
    : QObject{parent} {
  this->no_timeout = no_timeout;
  this->rxtimeout = rxtimeout;
  this->znulls = znulls;
  this->eflag = eflag;
  this->baudrate = baudrate;
  this->zctlesc = zctlesc;
  this->zrwindow = zrwindow;
  zm_escape_sequence_init();
}

int LowLevelStuff::zm_get_ascii_char(void) {
  int c;

  for (;;) {
    if ((c = zreadline_getc(rxtimeout)) < 0)
      return c;
    c &= 0x7F;
    switch (c) {
    case XON:
    case XOFF:
      continue;
    default:
      if (zctlesc && (c < ' '))
        continue;
      ZM_FALLTHROUGH();
    case '\r':
    case '\n':
    case ZDLE:
      return c;
    }
  }
}
int LowLevelStuff::zm_get_escaped_char(void) {
  int c = zreadline_getc(rxtimeout);

  /* Quick check for non control characters */
  if (ISPRINT(c))
    return c;
  return zm_get_escaped_char_internal(c);
}
int LowLevelStuff::zm_get_escaped_char_internal(int c) {
  goto jump_over; /* bad style */

again:
  /* Quick check for non control characters */
  c = zreadline_getc(rxtimeout);
  if (ISPRINT(c))
    return c;
jump_over:
  switch (c) {
    /* Spec 7.2: ZDLE represents a control sequence of some
       sort. */
  case ZDLE:
    break;

  /* Spec 7.2: The receiver ignores 021 (ASCII XON), 0221, 023
   * (ASCII XOFF), and 0223 characters in the data stream. */
  case XON:
  case (XON | 0x80):
  case XOFF:
  case (XOFF | 0x80):
    goto again;
  default:
    if (zctlesc && !ISPRINT(c)) {
      goto again;
    }
    return c;
  }
again2:
  /* We only end up here if the previous char was ZDLE.
   * We are in a control sequence. */

  if ((c = zreadline_getc(rxtimeout)) < 0)
    return c;

  /* Spec 7.2: Receipt of five successive CAN characters will
   * abort a ZMODEM session. [Note that ZDLE is also CAN, so the
   * ZDLE counts as the 1st CAN character, so we only need 4
   * CAN's.] */
  if (c == CAN && (c = zreadline_getc(rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = zreadline_getc(rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = zreadline_getc(rxtimeout)) < 0)
    return c;
  switch (c) {
  case CAN:
    return GOTCAN;
  case ZCRCE:
  case ZCRCG:
  case ZCRCQ:
  case ZCRCW:
    return (c | GOTOR);
  case ZRUB0:
    return 0x7F;
  case ZRUB1:
    return 0xFF;
  /* Spec 7.2: The receiver ignores 021 (ASCII XON), 0221, 023
   * (ASCII XOFF), and 0223 characters in the data stream. */
  case XON:
  case (XON | 0x80):
  case XOFF:
  case (XOFF | 0x80):
    goto again2;
  default:
    if (zctlesc && !ISPRINT(c)) {
      goto again2;
    }
    /* Spec 7.2: The receiving program decodes any sequence
     * of ZDLE followed by a byte with bit 6 set and bit 5 reset
     * (uppercase letter, either parity) to the equivalent control
     * character by inverting bit 6 */
    if ((c & 0b01100000) == 0b01000000)
      return (c ^ 0b01000000);
    break;
  }
  qDebug("Bad escape sequence %x", c);
  return ZM_ERROR;
}
int LowLevelStuff::zm_get_hex_encoded_byte(void) {
  int c, n;

  if ((c = zm_get_ascii_char()) < 0)
    return c;
  n = c - '0';
  if (n > 9)
    n -= ('a' - ':');
  if (n & ~0xF)
    return ZM_ERROR;
  if ((c = zm_get_ascii_char()) < 0)
    return c;
  c -= '0';
  if (c > 9)
    c -= ('a' - ':');
  if (c & ~0xF)
    return ZM_ERROR;
  c += (n << 4);
#ifdef DEBUGZ
  qDebug("zm_get_hex_encoded_byte: %02X", c);
#endif
  return c;
}
void LowLevelStuff::zputhex(int c, char *pos) {
  static char digits[] = "0123456789abcdef";

#ifdef DEBUGZ
  qDebug("zputhex: %02X", c);
#endif
  pos[0] = digits[(c & 0xF0) >> 4];
  pos[1] = digits[c & 0x0F];
}
int LowLevelStuff::zm_read_binary_header(void) {
  int c;
  unsigned short crc;

  if ((c = zm_get_escaped_char()) & ~0xFF)
    return c;
  rxtype = c;
  crc = updcrc(c, 0);

  for (int n = 0; n < 4; n++) {
    if ((c = zm_get_escaped_char()) & ~0xFF)
      return c;
    crc = updcrc(c, crc);
    Rxhdr[n] = c;
  }
  if ((c = zm_get_escaped_char()) & ~0xFF)
    return c;
  crc = updcrc(c, crc);
  if ((c = zm_get_escaped_char()) & ~0xFF)
    return c;
  crc = updcrc(c, crc);
  if (crc & 0xFFFF) {
    qCritical(badcrc);
    return ZM_ERROR;
  }
  zmodem_requested = TRUE;
  return rxtype;
}
int LowLevelStuff::zm_read_binary_header32(void) {
  int c;
  unsigned long crc;

  if ((c = zm_get_escaped_char()) & ~0xFF)
    return c;
  rxtype = c;
  crc = 0xFFFFFFFFL;
  crc = UPDC32(c, crc);
#ifdef DEBUGZ
  qDebug("zm_read_binary_header32 c=%X  crc=%lX", c, crc);
#endif

  for (int n = 0; n < 4; n++) {
    if ((c = zm_get_escaped_char()) & ~0xFF)
      return c;
    crc = UPDC32(c, crc);
    Rxhdr[n] = c;
#ifdef DEBUGZ
    qDebug("zm_read_binary_header32 c=%X  crc=%lX", c, crc);
#endif
  }
  for (int n = 0; n < 4; n++) {
    if ((c = zm_get_escaped_char()) & ~0xFF)
      return c;
    crc = UPDC32(c, crc);
#ifdef DEBUGZ
    qDebug("zm_read_binary_header32 c=%X  crc=%lX", c, crc);
#endif
  }
  if (crc != 0xDEBB20E3) {
    qCritical(badcrc);
    return ZM_ERROR;
  }
  zmodem_requested = TRUE;
  return rxtype;
}
int LowLevelStuff::zm_read_hex_header(void) {
  int c;
  unsigned short crc;

  if ((c = zm_get_hex_encoded_byte()) < 0)
    return c;
  rxtype = c;
  crc = updcrc(c, 0);

  for (int n = 0; n < 4; n++) {
    if ((c = zm_get_hex_encoded_byte()) < 0)
      return c;
    crc = updcrc(c, crc);
    Rxhdr[n] = c;
  }
  if ((c = zm_get_hex_encoded_byte()) < 0)
    return c;
  crc = updcrc(c, crc);
  if ((c = zm_get_hex_encoded_byte()) < 0)
    return c;
  crc = updcrc(c, crc);
  if (crc & 0xFFFF) {
    qCritical(badcrc);
    return ZM_ERROR;
  }
  switch (c = zreadline_getc(1)) {
  case 0215:
    /* **** FALL THRU TO **** */
  case 015:
    /* Throw away possible cr/lf */
    zreadline_getc(1);
    break;
  }
  zmodem_requested = TRUE;
  return rxtype;
}
int LowLevelStuff::zm_read_data32(char *buf, int length,
                                  size_t *bytes_received) {
  int c;
  unsigned long crc;
  int d;

  crc = 0xFFFFFFFFL;
  for (int i = 0; i < length + 1; i++) {
    if ((c = zm_get_escaped_char()) & ~0xFF) {
    crcfoo:
      switch (c) {
      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
      case GOTCRCW:
        d = c;
        c &= 0xFF;
        crc = UPDC32(c, crc);
        if ((c = zm_get_escaped_char()) & ~0xFF)
          goto crcfoo;
        crc = UPDC32(c, crc);
        if ((c = zm_get_escaped_char()) & ~0xFF)
          goto crcfoo;
        crc = UPDC32(c, crc);
        if ((c = zm_get_escaped_char()) & ~0xFF)
          goto crcfoo;
        crc = UPDC32(c, crc);
        if ((c = zm_get_escaped_char()) & ~0xFF)
          goto crcfoo;
        crc = UPDC32(c, crc);
        if (crc != 0xDEBB20E3) {
          qCritical(badcrc);
          return ZM_ERROR;
        }
        *bytes_received = i;
        COUNT_BLK(*bytes_received);
      #ifdef DEBUGZ
        qDebug("zm_read_data32: %lu %s", (unsigned long)*bytes_received,
               Zendnames[(d - GOTCRCE) & 3]);
      #endif
        return d;
      case GOTCAN:
        qCritical("Sender Canceled");
        return ZCAN;
      case TIMEOUT:
        qCritical("TIMEOUT");
        return c;
      default:
        qCritical("Bad data subpacket");
        return c;
      }
    }
    buf[i] = c;
    crc = UPDC32(c, crc);
  }
  qCritical("Data subpacket too long");
  return ZM_ERROR;
}
void LowLevelStuff::zm_send_binary_header32(int type) {
  unsigned long crc;

  /* Spec 7.3.2. A "32 bit CRC" binary header is similar to
   * a binary header, except the ZBIN character is replaced by a ZBIN32
   * character. */
  xsendline(ZBIN32);

  /* Put the type. */
  zm_put_escaped_char(type);

  crc = 0xFFFFFFFFL;
  crc = UPDC32(type, crc);

  /* Then, four bytes of flags or file position */
  for (int n = 0; n < 4; n++) {
    crc = UPDC32((0xFF & Txhdr[n]), crc);
    zm_put_escaped_char(Txhdr[n]);
  }
  crc = ~crc;

  /* Then, four bytes of CRC. */
  for (int n = 0; n < 4; n++) {
    zm_put_escaped_char((int)crc);
    crc >>= 8;
  }
}
void LowLevelStuff::zm_put_escaped_char(int c) {
  switch (escape_sequence_table[(unsigned)(c &= 0xFF)]) {
  case ZM_ESCAPE_NEVER:
    xsendline(lastsent = c);
    break;
  case ZM_ESCAPE_ALWAYS:
    xsendline(ZDLE);
    /* Spec 7.2: The receiving program decodes any sequence
     * of ZDLE followed by a byte with bit 6 set and bit 5 reset
     * (uppercase letter, either parity) to the equivalent control
     * character by inverting bit 6 */
    c ^= 0100;
    xsendline(lastsent = c);
    break;
  case ZM_ESCAPE_AFTER_AMPERSAND:
    if ((lastsent & 0x7F) != '@') {
      xsendline(lastsent = c);
    } else {
      xsendline(ZDLE);
      /* Spec 7.2: The receiving program decodes any sequence
       * of ZDLE followed by a byte with bit 6 set and bit 5 reset
       * (uppercase letter, either parity) to the equivalent control
       * character by inverting bit 6 */
      c ^= 0100;
      xsendline(lastsent = c);
    }
    break;
  }
}
void LowLevelStuff::zm_escape_sequence_init(void) {
  /* Spec 7.2: ZMODEM software escapes ZDLE, 020 (ASCII DLE), 0220,
   * 021 (DC1 aka XON), 0221, 023 (ASCII DC3 aka XOFF) and 0223. If
   * preceded by 0100 (ASCII '@') or 0300, 015 (ASCII CR) and 0215 are
   * also escaped to protect the Telenet command escape CR-@-CR. */
  for (int i = 0; i < 256; i++) {
    if (i & 0140)
      escape_sequence_table[i] = ZM_ESCAPE_NEVER;
    else {
      switch (i) {
      case ZDLE:
      case XOFF: /* ^Q */
      case XON:  /* ^S */
      case (XOFF | 0x80):
      case (XON | 0x80):
        escape_sequence_table[i] = ZM_ESCAPE_ALWAYS;
        break;
      case 020: /* ^P */
      case 0220:
        escape_sequence_table[i] = ZM_ESCAPE_ALWAYS;
        break;
      case 015:
      case 0215:
        if (zctlesc)
          escape_sequence_table[i] = ZM_ESCAPE_ALWAYS;
        else
          escape_sequence_table[i] = ZM_ESCAPE_AFTER_AMPERSAND;
        break;
      default:
        if (zctlesc)
          escape_sequence_table[i] = ZM_ESCAPE_ALWAYS;
        else
          escape_sequence_table[i] = ZM_ESCAPE_NEVER;
      }
    }
  }
}
void LowLevelStuff::zm_put_escaped_string(const char *s, size_t count) {
  const char *end = s + count;
  while (s != end) {
    int last_esc = 0;
    const char *t = s;
    while (t != end) {
      last_esc = escape_sequence_table[(unsigned)((*t) & 0xFF)];
      if (last_esc)
        break;
      t++;
    }
    if (t != s) {
      xsendlines(s, (size_t)(t - s));
      lastsent = t[-1];
      s = t;
    }
    if (last_esc) {
      int c = *s;
      switch (last_esc) {
      case 0:
        xsendline(lastsent = c);
        break;
      case 1:
        xsendline(ZDLE);
        c ^= 0100;
        xsendline(lastsent = c);
        break;
      case 2:
        if ((lastsent & 0x7F) != '@') {
          xsendline(lastsent = c);
        } else {
          xsendline(ZDLE);
          c ^= 0100;
          xsendline(lastsent = c);
        }
        break;
      }
      s++;
    }
  }
}

int LowLevelStuff::zm_get_zctlesc(void) { return zctlesc; }
void LowLevelStuff::zm_set_zctlesc(int x) { zctlesc = x; }
void LowLevelStuff::zm_escape_sequence_update(void) {
  zm_escape_sequence_init();
}
void LowLevelStuff::zm_send_binary_header(int type) {
  unsigned short crc;

#ifdef DEBUGZ
  qDebug("zm_send_binary_header: %s %lx", frametypes[type + FTOFFSET],
         zm_reclaim_send_header());
#endif

  if (type == ZDATA)
    for (int n = 0; n < znulls; n++)
      xsendline(0);

  xsendline(ZPAD);
  xsendline(ZDLE);

  crc32t = txfcs32;
  if (crc32t)
    zm_send_binary_header32(type);
  else {
    /* Spec 7.3.1. A binary header begins with the sequence
       ZPAD, ZDLE, ZBIN. */
    xsendline(ZBIN);
    /* .. The frame type byte is ZDLE encoded. */
    zm_put_escaped_char(type);
    crc = updcrc(type, 0);

    /* Then, the 4-byte flags or file position value. */
    for (int n = 0; n < 4; n++) {
      zm_put_escaped_char(Txhdr[n]);
      crc = updcrc((0xFF & Txhdr[n]), crc);
    }
    crc = updcrc(0, updcrc(0, crc));

    /* Then two bytes of CRC. */
    zm_put_escaped_char(crc >> 8);
    zm_put_escaped_char(crc);
  }
  if (type != ZDATA)
    flush_sendlines();
}
void LowLevelStuff::zm_send_hex_header(int type) {
  unsigned short crc;
  char s[30];
  size_t len;

#ifdef DEBUGZ
  qDebug("zm_send_hex_header: %s %lx", frametypes[(type & 0x7f) + FTOFFSET],
         zm_reclaim_send_header());
#endif

  /* Spec 7.3.3.  A hex header begins with the sequence ZPAD, ZPAD,
   * ZDLE, ZHEX. */
  s[0] = ZPAD;
  s[1] = ZPAD;
  s[2] = ZDLE;
  s[3] = ZHEX;
  zputhex(type & 0x7f, s + 4);
  len = 6;
  crc32t = 0;

  /* Spec 7.3.3.  The type byte, the four position/flag bytes,
   * and the 16-bit CRC thereof are sent in hex using
   * lower-case hex. */
  crc = updcrc((type & 0x7f), 0);
  for (int n = 0; n < 4; n++) {
    zputhex(Txhdr[n], s + len);
    len += 2;
    crc = updcrc((0xFF & Txhdr[n]), crc);
  }
  crc = updcrc(0, updcrc(0, crc));
  zputhex(crc >> 8, s + len);
  zputhex(crc, s + len + 2);
  len += 4;

  /* Spec 7.3.3.  A carriage return and line feed are sent with
   * HEX headers.  The receive routine expects to see at least
   * one of these characters. */
  s[len++] = 015;
  s[len++] = 0212;

  /* Spec 7.3.3. An XON character is appended to all HEX packets
   * except ZACK and ZFIN.  The XON releases the sender from
   * surious XOFF flow control characters generated by line
   * noise, a common occurrence.
   */
  if (type != ZFIN && type != ZACK) {
    s[len++] = 021;
  }
  flush_sendlines();
  xsendlines(s, len);
}
void LowLevelStuff::zm_send_data(const char *buf, size_t length, int frameend) {
  unsigned short crc;
#ifdef DEBUGZ
  qDebug("zm_send_data: %lu %s", (unsigned long)length,
         Zendnames[(frameend - ZCRCE) & 3]);
#endif
  crc = 0;
  for (size_t i = 0; i < length; i++) {
    zm_put_escaped_char(buf[i]);
    crc = updcrc((0xFF & buf[i]), crc);
  }
  xsendline(ZDLE);
  xsendline(frameend);
  crc = updcrc(frameend, crc);

  crc = updcrc(0, updcrc(0, crc));
  zm_put_escaped_char(crc >> 8);
  zm_put_escaped_char(crc);
  if (frameend == ZCRCW) {
    xsendline(XON);
    flush_sendlines();
  }
}
void LowLevelStuff::zm_send_data32(const char *buf, size_t length,
                                   int frameend) {
  int c;
  unsigned long crc;
#ifdef DEBUGZ
  qDebug("zsdat32: %zu %s", length, Zendnames[(frameend - ZCRCE) & 3]);
#endif

  crc = 0xFFFFFFFFL;
  zm_put_escaped_string(buf, length);
  for (size_t i = 0; i < length; i++) {
    c = buf[i] & 0xFF;
    crc = UPDC32(c, crc);
  }
  xsendline(ZDLE);
  xsendline(frameend);
  crc = UPDC32(frameend, crc);

  crc = ~crc;
  for (int i = 0; i < 4; i++) {
    c = (int)crc;
    if (c & 0140)
      xsendline(lastsent = c);
    else
      zm_put_escaped_char(c);
    crc >>= 8;
  }
  if (frameend == ZCRCW) {
    xsendline(XON);
    flush_sendlines();
  }
}
void LowLevelStuff::zm_set_header_payload(uint32_t val) {
  Txhdr[ZP0] = val;
  Txhdr[ZP1] = val >> 8;
  Txhdr[ZP2] = val >> 16;
  Txhdr[ZP3] = val >> 24;
}
void LowLevelStuff::zm_set_header_payload_bytes(uint8_t x0, uint8_t x1,
                                                uint8_t x2, uint8_t x3) {
  Txhdr[ZP0] = x0;
  Txhdr[ZP1] = x1;
  Txhdr[ZP2] = x2;
  Txhdr[ZP3] = x3;
}

long LowLevelStuff::zm_reclaim_send_header(void) {
  long l;

  l = (Txhdr[ZP3] & 0xFF);
  l = (l << 8) | (Txhdr[ZP2] & 0xFF);
  l = (l << 8) | (Txhdr[ZP1] & 0xFF);
  l = (l << 8) | (Txhdr[ZP0] & 0xFF);
  return l;
}
long LowLevelStuff::zm_reclaim_receive_header(void) {
  long l;

  l = (Rxhdr[ZP3] & 0xFF);
  l = (l << 8) | (Rxhdr[ZP2] & 0xFF);
  l = (l << 8) | (Rxhdr[ZP1] & 0xFF);
  l = (l << 8) | (Rxhdr[ZP0] & 0xFF);
  return l;
}
int LowLevelStuff::zm_receive_data(char *buf, int length,
                                   size_t *bytes_received) {
  int c;
  unsigned short crc;
  int d;

  *bytes_received = 0;
  if (rxframeind == ZBIN32)
    return zm_read_data32(buf, length, bytes_received);

  crc = 0;
  for (int i = 0; i < length + 1; i++) {
    if ((c = zm_get_escaped_char()) & ~0xFF) {
    crcfoo:
      switch (c) {
      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
      case GOTCRCW: {
        d = c;
        c &= 0xFF;
        crc = updcrc(c, crc);
        if ((c = zm_get_escaped_char()) & ~0xFF)
          goto crcfoo;
        crc = updcrc(c, crc);
        if ((c = zm_get_escaped_char()) & ~0xFF)
          goto crcfoo;
        crc = updcrc(c, crc);
        if (crc & 0xFFFF) {
          qCritical(badcrc);
          return ZM_ERROR;
        }
        *bytes_received = i;
        COUNT_BLK(*bytes_received);
#ifdef DEBUGZ
        qDebug("zm_receive_data: %lu  %s", (unsigned long)(*bytes_received),
               Zendnames[(d - GOTCRCE) & 3]);
#endif
        return d;
      }
      case GOTCAN:
        qCritical("Sender Canceled");
        return ZCAN;
      case TIMEOUT:
        qCritical("TIMEOUT");
        return c;
      default:
        qCritical("Bad data subpacket");
        return c;
      }
    }
    buf[i] = c;
    crc = updcrc(c, crc);
  }
  qCritical("Data subpacket too long");
  return ZM_ERROR;
}
int LowLevelStuff::zm_get_header(uint32_t *payload) {
  int c, cancount;
  unsigned int intro_msg_len, max_intro_msg_len;
  size_t rxpos = 0;
  char *intro_msg;

  /* Max bytes before start of frame */
  max_intro_msg_len = zrwindow + baudrate;
  intro_msg = (char *)calloc(max_intro_msg_len + 1, sizeof(char));
  intro_msg_len = 0;

  rxframeind = rxtype = 0;

startover:
  cancount = 5;
again:
  /* Return immediate ERROR if ZCRCW sequence seen */
  c = zreadline_getc(rxtimeout);
  switch (c) {
  case RCDO:
  case TIMEOUT:
    goto fifi;
    break;
  case CAN:
  gotcan:
    if (--cancount <= 0) {
      c = ZCAN;
      goto fifi;
    }
    switch (c = zreadline_getc(1)) {
    case TIMEOUT:
      goto again;
      break;
    case ZCRCW:
      c = ZM_ERROR;
      /* **** FALL THRU TO **** */
    case RCDO:
      goto fifi;
      break;
    default:
      break;
    case CAN:
      if (--cancount <= 0) {
        c = ZCAN;
        goto fifi;
      }
      goto again;
    }
    ZM_FALLTHROUGH();
  default:
  agn2:
    if (intro_msg_len > max_intro_msg_len) {
      qCritical("Intro message length exceeded");
      return (ZM_ERROR);
    }
    if (eflag == 1 && isprint(c))
      intro_msg[intro_msg_len++] = c;
    else if (eflag == 2)
      intro_msg[intro_msg_len++] = c;
    goto startover;
    break;

  /* Spec 7.3.1.  A binary header begins with the sequence ZPAD, ZDLE, ZBIN */
  /* Spec 7.3.2.  A 32 bit CRC binary header begins with ZPAD, ZDLE, ZBIN32 */
  /* Spec 7.3.3.  A hex header begins with the sequence ZPAD, ZPAD, ZDLE, ZHEX.
   */
  case ZPAD | 0x80:
  case ZPAD:
    /* Received 1st byte of a packet header. */
    break;
  }
  cancount = 5;
multiplezpad:
  switch (c = zm_get_ascii_char()) {
  case ZPAD:
    /* Multiple consecutive ZPADs can be treated as a single ZPAD. */
    goto multiplezpad;
  case RCDO:
  case TIMEOUT:
    goto fifi;
  default:
    goto agn2;
  case ZDLE:
    /* Received 2nd byte of a packet header. */
    break;
  }

  switch (c = zm_get_ascii_char()) {
  case RCDO:
  case TIMEOUT:
    goto fifi;
  case ZBIN:
    /* If the 3rd byte of a header is ZBIN, we're receiving a
     * binary packet with at 16-bit CRC. */
    rxframeind = ZBIN;
    crc32 = FALSE;
    c = zm_read_binary_header();
    break;
  case ZBIN32:
    /* If the 3rd byte of a header is ZBIN32, we're receiving a
     * binary packet with at 32-bit CRC. */
    crc32 = rxframeind = ZBIN32;
    c = zm_read_binary_header32();
    break;
  case ZHEX:
    /* If the 3rd byte of a header is ZHEX, we're receiving a
     * hex-encoded packet with at 16-bit CRC. */
    rxframeind = ZHEX;
    crc32 = FALSE;
    c = zm_read_hex_header();
    break;
  case CAN:
    goto gotcan;
  default:
    goto agn2;
  }
  rxpos = Rxhdr[ZP3] & 0xFF;
  rxpos = (rxpos << 8) + (Rxhdr[ZP2] & 0xFF);
  rxpos = (rxpos << 8) + (Rxhdr[ZP1] & 0xFF);
  rxpos = (rxpos << 8) + (Rxhdr[ZP0] & 0xFF);
fifi:
  /* 'c' should contain the TYPE byte from the packet header. */
  switch (c) {
  case GOTCAN:
    c = ZCAN;
    ZM_FALLTHROUGH();
  case ZNAK:
  case ZCAN:
  case ZM_ERROR:
  case TIMEOUT:
  case RCDO:
    qCritical("Got %s", frametypes[c + FTOFFSET]);
    ZM_FALLTHROUGH();
  default:
    if (c >= -3 && c <= FRTYPES) {
#ifdef DEBUGZ
      qDebug("zm_get_header: %s %lx", frametypes[c + FTOFFSET],
             (unsigned long)rxpos);
#endif
    } else {
#ifdef DEBUGZ
      qDebug("zm_get_header: %d %lx", c, (unsigned long)rxpos);
#endif
    }
  }
  if (payload)
    *payload = rxpos;

  /* If we got an intro message, log it. */
  if (intro_msg_len > 0) {
    qInfo("zm_get_header: received intro msg from sender...");
    qInfo("%s",intro_msg);
  }
  free(intro_msg);

  return c;
}
void LowLevelStuff::zm_ackbibi(void) {
  int n = 1; // modfied from 3 by QuardCRT

#ifdef DEBUGZ
  qDebug("ackbibi:");
#endif
  zm_set_header_payload(0);
  for (; --n >= 0;) {
    zreadline_flushline();
    zm_send_hex_header(ZFIN);
    switch (zreadline_getc(100)) {
    case 'O':
      zreadline_getc(1); /* Discard 2nd 'O' */
#ifdef DEBUGZ
      qDebug("ackbibi complete");
#endif
      return;
    case RCDO:
      return;
    case TIMEOUT:
    default:
      break;
    }
  }
}
void LowLevelStuff::zm_saybibi(void) {
  for (;;) {
    zm_set_header_payload(0); /* CAF Was zm_send_binary_header - minor change */

    /* Spec 8.3: "The sender closes the session with a
     * ZFIN header.  The receiver acknowledges this with
     * its own ZFIN header."  */
    zm_send_hex_header(ZFIN); /*  to make debugging easier */
    switch (zm_get_header(NULL)) {
    case ZFIN:
      /* Spec 8.3: "When the sender receives the
       * acknowledging header, it sends two
       * characters, "OO" (Over and Out) and exits
       * to the operating system or application that
       * invoked it." */
      xsendline('O');
      xsendline('O');
      flush_sendlines();
    case ZCAN:
    case TIMEOUT:
      return;
    }
  }
}
int LowLevelStuff::zm_do_crc_check(QFile *f, size_t remote_bytes,
                                   size_t check_bytes) {
  unsigned long crc;
  unsigned long rcrc;
  size_t n;
  char c;
  int t1 = 0, t2 = 0;
  QFileInfo fi(*f);
  if (check_bytes == 0 && ((size_t)fi.size()) != remote_bytes)
    return ZCRC_DIFFERS; /* shortcut */

  crc = 0xFFFFFFFFL;
  n = check_bytes;
  if (n == 0)
    n = fi.size();
  while (n-- && (f->getChar(&c)))
    crc = UPDC32(c, crc);
  crc = ~crc;
  f->seek(0);

  while (t1 < 3) {
    zm_set_header_payload(check_bytes);
    zm_send_hex_header(ZCRC);
    while (t2 < 3) {
      uint32_t tmp;
      c = zm_get_header(&tmp);
      rcrc = (unsigned long)tmp;
      switch (c) {
      default: /* ignore */
        break;
      case ZFIN:
        return ZM_ERROR;
      case ZRINIT:
        return ZM_ERROR;
      case ZCAN:
        qInfo("got ZCAN");
        return ZM_ERROR;
        break;
      case ZCRC:
        if (crc != rcrc)
          return ZCRC_DIFFERS;
        return ZCRC_EQUAL;
        break;
      }
    }
  }
  return ZM_ERROR;
}
