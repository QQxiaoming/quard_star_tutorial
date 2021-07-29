/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "osxbtgcdtimer_p.h"
#include "osxbtutility_p.h"

#include <QtCore/qdebug.h>

#include <algorithm>

QT_USE_NAMESPACE
using namespace OSXBluetooth;

@implementation QT_MANGLE_NAMESPACE(OSXBTGCDTimer) {
@private
    qint64 timeoutMS;
    qint64 timeoutStepMS;

    // Optional:
    id objectUnderWatch;
    OperationTimeout timeoutType;

    QElapsedTimer timer;
    id<QT_MANGLE_NAMESPACE(GCDTimerDelegate)> timeoutHandler;

    bool cancelled;
}

- (instancetype)initWithDelegate:(id<QT_MANGLE_NAMESPACE(GCDTimerDelegate)>)delegate
{
    if (self = [super init]) {
        timeoutHandler = delegate;
        timeoutMS = 0;
        timeoutStepMS = 0;
        objectUnderWatch = nil;
        timeoutType  = OperationTimeout::none;
        cancelled = false;
    }
    return self;
}

- (void)watchAfter:(id)object withTimeoutType:(OperationTimeout)type
{
    objectUnderWatch = object;
    timeoutType = type;
}

- (void)startWithTimeout:(qint64)ms step:(qint64)stepMS
{
    Q_ASSERT(!timeoutMS && !timeoutStepMS);
    Q_ASSERT(!cancelled);

    if (!timeoutHandler) {
        // Nobody to report timeout to, no need to start any task then.
        return;
    }

    if (ms <= 0 || stepMS <= 0) {
        qCWarning(QT_BT_OSX, "Invalid timeout/step parameters");
        return;
    }

    timeoutMS = ms;
    timeoutStepMS = stepMS;
    timer.start();

    [self handleTimeout];
}

- (void)handleTimeout
{
    if (cancelled)
        return;

    const qint64 elapsed = timer.elapsed();
    if (elapsed >= timeoutMS) {
        [timeoutHandler timeout:self];
    } else {
        // Re-schedule:
        dispatch_queue_t leQueue(qt_LE_queue());
        Q_ASSERT(leQueue);
        const qint64 timeChunkMS = std::min(timeoutMS - elapsed, timeoutStepMS);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                     int64_t(timeChunkMS / 1000. * NSEC_PER_SEC)),
                                     leQueue,
                                     ^{
                                           [self handleTimeout];
                                      });
    }
}

- (void)cancelTimer
{
    cancelled = true;
    timeoutHandler = nil;
    objectUnderWatch = nil;
    timeoutType = OperationTimeout::none;
}

- (id)objectUnderWatch
{
    return objectUnderWatch;
}

- (OperationTimeout)timeoutType
{
    return timeoutType;
}

@end
