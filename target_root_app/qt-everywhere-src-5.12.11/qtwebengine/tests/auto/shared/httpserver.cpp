/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
#include "httpserver.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(gHttpServerLog, "HttpServer")

HttpServer::HttpServer(QObject *parent) : QObject(parent)
{
    connect(&m_tcpServer, &QTcpServer::newConnection, this, &HttpServer::handleNewConnection);
}

bool HttpServer::start()
{
    m_error = false;
    m_expectingError = false;

    if (!m_tcpServer.listen()) {
        qCWarning(gHttpServerLog).noquote() << m_tcpServer.errorString();
        return false;
    }

    m_url.setScheme(QStringLiteral("http"));
    m_url.setHost(QStringLiteral("127.0.0.1"));
    m_url.setPort(m_tcpServer.serverPort());

    return true;
}

bool HttpServer::stop()
{
    m_tcpServer.close();
    return m_error == m_expectingError;
}

void HttpServer::setExpectError(bool b)
{
    m_expectingError = b;
}

QUrl HttpServer::url(const QString &path) const
{
    auto copy = m_url;
    copy.setPath(path);
    return copy;
}

void HttpServer::handleNewConnection()
{
    auto rr = new HttpReqRep(m_tcpServer.nextPendingConnection(), this);
    connect(rr, &HttpReqRep::requestReceived, [this, rr]() {
        Q_EMIT newRequest(rr);
        rr->close();
    });
    connect(rr, &HttpReqRep::responseSent, [this, rr]() {
        qCInfo(gHttpServerLog).noquote() << rr->requestMethod() << rr->requestPath()
                                         << rr->responseStatus() << rr->responseBody().size();
    });
    connect(rr, &HttpReqRep::error, [this, rr](const QString &error) {
        qCWarning(gHttpServerLog).noquote() << rr->requestMethod() << rr->requestPath()
                                            << error;
        m_error = true;
    });
    connect(rr, &HttpReqRep::closed, rr, &QObject::deleteLater);
}
