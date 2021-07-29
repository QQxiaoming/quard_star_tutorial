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

#ifndef QMLPROFILERCLIENT_H
#define QMLPROFILERCLIENT_H

#include <private/qqmlprofilerclient_p.h>
#include <private/qqmlprofilerclientdefinitions_p.h>
#include <private/qqmlprofilereventlocation_p.h>

class QmlProfilerData;
class QmlProfilerClientPrivate;
class QmlProfilerClient : public QQmlProfilerClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QmlProfilerClient)

public:
    QmlProfilerClient(QQmlDebugConnection *connection, QmlProfilerData *data);

signals:
    void enabledChanged(bool enabled);
    void error(const QString &error);

private:
    void onStateChanged(State state);
};

#endif // QMLPROFILERCLIENT_H
