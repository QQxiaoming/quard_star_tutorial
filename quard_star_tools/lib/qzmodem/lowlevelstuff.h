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
#ifndef LOWLEVELSTUFF_H
#define LOWLEVELSTUFF_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <stddef.h>

#define ZCRC_DIFFERS (ZM_ERROR + 1)
#define ZCRC_EQUAL (ZM_ERROR + 2)

/* These are the values for the escape sequence table. */
#define ZM_ESCAPE_NEVER ((char)0)
#define ZM_ESCAPE_ALWAYS ((char)1)
#define ZM_ESCAPE_AFTER_AMPERSAND ((char)2)

class LowLevelStuff : public QObject {
  Q_OBJECT

public:
  explicit LowLevelStuff(int no_timeout, int rxtimeout, int znulls, int eflag, int baudrate,
                         int zctlesc, int zrwindow, QObject *parent = nullptr);
  ~LowLevelStuff(){
    requestStop();
  };
  void requestStop(void) {
      m_abort = true;
      condition.wakeOne();
  }
  bool getStopFlag(void) {
      return m_abort;
  }
  int zm_get_zctlesc(void);
  void zm_set_zctlesc(int zctlesc);
  void zm_escape_sequence_update(void);
  void zm_put_escaped_char(int c);
  void zm_send_binary_header(int type);
  void zm_send_hex_header(int type);
  void zm_send_data(const char *buf, size_t length, int frameend);
  void zm_send_data32(const char *buf, size_t length, int frameend);
  void zm_set_header_payload(uint32_t val);
  void zm_set_header_payload_bytes(uint8_t x0, uint8_t x1, uint8_t x2,
                                   uint8_t x3);

  long zm_reclaim_send_header(void);
  long zm_reclaim_receive_header(void);
  int zm_receive_data(char *buf, int length, size_t *received);
  int zm_get_header(uint32_t *payload);
  void zm_ackbibi(void);
  void zm_saybibi(void);
  int zm_do_crc_check(QFile *f, size_t remote_bytes, size_t check_bytes);

  void xsendlines(const char *s, size_t n) { emit sendData(QByteArray(s, n)); }
  void flush_sendlines(void) { 
    emit flushSend(); 
  }
  void xsendbrk(void) { 
    emit sendBrk(); 
  }
  int zreadline_getc(int timeout) {
    uint8_t c;
    bool ok = false;
    if(getStopFlag()) {
      return -2;
    }
    mutex.lock();
    size_t size = dataRecv.size();
    if(size >= 1) {
      ok = true;
    } else {
      if(no_timeout) {
      retry:
        condition.wait(&mutex);
        if(getStopFlag()) {
          mutex.unlock();
          return -2;
        }
        size = dataRecv.size();
        if(size < 1) {
          goto retry;
        }
      } else {
        QDeadlineTimer timer(timeout*100);
        do {
          condition.wait(&mutex, timer.remainingTime());
          if(getStopFlag()) {
            mutex.unlock();
            return -2;
          }
          size = dataRecv.size();
          if(size >= 1) {
            ok = true;
          }
        } while(size < 1 && timer.remainingTime() > 0);
      }
    }
    if(ok) {
      memcpy(&c,dataRecv.data(),1);
      dataRecv.remove(0,1);
      mutex.unlock();
      return c;
    }
    mutex.unlock();
    return -2;
  }
  void zreadline_flushline(void) {
    emit flushRecv();
  }
  int bytesAvailable(void) {
    return dataRecv.size();
  }
  void cleanReadline(void) {
    emit resetRecv();
    QMutexLocker locker(&mutex);
    dataRecv.clear();
  }

  void xsendline(char c) { xsendlines(&c, 1); }
  void zreadline_canit(void) {
    static char canistr[] = {24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 8,
                             8,  8,  8,  8,  8,  8,  8,  8,  8,  0};
    zreadline_flushline();
    xsendlines(canistr, strlen(canistr));
  }
  /*
   * mode(n)
   *  3: save old tty stat, set raw mode with flow control
   *  2: set XON/XOFF for sb/sz with ZMODEM
   *  1: save old tty stat, set raw mode
   *  0: restore original tty mode
   * Returns the output baudrate, or zero on failure
   */
  int io_mode(int n) { Q_UNUSED(n);return 0; }

  int no_timeout; /* when true, readline does not timeout */

signals:
  void sendData(QByteArray data);
  void flushRecv(void);
  void flushSend(void);
  void sendBrk(void);
  void resetRecv(void);

public slots:
  void onRecvData(const QByteArray &data) {
    QMutexLocker locker(&mutex);
    dataRecv.append(data);
    condition.wakeOne();
  }

private:
  static const char *Zendnames[4];
  int zm_get_ascii_char(void);
  int zm_get_escaped_char(void);
  int zm_get_escaped_char_internal(int);
  int zm_get_hex_encoded_byte(void);
  void zputhex(int c, char *pos);
  int zm_read_binary_header(void);
  int zm_read_binary_header32(void);
  int zm_read_hex_header(void);
  int zm_read_data32(char *buf, int length, size_t *);
  void zm_send_binary_header32(int type);
  void zm_escape_sequence_init(void);
  void zm_put_escaped_string(const char *str, size_t len);
  void recvData(uint8_t *data, size_t length, int timeout);

public:
  QByteArray dataRecv;
  QMutex mutex;
  QWaitCondition condition;
  char Rxhdr[4]; /* Received header */
  char Txhdr[4]; /* Transmitted header */
  int rxtimeout; /* Constant: tenths of seconds to wait for something */
  int znulls; /* Constant: Number of nulls to send at beginning of ZDATA hdr */
  int eflag;  /* Constant: local display of non zmodem characters */
              /* 0:  no display */
              /* 1:  display printing characters only */
              /* 2:  display all non ZMODEM characters */
  int baudrate; /* Constant: in bps */
  int zrwindow; /* RX window size (controls garbage count) */

  int zctlesc; /* Variable: TRUE means to encode control characters */
  int txfcs32; /* Variable: TRUE means send binary frames with 32 bit FCS */

  int rxtype;                      /* State: type of header received */
  char escape_sequence_table[256]; /* State: conversion chart for zmodem escape
                                      sequence encoding */
  char lastsent;                   /* State: last byte send */
  int crc32t;     /* State: display flag indicating 32-bit CRC being sent */
  int crc32;      /* State: display flag indicating 32 bit CRC being received */
  int rxframeind; /* State: ZBIN, ZBIN32, or ZHEX type of frame received */
  int zmodem_requested;
  bool m_abort = false;
};

#endif // LOWLEVELSTUFF_H
