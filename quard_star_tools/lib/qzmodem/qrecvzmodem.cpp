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
#include "qrecvzmodem.h"
#include "zglobal.h"
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QDir>

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "crctab.h"
#include "timing.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
#include <utime.h>
#include <sys/stat.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#if defined(Q_CC_MSVC)
#define PATH_MAX _MAX_PATH
#endif
#endif

QRecvZmodem::QRecvZmodem(int32_t timeout, QObject *parent) : QThread{parent} {
  int32_t no_timeout = timeout==-1?1:0;
  timeout = timeout==-1?1000:timeout*10;
  this->zm = new LowLevelStuff(no_timeout, timeout, 0, 0, 115200, 0, 1400, this);
  this->under_rsh = 0;
  this->restricted = 1;
  this->lzmanag = 0;
  this->zconv = 0;
  this->nflag = 0;
  this->min_bps = 0;
  this->min_bps_time = 120;
  this->stop_time = 0;
  this->try_resume = 0;
  this->makelcpathname = 1;
  this->rxclob = 0;
  this->o_sync = 0;
  this->pathname = NULL;
  memset(this->tcp_buf, 0, 256);
  this->errors = 0;
  this->tryzhdrtype = ZRINIT;
  this->rxclob = 0;
  this->skip_if_not_found = 0;
	m_fileDirPath = QDir::homePath();
  connect(this->zm, &LowLevelStuff::sendData, this, &QRecvZmodem::sendData);
  connect(this->zm, &LowLevelStuff::flushRecv, this, &QRecvZmodem::flushRecv);
  connect(this->zm, &LowLevelStuff::flushSend, this, &QRecvZmodem::flushSend);
  connect(this->zm, &LowLevelStuff::sendBrk, this, &QRecvZmodem::sendBrk);
  connect(this->zm, &LowLevelStuff::resetRecv, this, &QRecvZmodem::resetRecv);
}

void QRecvZmodem::run(void) {
  zm->baudrate = zm->io_mode(1);
  int exitcode = 0;
  if (rz_receive() == ZM_ERROR) {
    exitcode = 0200;
    zm->zreadline_canit();
  }
  zm->io_mode(0);
  if (exitcode && !zm->zmodem_requested)
    zm->zreadline_canit();
  if (exitcode)
    qInfo("Transfer incomplete");
  else
    qInfo("Transfer complete");
}

int QRecvZmodem::rz_receive_files(struct zm_fileinfo *zi) {
  int c;

  for (;;) {
    timing(1, NULL);
    c = rz_receive_file(zi);
    switch (c) {
    case ZEOF: {
      double d;
      long bps;
      d = timing(0, NULL);
      if (d == 0)
        d = 0.5; /* can happen if timing uses time() */
      bps = (zi->bytes_received - zi->bytes_skipped) / d;
#ifdef DEBUGZ
      qInfo("Bytes received: %7ld/%7ld   BPS:%-6ld", (long)zi->bytes_received,
            (long)zi->bytes_total, bps);
#else
      Q_UNUSED(bps);
#endif
    }
      /* FALL THROUGH */
    case ZSKIP:
      if (c == ZSKIP) {
        qInfo("Skipped");
      }
      switch (rz_zmodem_session_startup()) {
      case ZCOMPL:
        return OK;
      default:
        return ZM_ERROR;
      case ZFILE:
        break;
      }
      continue;
    default:
      return c;
    case ZM_ERROR:
      return ZM_ERROR;
    }
  }
}
int QRecvZmodem::rz_zmodem_session_startup(void) {
  int c, n;
  int zrqinits_received = 0;
  size_t bytes_in_block = 0;

  /* Spec 8.1: "When the ZMODEM receive program starts, it
     immediately sends a ZRINIT header to initiate ZMODEM file
     transfers...  The receive program resends its header at
     intervals for a suitable period of time (40 seconds
     total)...."

     On startup tryzhdrtype is, by default, set to ZRINIT
  */

  for (n = zm->zmodem_requested ? 15 : 5;
       (--n + zrqinits_received) >= 0 && zrqinits_received < 10;) {
    /* Set buffer length (0) and capability flags */

    /* We're going to snd a ZRINIT packet. */
    zm->zm_set_header_payload_bytes(
#ifdef CANBREAK
        (zm->zctlesc ? (CANFC32 | CANFDX | CANOVIO | CANBRK | TESCCTL)
                     : (CANFC32 | CANFDX | CANOVIO | CANBRK)),
#else
        (zm->zctlesc ? (CANFC32 | CANFDX | CANOVIO | TESCCTL)
                     : (CANFC32 | CANFDX | CANOVIO)),
#endif
        0, 0, 0);
    zm->zm_send_hex_header(tryzhdrtype);

    if (tryzhdrtype == ZSKIP) /* Don't skip too far */
      tryzhdrtype = ZRINIT;   /* CAF 8-21-87 */
  again:
    switch (zm->zm_get_header(NULL)) {
    case ZRQINIT:

      /* Spec 8.1: "[after sending ZRINIT] if the
       * receiving program receives a ZRQINIT
       * header, it resends the ZRINIT header." */

      /* getting one ZRQINIT is totally ok. Normally a ZFILE follows
       * (and might be in our buffer, so don't flush it). But if we
       * get more ZRQINITs than the sender has started up before us
       * and sent ZRQINITs while waiting.
       */
      zrqinits_received++;
      continue;

    case ZEOF:
      continue;
    case TIMEOUT:
      continue;
    case ZFILE:
      zconv = zm->Rxhdr[ZF0];
      if (!zconv)
        /* resume with sz -r is impossible (at least with unix sz)
         * if this is not set */
        zconv = ZCBIN;
      if (zm->Rxhdr[ZF1] & ZF1_ZMSKNOLOC) {
        zm->Rxhdr[ZF1] &= ~(ZF1_ZMSKNOLOC);
        skip_if_not_found = TRUE;
      }
      zmanag = zm->Rxhdr[ZF1];
      ztrans = zm->Rxhdr[ZF2];
      tryzhdrtype = ZRINIT;
      c = zm->zm_receive_data(secbuf, MAX_BLOCK, &bytes_in_block);
      zm->baudrate = zm->io_mode(3);
      if (c == GOTCRCW)
        return ZFILE;
      zm->zm_send_hex_header(ZNAK);
      goto again;
    case ZSINIT:
      /* Spec 8.1: "[after receiving the ZRINIT]
       * then sender may then send an optional
       * ZSINIT frame to define the receiving
       * program's Attn sequence, or to specify
       * complete control character escaping.  If
       * the ZSINIT header specified ESCCTL or ESC8,
       * a HEX header is used, and the receiver
       * activates the specified ESC modes before
       * reading the following data subpacket.  */

      /* this once was:
       * Zctlesc = TESCCTL & zm->Rxhdr[ZF0];
       * trouble: if rz get --escape flag:
       * - it sends TESCCTL to sz,
       *   get a ZSINIT _without_ TESCCTL (yeah - sender didn't know),
       *   overwrites Zctlesc flag ...
       * - sender receives TESCCTL and uses "|=..."
       * so: sz escapes, but rz doesn't unescape ... not good.
       */
      zm->zctlesc |= (TESCCTL & zm->Rxhdr[ZF0]);
      if (zm->zm_receive_data(attn, ZATTNLEN, &bytes_in_block) == GOTCRCW) {
        /* Spec 8.1: "[after receiving a
         * ZSINIT] the receiver sends a ZACK
         * header in response, containing
         * either the serial number of the
         * receiving program, or 0." */
        zm->zm_set_header_payload(1L);
        zm->zm_send_hex_header(ZACK);
        goto again;
      }
      zm->zm_send_hex_header(ZNAK);
      goto again;
    case ZFREECNT:
      zm->zm_set_header_payload(getfree());
      zm->zm_send_hex_header(ZACK);
      goto again;
    case ZCOMPL:
      goto again;
    default:
      continue;
    case ZFIN:
      zm->zm_ackbibi();
      return ZCOMPL;
    case ZRINIT:
      /* Spec 8.1: "If [after sending ZRINIT] the
         receiving program receives a ZRINIT header,
         it is an echo indicating that the sending
         program is not operational."  */
      qInfo("got ZRINIT");
      return ZM_ERROR;
    case ZCAN:
      qInfo("got ZCAN");
      return ZM_ERROR;
    }
  }
  return 0;
}
void QRecvZmodem::rz_checkpath(const char *name) {
  if (restricted) {
    const char *p;
    p = strrchr(name, '/');
    if (p)
      p++;
    else
      p = name;
    /* don't overwrite any file in very restricted mode.
     * don't overwrite hidden files in restricted mode */
    if ((restricted == 2 || *name == '.') && fopen(name, "r") != NULL) {
      zm->zreadline_canit();
      qInfo("Program: %s exists", name);
      bibi(-1);
    }
    /* restrict pathnames to current tree or uucppublic */
    if (strstr(name, "../")
#ifdef PUBDIR
        || (name[0] == '/' && strncmp(name, PUBDIR, strlen(PUBDIR)))
#endif
    ) {
      zm->zreadline_canit();
      qInfo("Program: Security Violation");
      bibi(-1);
    }
    if (restricted > 1) {
      if (name[0] == '.' || strstr(name, "/.")) {
        zm->zreadline_canit();
        qInfo("Program: Security Violation");
        bibi(-1);
      }
    }
  }
}

void QRecvZmodem::bibi(int n) {
  // FIXME: figure out how to avoid global zmodem_requested
  //  if (zmodem_requested)
  //  	write_modem_escaped_string_to_stdout(Attn);
  //  canit(zr );
  zm->io_mode(0);
  qFatal("caught signal %d; exiting", n);
  //exit(128 + n);
}

void QRecvZmodem::report(int sct) { qDebug("Blocks received: %d", sct); }
void QRecvZmodem::uncaps(char *s) {
  for (; *s; ++s)
    if (isupper((unsigned char)(*s)))
      *s = tolower(*s);
}
int QRecvZmodem::IsAnyLower(const char *s) {
  for (; *s; ++s)
    if (islower((unsigned char)(*s)))
      return TRUE;
  return FALSE;
}
int QRecvZmodem::rz_write_string_to_file(struct zm_fileinfo *zi, char *buf,
                                         size_t n) {
  char *p;

  if (n == 0)
    return OK;
  if (thisbinary) {
    if (((size_t)fout->write(buf, n)) != n)
      return ZM_ERROR;
  } else {
    if (zi->eof_seen)
      return OK;
    for (p = buf; n > 0; ++p, n--) {
      if (*p == '\r')
        continue;
      if (*p == CPMEOF) {
        zi->eof_seen = TRUE;
        return OK;
      }
      fout->putChar(*p);
    }
  }
  return OK;
}
int QRecvZmodem::rz_process_header(char *name, struct zm_fileinfo *zi) {
  QFile::OpenModeFlag openmode;
  char *p;
  static char *name_static = NULL;
  char *nameend;

  if (name_static)
    free(name_static);
  name_static = (char *)malloc(strlen(name) + 1);
  if (!name_static) {
    qFatal("out of memory");
    //exit(1);
  }
  strcpy(name_static, name);
  zi->fname = name_static;

#ifdef DEBUGZ
  qDebug("zmanag=%d, Lzmanag=%d", zmanag, lzmanag);
  qDebug("zconv=%d", zconv);
#endif

  /* set default parameters and overrides */
  openmode = QIODevice::WriteOnly;
  thisbinary = TRUE;
  if (lzmanag)
    zmanag = lzmanag;

  /*
   *  Process ZMODEM remote file management requests
   */
  if (zconv == ZCNL) /* Remote ASCII override */
    thisbinary = 0;
  if (zconv == ZCBIN) /* Remote Binary override */
    thisbinary = TRUE;
  if (zconv == ZCBIN && try_resume)
    zconv = ZCRESUM;
  if (zmanag == ZF1_ZMAPND && zconv != ZCRESUM)
    openmode = QFile::Append;
  if (skip_if_not_found)
    openmode = QFile::ReadWrite;

  zi->bytes_total = DEFBYTL;
  zi->eof_seen = 0;
  zi->modtime = 0;

  nameend = name + 1 + strlen(name);
  if (*nameend) { /* file coming from Unix or DOS system */
    long bytes_total;
    unsigned long modtime;
    unsigned int mode;
    sscanf(nameend, "%ld%lo%o", &bytes_total, &modtime, &mode);
    zi->modtime = modtime;
    zi->bytes_total = bytes_total;
    ++thisbinary;
  }

  /* Check for existing file */
  if (zconv != ZCRESUM && !rxclob && (zmanag & ZF1_ZMMASK) != ZF1_ZMCLOB &&
      (zmanag & ZF1_ZMMASK) != ZF1_ZMAPND) {
    fout = new QFile(m_fileDirPath+QDir::separator()+QString(name));
    if (fout->exists() && fout->open(QIODevice::ReadOnly)) {
      QFileInfo fi(m_fileDirPath+QDir::separator()+QString(name));
      char *tmpname;
      char *ptr;
      int i;
      if (zmanag == ZF1_ZMNEW || zmanag == ZF1_ZMNEWL) {
        if (zmanag == ZF1_ZMNEW) {
          if (fi.lastModified().toSecsSinceEpoch() > zi->modtime) {
            return ZM_ERROR; /* skips file */
          }
        } else {
          /* newer-or-longer */
          if (((size_t)fi.size()) >= zi->bytes_total &&
              fi.lastModified().toSecsSinceEpoch() > zi->modtime) {
            return ZM_ERROR; /* skips file */
          }
        }
        fout->close();
      } else if (zmanag == ZF1_ZMCRC) {
        int r = zm->zm_do_crc_check(fout, zi->bytes_total, 0);
        if (r == ZM_ERROR) {
          fout->close();
          return ZM_ERROR;
        }
        if (r != ZCRC_DIFFERS) {
          return ZM_ERROR; /* skips */
        }
        fout->close();
      } else {
        size_t namelen;
        fout->close();
        if ((zmanag & ZF1_ZMMASK) != ZF1_ZMCHNG) {
          qInfo("file exists, skipped: %s", name);
          return ZM_ERROR;
        }
        /* try to rename */
        namelen = strlen(name);
        tmpname = (char *)malloc(namelen + 6);
        memcpy(tmpname, name, namelen);
        ptr = tmpname + namelen;
        *ptr++ = '.';
        i = 0;
        do {
          snprintf(ptr, 5, "%d", i++);
          QFileInfo fi(m_fileDirPath+QDir::separator()+QString(tmpname));
          if (!fi.exists())
            break;
        } while (i < 1000);
        if (i == 1000) {
          free(tmpname);
          return ZM_ERROR;
        }
        free(name_static);
        name_static = (char *)malloc(strlen(tmpname) + 1);
        if (!name_static) {
          qFatal("out of memory");
          //exit(1);
        }
        strcpy(name_static, tmpname);
        free(tmpname);
        zi->fname = name_static;
      }
    } else {
      delete fout;
      fout = nullptr;
    }
  }

  if (!*nameend) {                 /* File coming from CP/M system */
    for (p = name_static; *p; ++p) /* change / to _ */
      if (*p == '/')
        *p = '_';

    if (*--p == '.') /* zap trailing period */
      *p = 0;
  }

  bool ret = false;
  emit approver(name_static, zi->bytes_total, zi->modtime, &ret);
  if (!ret) {
    qInfo("%s: rejected by approver callback", pathname);
    return ZM_ERROR;
  }

  {
    if (pathname)
      free(pathname);
    pathname = (char *)malloc((PATH_MAX) * 2);
    if (!pathname) {
      qFatal("out of memory");
      //exit(1);
    }
    strcpy(pathname, name_static);
    /* overwrite the "waiting to receive" line */
    qInfo("Receiving: %s", name_static);
    emit transferring(QString(name_static));
    rz_checkpath(name_static);
    if (nflag) {
      free(name_static);
      name_static = (char *)strdup("/dev/null");
      if (!name_static) {
        qFatal("out of memory");
        //exit(1);
      }
    }
    if (thisbinary && zconv == ZCRESUM) {
      fout = new QFile(m_fileDirPath+QDir::separator()+QString(name_static));
      if (fout->open(QIODevice::ReadWrite)) {
        QFileInfo fi(m_fileDirPath+QDir::separator()+QString(name_static));
        int can_resume = TRUE;
        if (zmanag == ZF1_ZMCRC) {
          int r = zm->zm_do_crc_check(fout, zi->bytes_total, fi.size());
          if (r == ZM_ERROR) {
            fout->close();
            return ZFERR;
          }
          if (r == ZCRC_DIFFERS) {
            can_resume = FALSE;
          }
        }
        if ((unsigned long)fi.size() > zi->bytes_total) {
          can_resume = FALSE;
        }
        /* retransfer whole blocks */
        zi->bytes_skipped = fi.size() & ~(1023);
        if (can_resume) {
          if (!fout->seek(zi->bytes_skipped)) {
            fout->close();
            return ZFERR;
          }
        } else
          zi->bytes_skipped = 0; /* resume impossible, file has changed */
        goto buffer_it;
      } else {
        delete fout;
        fout = nullptr;
      }
      zi->bytes_skipped = 0;
    }
    fout = new QFile(m_fileDirPath+QDir::separator()+QString(name_static));
    if (!fout->open(openmode)) {
      qCritical("cannot open %s: %s", name_static, fout->errorString().toUtf8().constData());
      return ZM_ERROR;
    }
  }
buffer_it:
  if (o_sync) {
    QFile::OpenMode oldflags = fout->openMode();
    uint64_t oldpos = fout->pos();
    fout->close();
    fout->open(oldflags | QFile::Unbuffered);
    fout->seek(oldpos);
  }
  zi->bytes_received = zi->bytes_skipped;

  return OK;
}
int QRecvZmodem::rz_receive_sector(size_t *Blklen, char *rxbuf,
                                   unsigned int maxtime) {
  int checksum, wcj, firstch;
  unsigned short oldcrc;
  char *p;
  int sectcurr;

  lastrx = 0;
  for (errors = 0; errors < RETRYMAX; errors++) {
    if ((firstch = zm->zreadline_getc(maxtime)) == STX) {
      *Blklen = 1024;
      goto get2;
    }
    if (firstch == SOH) {
      *Blklen = 128;
    get2:
      sectcurr = zm->zreadline_getc(1);
      if ((sectcurr + (oldcrc = zm->zreadline_getc(1))) == 0377) {
        oldcrc = checksum = 0;
        for (p = rxbuf, wcj = *Blklen; --wcj >= 0;) {
          if ((firstch = zm->zreadline_getc(1)) < 0)
            goto bilge;
          oldcrc = updcrc(firstch, oldcrc);
          checksum += (*p++ = firstch);
        }
        if ((firstch = zm->zreadline_getc(1)) < 0)
          goto bilge;
        oldcrc = updcrc(firstch, oldcrc);
        if ((firstch = zm->zreadline_getc(1)) < 0)
          goto bilge;
        oldcrc = updcrc(firstch, oldcrc);
        if (oldcrc & 0xFFFF)
          qCritical("CRC");
        else {
          firstsec = FALSE;
          return sectcurr;
        }
      } else
        qCritical("Sector number garbled");
    }
    /* make sure eot really is eot and not just mixmash */
    else if (firstch == EOT && zm->zreadline_getc(1) == TIMEOUT)
      return WCEOT;
    else if (firstch == CAN) {
      if (lastrx == CAN) {
        qCritical("Sender Cancelled");
        return ZM_ERROR;
      } else {
        lastrx = CAN;
        continue;
      }
    } else if (firstch == TIMEOUT) {
      if (firstsec)
        goto humbug;
    bilge:
      qCritical("TIMEOUT");
    } else
      qCritical("Got 0%o sector header", firstch);

  humbug:
    lastrx = 0;
    {
      int cnt = 1000;
      while (cnt-- && zm->zreadline_getc(1) != TIMEOUT)
        ;
    }
    if (firstsec) {
      zm->xsendline(WANTCRC);
      zm->flush_sendlines();
      zm->zreadline_flushline(); /* Do read next time ... */
    } else {
      maxtime = 40;
      zm->xsendline(NAK);
      zm->flush_sendlines();
      zm->zreadline_flushline(); /* Do read next time ... */
    }
  }
  /* try to stop the bubble machine. */
  zm->zreadline_canit();
  return ZM_ERROR;
}
int QRecvZmodem::rz_receive_sectors(struct zm_fileinfo *zi) {
  int sectnum, sectcurr;
  char sendchar;
  size_t Blklen;

  firstsec = TRUE;
  sectnum = 0;
  zi->eof_seen = FALSE;
  sendchar = WANTCRC;

  for (;;) {
    zm->xsendline(sendchar); /* send it now, we're ready! */
    zm->flush_sendlines();
    zm->zreadline_flushline(); /* Do read next time ... */
    sectcurr = rz_receive_sector(&Blklen, secbuf,
                                 (unsigned int)((sectnum & 0177) ? 50 : 130));
    report(sectcurr);
    if (sectcurr == ((sectnum + 1) & 0377)) {
      sectnum++;
      if (zi->bytes_total && R_BYTESLEFT(zi) < Blklen)
        Blklen = R_BYTESLEFT(zi);
      zi->bytes_received += Blklen;
      if (rz_write_string_to_file(zi, secbuf, Blklen) == ZM_ERROR)
        return ZM_ERROR;
      sendchar = ACK;
    } else if (sectcurr == (sectnum & 0377)) {
      qCritical("Received dup Sector");
      sendchar = ACK;
    } else if (sectcurr == WCEOT) {
      if (rz_closeit(zi))
        return ZM_ERROR;
      zm->xsendline(ACK);
      zm->flush_sendlines();
      zm->zreadline_flushline(); /* Do read next time ... */
      return OK;
    } else if (sectcurr == ZM_ERROR)
      return ZM_ERROR;
    else {
      qCritical("Sync Error");
      return ZM_ERROR;
    }
  }
}
int QRecvZmodem::rz_receive_pathname(struct zm_fileinfo *zi, char *rpn) {
  int c;
  size_t Blklen = 0; /* record length of received packets */

  zm->zreadline_getc(1);

et_tu:
  firstsec = TRUE;
  zi->eof_seen = FALSE;
  zm->xsendline(WANTCRC);
  zm->flush_sendlines();
  zm->zreadline_flushline(); /* Do read next time ... */
  while ((c = rz_receive_sector(&Blklen, rpn, 100)) != 0) {
    if (c == WCEOT) {
      qCritical("Pathname fetch returned EOT");
      zm->xsendline(ACK);
      zm->flush_sendlines();
      zm->zreadline_flushline(); /* Do read next time ... */
      zm->zreadline_getc(1);
      goto et_tu;
    }
    return ZM_ERROR;
  }
  zm->xsendline(ACK);
  zm->flush_sendlines();
  return OK;
}
int QRecvZmodem::rz_receive(void) {
  int c;
  struct zm_fileinfo zi;
  zi.fname = NULL;
  zi.modtime = 0;
  zi.bytes_total = 0;
  zi.bytes_sent = 0;
  zi.bytes_received = 0;
  zi.bytes_skipped = 0;
  zi.eof_seen = 0;

  qInfo("Program waiting to receive.");
  c = 0;
  c = rz_zmodem_session_startup();
  if (c != 0) {
    if (c == ZCOMPL)
      return OK;
    if (c == ZM_ERROR)
      goto fubar;
    c = rz_receive_files(&zi);

    if (c)
      goto fubar;
  } else {
    for (;;) {
      timing(1, NULL);
      if (rz_receive_pathname(&zi, secbuf) == ZM_ERROR)
        goto fubar;
      if (secbuf[0] == 0)
        return OK;
      if (rz_process_header(secbuf, &zi) == ZM_ERROR)
        goto fubar;
      if (rz_receive_sectors(&zi) == ZM_ERROR)
        goto fubar;

      double d;
      long bps;
      d = timing(0, NULL);
      if (d == 0)
        d = 0.5; /* can happen if timing uses time() */
      bps = (zi.bytes_received - zi.bytes_skipped) / d;

#ifdef DEBUGZ
      qInfo("\rBytes received: %7ld/%7ld   BPS:%-6ld", (long)zi.bytes_received,
            (long)zi.bytes_total, bps);
#else
      Q_UNUSED(bps);
#endif
    }
  }
  return OK;
fubar:
  zm->zreadline_canit();
  if (fout)
    fout->close();

  if (restricted && pathname) {
    qInfo("Program: %s removed.", pathname);
  }
  return ZM_ERROR;
}
int QRecvZmodem::rz_receive_file(struct zm_fileinfo *zi) {
  int c, n;
  long last_rxbytes = 0;
  unsigned long last_bps = 0;
  long not_printed = 0;
  time_t low_bps = 0;
  size_t bytes_in_block = 0;

  zi->eof_seen = FALSE;

  n = 20;

  if (rz_process_header(secbuf, zi) == ZM_ERROR) {
    return (tryzhdrtype = ZSKIP);
  }

  for (;;) {
    zm->zm_set_header_payload(zi->bytes_received);
    zm->zm_send_hex_header(ZRPOS);
    goto skip_oosb;
  nxthdr:
    if (anker) {
      oosb_t *akt, *last, *next;
      for (akt = anker, last = NULL, next = NULL; akt; last = akt ? akt : last, akt = next) {
        if (akt->pos == zi->bytes_received) {
          rz_write_string_to_file(zi, akt->data, akt->len);
          zi->bytes_received += akt->len;
          qDebug("using saved out-of-sync-paket %lx, len %ld", akt->pos,
                 akt->len);
          goto nxthdr;
        }
        next = akt->next;
        if (akt->pos < zi->bytes_received) {
          qDebug("removing unneeded saved out-of-sync-paket %lx, len %ld",
                 akt->pos, akt->len);
          if (last)
            last->next = akt->next;
          else
            anker = akt->next;
          free(akt->data);
          free(akt);
          akt = NULL;
        }
      }
    }
  skip_oosb:
    c = zm->zm_get_header(NULL);
    switch (c) {
    default:
      qDebug("rz_receive_file: zm_get_header returned %d", c);
      return ZM_ERROR;
    case ZNAK:
    case TIMEOUT:
      if (--n < 0) {
        qDebug("rz_receive_file: zm_get_header returned %d", c);
        return ZM_ERROR;
      }
      ZM_FALLTHROUGH();
    case ZFILE:
      zm->zm_receive_data(secbuf, MAX_BLOCK, &bytes_in_block);
      continue;
    case ZEOF:
      if (zm->zm_reclaim_receive_header() != (long)zi->bytes_received) {
        /*
         * Ignore eof if it's at wrong place - force
         *  a timeout because the eof might have gone
         *  out before we sent our zrpos.
         */
        errors = 0;
        goto nxthdr;
      }
      if (rz_closeit(zi)) {
        tryzhdrtype = ZFERR;
        qDebug("rz_receive_file: rz_closeit returned <> 0");
        return ZM_ERROR;
      }
#ifdef DEBUGZ
      qDebug("rz_receive_file: normal EOF");
#endif
      emit complete(QString(zi->fname), 0, zi->bytes_sent, zi->modtime);
      return c;
    case ZM_ERROR: /* Too much garbage in header search error */
      if (--n < 0) {
        qDebug("rz_receive_file: zm_get_header returned %d", c);
        return ZM_ERROR;
      }
      write_modem_escaped_string_to_stdout(attn);
      continue;
    case ZSKIP:
      rz_closeit(zi);
      qDebug("rz_receive_file: Sender SKIPPED file");
      return c;
    case ZDATA:
      if (zm->zm_reclaim_receive_header() != (long)zi->bytes_received) {
        oosb_t *neu;
        size_t pos = zm->zm_reclaim_receive_header();
        if (--n < 0) {
          qDebug("rz_receive_file: out of sync");
          return ZM_ERROR;
        }
        switch (c = zm->zm_receive_data(secbuf, MAX_BLOCK, &bytes_in_block)) {
        case GOTCRCW:
        case GOTCRCG:
        case GOTCRCE:
        case GOTCRCQ:
          if (pos > zi->bytes_received) {
            neu = (oosb_t *)malloc(sizeof(oosb_t));
            if (neu)
              neu->data = (char *)malloc(bytes_in_block);
            if (neu && neu->data) {
              qDebug("saving out-of-sync-block %lx, len %lu", pos,
                     (unsigned long)bytes_in_block);
              memcpy(neu->data, secbuf, bytes_in_block);
              neu->pos = pos;
              neu->len = bytes_in_block;
              neu->next = anker;
              anker = neu;
            } else if (neu)
              free(neu);
          }
        }
        write_modem_escaped_string_to_stdout(attn);
        continue;
      }
    moredata:
      if (not_printed > (min_bps ? 3 : 7) ||
          zi->bytes_received > last_bps / 2 + last_rxbytes) {
        int minleft = 0;
        int secleft = 0;
        time_t now;
        double d;
        d = timing(0, &now);
        if (d == 0)
          d = 0.5; /* timing() might use time() */
        last_bps = zi->bytes_received / d;
        if (last_bps > 0) {
          minleft = (R_BYTESLEFT(zi)) / last_bps / 60;
          secleft = ((R_BYTESLEFT(zi)) / last_bps) % 60;
        }
        if (min_bps) {
          if (low_bps) {
            if (last_bps < min_bps) {
              if (now - low_bps >= min_bps_time) {
                /* too bad */
                qDebug("rz_receive_file: bps rate %ld below min %ld", last_bps,
                       min_bps);
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
          qDebug("rz_receive_file: reached stop time");
          return ZM_ERROR;
        }

#ifdef DEBUGZ
        qInfo("\rBytes received: %7ld/%7ld   BPS:%-6ld ETA %02d:%02d  ",
              (long)zi->bytes_received, (long)zi->bytes_total, last_bps,
              minleft, secleft);
#endif
        bool ret = false;
        emit tick(zi->fname, zi->bytes_received, zi->bytes_total, last_bps,
                  minleft, secleft, &ret);
        last_rxbytes = zi->bytes_received;
        not_printed = 0;
      } else
        not_printed++;
      switch (c = zm->zm_receive_data(secbuf, MAX_BLOCK, &bytes_in_block)) {
      case ZCAN:
        qDebug("rz_receive_file: zm_receive_data returned %d", c);
        return ZM_ERROR;
      case ZM_ERROR: /* CRC error */
        if (--n < 0) {
          qDebug("rz_receive_file: zm_get_header returned %d", c);
          return ZM_ERROR;
        }
        write_modem_escaped_string_to_stdout(attn);
        continue;
      case TIMEOUT:
        if (--n < 0) {
          qDebug("rz_receive_file: zm_get_header returned %d", c);
          return ZM_ERROR;
        }
        continue;
      case GOTCRCW:
        n = 20;
        rz_write_string_to_file(zi, secbuf, bytes_in_block);
        zi->bytes_received += bytes_in_block;
        zm->zm_set_header_payload(zi->bytes_received);
        zm->zm_send_hex_header(ZACK | 0x80);
        goto nxthdr;
      case GOTCRCQ:
        n = 20;
        rz_write_string_to_file(zi, secbuf, bytes_in_block);
        zi->bytes_received += bytes_in_block;
        zm->zm_set_header_payload(zi->bytes_received);
        zm->zm_send_hex_header(ZACK);
        goto moredata;
      case GOTCRCG:
        n = 20;
        rz_write_string_to_file(zi, secbuf, bytes_in_block);
        zi->bytes_received += bytes_in_block;
        goto moredata;
      case GOTCRCE:
        n = 20;
        rz_write_string_to_file(zi, secbuf, bytes_in_block);
        zi->bytes_received += bytes_in_block;
        goto nxthdr;
      }
    }
  }
}

int QRecvZmodem::rz_closeit(struct zm_fileinfo *zi) {
  fout->close();
  if (zi->modtime) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    struct utimbuf timep;
    timep.actime = time(NULL);
    timep.modtime = zi->modtime;
    utime(pathname, &timep);
#endif
  }
  return OK;
}
void QRecvZmodem::write_modem_escaped_string_to_stdout(const char *s) {
  const char *p;

  while (s && *s) {
    p = strpbrk(s, "\335\336");
    if (!p) {
      zm->xsendlines(s, strlen(s));
      return;
    }
    if (p != s) {
      zm->xsendlines(s, (size_t)(p - s));
      s = p;
    }
    if (*p == '\336')
      QThread::sleep(1);
    else
      zm->xsendbrk();
    p++;
  }
}
size_t QRecvZmodem::getfree(void) {
  return ((size_t)(~0L)); /* many free bytes ... */
}
