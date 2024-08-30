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
#include "qsendzmodem.h"
#include "zglobal.h"

#include <QFile>
#include <QFileInfo>

#include <fcntl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
#include <utime.h>
#include <sys/stat.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#if defined(Q_CC_MSVC)
#define PATH_MAX _MAX_PATH
#endif
#endif

#include "crctab.h"
#include "timing.h"

#define OVERHEAD 18
#define OVER_ERR 20

#define ZM_SEND_DATA(x, y, z)                                                  \
  do {                                                                         \
    if (zm->crc32t) {                                                          \
      zm->zm_send_data32(x, y, z);                                             \
    } else {                                                                   \
      zm->zm_send_data(x, y, z);                                               \
    }                                                                          \
  } while (0)
#define DATAADR (txbuf)

QSendZmodem::QSendZmodem(int32_t timeout,QObject *parent) : QThread{parent} {
  int32_t no_timeout = timeout==-1?1:0;
  timeout = timeout==-1?1000:timeout*10;
  this->zm = new LowLevelStuff(no_timeout, timeout, 0, 0, 115200, 1, 1400, this);
  this->lzconv = 0;
  this->lzmanag = 0;
  this->lskipnocor = 0;
  this->txwindow = 0;
  this->txwspac = 0;
  this->txwcnt = 0;
  this->under_rsh = 0;
  this->no_unixmode = 0;
  this->canseek = 1;
  this->filesleft = 0;
  this->restricted = 0;
  this->fullname = 0;
  this->rxbuflen = 16384;
  this->blkopt = 0;
  this->tframlen = 0;
  this->wantfcs32 = 1;
  this->max_blklen = 1024;
  this->start_blklen = 1024;
  this->stop_time = 0;
  this->min_bps = 0;
  this->min_bps_time = 0;
  this->tcp_server_address = NULL;
  this->hyperterm = 0;
  connect(this->zm, &LowLevelStuff::sendData, this, &QSendZmodem::sendData);
  connect(this->zm, &LowLevelStuff::flushRecv, this, &QSendZmodem::flushRecv);
  connect(this->zm, &LowLevelStuff::flushSend, this, &QSendZmodem::flushSend);
  connect(this->zm, &LowLevelStuff::sendBrk, this, &QSendZmodem::sendBrk);
  connect(this->zm, &LowLevelStuff::resetRecv, this, &QSendZmodem::resetRecv);
}

void QSendZmodem::setFilePath(QStringList filePathList, QStringList remotePathList) {
    m_filePathList = filePathList;
    m_remotePathList = remotePathList;
}

void QSendZmodem::run(void) {
  if (start_blklen == 0) {
    start_blklen = 1024;
    if (tframlen) {
      start_blklen = max_blklen = tframlen;
    }
  }

  zm->baudrate = zm->io_mode(1);

  /* Spec 8.1: "The sending program may send the string "rz\r" to
     invoke the receiving program from a possible command
     mode." */
  zm->xsendlines("rz\r", strlen("rz\r"));
  zm->flush_sendlines();

  /* Spec 8.1: "The sending program may then display a message
   * intended for human consumption."  That would happen here,
   * if we did it.  */

  /* throw away any input already received. This doesn't harm
   * as we invite the receiver to send it's data again, and
   * might be useful if the receiver has already died or
   * if there is dirt left if the line
   */

  zm->zreadline_flushline();
  zm->cleanReadline();
  zm->zreadline_flushline();

  /* Spec 8.1: "Then the sender may send a ZRQINIT. The ZRQINIT
     header causes a previously started receive program to send
     its ZRINIT header without delay." */
  zm->zm_set_header_payload(0L);
  zm->zm_send_hex_header(ZRQINIT);
  zrqinits_sent++;
  zm->flush_sendlines();

  /* This is the main loop.  */
  if (sz_transmit_files(m_filePathList,m_remotePathList) == ZM_ERROR) {
    exitcode = 0200;
    zm->zreadline_canit();
  }
  zm->flush_sendlines();
  zm->io_mode(0);
  int dm = 0;
  if (exitcode)
    dm = exitcode;
  else if (errcnt)
    dm = 1;
  else
    dm = 0;
  if (dm)
    qInfo("Transfer incomplete");
  else
    qInfo("Transfer complete");
}

/* Send file name and related info */
int QSendZmodem::sz_transmit_file_by_zmodem(struct zm_fileinfo *zi,
                                            const char *buf, size_t blen) {
  int c;
  unsigned long crc;
  uint32_t rxpos;

  /* we are going to send a ZFILE. There cannot be much useful
   * stuff in the line right now (*except* ZCAN?).
   */

  for (;;) {
    /* Spec 8.2: "The sender then sends a ZFILE header
     * with ZMODEM Conversion, Management, and Transport
     * options followed by a ZCRCW data subpacket
     * containing the file name, ...." */
    zm->Txhdr[ZF0] = lzconv;  /* file conversion request */
    zm->Txhdr[ZF1] = lzmanag; /* file management request */
    if (lskipnocor)
      zm->Txhdr[ZF1] |= ZF1_ZMSKNOLOC;
    zm->Txhdr[ZF2] = 0; /* file transport compression request */
    zm->Txhdr[ZF3] = 0; /* extended options */
    zm->zm_send_binary_header(ZFILE);
    ZM_SEND_DATA(buf, blen, ZCRCW);
  again:
    c = zm->zm_get_header(&rxpos);
    switch (c) {
    case ZRINIT:
      while ((c = zm->zreadline_getc(50)) > 0)
        if (c == ZPAD) {
          goto again;
        }
      /* **** FALL THRU TO **** */
    default:
      continue;
    case ZRQINIT: /* remote site is sender! */
      qInfo("got ZRQINIT");
      return ZM_ERROR;
    case ZCAN:
      qInfo("got ZCAN");
      return ZM_ERROR;
    case TIMEOUT:
      return ZM_ERROR;
    case ZABORT:
      return ZM_ERROR;
    case ZFIN:
      return ZM_ERROR;
    case ZCRC:
      /* Spec 8.2: "[if] the receiver has a file
       * with the same name and length, [it] may
       * respond with a ZCRC header with a byte
       * count, which requires the sender to perform
       * a 32-bit CRC on the specified number of
       * bytes in the file, and transmit the
       * complement of the CRC is an answering ZCRC
       * header." */
      crc = 0xFFFFFFFFL;
      if (canseek >= 0) {
        if (rxpos == 0) {
          QFileInfo fi = QFileInfo(*input);
          rxpos = fi.size();
        }
        uint32_t i = 0;
        for (; i < rxpos; i++) {
          if (!input->getChar((char *)&c))
            break;
          crc = UPDC32(c, crc);
        }
        rxpos -= i;
        crc = ~crc;
        input->seek(0);
      }
      zm->zm_set_header_payload(crc);
      zm->zm_send_binary_header(ZCRC);
      goto again;
    case ZSKIP:
      /* Spec 8.2: "[after deciding if the file name, file
       * size, etc are acceptable] The receiver may respond
       * with a ZSKIP header, which makes the sender proceed
       * to the next file (if any)." */
      if(input)
        input->close();

      qDebug("receiver skipped");
      return c;
    case ZRPOS:
      /* Spec 8.2: "A ZRPOS header from the receiver
       * initiates transmittion of the file data
       * starting at the offset in the file
       * specified by the ZRPOS header.  */
      /*
       * Suppress zcrcw request otherwise triggered by
       * lastsync==bytcnt
       */
      if (rxpos && (!input->seek(rxpos))) {
        qCritical() << "seek failed: " << input->errorString();
        return ZM_ERROR;
      }
      if (rxpos)
        zi->bytes_skipped = rxpos;
      bytcnt = zi->bytes_sent = rxpos;
      lastsync = rxpos - 1;
      /* Spec 8.2: [in response to ZRPOS] the sender
       * sends a ZDATA binary header (with file
       * position) followed by one or more data
       * subpackets."  */
      return sz_transmit_file_contents_by_zmodem(zi);
    }
  }
}

int QSendZmodem::sz_getnak(void) {
  int firstch;
  int tries = 0;

  lastrx = 0;
  for (;;) {
    tries++;
    switch (firstch = zm->zreadline_getc(100)) {
    case ZPAD:
      /* Spec 7.3.1: "A binary header begins with
       * the sequence ZPAD, ZDLE, ZBIN. */
      /* Spec 7.3.3: "A hex header begins with the
       * sequence ZPAD ZPAD ZDLE ZHEX." */
      if (sz_getzrxinit())
        return ZM_ERROR;

      return FALSE;
    case TIMEOUT:
      /* 30 seconds are enough */
      if (tries == 3) {
        qCritical("Timeout on pathname");
        return TRUE;
      }
      /* don't send a second ZRQINIT _directly_ after the
       * first one. Never send more then 4 ZRQINIT, because
       * omen rz stops if it saw 5 of them */
      if ((zrqinits_sent > 1 || tries > 1) && zrqinits_sent < 4) {
        /* if we already sent a ZRQINIT we are
         * using zmodem protocol and may send
         * further ZRQINITs
         */
        zm->zm_set_header_payload(0L);
        zm->zm_send_hex_header(ZRQINIT);
        zrqinits_sent++;
      }
      continue;
    case WANTG:
      /* Set cbreak, XON/XOFF, etc. */
      zm->io_mode(2);
      optiong = TRUE;
      blklen = 1024;
      ZM_FALLTHROUGH();
    case WANTCRC:
      crcflg = TRUE;
      ZM_FALLTHROUGH();
    case NAK:
      /* Spec 8.1: "The sending program awaits a
       * command from the receiving port to start
       * file transfers.  If a 'C', 'G', or NAK is
       * received, and XMODEM or YMODEM file
       * transfer is indicated.  */
      return FALSE;
    case CAN:
      if ((firstch = zm->zreadline_getc(20)) == CAN && lastrx == CAN)
        return TRUE;
    default:
      break;
    }
    lastrx = firstch;
  }
}

int QSendZmodem::sz_transmit_pathname(struct zm_fileinfo *zi) {
  char *p, *q;
  QFileInfo fi = QFileInfo(*input);

  /* The sz_getnak process is how the sender knows which protocol
   * is it allowed to use.  Hopefully the receiver allows
   * ZModem.  If it doesn't, we may fall back to YModem. */
  if (!zm->zmodem_requested)
    if (sz_getnak()) {
      qDebug("sz_getnak failed");
      return ZM_ERROR;
    }

  q = (char *)0;

  for (p = zi->fname, q = txbuf; *p;)
    if ((*q++ = *p++) == '/' && !fullname)
      q = txbuf;
  *q++ = 0;
  p = q;
  while (q < (txbuf + MAX_BLOCK))
    *q++ = 0;
  if (*zi->fname) {
    if (hyperterm) {
      snprintf(p, 1000, "%lu", (unsigned long)fi.size());
    } else {
      /* note that we may lose some information here
       * in case mode_t is wider than an int. But i believe
       * sending %lo instead of %o _could_ break compatability
       */

      /* Spec 8.2: "[the sender will send a] ZCRCW
       * data subpacket containing the file name,
       * file length, modification date, and other
       * information identical to that used by
       * YMODEM batch." */
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
      struct stat st;
      int fd = input->handle();
      fstat(fd, &st);
      snprintf(p, 1000, "%lu %lo %o 0 %d %ld", (unsigned long)(fi.size()), (unsigned long)(fi.lastModified().toSecsSinceEpoch()),
              (unsigned int)((no_unixmode) ? 0 : st.st_mode), filesleft, totalleft);
#else
      snprintf(p,1000, "%lu %lo %o 0 %d %ld", (unsigned long)fi.size(), (unsigned long)fi.lastModified().toSecsSinceEpoch(),
              0, filesleft, totalleft);
#endif
    }
  }
  qInfo("Sending: %s", txbuf);
  emit transferring(QString(txbuf));
  totalleft -= fi.size();
  if (--filesleft <= 0)
    totalleft = 0;
  if (totalleft < 0)
    totalleft = 0;

  /* force 1k blocks if name won't fit in 128 byte block */
  if (txbuf[125])
    blklen = 1024;
  else { /* A little goodie for IMP/KMD */
    txbuf[127] = (fi.size() + 127) >> 7;
    txbuf[126] = (fi.size() + 127) >> 15;
  }

  /* We'll send the file by ZModem, if the sz_getnak process succeeded.  */
  if (zm->zmodem_requested)
    return sz_transmit_file_by_zmodem(zi, txbuf, 1 + strlen(p) + (p - txbuf));

  /* We'll have to send the file by YModem, I guess.  */
  if (sz_transmit_sector(txbuf, 0, 128) == ZM_ERROR) {
    qDebug("sz_transmit_sector failed");
    return ZM_ERROR;
  }
  return OK;
}
int QSendZmodem::sz_transmit_file(QString oname, QString remotename) {
  char name[PATH_MAX + 1];
  struct zm_fileinfo zi;

  /* First we do many checks to ensure that the filename is
   * valid and that the user is permitted to send these
   * files. */
  //if (restricted) {
  //}

  /* [mlg] I guess it was a feature that a filename of '-'
   * would mean that the file data could be piped in
   * and the name of the file to be transmitted was from the
   * ONAME env var or a temp name was generated. */
  QFileInfo fi(oname);
  /* Check for directory or block special files */
  if (!(fi.isFile() || fi.isSymLink())) {
    qCritical() << "is not a file: " << oname;
    return OK; /* pass over it, there may be others */
  }
  input = new QFile(oname);
  if (!input->open(QIODevice::ReadOnly)) {
    delete input;
    input = nullptr;
    qCritical() << "cannot open " << oname << ": " << input->errorString();
    return OK;
  }

  /* Here we finally start filling in information about the
   * file in a ZI structure.  We need this for the ZMODEM
   * file header when we send it. */
  QByteArray data;
  if (!remotename.isEmpty()) {
    data = remotename.toUtf8();
  } else {
    data = oname.toUtf8();
  }
  strcpy(name, data.data());
  zi.fname = name;
  zi.modtime = fi.lastModified().toSecsSinceEpoch();
  zi.bytes_total = fi.size();
  zi.bytes_sent = 0;
  zi.bytes_received = 0;
  zi.bytes_skipped = 0;
  zi.eof_seen = 0;
  timing(1, NULL);

  ++filcnt;
  /* Now that the file information is validated and is in a ZI
   * structure, we try to transmit the file. */
  switch (sz_transmit_pathname(&zi)) {
  case ZM_ERROR:
    return ZM_ERROR;
  case ZSKIP:
    qCritical() << "skipped: " << oname;
    return OK;
  }
  if (!zm->zmodem_requested && sz_transmit_file_contents(&zi) == ZM_ERROR) {
    return ZM_ERROR;
  }

  /* Here we make a log message the transmission of a single
   * file. */
  long bps;
  double d = timing(0, NULL);
  if (d == 0) /* can happen if timing() uses time() */
    d = 0.5;
  bps = zi.bytes_sent / d;
#ifdef DEBUGZ
  qDebug("Bytes Sent:%7ld   BPS:%-8ld", (long)zi.bytes_sent, bps);
#else
  Q_UNUSED(bps);
#endif
  emit complete(oname, 0, zi.bytes_sent, zi.modtime);

  return 0;
}
size_t QSendZmodem::sz_zfilbuf(struct zm_fileinfo *zi) {
  size_t n;

  n = input->read(txbuf, blklen);
  if (n < blklen)
    zi->eof_seen = 1;
  else {
    /* save one empty paket in case file ends ob blklen boundary */
    char c;
    if(input->getChar((char *)&c))
      input->ungetChar(c);
    else
      zi->eof_seen = 1;
  }
  return n;
}
size_t QSendZmodem::sz_filbuf(char *buf, size_t count) {
  int c;
  size_t m;

  m = input->read(buf, count);
  if (m <= 0)
    return 0;
  while (m < count)
    buf[m++] = 032;
  return count;
  m = count;
  if (lfseen) {
    *buf++ = 012;
    --m;
    lfseen = 0;
  }
  while (input->getChar((char *)&c)) {
    if (c == 012) {
      *buf++ = 015;
      if (--m == 0) {
        lfseen = TRUE;
        break;
      }
    }
    *buf++ = c;
    if (--m == 0)
      break;
  }
  if (m == count)
    return 0;
  else
    while (m-- != 0)
      *buf++ = CPMEOF;
  return count;
}
int QSendZmodem::sz_getzrxinit(void) {
  static int dont_send_zrqinit = 1;
  int old_timeout = zm->rxtimeout;
  int n;
  uint32_t rxpos;
  int timeouts = 0;

  zm->rxtimeout = 100; /* 10 seconds */

  for (n = 10; --n >= 0;) {
    /* we might need to send another zrqinit in case the first is
     * lost. But *not* if getting here for the first time - in
     * this case we might just get a ZRINIT for our first ZRQINIT.
     * Never send more then 4 ZRQINIT, because
     * omen rz stops if it saw 5 of them.
     */
    if (zrqinits_sent < 4 && n != 10 && !dont_send_zrqinit) {
      zrqinits_sent++;
      zm->zm_set_header_payload(0L);
      zm->zm_send_hex_header(ZRQINIT);
    }
    dont_send_zrqinit = 0;

    switch (zm->zm_get_header(&rxpos)) {
    case ZCHALLENGE: /* Echo receiver's challenge numbr */
      zm->zm_set_header_payload(rxpos);
      zm->zm_send_hex_header(ZACK);
      continue;
    case ZRINIT:
      rxflags = 0377 & zm->Rxhdr[ZF0];
      rxflags2 = 0377 & zm->Rxhdr[ZF1];
      zm->txfcs32 = (wantfcs32 && (rxflags & CANFC32));
      {
        int old = zm->zctlesc;
        zm->zctlesc |= rxflags & TESCCTL;
        /* update table - was initialised to not escape */
        if (zm->zctlesc && !old)
          zm->zm_escape_sequence_update();
      }
      rxbuflen = (0377 & zm->Rxhdr[ZP0]) + ((0377 & zm->Rxhdr[ZP1]) << 8);
      if (!(rxflags & CANFDX))
        txwindow = 0;
    #ifdef DEBUGZ
      qDebug("Rxbuflen=%d Tframlen=%d", rxbuflen, tframlen);
    #endif
      zm->io_mode(2); /* Set cbreak, XON/XOFF, etc. */
      /* Override to force shorter frame length */
      if (tframlen && rxbuflen > tframlen)
        rxbuflen = tframlen;
      if (!rxbuflen)
        rxbuflen = 1024;
    #ifdef DEBUGZ
      qDebug("Rxbuflen=%d", rxbuflen);
    #endif

      /* Set initial subpacket length */
      if (blklen < 1024) { /* Command line override? */
        if (zm->baudrate > 300)
          blklen = 256;
        if (zm->baudrate > 1200)
          blklen = 512;
        if (zm->baudrate > 2400)
          blklen = 1024;
      }
      if (rxbuflen && blklen > rxbuflen)
        blklen = rxbuflen;
      if (blkopt && blklen > blkopt)
        blklen = blkopt;
#ifdef DEBUGZ
      qDebug("Rxbuflen=%d blklen=%ld", rxbuflen, blklen);
      qDebug("Txwindow = %u Txwspac = %d", txwindow, txwspac);
#endif
      zm->rxtimeout = old_timeout;
      return (sz_sendzsinit());
    case ZCAN:
    case TIMEOUT:
      if (timeouts++ == 0)
        continue; /* force one other ZRQINIT to be sent */
      return ZM_ERROR;
    case ZRQINIT:
      if (zm->Rxhdr[ZF0] == ZCOMMAND)
        continue;
      ZM_FALLTHROUGH();
    default:
      zm->zm_send_hex_header(ZNAK);
      continue;
    }
  }
  return ZM_ERROR;
}
int QSendZmodem::sz_calculate_block_length(long total_sent) {
  static long total_bytes = 0;
  static int calcs_done = 0;
  static long last_error_count = 0;
  static int last_blklen = 0;
  static long last_bytes_per_error = 0;
  unsigned long best_bytes = 0;
  long best_size = 0;
  long this_bytes_per_error;
  long d;
  unsigned int i;
  if (total_bytes == 0) {
    /* called from sz_countem */
    total_bytes = total_sent;
    return 0;
  }

  /* it's not good to calc blklen too early */
  if (calcs_done++ < 5) {
    if (error_count && start_blklen > 1024)
      return last_blklen = 1024;
    else
      last_blklen /= 2;
    return last_blklen = start_blklen;
  }

  if (!error_count) {
    /* that's fine */
    if (start_blklen == max_blklen)
      return start_blklen;
    this_bytes_per_error = LONG_MAX;
    goto calcit;
  }

  if (error_count != last_error_count) {
    /* the last block was bad. shorten blocks until one block is
     * ok. this is because very often many errors come in an
     * short period */
    if (error_count & 2) {
      last_blklen /= 2;
      if (last_blklen < 32)
        last_blklen = 32;
      else if (last_blklen > 512)
        last_blklen = 512;
    #ifdef DEBUGZ
      qDebug("sz_calculate_block_length: reduced to %d due to error",
             last_blklen);
    #endif
    }
    last_error_count = error_count;
    last_bytes_per_error = 0; /* force recalc */
    return last_blklen;
  }

  this_bytes_per_error = total_sent / error_count;
  /* we do not get told about every error, because
   * there may be more than one error per failed block.
   * but one the other hand some errors are reported more
   * than once: If a modem buffers more than one block we
   * get at least two ZRPOS for the same position in case
   * *one* block has to be resent.
   * so don't do this:
   * this_bytes_per_error/=2;
   */
  /* there has to be a margin */
  if (this_bytes_per_error < 100)
    this_bytes_per_error = 100;

  /* be nice to the poor machine and do the complicated things not
   * too often
   */
  if (last_bytes_per_error > this_bytes_per_error)
    d = last_bytes_per_error - this_bytes_per_error;
  else
    d = this_bytes_per_error - last_bytes_per_error;
  if (d < 4) {
  #ifdef DEBUGZ
    qDebug("sz_calculate_block_length: returned old value %d due to low bpe "
           "diff",
           last_blklen);
    qDebug("sz_calculate_block_length: old %ld, new %ld, d %ld",
           last_bytes_per_error, this_bytes_per_error, d);
  #endif
    return last_blklen;
  }
  last_bytes_per_error = this_bytes_per_error;

calcit:
#ifdef DEBUGZ
  qDebug("sz_calculate_block_length: calc total_bytes=%ld, bpe=%ld, ec=%ld",
         total_bytes, this_bytes_per_error, (long)error_count);
#endif
  for (i = 32; i <= max_blklen; i *= 2) {
    long ok;     /* some many ok blocks do we need */
    long failed; /* and that's the number of blocks not transmitted ok */
    unsigned long transmitted;
    ok = total_bytes / i + 1;
    failed = ((long)i + OVERHEAD) * ok / this_bytes_per_error;
    transmitted =
        total_bytes + ok * OVERHEAD + failed * ((long)i + OVERHEAD + OVER_ERR);
  #ifdef DEBUGZ
    qDebug("sz_calculate_block_length: blklen %d, ok %ld, failed %ld -> %lu",
           i, ok, failed, transmitted);
  #endif
    if (transmitted < best_bytes || !best_bytes) {
      best_bytes = transmitted;
      best_size = i;
    }
  }
  if (best_size > 2 * last_blklen)
    best_size = 2 * last_blklen;
  last_blklen = best_size;
#ifdef DEBUGZ
  qDebug("sz_calculate_block_length: returned %d as best", last_blklen);
#endif
  return last_blklen;
}
int QSendZmodem::sz_sendzsinit(void) {
  int c;

  if (Myattn[0] == '\0' && (!zm->zctlesc || (rxflags & TESCCTL)))
    return OK;
  errors = 0;
  for (;;) {
    zm->zm_set_header_payload(0L);
    if (zm->zctlesc) {
      zm->Txhdr[ZF0] |= TESCCTL;
      zm->zm_send_hex_header(ZSINIT);
    } else
      zm->zm_send_binary_header(ZSINIT);
    ZM_SEND_DATA(Myattn, 1 + strlen(Myattn), ZCRCW);
    c = zm->zm_get_header(NULL);
    switch (c) {
    case ZCAN:
      return ZM_ERROR;
    case ZACK:
      return OK;
    default:
      if (++errors > 19)
        return ZM_ERROR;
      continue;
    }
  }
}
int QSendZmodem::sz_transmit_file_contents(struct zm_fileinfo *zi) {
  size_t thisblklen;
  int sectnum, attempts, firstch;

  firstsec = TRUE;
  thisblklen = blklen;
  qDebug("sz_transmit_file_contents:file length=%ld", (long)zi->bytes_total);

  while ((firstch = zm->zreadline_getc(zm->rxtimeout)) != NAK &&
         firstch != WANTCRC && firstch != WANTG && firstch != TIMEOUT &&
         firstch != CAN)
    ;
  if (firstch == CAN) {
    qCritical("Receiver Cancelled");
    return ZM_ERROR;
  }
  if (firstch == WANTCRC)
    crcflg = TRUE;
  if (firstch == WANTG)
    crcflg = TRUE;
  sectnum = 0;
  for (;;) {
    if (zi->bytes_total <= (zi->bytes_sent + 896L))
      thisblklen = 128;
    if (!sz_filbuf(txbuf, thisblklen))
      break;
    if (sz_transmit_sector(txbuf, ++sectnum, thisblklen) == ZM_ERROR)
      return ZM_ERROR;
    zi->bytes_sent += thisblklen;
  }
  input->close();
  attempts = 0;
  do {
    zm->zreadline_flushline();
    zm->xsendline(EOT);
    zm->flush_sendlines();
    ++attempts;
  } while ((firstch = (zm->zreadline_getc(zm->rxtimeout)) != ACK) &&
           attempts < RETRYMAX);
  if (attempts == RETRYMAX) {
    qCritical("No ACK on EOT");
    return ZM_ERROR;
  } else
    return OK;
}
int QSendZmodem::sz_transmit_file_contents_by_zmodem(struct zm_fileinfo *zi) {
  static int c;
  static int junkcount; /* Counts garbage chars received by TX */
  static size_t last_txpos = 0;
  static long last_bps = 0;
  static long not_printed = 0;
  static long total_sent = 0;
  static time_t low_bps = 0;

  lrxpos = 0;
  junkcount = 0;
somemore:
  /* Note that this whole next block is a
   * setjmp block for error recovery.  The
   * normal code path follows it. */
  if (setjmp(intrjmp)) {
  waitack:
    junkcount = 0;
    c = sz_getinsync(zi, 0);
  gotack:
    switch (c) {
    default:
      if(input)
        input->close();
      return ZM_ERROR;
    case ZCAN:
      if(input)
        input->close();
      return ZM_ERROR;
    case ZSKIP:
      if(input)
        input->close();
      return c;
    case ZACK:
    case ZRPOS:
      break;
    case ZRINIT:
      return OK;
    }
    /*
     * If the reverse channel can be tested for data,
     *  this logic may be used to detect error packets
     *  sent by the receiver, in place of setjmp/longjmp
     *  bytesAvailable(fdes) returns non 0 if a character is available
     */
    while (zm->bytesAvailable()) {
      switch (zm->zreadline_getc(1)) {
      case CAN:
      case ZPAD:
        c = sz_getinsync(zi, 1);
        goto gotack;
      case XOFF: /* Wait a while for an XON */
      case XOFF | 0200:
        zm->zreadline_getc(100);
      }
    }
  }

  /* Spec 8.2: "A ZRPOS header from the receiver initiates
   * transmittion of the file data starting at the offset in the
   * file specified by the ZRPOS header.  */
  /* Spec 8.2: [in response to ZRPOS] the sender sends a ZDATA
   * binary header (with file position) followed by one or more
   * data subpackets."  */

  txwcnt = 0;
  zm->zm_set_header_payload(zi->bytes_sent);
  zm->zm_send_binary_header(ZDATA);

  do {
    size_t n;
    int e;
    unsigned old = blklen;
    blklen = sz_calculate_block_length(total_sent);
    total_sent += blklen + OVERHEAD;
#ifdef DEBUGZ
    if (blklen != old)
      qDebug("blklen now %ld", blklen);
#else
    Q_UNUSED(old);
#endif
    n = sz_zfilbuf(zi);
    if (zi->eof_seen) {
      /* Spec 8.2: "In the absence of fatal error,
       * the sender eventually encounters end of
       * file.  If the end of file is encountered
       * within a frame, the frame is closed with a
       * ZCRCE data subpacket, which does not elicit
       * a response except in case of error." */
      e = ZCRCE;
#ifdef DEBUGZ
      qDebug("e=ZCRCE/eof seen");
#endif
    } else if (junkcount > 3) {
      /* Spec 8.2: "ZCRCW data subpackets expect a
       * response before the next frame is sent." */
      e = ZCRCW;
#ifdef DEBUGZ
      qDebug("e=ZCRCW/junkcount > 3");
#endif
    } else if (bytcnt == lastsync) {
      /* Spec 8.2: "ZCRCW data subpackets expect a
       * response before the next frame is sent." */
      e = ZCRCW;
#ifdef DEBUGZ
      qDebug("e=ZCRCW/bytcnt == lastsync == %ld", (unsigned long)lastsync);
#endif
    } else if (txwindow && (txwcnt += n) >= txwspac) {
      /* Spec 8.2: "ZCRCQ data subpackets expect a
       * ZACK response with the receiver's file
       * offset if no error, otherwise a ZRPOS
       * response with the last good file
       * offset. */
      txwcnt = 0;
      e = ZCRCQ;
#ifdef DEBUGZ
      qDebug("e=ZCRCQ/Window");
#endif
    } else {
      /* Spec 8.2: "A data subpacket terminated by
       * ZCRCG ... does not elicit a response unles
       * an error is detected: more data
       * subpacket(s) follow immediately."  */
      e = ZCRCG;
#ifdef DEBUGZ
      qDebug("e=ZCRCG");
#endif
    }
    if (not_printed > (min_bps ? 3 : 7) ||
        zi->bytes_sent > last_bps / 2 + last_txpos) {
      int minleft = 0;
      int secleft = 0;
      time_t now;
      last_bps = (zi->bytes_sent / timing(0, &now));
      if (last_bps > 0) {
        minleft = (zi->bytes_total - zi->bytes_sent) / last_bps / 60;
        secleft = ((zi->bytes_total - zi->bytes_sent) / last_bps) % 60;
      }
      if (min_bps) {
        if (low_bps) {
          if (last_bps < min_bps) {
            if (now - low_bps >= min_bps_time) {
              /* too bad */
              qInfo("sz_transmit_file_contents_by_zmodem: bps rate %ld below min %ld",
                    last_bps, min_bps);
              return ZM_ERROR;
            }
          } else
            low_bps = 0;
        } else if (last_bps < min_bps) {
          low_bps = now;
        }
      }
      if (stop_time && now >= stop_time) {
        /* too bad */
        qInfo("sz_transmit_file_contents_by_zmodem: reached stop time");
        return ZM_ERROR;
      }

#ifdef DEBUGZ
      qDebug("Bytes Sent:%7ld/%7ld   BPS:%-8ld ETA %02d:%02d  ",
             (long)zi->bytes_sent, (long)zi->bytes_total, last_bps, minleft,
             secleft);
#endif
      bool more = true;
      emit tick(zi->fname, (long)zi->bytes_sent, (long)zi->bytes_total,
                last_bps, minleft, secleft, &more);
      if (!more) {
        qInfo("sz_transmit_file_contents_by_zmodem: tick callback returns "
                "FALSE");
        return ZM_ERROR;
      }
      last_txpos = zi->bytes_sent;
    } else
      not_printed++;
    ZM_SEND_DATA(DATAADR, n, e);
    bytcnt = zi->bytes_sent += n;
    if (e == ZCRCW)
      /* Spec 8.2: "ZCRCW data subpackets expect a
       * response before the next frame is sent." */
      goto waitack;
    /*
     * If the reverse channel can be tested for data,
     *  this logic may be used to detect error packets
     *  sent by the receiver, in place of setjmp/longjmp
     *  bytesAvailable(fdes) returns non 0 if a character is available
     */
    zm->flush_sendlines();
    while (zm->bytesAvailable()) {
      switch (zm->zreadline_getc(1)) {
      case CAN:
      case ZPAD:
        c = sz_getinsync(zi, 1);
        if (c == ZACK)
          break;
        /* zcrce - dinna wanna starta ping-pong game */
        ZM_SEND_DATA(txbuf, 0, ZCRCE);
        goto gotack;
      case XOFF: /* Wait a while for an XON */
      case XOFF | 0200:
        zm->zreadline_getc(100);
        ZM_FALLTHROUGH();
      default:
        ++junkcount;
      }
    }
    if (txwindow) {
      size_t tcount = 0;
      while ((tcount = zi->bytes_sent - lrxpos) >= txwindow) {
#ifdef DEBUGZ
        qDebug("%ld (%ld,%ld) window >= %u", tcount, (long)zi->bytes_sent,
               (long)lrxpos, txwindow);
#endif
        if (e != ZCRCQ)
          ZM_SEND_DATA(txbuf, 0, e = ZCRCQ);
        c = sz_getinsync(zi, 1);
        if (c != ZACK) {
          ZM_SEND_DATA(txbuf, 0, ZCRCE);
          goto gotack;
        }
      }
#ifdef DEBUGZ
      qDebug("window = %ld", tcount);
#endif
    }
  } while (!zi->eof_seen);

  for (;;) {
    /* Spec 8.2: [after sending a file] The sender sends a
     * ZEOF header with the file ending offset equal to
     * the number of characters in the file. */
    zm->zm_set_header_payload(zi->bytes_sent);
    zm->zm_send_binary_header(ZEOF);
    switch (sz_getinsync(zi, 0)) {
    case ZACK:
      continue;
    case ZRPOS:
      goto somemore;
    case ZRINIT:
      /* If the receiver is satisfied with the file,
       * it returns ZRINIT. */
      return OK;
    case ZSKIP:
      if(input)
        input->close();
      return c;
    default:
      if(input)
        input->close();
      return ZM_ERROR;
    }
  }
}
int QSendZmodem::sz_getinsync(struct zm_fileinfo *zi, int flag) {
  int c;
  uint32_t rxpos;

  for (;;) {
    c = zm->zm_get_header(&rxpos);
    switch (c) {
    case ZCAN:
    case ZABORT:
    case ZFIN:
    case TIMEOUT:
      return ZM_ERROR;
    case ZRPOS:
      if (input){
        if(!input->seek(rxpos)) {
          return ZM_ERROR;
        }
      }
      zi->eof_seen = 0;
      bytcnt = lrxpos = zi->bytes_sent = rxpos;
      if (lastsync == rxpos) {
        error_count++;
      }
      lastsync = rxpos;
      return c;
    case ZACK:
      lrxpos = rxpos;
      if (flag || zi->bytes_sent == rxpos)
        return ZACK;
      continue;
    case ZRINIT:
    case ZSKIP:
      if (input)
        input->close();
      return c;
    case ZM_ERROR:
    default:
      error_count++;
      zm->zm_send_binary_header(ZNAK);
      continue;
    }
  }
}

int QSendZmodem::sz_transmit_files(QStringList filePathList, QStringList remotePathList) {
  crcflg = FALSE;
  firstsec = TRUE;
  bytcnt = (size_t)-1;

  /* Begin the main loop. */
  foreach (QString file, filePathList) {
    int index = filePathList.indexOf(file);
    totsecs = 0;
    /* The files are transmitted one at a time, here. */
    if (sz_transmit_file(file, remotePathList[index]) == ZM_ERROR)
      return ZM_ERROR;
  }
  totsecs = 0;
  if (filcnt == 0) { /* bitch if we couldn't open ANY files */
    zm->zreadline_canit();
    qInfo("Can't open any requested files.");
    return ZM_ERROR;
  }
  if (zm->zmodem_requested)
    /* The session to the receiver is terminated here. */
    zm->zm_saybibi();
  else {
    struct zm_fileinfo zi;
    char pa[PATH_MAX + 1];
    *pa = '\0';
    zi.fname = pa;
    zi.modtime = 0;
    zi.bytes_total = 0;
    zi.bytes_sent = 0;
    zi.bytes_received = 0;
    zi.bytes_skipped = 0;
    sz_transmit_pathname(&zi);
  }
  return OK;
}

int QSendZmodem::sz_transmit_sector(char *buf, int sectnum, size_t cseclen) {
  int checksum, wcj;
  char *cp;
  unsigned oldcrc;
  int firstch;
  int attempts;

  firstch = 0; /* part of logic to detect CAN CAN */

  qDebug("Zmodem sectors/kbytes sent: %3d/%2dk", totsecs, totsecs / 8);
  for (attempts = 0; attempts <= RETRYMAX; attempts++) {
    lastrx = firstch;
    zm->xsendline(cseclen == 1024 ? STX : SOH);
    zm->xsendline(sectnum & 0xFF);
    /* FIXME: clarify the following line - mlg */
    zm->xsendline((-sectnum - 1) & 0xFF);
    oldcrc = checksum = 0;
    for (wcj = cseclen, cp = buf; --wcj >= 0;) {
      zm->xsendline(*cp);
      oldcrc = updcrc((0377 & *cp), oldcrc);
      checksum += *cp++;
    }
    if (crcflg) {
      oldcrc = updcrc(0, updcrc(0, oldcrc));
      zm->xsendline(((int)oldcrc >> 8) & 0xFF);
      zm->xsendline(((int)oldcrc) & 0xFF);
    } else
      zm->xsendline(checksum & 0xFF);

    zm->flush_sendlines();
    if (optiong) {
      firstsec = FALSE;
      return OK;
    }
    firstch = zm->zreadline_getc(zm->rxtimeout);
  gotnak:
    switch (firstch) {
    case CAN:
      if (lastrx == CAN) {
      cancan:
        qCritical("Cancelled");
        return ZM_ERROR;
      }
      break;
    case TIMEOUT:
      qCritical("Timeout on sector ACK");
      continue;
    case WANTCRC:
      if (firstsec)
        crcflg = TRUE;
      ZM_FALLTHROUGH();
    case NAK:
      qCritical("NAK on sector");
      continue;
    case ACK:
      firstsec = FALSE;
      totsecs += (cseclen >> 7);
      return OK;
    case ZM_ERROR:
      qCritical("Got burst for sector ACK");
      break;
    default:
      qCritical("Got %02x for sector ACK", firstch);
      break;
    }
    for (;;) {
      lastrx = firstch;
      if ((firstch = zm->zreadline_getc(zm->rxtimeout)) == TIMEOUT)
        break;
      if (firstch == NAK || firstch == WANTCRC)
        goto gotnak;
      if (firstch == CAN && lastrx == CAN)
        goto cancan;
    }
  }
  qCritical("Retry Count Exceeded");
  return ZM_ERROR;
}
