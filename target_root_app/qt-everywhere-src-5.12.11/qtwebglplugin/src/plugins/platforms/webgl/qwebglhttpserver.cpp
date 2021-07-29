/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt WebGL module of the Qt Toolkit.
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

#include "qwebglhttpserver.h"

#include "qwebglintegration.h"
#include "qwebglwebsocketserver.h"

#include <QtCore/qbuffer.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qpointer.h>
#include <QtCore/qtimer.h>
#include <QtCore/qurlquery.h>
#include <QtGui/qclipboard.h>
#include <QtGui/qicon.h>
#include <QtGui/qguiapplication.h>
#include <QtNetwork/qnetworkinterface.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

#include <cctype>
#include <cstdlib>

QT_BEGIN_NAMESPACE

static Q_LOGGING_CATEGORY(lc, "qt.qpa.webgl.httpserver")

struct HttpRequest {
    quint16 port = 0;

    bool readMethod(QTcpSocket *socket);
    bool readUrl(QTcpSocket *socket);
    bool readStatus(QTcpSocket *socket);
    bool readHeader(QTcpSocket *socket);

    enum class State {
        ReadingMethod,
        ReadingUrl,
        ReadingStatus,
        ReadingHeader,
        ReadingBody,
        AllDone
    } state = State::ReadingMethod;
    QByteArray fragment;

    enum class Method {
        Unknown,
        Head,
        Get,
        Put,
        Post,
        Delete,
    } method = Method::Unknown;
    quint32 byteSize = 0;
    QUrl url;
    QPair<quint8, quint8> version;
    QMap<QByteArray, QByteArray> headers;
};

class QWebGLHttpServerPrivate
{
public:
    QMap<QTcpSocket *, HttpRequest> clients;
    QMap<QString, QPointer<QIODevice>> customRequestDevices;
    QTcpServer server;
    QPointer<QWebGLWebSocketServer> webSocketServer;
};

QWebGLHttpServer::QWebGLHttpServer(QWebGLWebSocketServer *webSocketServer, QObject *parent) :
    QObject(parent),
    d_ptr(new QWebGLHttpServerPrivate)
{
    Q_D(QWebGLHttpServer);
    d->webSocketServer = webSocketServer;

    connect(&d->server, &QTcpServer::newConnection, this, &QWebGLHttpServer::clientConnected);
}

QWebGLHttpServer::~QWebGLHttpServer()
{}

bool QWebGLHttpServer::listen(const QHostAddress &address, quint16 port)
{
    Q_D(QWebGLHttpServer);
    const auto ok = d->server.listen(address, port);
    qCDebug(lc, "Listening in port %d", port);
    return ok;
}

bool QWebGLHttpServer::isListening() const
{
    Q_D(const QWebGLHttpServer);
    return d->server.isListening();
}

quint16 QWebGLHttpServer::serverPort() const
{
    Q_D(const QWebGLHttpServer);
    return d->server.serverPort();
}

QIODevice *QWebGLHttpServer::customRequestDevice(const QString &name)
{
    Q_D(const QWebGLHttpServer);
    return d->customRequestDevices.value(name, nullptr).data();
}

void QWebGLHttpServer::setCustomRequestDevice(const QString &name, QIODevice *device)
{
    Q_D(QWebGLHttpServer);
    if (d->customRequestDevices.value(name))
        d->customRequestDevices[name]->deleteLater();
    d->customRequestDevices.insert(name, device);
}

QString QWebGLHttpServer::errorString() const
{
    Q_D(const QWebGLHttpServer);
    return d->server.errorString();
}

void QWebGLHttpServer::clientConnected()
{
    Q_D(QWebGLHttpServer);
    auto socket = d->server.nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, this, &QWebGLHttpServer::clientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &QWebGLHttpServer::readData);
}

void QWebGLHttpServer::clientDisconnected()
{
    Q_D(QWebGLHttpServer);
    auto socket = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(socket);
    d->clients.remove(socket);
    socket->deleteLater();
}

void QWebGLHttpServer::readData()
{
    Q_D(QWebGLHttpServer);
    auto socket = qobject_cast<QTcpSocket *>(sender());
    if (!d->clients.contains(socket))
        d->clients[socket].port = d->server.serverPort();

    auto request = &d->clients[socket];
    bool error = false;

    request->byteSize += socket->bytesAvailable();
    if (Q_UNLIKELY(request->byteSize > 2048)) {
        socket->write(QByteArrayLiteral("HTTP 413 – Request entity too large\r\n"));
        socket->disconnectFromHost();
        d->clients.remove(socket);
        return;
    }

    if (Q_LIKELY(request->state == HttpRequest::State::ReadingMethod))
        if (Q_UNLIKELY(error = !request->readMethod(socket)))
            qCWarning(lc, "QWebGLHttpServer::readData: Invalid Method");

    if (Q_LIKELY(!error && request->state == HttpRequest::State::ReadingUrl))
        if (Q_UNLIKELY(error = !request->readUrl(socket)))
            qCWarning(lc, "QWebGLHttpServer::readData: Invalid URL");

    if (Q_LIKELY(!error && request->state == HttpRequest::State::ReadingStatus))
        if (Q_UNLIKELY(error = !request->readStatus(socket)))
            qCWarning(lc, "QWebGLHttpServer::readData: Invalid Status");

    if (Q_LIKELY(!error && request->state == HttpRequest::State::ReadingHeader))
        if (Q_UNLIKELY(error = !request->readHeader(socket)))
            qCWarning(lc, "QWebGLHttpServer::readData: Invalid Header");

    if (error) {
        socket->disconnectFromHost();
        d->clients.remove(socket);
    } else if (!request->url.isEmpty()) {
        Q_ASSERT(request->state != HttpRequest::State::ReadingUrl);
        answerClient(socket, request->url);
        d->clients.remove(socket);
    }
}

void QWebGLHttpServer::answerClient(QTcpSocket *socket, const QUrl &url)
{
    Q_D(QWebGLHttpServer);
    bool disconnect = true;
    const auto path = url.path();

    qCDebug(lc, "%s requested: %s",
           qPrintable(socket->localAddress().toString()), qPrintable(path));

    QByteArray answer = QByteArrayLiteral("HTTP/1.1 404 Not Found\r\n"
                                          "Content-Type: text/html\r\n"
                                          "Content-Length: 136\r\n\r\n"
                                          "<html>"
                                          "<head><title>404 Not Found</title></head>"
                                          "<body bgcolor=\"white\">"
                                          "<center><h1>404 Not Found</h1></center>"
                                          "</body>"
                                          "</html>");
    const auto addData = [&answer](const QByteArray &contentType, const QByteArray &data)
    {
        answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n");
        QByteArray ret;
        const auto dataSize = QString::number(data.size()).toUtf8();
        answer += QByteArrayLiteral("Content-Type: ") + contentType + QByteArrayLiteral("\r\n") +
                  QByteArrayLiteral("Content-Length: ") + dataSize + QByteArrayLiteral("\r\n\r\n") +
                  data;
    };

    if (path == QLatin1String("/")) {
        QFile file(QStringLiteral(":/webgl/index.html"));
        Q_ASSERT(file.exists());
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_ASSERT(file.isOpen());
        auto data = file.readAll();
        addData(QByteArrayLiteral("text/html; charset=\"utf-8\""), data);
    } else if (path == QStringLiteral("/clipboard")) {
#ifndef QT_NO_CLIPBOARD
        auto data = qGuiApp->clipboard()->text().toUtf8();
        addData(QByteArrayLiteral("text/html; charset=\"utf-8\""), data);
#else
        qCWarning(lc, "Qt was built without clipboard support");
#endif
    } else if (path == QStringLiteral("/webqt.js")) {
        QFile file(QStringLiteral(":/webgl/webqt.jsx"));
        Q_ASSERT(file.exists());
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        Q_ASSERT(file.isOpen());
        const auto host = url.host().toUtf8();
        const auto port = QString::number(d->webSocketServer->port()).toUtf8();
        QByteArray data = "var host = \"" + host + "\";\r\nvar port = " + port + ";\r\n";
        data += file.readAll();
        addData(QByteArrayLiteral("application/javascript"), data);
    } else if (path == QStringLiteral("/favicon.ico")) {
        QFile file(QStringLiteral(":/webgl/favicon.ico"));
        Q_ASSERT(file.exists());
        file.open(QIODevice::ReadOnly);
        Q_ASSERT(file.isOpen());
        auto data = file.readAll();
        addData(QByteArrayLiteral("image/x-icon"), data);
    } else if (path == QStringLiteral("/favicon.png")) {
        QBuffer buffer;
        qGuiApp->windowIcon().pixmap(16, 16).save(&buffer, "png");
        addData(QByteArrayLiteral("image/x-icon"), buffer.data());
    } else if (auto device = d->customRequestDevices.value(path)) {
        answer = QByteArrayLiteral("HTTP/1.0 200 OK \r\n"
                                   "Content-Type: text/plain; charset=\"utf-8\"\r\n"
                                   "Connection: Keep.Alive\r\n\r\n") +
                device->readAll();
        auto timer = new QTimer(device);
        timer->setSingleShot(false);
        connect(timer, &QTimer::timeout, [device, socket]()
        {
            if (device->bytesAvailable())
                socket->write(device->readAll());
        });
        timer->start(1000);
        disconnect = false;
    }
    socket->write(answer);
    if (disconnect)
        socket->disconnectFromHost();
}

bool HttpRequest::readMethod(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isupper(c) && fragment.size() < 6)
            fragment += c;
        else
            finished = true;
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
            qCWarning(lc, "QWebGLHttpServer::HttpRequest::readMethod: Invalid operation %s",
                     fragment.data());

        state = State::ReadingUrl;
        fragment.clear();

        return method != Method::Unknown;
    }
    return true;
}

bool HttpRequest::readUrl(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        char c;
        if (!socket->getChar(&c))
            return false;
        if (std::isspace(c))
            finished = true;
        else
            fragment += c;
    }
    if (finished) {
        if (!fragment.startsWith("/")) {
            qCWarning(lc, "QWebGLHttpServer::HttpRequest::readUrl: Invalid URL path %s",
                     fragment.constData());
            return false;
        }
        url.setUrl(QStringLiteral("http://localhost:") + QString::number(port) +
                   QString::fromUtf8(fragment));
        state = State::ReadingStatus;
        if (!url.isValid()) {
            qCWarning(lc, "QWebGLHttpServer::HttpRequest::readUrl: Invalid URL %s",
                     fragment.constData());
            return false;
        }
        fragment.clear();
        return true;
    }
    return true;
}

bool HttpRequest::readStatus(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            finished = true;
            fragment.chop(2);
        }
    }
    if (finished) {
        if (!std::isdigit(fragment.at(fragment.size() - 3)) ||
                !std::isdigit(fragment.at(fragment.size() - 1))) {
            qCWarning(lc, "QWebGLHttpServer::HttpRequest::::readStatus: Invalid version");
            return false;
        }
        version = qMakePair(fragment.at(fragment.size() - 3) - '0',
                            fragment.at(fragment.size() - 1) - '0');
        state = State::ReadingHeader;
        fragment.clear();
    }
    return true;
}

bool HttpRequest::readHeader(QTcpSocket *socket)
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
                if (QStringLiteral("host").compare(key, Qt::CaseInsensitive) == 0) {
                    auto parts = value.split(':');
                    if (parts.size() == 1) {
                        url.setHost(parts.first());
                        url.setPort(80);
                    } else {
                        url.setHost(parts.first());
                        url.setPort(std::strtoul(parts.at(1).constData(), nullptr, 10));
                    }
                }
                fragment.clear();
            }
        }
    }
    return false;
}

QT_END_NAMESPACE
