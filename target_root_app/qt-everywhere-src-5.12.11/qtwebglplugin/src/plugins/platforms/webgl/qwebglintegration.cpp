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

#include "qwebglintegration.h"
#include "qwebglintegration_p.h"

#include "qwebglwindow.h"
#include "qwebglcontext.h"
#include "qwebglcontext.h"
#include "qwebglhttpserver.h"
#include "qwebglwebsocketserver.h"
#include "qwebglplatformservices.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstring.h>
#include <QtGui/qclipboard.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include <QtGui/qsurfaceformat.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qoffscreensurface.h>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtThemeSupport/private/qgenericunixthemes_p.h>
#include <QtWebSockets/qwebsocket.h>

#if defined(QT_QUICK_LIB)
#include <QtQuick/qquickwindow.h>
#endif

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcWebGL, "qt.qpa.webgl")

QWebGLIntegrationPrivate *QWebGLIntegrationPrivate::instance()
{
    auto platformIntegration = QGuiApplicationPrivate::instance()->platformIntegration();
    return static_cast<QWebGLIntegration *>(platformIntegration)->d_ptr.data();
}

QWebGLIntegration::QWebGLIntegration(quint16 port) :
    d_ptr(new QWebGLIntegrationPrivate)
{
    Q_D(QWebGLIntegration);
    d->q_ptr = this;
    d->httpPort = port;
    d->touchDevice = new QTouchDevice;
    d->touchDevice->setName("EmulatedTouchDevice");
    d->touchDevice->setType(QTouchDevice::TouchScreen);
    d->touchDevice->setCapabilities(QTouchDevice::Position | QTouchDevice::Pressure |
                                    QTouchDevice::MouseEmulation);
    d->touchDevice->setMaximumTouchPoints(6);
    QWindowSystemInterface::registerTouchDevice(d->touchDevice);

    qCDebug(lcWebGL, "WebGL QPA Plugin created");
    qRegisterMetaType<QWebSocket *>("QWebSocket *");
    qRegisterMetaType<QWebGLWebSocketServer::MessageType>("QWebGLWebSocketServer::MessageType");
}

QWebGLIntegration::~QWebGLIntegration()
{
    Q_D(QWebGLIntegration);
    QWindowSystemInterface::unregisterTouchDevice(d->touchDevice);
}

QWebGLIntegration *QWebGLIntegration::instance()
{
    return static_cast<QWebGLIntegration *>(qGuiApp->platformNativeInterface());
}

void QWebGLIntegration::initialize()
{
    Q_D(QWebGLIntegration);

#if defined(QT_QUICK_LIB)
    qputenv("QSG_RENDER_LOOP", "threaded"); // Force threaded QSG_RENDER_LOOP
#endif

    d->inputContext = QPlatformInputContextFactory::create();
    d->screen = new QWebGLScreen;
    screenAdded(d->screen, true);

    d->webSocketServer = new QWebGLWebSocketServer;
    d->httpServer = new QWebGLHttpServer(d->webSocketServer, this);
    bool ok = d->httpServer->listen(QHostAddress::Any, d->httpPort);
    if (!ok) {
        qFatal("QWebGLIntegration::initialize: Failed to initialize: %s",
               qPrintable(d->httpServer->errorString()));
    }
    d->webSocketServerThread = new QThread(this);
    d->webSocketServerThread->setObjectName("WebSocketServer");
    d->webSocketServer->moveToThread(d->webSocketServerThread);
    connect(d->webSocketServerThread, &QThread::finished,
            d->webSocketServer, &QObject::deleteLater);
    QMetaObject::invokeMethod(d->webSocketServer, "create", Qt::QueuedConnection);
    QMutexLocker lock(d->webSocketServer->mutex());
    d->webSocketServerThread->start();
    d->webSocketServer->waitCondition()->wait(d->webSocketServer->mutex());

    qGuiApp->setQuitOnLastWindowClosed(false);
}

void QWebGLIntegration::destroy()
{
    Q_D(QWebGLIntegration);
    foreach (QWindow *w, qGuiApp->topLevelWindows())
        w->destroy();

    destroyScreen(d->screen);

    d->screen = nullptr;

    d->webSocketServerThread->quit();
    d->webSocketServerThread->wait();
    delete d->webSocketServerThread;
}

QAbstractEventDispatcher *QWebGLIntegration::createEventDispatcher() const
{
#ifdef Q_OS_WIN
    return new QWindowsGuiEventDispatcher;
#else
    return createUnixEventDispatcher();
#endif // Q_OS_WIN
}

QPlatformServices *QWebGLIntegration::services() const
{
    Q_D(const QWebGLIntegration);
    return &d->services;
}

QPlatformInputContext *QWebGLIntegration::inputContext() const
{
    Q_D(const QWebGLIntegration);
    return d->inputContext;
}

QPlatformFontDatabase *QWebGLIntegration::fontDatabase() const
{
    Q_D(const QWebGLIntegration);
    return &d->fontDatabase;
}

QPlatformTheme *QWebGLIntegration::createPlatformTheme(const QString &name) const
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return QPlatformIntegration::createPlatformTheme(name);
#else
    return QGenericUnixTheme::createUnixTheme(name);
#endif // Q_OS_WIN
}

QPlatformBackingStore *QWebGLIntegration::createPlatformBackingStore(QWindow *window) const
{
    Q_UNUSED(window);
    qCCritical(lcWebGL, "WebGL QPA platform plugin: Raster surfaces are not supported");
    return nullptr;
}

QPlatformWindow *QWebGLIntegration::createPlatformWindow(QWindow *window) const
{
    Q_D(const QWebGLIntegration);
    qCDebug(lcWebGL, "Creating platform window for: %p", window);

#if defined(QT_QUICK_LIB)
    if (window->inherits("QQuickWindow")) {
        auto quickWindow = (QQuickWindow *)window;
        quickWindow->setPersistentSceneGraph(false);
        quickWindow->setPersistentOpenGLContext(false);
    }
#endif

    d->windows.append(window);
    QObject::connect(window, &QWindow::destroyed, [=] ()
    {
        d->windows.removeAll(window);
    });

    QWindowSystemInterface::flushWindowSystemEvents();

    QWebGLWindow *platformWindow = nullptr;
    QWebSocket *socket = nullptr;
    auto winId = WId(-1);
    {
        QMutexLocker locker(&d->clients.mutex);

        if (d->clients.list.isEmpty()) {
            QMetaObject::invokeMethod(window, "close", Qt::QueuedConnection);
            return new QWebGLWindow(window);
        }

        auto client = &d->clients.list.first();
        window->setScreen(client->platformScreen->screen());
        client->platformWindows.append(new QWebGLWindow(window));
        platformWindow = client->platformWindows.last();
        socket = client->socket;
        platformWindow->create();
        platformWindow->requestActivateWindow();
        winId = platformWindow->winId();
    }

    const QVariantMap values {
        { "x", platformWindow->geometry().x() },
        { "y", platformWindow->geometry().y() },
        { "width", platformWindow->geometry().width() },
        { "height", platformWindow->geometry().height() },
        { "winId", winId },
        { "title", qApp->applicationDisplayName() }
    };
    d->sendMessage(socket, QWebGLWebSocketServer::MessageType::CreateCanvas, values);

    QObject::connect(window, &QWindow::windowTitleChanged, [=](const QString &title)
    {
        const QVariantMap values{{ "title", title }, { "winId", winId }};
        d->sendMessage(socket, QWebGLWebSocketServer::MessageType::ChangeTitle, values);
    });
    qCDebug(lcWebGL, "Created platform window %p for: %p", platformWindow, window);
    return platformWindow;
}

QPlatformOpenGLContext *QWebGLIntegration::createPlatformOpenGLContext(QOpenGLContext *context)
    const
{
    qCDebug(lcWebGL, "%p", context);
    QVariant nativeHandle = context->nativeHandle();

    const QSurfaceFormat adjustedFormat = context->format();
    QWebGLContext *ctx = new QWebGLContext(adjustedFormat);
    context->setNativeHandle(nativeHandle);
    return ctx;
}

bool QWebGLIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case OpenGL:
    case ThreadedOpenGL:
    case ThreadedPixmaps:
        return true;
    default:
        return false;
    }
}

QPlatformNativeInterface *QWebGLIntegration::nativeInterface() const
{
    return const_cast<QWebGLIntegration *>(this);
}

void QWebGLIntegration::openUrl(const QUrl &url)
{
    Q_D(QWebGLIntegration);
    qCDebug(lcWebGL, "%s", qPrintable(url.toString()));
    QMutexLocker locker(&d->clients.mutex);
    for (auto &clientData : d->clients.list) {
        const QVariantMap values {
            { "url", url }
        };
        d->sendMessage(clientData.socket, QWebGLWebSocketServer::MessageType::OpenUrl, values);
    }
}

QWebGLIntegrationPrivate::ClientData *QWebGLIntegrationPrivate::findClientData(
        const QWebSocket *socket)
{
    QMutexLocker locker(&clients.mutex);
    auto it = std::find_if(clients.list.begin(), clients.list.end(), [=](const ClientData &data)
    {
        return data.socket == socket;
    });

    return it != clients.list.end() ? &*it : nullptr;
}

QWebGLIntegrationPrivate::ClientData *QWebGLIntegrationPrivate::findClientData(
        const QPlatformSurface *surface)
{
    QMutexLocker locker(&clients.mutex);
    auto it = std::find_if(clients.list.begin(), clients.list.end(), [=](const ClientData &data)
    {
        if (!data.platformWindows.isEmpty() && data.platformWindows.last()->surface())
            return surface == data.platformWindows.last()->surface()->surfaceHandle();
        return false;
    });
    return it != clients.list.end() ? &*it : nullptr;
}

QWebGLWindow *QWebGLIntegrationPrivate::findWindow(const ClientData &clientData, WId winId)
{
    auto &windows = clientData.platformWindows;
    auto it = std::find_if(windows.begin(), windows.end(), [winId](QWebGLWindow *window)
    {
        return window->winId() == winId;
    });
    Q_ASSERT(it != windows.end());
    return *it;
}

void QWebGLIntegrationPrivate::clientConnected(QWebSocket *socket,
                                               const int width,
                                               const int height,
                                               const double physicalWidth,
                                               const double physicalHeight)
{
    Q_Q(QWebGLIntegration);
    qCDebug(lcWebGL, "%p, Size: %dx%d. Physical Size: %fx%f",
            socket, width, height, physicalWidth, physicalHeight);
    QWebGLIntegrationPrivate::ClientData client;
    client.socket = socket;
    client.platformScreen = new QWebGLScreen(QSize(width, height),
                                             QSizeF(physicalWidth, physicalHeight));
    clients.mutex.lock();
    clients.list.append(client);
    clients.mutex.unlock();
    q->screenAdded(client.platformScreen, true);
    connectNextClient();
}

void QWebGLIntegrationPrivate::clientDisconnected(QWebSocket *socket)
{
    qCDebug(lcWebGL, "%p", socket);
    const auto predicate = [=](const QWebGLIntegrationPrivate::ClientData &item)
    {
        return socket == item.socket;
    };

    clients.mutex.lock();
    auto it = std::find_if(clients.list.begin(), clients.list.end(), predicate);
    if (it != clients.list.end()) {
        for (auto platformWindow : it->platformWindows) {
            auto window = platformWindow->window();
            QTimer::singleShot(0, window, &QWindow::close);
        }
        clients.list.erase(it);
    }
    clients.mutex.unlock();
    connectNextClient();
}

void QWebGLIntegrationPrivate::connectNextClient()
{
    static QMutex connecting;
    if (connecting.tryLock()) {
        QTimer::singleShot(1000, [=]() {
            clients.mutex.lock();
            if (!clients.list.isEmpty()) {
                const auto clientData = clients.list.first();
                qCDebug(lcWebGL, "Connecting first client in the queue (%p)",
                       clientData.socket);
                for (auto window : windows)
                    QMetaObject::invokeMethod(window, "showFullScreen", Qt::QueuedConnection);
            }
            clients.mutex.unlock();
            connecting.unlock();
        });
    };
}

void QWebGLIntegrationPrivate::sendMessage(QWebSocket *socket,
                                           QWebGLWebSocketServer::MessageType type,
                                           const QVariantMap &values) const
{
    const auto ok = QMetaObject::invokeMethod(webSocketServer, "sendMessage",
                                              Q_ARG(QWebSocket*, socket),
                                              Q_ARG(QWebGLWebSocketServer::MessageType, type),
                                              Q_ARG(QVariantMap, values));
#if defined(QT_DEBUG)
    Q_ASSERT(ok);
#else
    Q_UNUSED(ok);
#endif
}

void QWebGLIntegrationPrivate::onTextMessageReceived(QWebSocket *socket, const QString &message)
{
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    Q_ASSERT(parseError.error == QJsonParseError::NoError);
    Q_ASSERT(document.isObject());
    const auto object = document.object();
    Q_ASSERT(object.contains("type"));
    const auto type = object[QStringLiteral("type")].toString();

    auto integrationPrivate = QWebGLIntegrationPrivate::instance();
    const auto clientData = integrationPrivate->findClientData(socket);

    if (type == QStringLiteral("connect"))
        clientConnected(socket, object["width"].toInt(), object["height"].toInt(),
                          object["physicalWidth"].toDouble(), object["physicalHeight"].toDouble());
    else if (!clientData || clientData->platformWindows.isEmpty())
        qCWarning(lcWebGL, "Message received before connect %s", qPrintable(message));
    else if (type == QStringLiteral("default_context_parameters"))
        handleDefaultContextParameters(*clientData, object);
    else if (type == QStringLiteral("gl_response"))
        handleGlResponse(object);
    else if (type == QStringLiteral("mouse"))
        handleMouse(*clientData, object);
    else if (type == QStringLiteral("wheel"))
        handleWheel(*clientData, object);
    else if (type == QStringLiteral("touch"))
        handleTouch(*clientData, object);
    else if (type.startsWith("key"))
        handleKeyboard(*clientData, type, object);
    else if (type == QStringLiteral("canvas_resize"))
        handleCanvasResize(*clientData, object);
}

void QWebGLIntegrationPrivate::handleDefaultContextParameters(const ClientData &clientData,
                                                              const QJsonObject &object)
{
    const auto winId = object.value("name").toInt(-1);
    Q_ASSERT(winId != -1);
    QWebGLWindow *platformWindow = findWindow(clientData, winId);
    Q_ASSERT(platformWindow);
    auto data = object.toVariantMap();
    data.remove("name");
    data.remove("type");
    QMap<GLenum, QVariant> result;
    for (auto it = data.cbegin(), end = data.cend(); it != end; ++it)
        result.insert(it.key().toInt(), *it);
    platformWindow->setDefaults(result);
}

void QWebGLIntegrationPrivate::handleGlResponse(const QJsonObject &object)
{
    qCDebug(lcWebGL, ) << "gl_response message received" << object;
    QMutexLocker locker(&waitMutex);
    const auto id = object["id"];
    const auto value = object["value"].toVariant();
    Q_ASSERT(pendingResponses.contains(id.toInt()));
    receivedResponses.insert(id.toInt(), value);
    pendingResponses.removeOne(id.toInt());
    waitCondition.wakeAll();
}

void QWebGLIntegrationPrivate::handleCanvasResize(const ClientData &clientData,
                                                  const QJsonObject &object)
{
    qCDebug(lcWebGL, ) << "canvas_resize message received" << object;
    const auto width = object["width"].toInt();
    const auto height = object["height"].toInt();
    const auto physicalWidth = object["physicalWidth"].toDouble();
    const auto physicalHeight = object["physicalHeight"].toDouble();
    clientData.platformScreen->setGeometry(width, height, physicalWidth, physicalHeight);
}

void QWebGLIntegrationPrivate::handleMouse(const ClientData &clientData, const QJsonObject &object)
{
    const auto winId = object.value("name").toInt(-1);
    Q_ASSERT(winId != -1);
    QPointF localPos(object.value("layerX").toDouble(),
                     object.value("layerY").toDouble());
    QPointF globalPos(object.value("clientX").toDouble(),
                      object.value("clientY").toDouble());
    auto buttons = static_cast<Qt::MouseButtons>(object.value("buttons").toInt());
    auto time = object.value("time").toDouble();
    auto platformWindow = findWindow(clientData, winId);
    QWindowSystemInterface::handleMouseEvent(platformWindow->window(),
                                             static_cast<ulong>(time),
                                             localPos,
                                             globalPos,
                                             Qt::MouseButtons(buttons),
                                             Qt::NoButton,
                                             QEvent::None,
                                             Qt::NoModifier,
                                             Qt::MouseEventNotSynthesized);
}

void QWebGLIntegrationPrivate::handleWheel(const ClientData &clientData, const QJsonObject &object)
{
    const auto winId = object.value("name").toInt(-1);
    Q_ASSERT(winId != -1);
    auto platformWindow = findWindow(clientData, winId);
    auto time = object.value("time").toDouble();
    QPointF localPos(object.value("layerX").toDouble(),
                     object.value("layerY").toDouble());
    QPointF globalPos(object.value("clientX").toDouble(),
                      object.value("clientY").toDouble());
    const int deltaX = -object.value("deltaX").toInt(0);
    const int deltaY = -object.value("deltaY").toInt(0);
    auto orientation = deltaY != 0 ? Qt::Vertical : Qt::Horizontal;

    QPoint point = (orientation == Qt::Vertical) ? QPoint(0, deltaY) : QPoint(deltaX, 0);
    QWindowSystemInterface::handleWheelEvent(platformWindow->window(),
                                             time,
                                             localPos,
                                             globalPos,
                                             QPoint(),
                                             point,
                                             Qt::NoModifier);
}

void QWebGLIntegrationPrivate::handleTouch(const ClientData &clientData, const QJsonObject &object)
{
    const auto winId = object.value("name").toInt(-1);
    Q_ASSERT(winId != -1);
    auto window = findWindow(clientData, winId)->window();
    const auto time = object.value("time").toDouble();
    const auto eventType = object.value("event").toString();
    if (eventType == QStringLiteral("touchcancel")) {
        QWindowSystemInterface::handleTouchCancelEvent(window,
                                                       time,
                                                       touchDevice,
                                                       Qt::NoModifier);
    } else {
        QList<QWindowSystemInterface::TouchPoint> points;
        auto touchToPoint = [](const QJsonValue &touch) -> QWindowSystemInterface::TouchPoint {
            QWindowSystemInterface::TouchPoint point; // support more than one
            const auto pageX = touch.toObject().value("pageX").toDouble();
            const auto pageY = touch.toObject().value("pageY").toDouble();
            const auto radiusX = touch.toObject().value("radiusX").toDouble();
            const auto radiusY = touch.toObject().value("radiusY").toDouble();
            const auto clientX = touch.toObject().value("clientX").toDouble();
            const auto clientY = touch.toObject().value("clientY").toDouble();
            point.id = touch.toObject().value("identifier").toInt(0);
            point.pressure = touch.toObject().value("force").toDouble(1.);
            point.area.setX(pageX - radiusX);
            point.area.setY(pageY - radiusY);
            point.area.setWidth(radiusX * 2);
            point.area.setHeight(radiusY * 2);
            point.normalPosition.setX(touch.toObject().value("normalPositionX").toDouble());
            point.normalPosition.setY(touch.toObject().value("normalPositionY").toDouble());
            point.rawPositions = {{ clientX, clientY }};
            return point;
        };

        for (const auto &touch : object.value("changedTouches").toArray()) {
            auto point = touchToPoint(touch);
            if (eventType == QStringLiteral("touchstart")) {
                point.state = Qt::TouchPointPressed;
            } else if (eventType == QStringLiteral("touchend")) {
                qCDebug(lcWebGL, ) << "end" << object;
                point.state = Qt::TouchPointReleased;
            } else {
                Q_ASSERT(eventType == QStringLiteral("touchmove"));
                point.state = Qt::TouchPointMoved;
            }
            points.append(point);
        }

        for (const auto &touch : object.value("stationaryTouches").toArray()) {
            auto point = touchToPoint(touch);
            point.state = Qt::TouchPointStationary;
            points.append(point);
        }

        QWindowSystemInterface::handleTouchEvent(window,
                                                 time,
                                                 touchDevice,
                                                 points,
                                                 Qt::NoModifier);
    }
}

void QWebGLIntegrationPrivate::handleKeyboard(const ClientData &clientData,
                                              const QString &type,
                                              const QJsonObject &object)
{
    const QHash<QString, Qt::Key> keyMap {
        { "Alt", Qt::Key_Alt },
        { "ArrowDown", Qt::Key_Down },
        { "ArrowLeft", Qt::Key_Left },
        { "ArrowRight", Qt::Key_Right },
        { "ArrowUp", Qt::Key_Up },
        { "Backspace", Qt::Key_Backspace },
        { "Control", Qt::Key_Control },
        { "Delete", Qt::Key_Delete },
        { "End", Qt::Key_End },
        { "Enter", Qt::Key_Enter },
        { "F1", Qt::Key_F1 },
        { "F2", Qt::Key_F2 },
        { "F3", Qt::Key_F3 },
        { "F4", Qt::Key_F4 },
        { "F5", Qt::Key_F5 },
        { "F6", Qt::Key_F6 },
        { "F7", Qt::Key_F7 },
        { "F8", Qt::Key_F8 },
        { "F9", Qt::Key_F9 },
        { "F10", Qt::Key_F10 },
        { "F11", Qt::Key_F11 },
        { "F12", Qt::Key_F12 },
        { "Escape", Qt::Key_Escape },
        { "Home", Qt::Key_Home },
        { "Insert", Qt::Key_Insert },
        { "Meta", Qt::Key_Meta },
        { "PageDown", Qt::Key_PageDown },
        { "PageUp", Qt::Key_PageUp },
        { "Shift", Qt::Key_Shift },
        { "Space", Qt::Key_Space },
        { "AltGraph", Qt::Key_AltGr },
        { "Tab", Qt::Key_Tab },
        { "Unidentified", Qt::Key_F },
        { "OS", Qt::Key_Super_L }
    };
    const auto timestamp = static_cast<ulong>(object.value("time").toDouble(-1));
    const auto keyName = object.value("key").toString();
    const auto specialKey = keyMap.find(keyName);
    QEvent::Type eventType;
    if (type == QStringLiteral("keydown"))
        eventType = QEvent::KeyPress;
    else if (type == QStringLiteral("keyup"))
        eventType = QEvent::KeyRelease;
    else
        return;
    QString string(object.value("key").toString());
    int key = object.value("which").toInt(0);
    if (specialKey != keyMap.end()) {
        key = *specialKey;
        string.clear();

        // special case: match Qt's behavior on other platforms and differentiate:
        // * "Enter": Qt::Key_Return
        // * "NumpadEnter": Qt::Key_Enter
        // TODO: consider whether "code" could be used rather than "keyName" above
        if (key == Qt::Key_Enter && object.value("code").toString() == QStringLiteral("Enter"))
            key = Qt::Key_Return;
    }

    const auto window = clientData.platformWindows.last()->window();
    QWindowSystemInterface::handleKeyEvent(window,
                                           timestamp,
                                           eventType,
                                           key,
                                           convertKeyboardModifiers(object),
                                           string);
}

Qt::KeyboardModifiers QWebGLIntegrationPrivate::convertKeyboardModifiers(const QJsonObject &object)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    if (object.value("ctrlKey").toBool())
        modifiers |= Qt::ControlModifier;
    if (object.value("shiftKey").toBool())
        modifiers |= Qt::ShiftModifier;
    if (object.value("altKey").toBool())
        modifiers |= Qt::AltModifier;
    if (object.value("metaKey").toBool())
        modifiers |= Qt::MetaModifier;
    return modifiers;
}

QT_END_NAMESPACE
