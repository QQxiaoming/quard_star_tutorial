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
#include <QCoreApplication>

#if defined(Q_OS_WIN)
#include <windows.h>
#if defined(Q_CC_MSVC)
struct timezone {
  int tz_minuteswest; /* minutes W of Greenwich */
  int tz_dsttime;     /* type of dst correction */
};
#endif
int gettimeofday(struct timeval *tv, struct timezone *tz) {
    time_t rawtime;

    time(&rawtime);
    tv->tv_sec = (long)rawtime;

    // here starts the microsecond resolution:

    LARGE_INTEGER tickPerSecond;
    LARGE_INTEGER tick; // a point in time

    // get the high resolution counter's accuracy
    QueryPerformanceFrequency(&tickPerSecond);

    // what time is it ?
    QueryPerformanceCounter(&tick);

    // and here we get the current microsecond! \o/
    tv->tv_usec = (tick.QuadPart % tickPerSecond.QuadPart);

    Q_UNUSED(tz);
    return 0;
}
#else
#include <sys/time.h>
#endif

#include "zglobal.h"
#include "timing.h"

double timing (int reset, time_t *nowp)
{
  static double elaptime, starttime, stoptime;
  double yet;
  struct timeval tv;
  struct timezone tz;

  tz.tz_dsttime = 0;
  gettimeofday (&tv, &tz);
  yet=tv.tv_sec + tv.tv_usec/1000000.0;

  if (nowp)
    *nowp=(time_t) yet;
  if (reset) {
    starttime = yet;
    return starttime;
  }
  else {
    stoptime = yet;
    elaptime = stoptime - starttime;
    return elaptime;
  }
}
