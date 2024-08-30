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
#ifndef QRECVZMODEM_H
#define QRECVZMODEM_H

#include "lowlevelstuff.h"
#include "zglobal.h"
#include <QThread>
#include <QFile>

class QRecvZmodem : public QThread {
  Q_OBJECT

public:
  explicit QRecvZmodem(int32_t timeout = -1, QObject *parent = nullptr);
  ~QRecvZmodem(){
    requestStop();
    wait();
  };
  void setFileDirPath(const QString &path) {
    m_fileDirPath = path;
  }
  void requestStop(void) {
      m_abort = true;
      zm->requestStop();
  }
  bool getStopFlag(void) {
      return m_abort;
  }

signals:
  void approver(const char *filename, size_t size, time_t date, bool *ret);
  void transferring(QString filename);
  void tick(const char *fname, long bytes_sent, long bytes_total, long last_bps,
            int min_left, int sec_left, bool *ret);
  void complete(QString filename, int result, size_t size, time_t date);
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
  int rz_receive_files(struct zm_fileinfo *);
  int rz_zmodem_session_startup(void);
  void rz_checkpath(const char *name);
  void report(int sct);
  void uncaps(char *s);
  int IsAnyLower(const char *s);
  int rz_write_string_to_file(struct zm_fileinfo *zi, char *buf, size_t n);
  int rz_process_header(char *name, struct zm_fileinfo *);
  int rz_receive_sector(size_t *Blklen, char *rxbuf, unsigned int maxtime);
  int rz_receive_sectors(struct zm_fileinfo *);
  int rz_receive_pathname(struct zm_fileinfo *, char *rpn);
  int rz_receive(void);
  int rz_receive_file(struct zm_fileinfo *);
  int rz_closeit(struct zm_fileinfo *);
  void write_modem_escaped_string_to_stdout(const char *s);
  size_t getfree(void);
  void bibi(int n);

private:
  struct oosb_t {
    size_t pos;
    size_t len;
    char *data;
    struct oosb_t *next;
  };
  oosb_t *anker = NULL;

  QString m_fileDirPath;
  LowLevelStuff *zm; /* zmodem comm primitives' state */
  // Workspaces
  char tcp_buf[256];          /* Buffer to receive TCP protocol
                               * synchronization strings from
                               * fout */
  char attn[ZATTNLEN + 1];    /* Attention string rx sends to tx on err */
  char secbuf[MAX_BLOCK + 1]; /* Workspace to store up to 8k
                               * blocks */
  // Dynamic state
  QFile *fout = nullptr; /* File being received */
  int lastrx; /* Either 0, or CAN if last receipt
               * was sender-cancelled */
  int firstsec;
  int errors;            /* Count of read failures */
  int skip_if_not_found; /* When true, the operation opens and
                          * appends to an existing file. */
  char *pathname;        /* filename of the file being received */
  int thisbinary;        /* When > 0, current file is to be
                          * received in bin mode */
  char zconv;            /* ZMODEM file conversion request. */
  char zmanag;           /* ZMODEM file management request. */
  char ztrans;           /* SET BUT UNUSED: ZMODEM file transport
                          * request byte */
  int tryzhdrtype;       /* Header type to send corresponding
                          * to Last rx close */

  // Constant
  int restricted;     /* restricted; no /.. or ../ in filenames */
                      /*  restricted > 0 prevents unlinking
                         restricted > 0 prevents overwriting dot files
                         restricted = 2 prevents overwriting files
                         restricted > 0 restrict files to curdir or PUBDIR
                       */
  int makelcpathname; /* A flag. When true, make received pathname lowercase. */
  int nflag;          /* A flag. Don't really transfer files */
  int rxclob;         /* A flag. Allow clobbering existing file */
  int try_resume;     /* A flag. When true, try restarting downloads */
  int o_sync;         /* A flag. When true, each write will
                       * be reliably completed before
                       * returning. */
  unsigned long min_bps; /* When non-zero, sets a minimum allow
                          * transmission rate.  Dropping below
                          * that rate will cancel the
                          * transfer. */
  long min_bps_time;     /* Length of time transmission is
                          * allowed to be below 'min_bps' */
  char lzmanag;          /* Local file management
                            request. ZF1_ZMAPND, ZF1_ZMCHNG,
                            ZF1_ZMCRC, ZF1_ZMNEWL, ZF1_ZMNEW,
                            ZF1_ZMPROT, ZF1_ZMCRC, or 0 */
  time_t stop_time;      /* Zero or seconds in the epoch.  When
                          * non-zero, indicates a shutdown
                          * time. */
  int under_rsh;         /* A flag.  Set to true if we're
                          * running under a restricted
                          * environment. When true, files save
                          * as 'rw' not 'rwx' */

  bool m_abort = false;
};

#endif // QRECVZMODEM_H
