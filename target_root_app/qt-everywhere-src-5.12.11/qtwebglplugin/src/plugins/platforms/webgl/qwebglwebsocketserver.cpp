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

#include "qwebglwebsocketserver.h"

#include "qwebglcontext.h"
#include "qwebglfunctioncall.h"
#include "qwebglintegration.h"
#include "qwebglintegration_p.h"
#include "qwebglwindow.h"
#include "qwebglwindow_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qdebug.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qwaitcondition.h>
#include <QtGui/qevent.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>

#include <cstring>

QT_BEGIN_NAMESPACE

static Q_LOGGING_CATEGORY(lc, "qt.qpa.webgl.websocketserver")

inline QWebGLIntegration *webGLIntegration()
{
#ifdef QT_DEBUG
    auto nativeInterface = dynamic_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
    Q_ASSERT(nativeInterface);
#else
    auto nativeInterface = static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
#endif // QT_DEBUG
    return nativeInterface;
}

class QWebGLWebSocketServerPrivate
{
public:
    QWebSocketServer *server = nullptr;
};

QWebGLWebSocketServer::QWebGLWebSocketServer(QObject *parent) :
    QObject(parent),
    d_ptr(new QWebGLWebSocketServerPrivate)
{}

QWebGLWebSocketServer::~QWebGLWebSocketServer()
{}

quint16 QWebGLWebSocketServer::port() const
{
    Q_D(const QWebGLWebSocketServer);
    return d->server->serverPort();
}

QMutex *QWebGLWebSocketServer::mutex()
{
    return &QWebGLIntegrationPrivate::instance()->waitMutex;
}

QWaitCondition *QWebGLWebSocketServer::waitCondition()
{
    return &QWebGLIntegrationPrivate::instance()->waitCondition;
}

QVariant QWebGLWebSocketServer::queryValue(int id)
{
    QMutexLocker locker(&QWebGLIntegrationPrivate::instance()->waitMutex);
    if (QWebGLIntegrationPrivate::instance()->receivedResponses.contains(id))
        return QWebGLIntegrationPrivate::instance()->receivedResponses.take(id);
    return QVariant();
}

void QWebGLWebSocketServer::create()
{
    Q_D(QWebGLWebSocketServer);
    const QString serverName = QLatin1String("qtwebgl");
    const QUrl url(QString::fromUtf8(qgetenv("QT_WEBGL_WEBSOCKETSERVER")));
    QHostAddress hostAddress(url.host());
    if (!url.isValid() || url.isEmpty() || !(url.scheme() == "ws" || url.scheme() == "wss")) {
        d->server = new QWebSocketServer(serverName, QWebSocketServer::NonSecureMode);
        hostAddress = QHostAddress::Any;
    } else {
        d->server = new QWebSocketServer(serverName,
#if QT_CONFIG(ssl)
                                         url.scheme() == "wss" ? QWebSocketServer::SecureMode :
#endif
                                                                 QWebSocketServer::NonSecureMode);
    }
    if (d->server->listen(hostAddress, url.port(0))) {
        connect(d->server, &QWebSocketServer::newConnection,
                this, &QWebGLWebSocketServer::onNewConnection);
    } else {
        qCCritical(lc, "The WebSocket Server cannot start: %s",
                   qPrintable(d->server->errorString()));
    }

    QMutexLocker lock(&QWebGLIntegrationPrivate::instance()->waitMutex);
    QWebGLIntegrationPrivate::instance()->waitCondition.wakeAll();
}

void QWebGLWebSocketServer::sendMessage(QWebSocket *socket,
                                        MessageType type,
                                        const QVariantMap &values)
{
    if (!socket)
        return;
    QString typeString;
    switch (type) {
    case MessageType::Connect:
        typeString = QStringLiteral("connect");
        qCDebug(lc) << "Sending connect to " << socket << values;
        break;
    case MessageType::GlCommand: {
        const auto functionName = values["function"].toString().toUtf8();
        const auto parameters = values["parameters"].toList();
        const quint32 parameterCount = parameters.size();
        qCDebug(lc, "Sending gl_command %s to %p with %d parameters",
                qPrintable(functionName), socket, parameterCount);
        QByteArray data;
        {
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream << QWebGLContext::functionIndex(functionName);
            if (values.contains("id")) {
                auto ok = false;
                stream << quint32(values["id"].toUInt(&ok));
                Q_ASSERT(ok);
            }
            const std::function<void(const QVariantList &)> serialize = [&stream, &serialize](
                    const QVariantList &parameters) {
                for (const auto &value : parameters) {
                    if (value.isNull()) {
                        stream << (quint8)'n';
                    } else switch (value.type()) {
                    case QVariant::Int:
                        stream << (quint8)'i' << value.toInt();
                        break;
                    case QVariant::UInt:
                        stream << (quint8)'u' << value.toUInt();
                        break;
                    case QVariant::Bool:
                        stream << (quint8)'b' << (quint8)value.toBool();
                        break;
                    case QVariant::Double:
                        stream << (quint8)'d' << value.toDouble();
                        break;
                    case QVariant::String:
                        stream << (quint8)'s' << value.toString().toUtf8();
                        break;
                    case QVariant::ByteArray: {
                        const auto byteArray = value.toByteArray();
                        if (byteArray.isNull())
                            stream << (quint8)'n';
                        else
                            stream << (quint8)'x' << byteArray;
                        break;
                    }
                    case QVariant::List: {
                        const auto list = value.toList();
                        stream << quint8('a') << quint8(list.size());
                        serialize(list);
                        break;
                    }
                    default:
                        qCCritical(lc, "Unsupported type: %d", value.type());
                        break;
                    }
                }
            };
            serialize(parameters);
            stream << (quint32)0xbaadf00d;
        }
        socket->sendBinaryMessage(data);
        return;
    }
    case MessageType::CreateCanvas:
        qCDebug(lc) << "Sending create_canvas to " << socket << values;
        typeString = QStringLiteral("create_canvas");
        break;
    case MessageType::DestroyCanvas:
        return; // TODO: In current implementation the canvas is not destroyed
        qCDebug(lc) << "Sending destroy_canvas to " << socket << values;
        typeString = QStringLiteral("destroy_canvas");
        break;
    case MessageType::OpenUrl:
        qCDebug(lc) << "Sending open_url to " << socket << values;
        typeString = QStringLiteral("open_url");
        break;
    case MessageType::ChangeTitle:
        qCDebug(lc) << "Sending change_title to " << socket << values;
        typeString = QStringLiteral("changle_title");
        break;
    }
    QJsonDocument document;
    auto commandObject = QJsonObject::fromVariantMap(values);
    commandObject["type"] = typeString;
    document.setObject(commandObject);
    auto data = document.toJson(QJsonDocument::Compact);
    socket->sendTextMessage(data);
}

bool QWebGLWebSocketServer::event(QEvent *event)
{
    int type = event->type();
    if (type == QWebGLFunctionCall::type()) {
        auto e = static_cast<QWebGLFunctionCall *>(event);
        QVariantMap values {
           { "function", e->functionName() },
           { "parameters", e->parameters() }
        };
        if (e->id() != -1)
            values.insert("id", e->id());
        auto integrationPrivate = QWebGLIntegrationPrivate::instance();
        auto clientData = integrationPrivate->findClientData(e->surface());
        if (clientData && clientData->socket) {
            sendMessage(clientData->socket, MessageType::GlCommand, values);
            if (e->isBlocking())
                integrationPrivate->pendingResponses.append(e->id());
            return true;
        }
        return false;
    }
    return QObject::event(event);
}

void QWebGLWebSocketServer::onNewConnection()
{
    Q_D(QWebGLWebSocketServer);
    QWebSocket *socket = d->server->nextPendingConnection();
    if (socket) {
        connect(socket, &QWebSocket::disconnected, this, &QWebGLWebSocketServer::onDisconnect);
        connect(socket, &QWebSocket::textMessageReceived, this,
                &QWebGLWebSocketServer::onTextMessageReceived);

        const QVariantMap values{
            {
                QStringLiteral("debug"),
#ifdef QT_DEBUG
                true
#else
                false
#endif
            },
            { QStringLiteral("loadingScreen"), qgetenv("QT_WEBGL_LOADINGSCREEN") },
            { QStringLiteral("mouseTracking"), qgetenv("QT_WEBGL_MOUSETRACKING") },
            { QStringLiteral("supportedFunctions"),
              QVariant::fromValue(QWebGLContext::supportedFunctions()) },
            { "sysinfo",
                QVariantMap {
                    { QStringLiteral("buildAbi"), QSysInfo::buildAbi() },
                    { QStringLiteral("buildCpuArchitecture"), QSysInfo::buildCpuArchitecture() },
                    { QStringLiteral("currentCpuArchitecture"),
                      QSysInfo::currentCpuArchitecture() },
                    { QStringLiteral("kernelType"), QSysInfo::kernelType() },
                    { QStringLiteral("machineHostName"), QSysInfo::machineHostName() },
                    { QStringLiteral("prettyProductName"), QSysInfo::prettyProductName() },
                    { QStringLiteral("productType"), QSysInfo::productType() },
                    { QStringLiteral("productVersion"), QSysInfo::productVersion() },
                }
            }
        };

        sendMessage(socket, MessageType::Connect, values);
    }
}

void QWebGLWebSocketServer::onDisconnect()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    Q_ASSERT(socket);
    QWebGLIntegrationPrivate::instance()->clientDisconnected(socket);
    socket->deleteLater();
}

void QWebGLWebSocketServer::onTextMessageReceived(const QString &message)
{
    const auto socket = qobject_cast<QWebSocket *>(sender());
    QWebGLIntegrationPrivate::instance()->onTextMessageReceived(socket, message);
}

QT_END_NAMESPACE
