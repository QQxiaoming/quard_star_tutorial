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
#ifndef QSENDZMODEM_H
#define QSENDZMODEM_H

#include "zglobal.h"
#include <setjmp.h>
#include "lowlevelstuff.h"
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

class QSendZmodem : public QThread {
  Q_OBJECT

public:
  explicit QSendZmodem(int32_t timeout = -1, QObject *parent = nullptr);
  ~QSendZmodem(){
    requestStop();
    wait();
  };
  void setFilePath(QStringList filePathList, QStringList remotePathList);
  void requestStop(void) {
      m_abort = true;
      zm->requestStop();
  }
  bool getStopFlag(void) {
      return m_abort;
  }

signals:
  void transferring(QString filename);
  void complete(QString filename, int result, size_t size, time_t date);
  void tick(const char *fname, long bytes_sent, long bytes_total, long last_bps,
            int min_left, int sec_left, bool *ret);
  void sendData(QByteArray data);
  void flushRecv(void);
  void flushSend(void);
  void sendBrk(void);
  void resetRecv(void);

public slots:
  void onRecvData(const QByteArray &data) {
    zm->onRecvData(data);
  }

protected:
  void run(void);

private:
  int sz_transmit_file_by_zmodem(struct zm_fileinfo *zi, const char *buf,
                                 size_t blen);
  int sz_getnak(void);
  int sz_transmit_pathname(struct zm_fileinfo *);
  int sz_transmit_file(QString oname, QString remotename);
  size_t sz_zfilbuf(struct zm_fileinfo *zi);
  size_t sz_filbuf(char *buf, size_t count);
  int sz_getzrxinit(void);
  int sz_calculate_block_length(long total_sent);
  int sz_sendzsinit(void);
  int sz_transmit_file_contents(struct zm_fileinfo *);
  int sz_transmit_file_contents_by_zmodem(struct zm_fileinfo *);
  int sz_getinsync(struct zm_fileinfo *, int flag);
  int sz_transmit_files(QStringList filePathList, QStringList remotePathList);
  int sz_transmit_sector(char *buf, int sectnum, size_t cseclen);

private:
  QStringList m_filePathList;
  QStringList m_remotePathList;
  LowLevelStuff *zm; /* zmodem comm primitives' state */
  // state
  char txbuf[MAX_BLOCK];
  QFile *input = nullptr;
  size_t lastsync; /* Last offset to which we got a ZRPOS */
  size_t bytcnt;
  char crcflg;
  int firstsec;
  unsigned txwindow; /* Control the size of the transmitted window */
  unsigned txwspac;  /* Spacing between zcrcq requests */
  unsigned txwcnt;   /* Counter used to space ack requests */
  size_t lrxpos;     /* Receiver's last reported offset */
  int errors;
  int under_rsh;
  char lastrx;
  long totalleft;
  int canseek;   /* 1: can; 0: only rewind, -1: neither */
  size_t blklen; /* length of transmitted records */
  int totsecs;   /* total number of sectors this file */
  int filcnt;    /* count of number of files opened */
  int lfseen;
  unsigned tframlen; /* Override for tx frame length */
  unsigned blkopt;   /* Override value for zmodem blklen */
  int rxflags;
  int rxflags2;
  int exitcode;
  time_t stop_time;
  char *tcp_server_address;
  int error_count;
  jmp_buf intrjmp; /* For the interrupt on RX CAN */
  int zrqinits_sent;
  char Myattn[2] = {0};

  // parameters
  char lzconv;  /* Local ZMODEM file conversion request */
  char lzmanag; /* Local ZMODEM file management request */
  int lskipnocor;
  int no_unixmode;
  int filesleft;
  int restricted;
  int fullname;
  int errcnt;        /* number of files unreadable */
  int optiong;       /* Let it rip no wait for sector ACK's */
  unsigned rxbuflen; /* Receiver's max buffer length */
  int wantfcs32;     /* want to send 32 bit FCS */
  size_t max_blklen;
  size_t start_blklen;
  long min_bps;
  long min_bps_time;
  int hyperterm;
  bool m_abort = false;
};

#endif // QSENDZMODEM_H
