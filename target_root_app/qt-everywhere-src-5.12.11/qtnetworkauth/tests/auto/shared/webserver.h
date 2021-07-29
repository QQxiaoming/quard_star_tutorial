/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Network Auth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <functional>
#include <cctype>
#include <QtCore/qcoreapplication.h>
#include <QtNetwork/qtcpserver.h>
#include <QTcpSocket>

class WebServer : public QTcpServer
{
public:
    class HttpRequest {
        friend class WebServer;

        quint16 port = 0;
        enum class State {
            ReadingMethod,
            ReadingUrl,
            ReadingStatus,
            ReadingHeader,
            ReadingBody,
            AllDone
        } state = State::ReadingMethod;
        QByteArray fragment;
        int bytesLeft = 0;

        bool readMethod(QTcpSocket *socket);
        bool readUrl(QTcpSocket *socket);
        bool readStatus(QTcpSocket *socket);
        bool readHeaders(QTcpSocket *socket);
        bool readBody(QTcpSocket *socket);

    public:
        enum class Method {
            Unknown,
            Head,
            Get,
            Put,
            Post,
            Delete,
        } method = Method::Unknown;
        QUrl url;
        QPair<quint8, quint8> version;
        QMap<QByteArray, QByteArray> headers;
        QByteArray body;
    };

    typedef std::function<void(const HttpRequest &request, QTcpSocket *socket)> Handler;

    WebServer(Handler handler, QObject *parent = nullptr);

    QUrl url(const QString &path);

private:
    Handler handler;

    QMap<QTcpSocket *, HttpRequest> clients;
};

WebServer::WebServer(Handler handler, QObject *parent) :
    QTcpServer(parent),
    handler(handler)
{
    connect(this, &QTcpServer::newConnection, [=]() {
        auto socket = nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        connect(socket, &QTcpSocket::readyRead, [=]() {
            if (!clients.contains(socket))
                clients[socket].port = serverPort();

            auto *request = &clients[socket];
            auto ok = true;

            while (socket->bytesAvailable()) {
                if (Q_LIKELY(request->state == HttpRequest::State::ReadingMethod))
                    if (Q_UNLIKELY(!(ok = request->readMethod(socket))))
                        qWarning("Invalid Method");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingUrl))
                    if (Q_UNLIKELY(!(ok = request->readUrl(socket))))
                        qWarning("Invalid URL");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingStatus))
                    if (Q_UNLIKELY(!(ok = request->readStatus(socket))))
                        qWarning("Invalid Status");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingHeader))
                    if (Q_UNLIKELY(!(ok = request->readHeaders(socket))))
                        qWarning("Invalid Header");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingBody))
                    if (Q_UNLIKELY(!(ok = request->readBody(socket))))
                        qWarning("Invalid Body");
            }
            if (Q_UNLIKELY(!ok)) {
                socket->disconnectFromHost();
                clients.remove(socket);
            } else if (Q_LIKELY(request->state == HttpRequest::State::AllDone)) {
                Q_ASSERT(handler);
                if (request->headers.contains("Host")) {
                    const auto parts = request->headers["Host"].split(':');
                    request->url.setHost(parts.at(0));
                    if (parts.size() == 2)
                        request->url.setPort(parts.at(1).toUInt());
                }
                handler(*request, socket);
                socket->disconnectFromHost();
                clients.remove(socket);
            }
        });
    });

    const auto ok = listen(QHostAddress::LocalHost);
    Q_ASSERT(ok);
}

QUrl WebServer::url(const QString &path)
{
    const QString format("http://127.0.0.1:%1%2");
    return QUrl(format.arg(serverPort()).arg(path.startsWith('/') ? path : "/" + path));
}

bool WebServer::HttpRequest::readMethod(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isspace(c))
            finished = true;
        else if (std::isupper(c) && fragment.size() < 8)
            fragment += c;
        else
            return false;
    }
    if (finished) {
        if (fragment == "HEAD")
            method = Method::Head;
        else if (fragment == "GET")
            method = Method::Get;
        else if (fragment == "PUT")
            method = Method::Put;
        else if (fragment == "POST")
            method = Method::Post;
        else if (fragment == "DELETE")
            method = Method::Delete;
        else
            qWarning("Invalid operation %s", fragment.data());

        state = State::ReadingUrl;
        fragment.clear();

        return method != Method::Unknown;
    }
    return true;
}

bool WebServer::HttpRequest::readUrl(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isspace(c))
            finished = true;
        else
            fragment += c;
    }
    if (finished) {
        if (!fragment.startsWith("/")) {
            qWarning("Invalid URL path %s", fragment.constData());
            return false;
        }
        url.setUrl(QStringLiteral("http://127.0.0.1:") + QString::number(port) +
                   QString::fromUtf8(fragment));
        state = State::ReadingStatus;
        if (!url.isValid()) {
            qWarning("Invalid URL %s", fragment.constData());
            return false;
        }
        fragment.clear();
    }
    return true;
}

bool WebServer::HttpRequest::readStatus(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            finished = true;
            fragment.resize(fragment.size() - 2);
        }
    }
    if (finished) {
        if (!std::isdigit(fragment.at(fragment.size() - 3)) ||
                fragment.at(fragment.size() - 2) != '.' ||
                !std::isdigit(fragment.at(fragment.size() - 1))) {
            qWarning("Invalid version");
            return false;
        }
        version = qMakePair(fragment.at(fragment.size() - 3) - '0',
                            fragment.at(fragment.size() - 1) - '0');
        state = State::ReadingHeader;
        fragment.clear();
    }
    return true;
}

bool WebServer::HttpRequest::readHeaders(QTcpSocket *socket)
{
    while (socket->bytesAvailable()) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            if (fragment == "\r\n") {
                state = State::ReadingBody;
                fragment.clear();
                return true;
            } else {
                fragment.chop(2);
                const int index = fragment.indexOf(':');
                if (index == -1)
                    return false;

                const QByteArray key = fragment.mid(0, index).trimmed();
                const QByteArray value = fragment.mid(index + 1).trimmed();
                headers.insert(key, value);
                fragment.clear();
            }
        }
    }
    return true;
}

bool WebServer::HttpRequest::readBody(QTcpSocket *socket)
{
    if (headers.contains("Content-Length")) {
        bool conversionResult;
        bytesLeft = headers["Content-Length"].toInt(&conversionResult);
        if (Q_UNLIKELY(!conversionResult))
            return false;
        fragment.resize(bytesLeft);
    }
    while (bytesLeft) {
        int got = socket->read(&fragment.data()[fragment.size() - bytesLeft], bytesLeft);
        if (got < 0)
            return false; // error
        bytesLeft -= got;
        if (bytesLeft)
            qApp->processEvents();
    }
    fragment.swap(body);
    state = State::AllDone;
    return true;
}

#endif // WEBSERVER_H
