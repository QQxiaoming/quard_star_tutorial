/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
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

#include "qquickrangeddate_p.h"

QT_BEGIN_NAMESPACE

// JavaScript Date > QDate conversion is not correct for large negative dates.
Q_GLOBAL_STATIC_WITH_ARGS(const QDate, jsMinimumDate, (1, 1, 1))
Q_GLOBAL_STATIC_WITH_ARGS(const QDate, jsMaximumDate, (275759, 10, 25))

QQuickRangedDate1::QQuickRangedDate1() :
    QObject(0),
    mDate(QDateTime::currentDateTime()),
    mMinimumDate(*jsMinimumDate),
    mMaximumDate(*jsMaximumDate)
{
}

/*! \internal
    \qmlproperty date QQuickRangedDate::date
*/
void QQuickRangedDate1::setDate(const QDateTime &date)
{
    if (date == mDate)
        return;

    const QDate tmpDate = date.date();
    if (tmpDate < mMinimumDate) {
        mDate = minimumDate();
    } else if (tmpDate > mMaximumDate) {
        mDate = maximumDate();
    } else {
        mDate = date;
    }

    emit dateChanged();
}

/*! \internal
    \qmlproperty date QQuickRangedDate::minimumDate
*/
void QQuickRangedDate1::setMinimumDate(const QDateTime &minDate)
{
    const QDate tmpDate = minDate.date();
    if (tmpDate == mMinimumDate)
        return;

    mMinimumDate = qMax(tmpDate, *jsMinimumDate);
    emit minimumDateChanged();

    // If the new minimumDate makes date invalid, clamp date to it.
    if (mDate < minimumDate()) {
        mDate = minimumDate();
        emit dateChanged();
    }
}

/*! \internal
    \qmlproperty date QQuickRangedDate::maximumDate
*/
void QQuickRangedDate1::setMaximumDate(const QDateTime &maxDate)
{
    const QDate tmpDate = maxDate.date();
    if (tmpDate == mMaximumDate)
        return;

    // If the new maximumDate is smaller than minimumDate, clamp maximumDate to it.
    // If the new maximumDate is larger than jsMaximumDate, also clamp it.
    mMaximumDate = tmpDate < mMinimumDate ? mMinimumDate : qMin(tmpDate, *jsMaximumDate);
    emit maximumDateChanged();

    // If the new maximumDate makes the date invalid, clamp it.
    if (mDate > maximumDate()) {
        mDate = maximumDate();
        emit dateChanged();
    }
}

QT_END_NAMESPACE
