/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qv4dateobject_p.h"
#include "qv4objectproto_p.h"
#include "qv4scopedvalue_p.h"
#include "qv4runtime_p.h"
#include "qv4string_p.h"
#include "qv4jscall_p.h"
#include "qv4symbol_p.h"

#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>

#include <time.h>

#include <private/qqmljsengine_p.h>

#include <wtf/MathExtras.h>

#if defined(Q_OS_LINUX) && QT_CONFIG(timezone)
/*
  See QTBUG-56899.  Although we don't (yet) have a proper way to reset the
  system zone, the code below, that uses QTimeZone::systemTimeZone(), works
  adequately on Linux, when the TZ environment variable is changed.
 */
#define USE_QTZ_SYSTEM_ZONE
#endif

#ifdef USE_QTZ_SYSTEM_ZONE
#include <QtCore/QTimeZone>
#else
#  ifdef Q_OS_WIN
#    include <windows.h>
#  else
#    ifndef Q_OS_VXWORKS
#      include <sys/time.h>
#    else
#      include "qplatformdefs.h"
#    endif
#    include <unistd.h> // for _POSIX_THREAD_SAFE_FUNCTIONS
#  endif
#endif // USE_QTZ_SYSTEM_ZONE

using namespace QV4;

static const double HoursPerDay = 24.0;
static const double MinutesPerHour = 60.0;
static const double SecondsPerMinute = 60.0;
static const double msPerSecond = 1000.0;
static const double msPerMinute = 60000.0;
static const double msPerHour = 3600000.0;
static const double msPerDay = 86400000.0;

static inline double TimeWithinDay(double t)
{
    double r = ::fmod(t, msPerDay);
    return (r >= 0) ? r : r + msPerDay;
}

static inline int HourFromTime(double t)
{
    int r = int(::fmod(::floor(t / msPerHour), HoursPerDay));
    return (r >= 0) ? r : r + int(HoursPerDay);
}

static inline int MinFromTime(double t)
{
    int r = int(::fmod(::floor(t / msPerMinute), MinutesPerHour));
    return (r >= 0) ? r : r + int(MinutesPerHour);
}

static inline int SecFromTime(double t)
{
    int r = int(::fmod(::floor(t / msPerSecond), SecondsPerMinute));
    return (r >= 0) ? r : r + int(SecondsPerMinute);
}

static inline int msFromTime(double t)
{
    int r = int(::fmod(t, msPerSecond));
    return (r >= 0) ? r : r + int(msPerSecond);
}

static inline double Day(double t)
{
    return ::floor(t / msPerDay);
}

static inline double DaysInYear(double y)
{
    if (::fmod(y, 4))
        return 365;

    else if (::fmod(y, 100))
        return 366;

    else if (::fmod(y, 400))
        return 365;

    return 366;
}

static inline double DayFromYear(double y)
{
    return 365 * (y - 1970)
        + ::floor((y - 1969) / 4)
        - ::floor((y - 1901) / 100)
        + ::floor((y - 1601) / 400);
}

static inline double TimeFromYear(double y)
{
    return msPerDay * DayFromYear(y);
}

static inline double YearFromTime(double t)
{
    int y = 1970;
    y += (int) ::floor(t / (msPerDay * 365.2425));

    double t2 = TimeFromYear(y);
    return (t2 > t) ? y - 1 : ((t2 + msPerDay * DaysInYear(y)) <= t) ? y + 1 : y;
}

static inline bool InLeapYear(double t)
{
    double x = DaysInYear(YearFromTime(t));
    if (x == 365)
        return 0;

    Q_ASSERT(x == 366);
    return 1;
}

static inline double DayWithinYear(double t)
{
    return Day(t) - DayFromYear(YearFromTime(t));
}

static inline double MonthFromTime(double t)
{
    double d = DayWithinYear(t);
    double l = InLeapYear(t);

    if (d < 31.0)
        return 0;

    else if (d < 59.0 + l)
        return 1;

    else if (d < 90.0 + l)
        return 2;

    else if (d < 120.0 + l)
        return 3;

    else if (d < 151.0 + l)
        return 4;

    else if (d < 181.0 + l)
        return 5;

    else if (d < 212.0 + l)
        return 6;

    else if (d < 243.0 + l)
        return 7;

    else if (d < 273.0 + l)
        return 8;

    else if (d < 304.0 + l)
        return 9;

    else if (d < 334.0 + l)
        return 10;

    else if (d < 365.0 + l)
        return 11;

    return qt_qnan(); // ### assert?
}

static inline double DateFromTime(double t)
{
    int m = (int) QV4::Value::toInteger(MonthFromTime(t));
    double d = DayWithinYear(t);
    double l = InLeapYear(t);

    switch (m) {
    case 0: return d + 1.0;
    case 1: return d - 30.0;
    case 2: return d - 58.0 - l;
    case 3: return d - 89.0 - l;
    case 4: return d - 119.0 - l;
    case 5: return d - 150.0 - l;
    case 6: return d - 180.0 - l;
    case 7: return d - 211.0 - l;
    case 8: return d - 242.0 - l;
    case 9: return d - 272.0 - l;
    case 10: return d - 303.0 - l;
    case 11: return d - 333.0 - l;
    }

    return qt_qnan(); // ### assert
}

static inline double WeekDay(double t)
{
    double r = ::fmod (Day(t) + 4.0, 7.0);
    return (r >= 0) ? r : r + 7.0;
}


static inline double MakeTime(double hour, double min, double sec, double ms)
{
    if (!qIsFinite(hour) || !qIsFinite(min) || !qIsFinite(sec) || !qIsFinite(ms))
        return qQNaN();
    hour = QV4::Value::toInteger(hour);
    min = QV4::Value::toInteger(min);
    sec = QV4::Value::toInteger(sec);
    ms = QV4::Value::toInteger(ms);
    return ((hour * MinutesPerHour + min) * SecondsPerMinute + sec) * msPerSecond + ms;
}

static inline double DayFromMonth(double month, double leap)
{
    switch ((int) month) {
    case 0: return 0;
    case 1: return 31.0;
    case 2: return 59.0 + leap;
    case 3: return 90.0 + leap;
    case 4: return 120.0 + leap;
    case 5: return 151.0 + leap;
    case 6: return 181.0 + leap;
    case 7: return 212.0 + leap;
    case 8: return 243.0 + leap;
    case 9: return 273.0 + leap;
    case 10: return 304.0 + leap;
    case 11: return 334.0 + leap;
    }

    return qt_qnan(); // ### assert?
}

static double MakeDay(double year, double month, double day)
{
    if (!qIsFinite(year) || !qIsFinite(month) || !qIsFinite(day))
        return qQNaN();
    year = QV4::Value::toInteger(year);
    month = QV4::Value::toInteger(month);
    day = QV4::Value::toInteger(day);

    year += ::floor(month / 12.0);

    month = ::fmod(month, 12.0);
    if (month < 0)
        month += 12.0;

    double d = DayFromYear(year);
    bool leap = InLeapYear(d*msPerDay);

    d += DayFromMonth(month, leap);
    d += day - 1;

    return d;
}

static inline double MakeDate(double day, double time)
{
    return day * msPerDay + time;
}

#ifdef USE_QTZ_SYSTEM_ZONE
/*
  ECMAScript specifies use of a fixed (current, standard) time-zone offset,
  LocalTZA; and LocalTZA + DaylightSavingTA(t) is taken to be (see LocalTime and
  UTC, following) local time's offset from UTC at time t.  For simple zones,
  DaylightSavingTA(t) is thus the DST offset applicable at date/time t; however,
  if a zone has changed its standard offset, the only way to make LocalTime and
  UTC (if implemented in accord with the spec) perform correct transformations
  is to have DaylightSavingTA(t) correct for the zone's standard offset change
  as well as its actual DST offset.

  This means we have to treat any historical changes in the zone's standard
  offset as DST perturbations, regardless of historical reality.  (This shall
  mean a whole day of DST offset for some zones, that have crossed the
  international date line.  This shall confuse client code.)  The bug report
  against the ECMAScript spec is https://github.com/tc39/ecma262/issues/725
*/

static inline double DaylightSavingTA(double t, double localTZA) // t is a UTC time
{
    return QTimeZone::systemTimeZone().offsetFromUtc(
        QDateTime::fromMSecsSinceEpoch(qint64(t), Qt::UTC)) * 1e3 - localTZA;
}
#else
// This implementation fails to take account of past changes in standard offset.
static inline double DaylightSavingTA(double t, double /*localTZA*/)
{
    struct tm tmtm;
#if defined(Q_CC_MSVC)
    __time64_t  tt = (__time64_t)(t / msPerSecond);
    // _localtime_64_s returns non-zero on failure
    if (_localtime64_s(&tmtm, &tt) != 0)
#elif !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    long int tt = (long int)(t / msPerSecond);
    if (!localtime_r((const time_t*) &tt, &tmtm))
#else
    // Returns shared static data which may be overwritten at any time
    // (for MinGW/Windows localtime is luckily thread safe)
    long int tt = (long int)(t / msPerSecond);
    if (struct tm *tmtm_p = localtime((const time_t*) &tt))
        tmtm = *tmtm_p;
    else
#endif
        return 0;
    return (tmtm.tm_isdst > 0) ? msPerHour : 0;
}
#endif // USE_QTZ_SYSTEM_ZONE

static inline double LocalTime(double t, double localTZA)
{
    // Flawed, yet verbatim from the spec:
    return t + localTZA + DaylightSavingTA(t, localTZA);
}

// The spec does note [*] that UTC and LocalTime are not quite mutually inverse.
// [*] http://www.ecma-international.org/ecma-262/7.0/index.html#sec-utc-t

static inline double UTC(double t, double localTZA)
{
    // Flawed, yet verbatim from the spec:
    return t - localTZA - DaylightSavingTA(t - localTZA, localTZA);
}

static inline double currentTime()
{
    return QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
}

static inline double TimeClip(double t)
{
    if (! qt_is_finite(t) || fabs(t) > 8.64e15)
        return qt_qnan();

    // +0 looks weird, but is correct. See ES6 20.3.1.15. We must not return -0.
    return QV4::Value::toInteger(t) + 0;
}

static inline double ParseString(const QString &s, double localTZA)
{
    /*
      First, try the format defined in ECMA 262's "Date Time String Format";
      only if that fails, fall back to QDateTime for parsing

      The defined string format is YYYY-MM-DDTHH:mm:ss.sssZ; the time (T and all
      after it) may be omitted; in each part, the second and later components
      are optional; and there's an extended syntax for negative and large
      positive years: +/-YYYYYY; the leading sign, even when +, isn't optional.
      If month or day is omitted, it is 01; if minute or second is omitted, it's
      00; if milliseconds are omitted, they're 000.

      When the time zone offset is absent, date-only forms are interpreted as
      indicating a UTC time and date-time forms are interpreted in local time.
    */

    enum Format {
        Year,
        Month,
        Day,
        Hour,
        Minute,
        Second,
        MilliSecond,
        TimezoneHour,
        TimezoneMinute,
        Done
    };

    const QChar *ch = s.constData();
    const QChar *end = ch + s.length();

    uint format = Year;
    int current = 0;
    int currentSize = 0;
    bool extendedYear = false;

    int yearSign = 1;
    int year = 0;
    int month = 0;
    int day = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int msec = 0;
    int offsetSign = 1;
    int offset = 0;
    bool seenT = false;
    bool seenZ = false; // Have seen zone, i.e. +HH:mm or literal Z.

    bool error = false;
    if (*ch == '+' || *ch == '-') {
        extendedYear = true;
        if (*ch == '-')
            yearSign = -1;
        ++ch;
    }
    for (; ch <= end && !error && format != Done; ++ch) {
        if (*ch >= '0' && *ch <= '9') {
            current *= 10;
            current += ch->unicode() - '0';
            ++currentSize;
        } else { // other char, delimits field
            switch (format) {
            case Year:
                year = current;
                if (extendedYear)
                    error = (currentSize != 6);
                else
                    error = (currentSize != 4);
                break;
            case Month:
                month = current - 1;
                error = (currentSize != 2) || month > 11;
                break;
            case Day:
                day = current;
                error = (currentSize != 2) || day > 31;
                break;
            case Hour:
                hour = current;
                error = (currentSize != 2) || hour > 24;
                break;
            case Minute:
                minute = current;
                error = (currentSize != 2) || minute >= 60;
                break;
            case Second:
                second = current;
                error = (currentSize != 2) || second > 60;
                break;
            case MilliSecond:
                msec = current;
                error = (currentSize != 3);
                break;
            case TimezoneHour:
                Q_ASSERT(offset == 0 && !seenZ);
                offset = current * 60;
                error = (currentSize != 2) || current > 23;
                seenZ = true;
                break;
            case TimezoneMinute:
                offset += current;
                error = (currentSize != 2) || current >= 60;
                break;
            }
            if (*ch == 'T') {
                if (format >= Hour)
                    error = true;
                format = Hour;
                seenT = true;
            } else if (*ch == '-') {
                if (format < Day)
                    ++format;
                else if (format < Minute)
                    error = true;
                else if (format >= TimezoneHour)
                    error = true;
                else {
                    Q_ASSERT(offset == 0 && !seenZ);
                    offsetSign = -1;
                    format = TimezoneHour;
                }
            } else if (*ch == ':') {
                if (format != Hour && format != Minute && format != TimezoneHour)
                    error = true;
                ++format;
            } else if (*ch == '.') {
                if (format != Second)
                    error = true;
                ++format;
            } else if (*ch == '+') {
                if (seenZ || format < Minute || format >= TimezoneHour)
                    error = true;
                format = TimezoneHour;
            } else if (*ch == 'Z') {
                if (seenZ || format < Minute || format >= TimezoneHour)
                    error = true;
                else
                    Q_ASSERT(offset == 0);
                format = Done;
                seenZ = true;
            } else if (ch->unicode() == 0) {
                format = Done;
            }
            current = 0;
            currentSize = 0;
        }
    }

    if (!error) {
        double t = MakeDate(MakeDay(year * yearSign, month, day), MakeTime(hour, minute, second, msec));
        if (seenZ)
            t -= offset * offsetSign * 60 * 1000;
        else if (seenT) // No zone specified, treat date-time as local time
            t = UTC(t, localTZA);
        // else: treat plain date as already in UTC
        return TimeClip(t);
    }

    QDateTime dt = QDateTime::fromString(s, Qt::TextDate);
    if (!dt.isValid())
        dt = QDateTime::fromString(s, Qt::ISODate);
    if (!dt.isValid())
        dt = QDateTime::fromString(s, Qt::RFC2822Date);
    if (!dt.isValid()) {
        const QString formats[] = {
            QStringLiteral("M/d/yyyy"),
            QStringLiteral("M/d/yyyy hh:mm"),
            QStringLiteral("M/d/yyyy hh:mm A"),

            QStringLiteral("M/d/yyyy, hh:mm"),
            QStringLiteral("M/d/yyyy, hh:mm A"),

            QStringLiteral("MMM d yyyy"),
            QStringLiteral("MMM d yyyy hh:mm"),
            QStringLiteral("MMM d yyyy hh:mm:ss"),
            QStringLiteral("MMM d yyyy, hh:mm"),
            QStringLiteral("MMM d yyyy, hh:mm:ss"),

            QStringLiteral("MMMM d yyyy"),
            QStringLiteral("MMMM d yyyy hh:mm"),
            QStringLiteral("MMMM d yyyy hh:mm:ss"),
            QStringLiteral("MMMM d yyyy, hh:mm"),
            QStringLiteral("MMMM d yyyy, hh:mm:ss"),

            QStringLiteral("MMM d, yyyy"),
            QStringLiteral("MMM d, yyyy hh:mm"),
            QStringLiteral("MMM d, yyyy hh:mm:ss"),

            QStringLiteral("MMMM d, yyyy"),
            QStringLiteral("MMMM d, yyyy hh:mm"),
            QStringLiteral("MMMM d, yyyy hh:mm:ss"),

            QStringLiteral("d MMM yyyy"),
            QStringLiteral("d MMM yyyy hh:mm"),
            QStringLiteral("d MMM yyyy hh:mm:ss"),
            QStringLiteral("d MMM yyyy, hh:mm"),
            QStringLiteral("d MMM yyyy, hh:mm:ss"),

            QStringLiteral("d MMMM yyyy"),
            QStringLiteral("d MMMM yyyy hh:mm"),
            QStringLiteral("d MMMM yyyy hh:mm:ss"),
            QStringLiteral("d MMMM yyyy, hh:mm"),
            QStringLiteral("d MMMM yyyy, hh:mm:ss"),

            QStringLiteral("d MMM, yyyy"),
            QStringLiteral("d MMM, yyyy hh:mm"),
            QStringLiteral("d MMM, yyyy hh:mm:ss"),

            QStringLiteral("d MMMM, yyyy"),
            QStringLiteral("d MMMM, yyyy hh:mm"),
            QStringLiteral("d MMMM, yyyy hh:mm:ss"),
        };

        for (uint i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
            const QString &format(formats[i]);
            dt = format.indexOf(QLatin1String("hh:mm")) < 0
                ? QDateTime(QDate::fromString(s, format),
                            QTime(0, 0, 0), Qt::UTC)
                : QDateTime::fromString(s, format); // as local time
            if (dt.isValid())
                break;
        }
    }
    if (!dt.isValid())
        return qt_qnan();
    return TimeClip(dt.toMSecsSinceEpoch());
}

/*!
  \internal

  Converts the ECMA Date value \tt (in UTC form) to QDateTime
  according to \a spec.
*/
static inline QDateTime ToDateTime(double t, Qt::TimeSpec spec)
{
    if (std::isnan(t))
        return QDateTime();
    return QDateTime::fromMSecsSinceEpoch(t, Qt::UTC).toTimeSpec(spec);
}

static inline QString ToString(double t, double localTZA)
{
    if (std::isnan(t))
        return QStringLiteral("Invalid Date");
    QString str = ToDateTime(t, Qt::LocalTime).toString() + QLatin1String(" GMT");
    double tzoffset = localTZA + DaylightSavingTA(t, localTZA);
    if (tzoffset) {
        int hours = static_cast<int>(::fabs(tzoffset) / 1000 / 60 / 60);
        int mins = int(::fabs(tzoffset) / 1000 / 60) % 60;
        str.append(QLatin1Char((tzoffset > 0) ?  '+' : '-'));
        if (hours < 10)
            str.append(QLatin1Char('0'));
        str.append(QString::number(hours));
        if (mins < 10)
            str.append(QLatin1Char('0'));
        str.append(QString::number(mins));
    }
    return str;
}

static inline QString ToUTCString(double t)
{
    if (std::isnan(t))
        return QStringLiteral("Invalid Date");
    return ToDateTime(t, Qt::UTC).toString();
}

static inline QString ToDateString(double t)
{
    return ToDateTime(t, Qt::LocalTime).date().toString();
}

static inline QString ToTimeString(double t)
{
    return ToDateTime(t, Qt::LocalTime).time().toString();
}

static inline QString ToLocaleString(double t)
{
    return ToDateTime(t, Qt::LocalTime).toString(Qt::LocaleDate);
}

static inline QString ToLocaleDateString(double t)
{
    return ToDateTime(t, Qt::LocalTime).date().toString(Qt::LocaleDate);
}

static inline QString ToLocaleTimeString(double t)
{
    return ToDateTime(t, Qt::LocalTime).time().toString(Qt::LocaleDate);
}

static double getLocalTZA()
{
#ifndef Q_OS_WIN
    tzset();
#endif
#ifdef USE_QTZ_SYSTEM_ZONE
    // TODO: QTimeZone::resetSystemTimeZone(), see QTBUG-56899 and comment above.
    // Standard offset, with no daylight-savings adjustment, in ms:
    return QTimeZone::systemTimeZone().standardTimeOffset(QDateTime::currentDateTime()) * 1e3;
#else
#  ifdef Q_OS_WIN
    TIME_ZONE_INFORMATION tzInfo;
    GetTimeZoneInformation(&tzInfo);
    return -tzInfo.Bias * 60.0 * 1000.0;
#  else
    struct tm t;
    time_t curr;
    time(&curr);
    localtime_r(&curr, &t); // Wrong: includes DST offset
    time_t locl = mktime(&t);
    gmtime_r(&curr, &t);
    time_t globl = mktime(&t);
    return (double(locl) - double(globl)) * 1000.0;
#  endif
#endif // USE_QTZ_SYSTEM_ZONE
}

DEFINE_OBJECT_VTABLE(DateObject);

void Heap::DateObject::init(const QDateTime &date)
{
    Object::init();
    this->date = date.isValid() ? TimeClip(date.toMSecsSinceEpoch()) : qt_qnan();
}

void Heap::DateObject::init(const QTime &time)
{
    Object::init();
    if (!time.isValid()) {
        date = qt_qnan();
        return;
    }

    /* We have to chose a date on which to instantiate this time.  All we really
     * care about is that it round-trips back to the same time if we extract the
     * time from it, which shall (via toQDateTime(), below) discard the date
     * part.  We need a date for which time-zone data is likely to be sane (so
     * MakeDay(0, 0, 0) was a bad choice; 2 BC, December 31st is before
     * time-zones were standardized), with no transition nearby in date.  We
     * ignore DST transitions before 1970, but even then zone transitions did
     * happen.  Some do happen at new year, others on DST transitions in spring
     * and autumn; so pick the three hundredth anniversary of the birth of
     * Giovanni Domenico Cassini (1625-06-08), whose work first let us
     * synchronize clocks tolerably accurately at distant locations.
     */
    static const double d = MakeDay(1925, 5, 8);
    double t = MakeTime(time.hour(), time.minute(), time.second(), time.msec());
    date = TimeClip(UTC(MakeDate(d, t), internalClass->engine->localTZA));
}

QDateTime DateObject::toQDateTime() const
{
    return ToDateTime(date(), Qt::LocalTime);
}

DEFINE_OBJECT_VTABLE(DateCtor);

void Heap::DateCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Date"));
}

ReturnedValue DateCtor::virtualCallAsConstructor(const FunctionObject *that, const Value *argv, int argc, const Value *newTarget)
{
    ExecutionEngine *v4 = that->engine();
    double t = 0;

    if (argc == 0)
        t = currentTime();

    else if (argc == 1) {
        Scope scope(v4);
        ScopedValue arg(scope, argv[0]);
        if (DateObject *d = arg->as<DateObject>()) {
            t = d->date();
        } else {
            arg = RuntimeHelpers::toPrimitive(arg, PREFERREDTYPE_HINT);

            if (String *s = arg->stringValue())
                t = ParseString(s->toQString(), v4->localTZA);
            else
                t = TimeClip(arg->toNumber());
        }
    }

    else { // d.argc > 1
        double year  = argv[0].toNumber();
        double month = argv[1].toNumber();
        double day  = argc >= 3 ? argv[2].toNumber() : 1;
        double hours = argc >= 4 ? argv[3].toNumber() : 0;
        double mins = argc >= 5 ? argv[4].toNumber() : 0;
        double secs = argc >= 6 ? argv[5].toNumber() : 0;
        double ms    = argc >= 7 ? argv[6].toNumber() : 0;
        if (year >= 0 && year <= 99)
            year += 1900;
        t = MakeDate(MakeDay(year, month, day), MakeTime(hours, mins, secs, ms));
        t = TimeClip(UTC(t, v4->localTZA));
    }

    ReturnedValue o = Encode(v4->newDateObject(Value::fromDouble(t)));
    if (!newTarget)
        return o;
    Scope scope(v4);
    ScopedObject obj(scope, o);
    obj->setProtoFromNewTarget(newTarget);
    return obj->asReturnedValue();
}

ReturnedValue DateCtor::virtualCall(const FunctionObject *m, const Value *, const Value *, int)
{
    ExecutionEngine *e = m->engine();
    double t = currentTime();
    return e->newString(ToString(t, e->localTZA))->asReturnedValue();
}

void DatePrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(7));
    engine->localTZA = getLocalTZA();

    ctor->defineDefaultProperty(QStringLiteral("parse"), method_parse, 1);
    ctor->defineDefaultProperty(QStringLiteral("UTC"), method_UTC, 7);
    ctor->defineDefaultProperty(QStringLiteral("now"), method_now, 0);

    defineDefaultProperty(QStringLiteral("constructor"), (o = ctor));
    defineDefaultProperty(engine->id_toString(), method_toString, 0);
    defineDefaultProperty(QStringLiteral("toDateString"), method_toDateString, 0);
    defineDefaultProperty(QStringLiteral("toTimeString"), method_toTimeString, 0);
    defineDefaultProperty(engine->id_toLocaleString(), method_toLocaleString, 0);
    defineDefaultProperty(QStringLiteral("toLocaleDateString"), method_toLocaleDateString, 0);
    defineDefaultProperty(QStringLiteral("toLocaleTimeString"), method_toLocaleTimeString, 0);
    defineDefaultProperty(engine->id_valueOf(), method_valueOf, 0);
    defineDefaultProperty(QStringLiteral("getTime"), method_getTime, 0);
    defineDefaultProperty(QStringLiteral("getYear"), method_getYear, 0);
    defineDefaultProperty(QStringLiteral("getFullYear"), method_getFullYear, 0);
    defineDefaultProperty(QStringLiteral("getUTCFullYear"), method_getUTCFullYear, 0);
    defineDefaultProperty(QStringLiteral("getMonth"), method_getMonth, 0);
    defineDefaultProperty(QStringLiteral("getUTCMonth"), method_getUTCMonth, 0);
    defineDefaultProperty(QStringLiteral("getDate"), method_getDate, 0);
    defineDefaultProperty(QStringLiteral("getUTCDate"), method_getUTCDate, 0);
    defineDefaultProperty(QStringLiteral("getDay"), method_getDay, 0);
    defineDefaultProperty(QStringLiteral("getUTCDay"), method_getUTCDay, 0);
    defineDefaultProperty(QStringLiteral("getHours"), method_getHours, 0);
    defineDefaultProperty(QStringLiteral("getUTCHours"), method_getUTCHours, 0);
    defineDefaultProperty(QStringLiteral("getMinutes"), method_getMinutes, 0);
    defineDefaultProperty(QStringLiteral("getUTCMinutes"), method_getUTCMinutes, 0);
    defineDefaultProperty(QStringLiteral("getSeconds"), method_getSeconds, 0);
    defineDefaultProperty(QStringLiteral("getUTCSeconds"), method_getUTCSeconds, 0);
    defineDefaultProperty(QStringLiteral("getMilliseconds"), method_getMilliseconds, 0);
    defineDefaultProperty(QStringLiteral("getUTCMilliseconds"), method_getUTCMilliseconds, 0);
    defineDefaultProperty(QStringLiteral("getTimezoneOffset"), method_getTimezoneOffset, 0);
    defineDefaultProperty(QStringLiteral("setTime"), method_setTime, 1);
    defineDefaultProperty(QStringLiteral("setMilliseconds"), method_setMilliseconds, 1);
    defineDefaultProperty(QStringLiteral("setUTCMilliseconds"), method_setUTCMilliseconds, 1);
    defineDefaultProperty(QStringLiteral("setSeconds"), method_setSeconds, 2);
    defineDefaultProperty(QStringLiteral("setUTCSeconds"), method_setUTCSeconds, 2);
    defineDefaultProperty(QStringLiteral("setMinutes"), method_setMinutes, 3);
    defineDefaultProperty(QStringLiteral("setUTCMinutes"), method_setUTCMinutes, 3);
    defineDefaultProperty(QStringLiteral("setHours"), method_setHours, 4);
    defineDefaultProperty(QStringLiteral("setUTCHours"), method_setUTCHours, 4);
    defineDefaultProperty(QStringLiteral("setDate"), method_setDate, 1);
    defineDefaultProperty(QStringLiteral("setUTCDate"), method_setUTCDate, 1);
    defineDefaultProperty(QStringLiteral("setMonth"), method_setMonth, 2);
    defineDefaultProperty(QStringLiteral("setUTCMonth"), method_setUTCMonth, 2);
    defineDefaultProperty(QStringLiteral("setYear"), method_setYear, 1);
    defineDefaultProperty(QStringLiteral("setFullYear"), method_setFullYear, 3);
    defineDefaultProperty(QStringLiteral("setUTCFullYear"), method_setUTCFullYear, 3);

    // ES6: B.2.4.3 & 20.3.4.43:
    // We have to use the *same object* for toUTCString and toGMTString
    {
        QString toUtcString(QStringLiteral("toUTCString"));
        QString toGmtString(QStringLiteral("toGMTString"));
        ScopedString us(scope, engine->newIdentifier(toUtcString));
        ScopedString gs(scope, engine->newIdentifier(toGmtString));
        ScopedFunctionObject toUtcGmtStringFn(scope, FunctionObject::createBuiltinFunction(engine, us, method_toUTCString, 0));
        defineDefaultProperty(us, toUtcGmtStringFn);
        defineDefaultProperty(gs, toUtcGmtStringFn);
    }

    defineDefaultProperty(QStringLiteral("toISOString"), method_toISOString, 0);
    defineDefaultProperty(QStringLiteral("toJSON"), method_toJSON, 1);
    defineDefaultProperty(engine->symbol_toPrimitive(), method_symbolToPrimitive, 1, Attr_ReadOnly_ButConfigurable);
}

double DatePrototype::getThisDate(ExecutionEngine *v4, const Value *thisObject)
{
    if (const DateObject *that = thisObject->as<DateObject>())
        return that->date();
    v4->throwTypeError();
    return 0;
}

ReturnedValue DatePrototype::method_parse(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    if (!argc)
        return Encode(qt_qnan());
    else
        return Encode(ParseString(argv[0].toQString(), f->engine()->localTZA));
}

ReturnedValue DatePrototype::method_UTC(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    const int numArgs = argc;
    if (numArgs < 1)
        return Encode(qQNaN());
    ExecutionEngine *e = f->engine();
    double year  = argv[0].toNumber();
    if (e->hasException)
        return Encode::undefined();
    double month = numArgs >= 2 ? argv[1].toNumber() : 0;
    if (e->hasException)
        return Encode::undefined();
    double day   = numArgs >= 3 ? argv[2].toNumber() : 1;
    if (e->hasException)
        return Encode::undefined();
    double hours = numArgs >= 4 ? argv[3].toNumber() : 0;
    if (e->hasException)
        return Encode::undefined();
    double mins  = numArgs >= 5 ? argv[4].toNumber() : 0;
    if (e->hasException)
        return Encode::undefined();
    double secs  = numArgs >= 6 ? argv[5].toNumber() : 0;
    if (e->hasException)
        return Encode::undefined();
    double ms    = numArgs >= 7 ? argv[6].toNumber() : 0;
    if (e->hasException)
        return Encode::undefined();
    double iyear = QV4::Value::toInteger(year);
    if (!qIsNaN(year) && iyear >= 0 && iyear <= 99)
        year = 1900 + iyear;
    double t = MakeDate(MakeDay(year, month, day),
                        MakeTime(hours, mins, secs, ms));
    return Encode(TimeClip(t));
}

ReturnedValue DatePrototype::method_now(const FunctionObject *, const Value *, const Value *, int)
{
    return Encode(currentTime());
}

ReturnedValue DatePrototype::method_toString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(v4->newString(ToString(t, v4->localTZA)));
}

ReturnedValue DatePrototype::method_toDateString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(v4->newString(ToDateString(t)));
}

ReturnedValue DatePrototype::method_toTimeString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(v4->newString(ToTimeString(t)));
}

ReturnedValue DatePrototype::method_toLocaleString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(v4->newString(ToLocaleString(t)));
}

ReturnedValue DatePrototype::method_toLocaleDateString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(v4->newString(ToLocaleDateString(t)));
}

ReturnedValue DatePrototype::method_toLocaleTimeString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(v4->newString(ToLocaleTimeString(t)));
}

ReturnedValue DatePrototype::method_valueOf(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getTime(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getYear(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = YearFromTime(LocalTime(t, v4->localTZA)) - 1900;
    return Encode(t);
}

ReturnedValue DatePrototype::method_getFullYear(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = YearFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCFullYear(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = YearFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getMonth(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = MonthFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCMonth(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = MonthFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getDate(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = DateFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCDate(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = DateFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getDay(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = WeekDay(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCDay(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = WeekDay(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getHours(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = HourFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCHours(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = HourFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getMinutes(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = MinFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCMinutes(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = MinFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getSeconds(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = SecFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCSeconds(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = SecFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getMilliseconds(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = msFromTime(LocalTime(t, v4->localTZA));
    return Encode(t);
}

ReturnedValue DatePrototype::method_getUTCMilliseconds(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = msFromTime(t);
    return Encode(t);
}

ReturnedValue DatePrototype::method_getTimezoneOffset(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    double t = getThisDate(v4, thisObject);
    if (!std::isnan(t))
        t = (t - LocalTime(t, v4->localTZA)) / msPerMinute;
    return Encode(t);
}

ReturnedValue DatePrototype::method_setTime(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    self->setDate(TimeClip(t));
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setMilliseconds(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    double ms = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    self->setDate(TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), SecFromTime(t), ms)), v4->localTZA)));
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCMilliseconds(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double ms = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    self->setDate(TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), SecFromTime(t), ms))));
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setSeconds(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    double sec = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double ms = (argc < 2) ? msFromTime(t) : argv[1].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), sec, ms)), v4->localTZA));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCSeconds(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    double sec = argc ? argv[0].toNumber() : qt_qnan();
    double ms = (argc < 2) ? msFromTime(t) : argv[1].toNumber();
    t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), sec, ms)));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setMinutes(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    double min = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double sec = (argc < 2) ? SecFromTime(t) : argv[1].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double ms = (argc < 3) ? msFromTime(t) : argv[2].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), min, sec, ms)), v4->localTZA));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCMinutes(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    double min = argc ? argv[0].toNumber() : qt_qnan();
    double sec = (argc < 2) ? SecFromTime(t) : argv[1].toNumber();
    double ms = (argc < 3) ? msFromTime(t) : argv[2].toNumber();
    t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), min, sec, ms)));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setHours(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    double hour = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double min = (argc < 2) ? MinFromTime(t) : argv[1].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double sec = (argc < 3) ? SecFromTime(t) : argv[2].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double ms = (argc < 4) ? msFromTime(t) : argv[3].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(UTC(MakeDate(Day(t), MakeTime(hour, min, sec, ms)), v4->localTZA));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCHours(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    double hour = argc ? argv[0].toNumber() : qt_qnan();
    double min = (argc < 2) ? MinFromTime(t) : argv[1].toNumber();
    double sec = (argc < 3) ? SecFromTime(t) : argv[2].toNumber();
    double ms = (argc < 4) ? msFromTime(t) : argv[3].toNumber();
    t = TimeClip(MakeDate(Day(t), MakeTime(hour, min, sec, ms)));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setDate(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    double date = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(UTC(MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), date), TimeWithinDay(t)), v4->localTZA));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCDate(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double date = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), date), TimeWithinDay(t)));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setMonth(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    double month = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double date = (argc < 2) ? DateFromTime(t) : argv[1].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(UTC(MakeDate(MakeDay(YearFromTime(t), month, date), TimeWithinDay(t)), v4->localTZA));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCMonth(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    double month = argc ? argv[0].toNumber() : qt_qnan();
    double date = (argc < 2) ? DateFromTime(t) : argv[1].toNumber();
    t = TimeClip(MakeDate(MakeDay(YearFromTime(t), month, date), TimeWithinDay(t)));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setYear(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    if (std::isnan(t))
        t = 0;
    else
        t = LocalTime(t, v4->localTZA);
    double year = argc ? argv[0].toNumber() : qt_qnan();
    double r;
    if (std::isnan(year)) {
        r = qt_qnan();
    } else {
        if ((QV4::Value::toInteger(year) >= 0) && (QV4::Value::toInteger(year) <= 99))
            year += 1900;
        r = MakeDay(year, MonthFromTime(t), DateFromTime(t));
        r = UTC(MakeDate(r, TimeWithinDay(t)), v4->localTZA);
        r = TimeClip(r);
    }
    self->setDate(r);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setUTCFullYear(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    double year = argc ? argv[0].toNumber() : qt_qnan();
    double month = (argc < 2) ? MonthFromTime(t) : argv[1].toNumber();
    double date = (argc < 3) ? DateFromTime(t) : argv[2].toNumber();
    t = TimeClip(MakeDate(MakeDay(year, month, date), TimeWithinDay(t)));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_setFullYear(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = LocalTime(self->date(), v4->localTZA);
    if (v4->hasException)
        return QV4::Encode::undefined();
    if (std::isnan(t))
        t = 0;
    double year = argc ? argv[0].toNumber() : qt_qnan();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double month = (argc < 2) ? MonthFromTime(t) : argv[1].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    double date = (argc < 3) ? DateFromTime(t) : argv[2].toNumber();
    if (v4->hasException)
        return QV4::Encode::undefined();
    t = TimeClip(UTC(MakeDate(MakeDay(year, month, date), TimeWithinDay(t)), v4->localTZA));
    self->setDate(t);
    return Encode(self->date());
}

ReturnedValue DatePrototype::method_toUTCString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    return Encode(v4->newString(ToUTCString(t)));
}

static void addZeroPrefixedInt(QString &str, int num, int nDigits)
{
    str.resize(str.size() + nDigits);

    QChar *c = str.data() + str.size() - 1;
    while (nDigits) {
        *c = QChar(num % 10 + '0');
        num /= 10;
        --c;
        --nDigits;
    }
}

ReturnedValue DatePrototype::method_toISOString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    DateObject *self = const_cast<DateObject *>(thisObject->as<DateObject>());
    if (!self)
        return v4->throwTypeError();

    double t = self->date();
    if (!std::isfinite(t))
        RETURN_RESULT(v4->throwRangeError(*thisObject));

    QString result;
    int year = (int)YearFromTime(t);
    if (year < 0 || year > 9999) {
        if (qAbs(year) >= 1000000)
            RETURN_RESULT(v4->throwRangeError(*thisObject));
        result += year < 0 ? QLatin1Char('-') : QLatin1Char('+');
        year = qAbs(year);
        addZeroPrefixedInt(result, year, 6);
    } else {
        addZeroPrefixedInt(result, year, 4);
    }
    result += QLatin1Char('-');
    addZeroPrefixedInt(result, (int)MonthFromTime(t) + 1, 2);
    result += QLatin1Char('-');
    addZeroPrefixedInt(result, (int)DateFromTime(t), 2);
    result += QLatin1Char('T');
    addZeroPrefixedInt(result, HourFromTime(t), 2);
    result += QLatin1Char(':');
    addZeroPrefixedInt(result, MinFromTime(t), 2);
    result += QLatin1Char(':');
    addZeroPrefixedInt(result, SecFromTime(t), 2);
    result += QLatin1Char('.');
    addZeroPrefixedInt(result, msFromTime(t), 3);
    result += QLatin1Char('Z');

    return Encode(v4->newString(result));
}

ReturnedValue DatePrototype::method_toJSON(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    ExecutionEngine *v4 = b->engine();
    Scope scope(v4);
    ScopedObject O(scope, thisObject->toObject(v4));
    if (v4->hasException)
        return QV4::Encode::undefined();

    ScopedValue tv(scope, RuntimeHelpers::toPrimitive(O, NUMBER_HINT));

    if (tv->isNumber() && !std::isfinite(tv->toNumber()))
        return Encode::null();

    ScopedString s(scope, v4->newString(QStringLiteral("toISOString")));
    ScopedValue v(scope, O->get(s));
    FunctionObject *toIso = v->as<FunctionObject>();

    if (!toIso)
        return v4->throwTypeError();

    return checkedResult(v4, toIso->call(O, nullptr, 0));
}

ReturnedValue DatePrototype::method_symbolToPrimitive(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *e = f->engine();
    if (!thisObject->isObject() || !argc || !argv->isString())
        return e->throwTypeError();

    String *hint = argv->stringValue();
    PropertyKey id = hint->toPropertyKey();
    if (id == e->id_default()->propertyKey())
        hint = e->id_string();
    else if (id != e->id_string()->propertyKey() && id != e->id_number()->propertyKey())
        return e->throwTypeError();

    return RuntimeHelpers::ordinaryToPrimitive(e, static_cast<const Object *>(thisObject), hint);
}

void DatePrototype::timezoneUpdated(ExecutionEngine *e)
{
    e->localTZA = getLocalTZA();
}
