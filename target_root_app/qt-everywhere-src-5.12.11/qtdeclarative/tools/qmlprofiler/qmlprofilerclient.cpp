/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmlprofilerclient.h"
#include "qmlprofilerdata.h"

#include <private/qqmlprofilerclient_p_p.h>

#include <QtCore/QStack>
#include <QtCore/QStringList>

#include <limits>

class QmlProfilerClientPrivate : public QQmlProfilerClientPrivate
{
    Q_DECLARE_PUBLIC(QmlProfilerClient)
public:
    QmlProfilerClientPrivate(QQmlDebugConnection *connection, QmlProfilerData *data);

    QmlProfilerData *data;
    bool enabled;
};

QmlProfilerClientPrivate::QmlProfilerClientPrivate(QQmlDebugConnection *connection,
                                                   QmlProfilerData *data) :
    QQmlProfilerClientPrivate(connection, data), data(data), enabled(false)
{
}

QmlProfilerClient::QmlProfilerClient(QQmlDebugConnection *connection, QmlProfilerData *data) :
    QQmlProfilerClient(*(new QmlProfilerClientPrivate(connection, data)))
{
    Q_D(QmlProfilerClient);
    setRequestedFeatures(std::numeric_limits<quint64>::max());
    connect(this, &QQmlDebugClient::stateChanged,
            this, &QmlProfilerClient::onStateChanged);
    connect(this, &QQmlProfilerClient::traceStarted,
            d->data, &QmlProfilerData::setTraceStartTime);
    connect(this, &QQmlProfilerClient::traceFinished,
            d->data, &QmlProfilerData::setTraceEndTime);
    connect(this, &QQmlProfilerClient::complete,
            d->data, &QmlProfilerData::complete);
}

void QmlProfilerClient::onStateChanged(State state)
{
    Q_D(QmlProfilerClient);
    if ((d->enabled && state != Enabled) || (!d->enabled && state == Enabled)) {
        d->enabled = (state == Enabled);
        emit enabledChanged(d->enabled);
    }
}

#include "moc_qmlprofilerclient.cpp"
