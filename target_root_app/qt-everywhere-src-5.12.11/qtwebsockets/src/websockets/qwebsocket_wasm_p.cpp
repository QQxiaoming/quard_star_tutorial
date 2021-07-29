/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
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

#include "qwebsocket_p.h"

#include <QtCore/qcoreapplication.h>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

static void q_onErrorCallback(val event)
{
    val target = event["target"];

    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate*>(target["data-context"].as<quintptr>());
    Q_ASSERT (wsp);

    emit wsp->q_func()->error(wsp->error());
}

static void q_onCloseCallback(val event)
{
    val target = event["target"];

    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate*>(target["data-context"].as<quintptr>());
    Q_ASSERT (wsp);

    wsp->setSocketState(QAbstractSocket::UnconnectedState);
    emit wsp->q_func()->disconnected();
}

static void q_onOpenCallback(val event)
{
    val target = event["target"];

    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate*>(target["data-context"].as<quintptr>());
    Q_ASSERT (wsp);

    wsp->setSocketState(QAbstractSocket::ConnectedState);
    emit wsp->q_func()->connected();
}

static void q_onIncomingMessageCallback(val event)
{
    val target = event["target"];

    if (event["data"].typeOf().as<std::string>() == "string") {
        QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate*>(target["data-context"].as<quintptr>());
        Q_ASSERT (wsp);

        const QString message = QString::fromStdString(event["data"].as<std::string>());
        if (!message.isEmpty())
            wsp->q_func()->textMessageReceived(message);
    } else {
        val reader = val::global("FileReader").new_();
        reader.set("onload", val::module_property("QWebSocketPrivate_readBlob"));
        reader.set("data-context", target["data-context"]);
        reader.call<void>("readAsArrayBuffer", event["data"]);
    }
}

static void q_readBlob(val event)
{
    val fileReader = event["target"];

    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate*>(fileReader["data-context"].as<quintptr>());
    Q_ASSERT (wsp);

    // Set up source typed array
    val result = fileReader["result"]; // ArrayBuffer
    val Uint8Array = val::global("Uint8Array");
    val sourceTypedArray = Uint8Array.new_(result);

    // Allocate and set up destination typed array
    const size_t size = result["byteLength"].as<size_t>();
    QByteArray buffer(size, Qt::Uninitialized);

    val destinationTypedArray = Uint8Array.new_(val::module_property("HEAPU8")["buffer"],
                                                reinterpret_cast<quintptr>(buffer.data()), size);
    destinationTypedArray.call<void>("set", sourceTypedArray);

    wsp->q_func()->binaryMessageReceived(buffer);
}


EMSCRIPTEN_BINDINGS(wasm_module) {
    function("QWebSocketPrivate_onErrorCallback", q_onErrorCallback);
    function("QWebSocketPrivate_onCloseCallback", q_onCloseCallback);
    function("QWebSocketPrivate_onOpenCallback", q_onOpenCallback);
    function("QWebSocketPrivate_onIncomingMessageCallback", q_onIncomingMessageCallback);
    function("QWebSocketPrivate_readBlob", q_readBlob);
}

qint64 QWebSocketPrivate::sendTextMessage(const QString &message)
{
    socketContext.call<void>("send", message.toStdString());
    return message.length();
}

qint64 QWebSocketPrivate::sendBinaryMessage(const QByteArray &data)
{
    socketContext.call<void>("send",
                             val(typed_memory_view(data.size(),
                                                   reinterpret_cast<const unsigned char *>
                                                   (data.constData()))));

    return data.length();
}

void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    Q_Q(QWebSocket);
    m_closeCode = closeCode;
    m_closeReason = reason;
    Q_EMIT q->aboutToClose();
    setSocketState(QAbstractSocket::ClosingState);

    socketContext.call<void>("close", static_cast<quint16>(closeCode),
                             reason.toLatin1().toStdString());
}

void QWebSocketPrivate::open(const QNetworkRequest &request, bool mask)
{
    Q_UNUSED(mask)
    Q_Q(QWebSocket);
    const QUrl url = request.url();
    if (!url.isValid() || url.toString().contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("Invalid URL."));
        Q_EMIT q->error(QAbstractSocket::ConnectionRefusedError);
        return;
    }

    setSocketState(QAbstractSocket::ConnectingState);
    const std::string urlbytes = url.toString().toStdString();

    // HTML WebSockets do not support arbitrary request headers, but
    // do support the WebSocket protocol header. This header is
    // required for some use cases like MQTT.
    const std::string protocolHeaderValue = request.rawHeader("Sec-WebSocket-Protocol").toStdString();
    val webSocket = val::global("WebSocket");

    socketContext = !protocolHeaderValue.empty()
            ? webSocket.new_(urlbytes, protocolHeaderValue)
            : webSocket.new_(urlbytes);

    socketContext.set("onerror", val::module_property("QWebSocketPrivate_onErrorCallback"));
    socketContext.set("onclose", val::module_property("QWebSocketPrivate_onCloseCallback"));
    socketContext.set("onopen", val::module_property("QWebSocketPrivate_onOpenCallback"));
    socketContext.set("onmessage", val::module_property("QWebSocketPrivate_onIncomingMessageCallback"));
    socketContext.set("data-context", val(quintptr(reinterpret_cast<void *>(this))));
}
