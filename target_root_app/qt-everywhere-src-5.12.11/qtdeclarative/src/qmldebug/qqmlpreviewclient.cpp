/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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


#include "qqmlpreviewclient_p_p.h"
#include <private/qpacket_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtQml/qqmlfile.h>

QT_BEGIN_NAMESPACE

QQmlPreviewClient::QQmlPreviewClient(QQmlDebugConnection *connection)
    : QQmlDebugClient(*(new QQmlPreviewClientPrivate(connection)))
{
}

void QQmlPreviewClient::messageReceived(const QByteArray &message)
{
    QPacket packet(connection()->currentDataStreamVersion(), message);

    qint8 command;
    packet >> command;

    switch (command) {
    case Error: {
        QString seviceError;
        packet >> seviceError;
        emit error(seviceError);
        break;
    }
    case Request: {
        QString fileName;
        packet >> fileName;
        emit request(fileName);
        break;
    }
    case Fps: {
        FpsInfo info;
        packet >> info.numSyncs >> info.minSync >> info.maxSync >> info.totalSync
               >> info.numRenders >> info.minRender >> info.maxRender >> info.totalRender;
        emit fps(info);
        break;
    }
    default:
        emit error(QString::fromLatin1("Unknown command received: %1").arg(command));
        break;
    }
}

void QQmlPreviewClient::sendDirectory(const QString &path, const QStringList &entries)
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(Directory) << path << entries;
    sendMessage(packet.data());
}

void QQmlPreviewClient::sendFile(const QString &path, const QByteArray &contents)
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(File) << path << contents;
    sendMessage(packet.data());
}

void QQmlPreviewClient::sendError(const QString &path)
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(Error) << path;
    sendMessage(packet.data());
}

void QQmlPreviewClient::triggerLoad(const QUrl &url)
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(Load) << url;
    sendMessage(packet.data());
}

void QQmlPreviewClient::triggerRerun()
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(Rerun);
    sendMessage(packet.data());
}

void QQmlPreviewClient::triggerZoom(float factor)
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(Zoom) << factor;
    sendMessage(packet.data());
}

void QQmlPreviewClient::triggerLanguage(const QUrl &url, const QString &locale)
{
    QPacket packet(connection()->currentDataStreamVersion());
    packet << static_cast<qint8>(Language) << url << locale;
    sendMessage(packet.data());
}

QT_END_NAMESPACE
