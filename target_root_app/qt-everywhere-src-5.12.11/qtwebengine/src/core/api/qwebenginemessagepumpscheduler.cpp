/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "qwebenginemessagepumpscheduler_p.h"

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QTimerEvent>

QWebEngineMessagePumpScheduler::QWebEngineMessagePumpScheduler(std::function<void()> callback)
    : m_callback(std::move(callback))
{}

void QWebEngineMessagePumpScheduler::scheduleWork()
{
    QCoreApplication::postEvent(this, new QTimerEvent(0));
}

void QWebEngineMessagePumpScheduler::scheduleDelayedWork(int delay)
{
    if (delay < 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    } else if (!m_timerId || delay < QAbstractEventDispatcher::instance()->remainingTime(m_timerId)) {
        killTimer(m_timerId);
        m_timerId = startTimer(delay);
    }
}

void QWebEngineMessagePumpScheduler::timerEvent(QTimerEvent *ev)
{
    Q_ASSERT(!ev->timerId() || m_timerId == ev->timerId());
    killTimer(m_timerId);
    m_timerId = 0;
    m_callback();
}
