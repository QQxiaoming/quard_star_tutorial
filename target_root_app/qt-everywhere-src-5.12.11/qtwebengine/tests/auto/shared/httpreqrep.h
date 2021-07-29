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
#ifndef HTTPREQREP_H
#define HTTPREQREP_H

#include <QTcpSocket>

#include <map>
#include <utility>

// Represents an HTTP request-response exchange.
class HttpReqRep : public QObject
{
    Q_OBJECT
public:
    explicit HttpReqRep(QTcpSocket *socket, QObject *parent = nullptr);

    void sendResponse();
    void close();

    // Request parameters (only valid after requestReceived())

    QByteArray requestMethod() const { return m_requestMethod; }
    QByteArray requestPath() const { return m_requestPath; }
    QByteArray requestHeader(const QByteArray &key) const;

    // Response parameters (can be set until sendResponse()/close()).

    int responseStatus() const { return m_responseStatusCode; }
    void setResponseStatus(int statusCode)
    {
        m_responseStatusCode = statusCode;
    }
    void setResponseHeader(const QByteArray &key, QByteArray value)
    {
        m_responseHeaders[key.toLower()] = std::move(value);
    }
    QByteArray responseBody() const { return m_responseBody; }
    void setResponseBody(QByteArray content)
    {
        m_responseHeaders["content-length"] = QByteArray::number(content.size());
        m_responseBody = std::move(content);
    }

Q_SIGNALS:
    // Emitted when the request has been correctly parsed.
    void requestReceived();
    // Emitted on first call to sendResponse().
    void responseSent();
    // Emitted when something goes wrong.
    void error(const QString &error);
    // Emitted during or some time after sendResponse() or close().
    void closed();

private Q_SLOTS:
    void handleReadyRead();
    void handleDisconnected();

private:
    enum class State {
        // Waiting for first line of request.
        RECEIVING_REQUEST,      // Next: RECEIVING_HEADERS or DISCONNECTING.
        // Waiting for header lines.
        RECEIVING_HEADERS,      // Next: REQUEST_RECEIVED or DISCONNECTING.
        // Request parsing succeeded, waiting for sendResponse() or close().
        REQUEST_RECEIVED,       // Next: DISCONNECTING.
        // Waiting for network.
        DISCONNECTING,          // Next: DISCONNECTED.
        // Connection is dead.
        DISCONNECTED,           // Next: -
    };
    QTcpSocket *m_socket = nullptr;
    State m_state = State::RECEIVING_REQUEST;
    QByteArray m_requestMethod;
    QByteArray m_requestPath;
    std::map<QByteArray, QByteArray> m_requestHeaders;
    int m_responseStatusCode = 200;
    std::map<QByteArray, QByteArray> m_responseHeaders;
    QByteArray m_responseBody;
};

#endif // !HTTPREQREP_H
