/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qqmldebugmessageclient_p.h"

#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

/*!
  \class QQmlDebugMessageClient
  \internal

  \brief Client for the debug message service

  The QQmlDebugMessageClient receives debug messages routed through the QML
  debug connection via QDebugMessageService.
 */

QQmlDebugMessageClient::QQmlDebugMessageClient(QQmlDebugConnection *client)
    : QQmlDebugClient(QLatin1String("DebugMessages"), client)
{
}

void QQmlDebugMessageClient::messageReceived(const QByteArray &data)
{
    QDataStream ds(data);
    QByteArray command;
    ds >> command;

    if (command == "MESSAGE") {
        int type;
        int line;
        QByteArray debugMessage;
        QByteArray file;
        QByteArray function;
        ds >> type >> debugMessage >> file >> line >> function;
        QQmlDebugContextInfo info;
        info.line = line;
        info.file = QString::fromUtf8(file);
        info.function = QString::fromUtf8(function);
        info.timestamp = -1;
        if (!ds.atEnd()) {
            QByteArray category;
            ds >> category;
            info.category = QString::fromUtf8(category);
            if (!ds.atEnd())
                ds >> info.timestamp;
        }
        emit message(QtMsgType(type), QString::fromUtf8(debugMessage), info);
    }
}

QT_END_NAMESPACE
