/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chatserver.h"

#include <QtBluetooth/qbluetoothserver.h>
#include <QtBluetooth/qbluetoothsocket.h>

//! [Service UUID]
static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");
//! [Service UUID]

ChatServer::ChatServer(QObject *parent)
    :   QObject(parent)
{
}

ChatServer::~ChatServer()
{
    stopServer();
}

void ChatServer::startServer(const QBluetoothAddress& localAdapter)
{
    if (rfcommServer)
        return;

    //! [Create the server]
    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, &QBluetoothServer::newConnection,
            this, QOverload<>::of(&ChatServer::clientConnected));
    bool result = rfcommServer->listen(localAdapter);
    if (!result) {
        qWarning() << "Cannot bind chat server to" << localAdapter.toString();
        return;
    }
    //! [Create the server]

    //serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

    QBluetoothServiceInfo::Sequence profileSequence;
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    classId << QVariant::fromValue(quint16(0x100));
    profileSequence.append(QVariant::fromValue(classId));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                             profileSequence);

    classId.clear();
    classId << QVariant::fromValue(QBluetoothUuid(serviceUuid));
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

    //! [Service name, description and provider]
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Bt Chat Server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             tr("Example bluetooth chat server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("qt-project.org"));
    //! [Service name, description and provider]

    //! [Service UUID set]
    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));
    //! [Service UUID set]

    //! [Service Discoverability]
    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                             publicBrowse);
    //! [Service Discoverability]

    //! [Protocol descriptor list]
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(rfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);
    //! [Protocol descriptor list]

    //! [Register service]
    serviceInfo.registerService(localAdapter);
    //! [Register service]
}

//! [stopServer]
void ChatServer::stopServer()
{
    // Unregister service
    serviceInfo.unregisterService();

    // Close sockets
    qDeleteAll(clientSockets);

    // Close server
    delete rfcommServer;
    rfcommServer = nullptr;
}
//! [stopServer]

//! [sendMessage]
void ChatServer::sendMessage(const QString &message)
{
    QByteArray text = message.toUtf8() + '\n';

    for (QBluetoothSocket *socket : qAsConst(clientSockets))
        socket->write(text);
}
//! [sendMessage]

//! [clientConnected]
void ChatServer::clientConnected()
{
    QBluetoothSocket *socket = rfcommServer->nextPendingConnection();
    if (!socket)
        return;

    connect(socket, &QBluetoothSocket::readyRead, this, &ChatServer::readSocket);
    connect(socket, &QBluetoothSocket::disconnected, this, QOverload<>::of(&ChatServer::clientDisconnected));
    clientSockets.append(socket);
    emit clientConnected(socket->peerName());
}
//! [clientConnected]

//! [clientDisconnected]
void ChatServer::clientDisconnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    emit clientDisconnected(socket->peerName());

    clientSockets.removeOne(socket);

    socket->deleteLater();
}
//! [clientDisconnected]

//! [readSocket]
void ChatServer::readSocket()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(socket->peerName(),
                             QString::fromUtf8(line.constData(), line.length()));
    }
}
//! [readSocket]
