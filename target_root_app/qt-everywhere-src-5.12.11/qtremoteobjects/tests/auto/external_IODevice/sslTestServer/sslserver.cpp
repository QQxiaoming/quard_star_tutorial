/****************************************************************************
**
** Copyright (C) 2018 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
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

#include "sslserver.h"
#include <QSslSocket>

SslServer::SslServer(QObject *parent)
    : QTcpServer(parent)
{}


void SslServer::incomingConnection(qintptr socketDescriptor)
{
    auto serverSocket = new QSslSocket;
    if (serverSocket->setSocketDescriptor(socketDescriptor)) {
        addPendingConnection(serverSocket);
        connect(serverSocket, &QSslSocket::encrypted, this, [this, serverSocket] {
           Q_EMIT encryptedSocketReady(serverSocket);
        });
        connect(serverSocket, static_cast<void (QSslSocket::*)(const QList<QSslError>&)>(&QSslSocket::sslErrors),
                this, [serverSocket](const QList<QSslError>& errors){
            qWarning() << "Error:" << serverSocket << errors;
            delete serverSocket;
        });
        serverSocket->setPeerVerifyMode(QSslSocket::VerifyPeer);
        serverSocket->setLocalCertificate(QStringLiteral(":/sslcert/server.crt"));
        serverSocket->setPrivateKey(QStringLiteral(":/sslcert/server.key"));
        serverSocket->startServerEncryption();
    } else {
        delete serverSocket;
    }
}
