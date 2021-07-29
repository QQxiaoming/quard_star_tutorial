/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "proxyserver.h"
#include <QDataStream>
#include <QTcpSocket>
#include <QDebug>

ProxyServer::ProxyServer(QObject *parent) : QObject(parent),
    m_port(5555)
{
    connect(&m_server, &QTcpServer::newConnection, this, &ProxyServer::handleNewConnection);
}

void ProxyServer::setPort(int port)
{
    m_port = port;
}

bool ProxyServer::isListening()
{
    return m_server.isListening();
}

void ProxyServer::run()
{
    if (!m_server.listen(QHostAddress::LocalHost, m_port))
        qFatal("Could not start the test server");
}

void ProxyServer::handleNewConnection()
{
    // do one connection at the time
    Q_ASSERT(m_data.isEmpty());
    QTcpSocket *socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);
    connect(socket, &QAbstractSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QAbstractSocket::readyRead, this, &ProxyServer::handleReadReady);
}

void ProxyServer::handleReadReady()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    Q_ASSERT(socket);

    m_data.append(socket->readAll());

    //simply wait for whole request
    if (!m_data.endsWith("\r\n\r\n"))
        return;

    // add fake proxy authetication
    socket->write("HTTP/1.1 407 Proxy Auth Required\nProxy-Authenticate: "
                      "Basic realm=\"Proxy requires authentication\"\r\n\r\n");

    m_data.clear();
    socket->disconnectFromHost();
    emit requestReceived();
}
