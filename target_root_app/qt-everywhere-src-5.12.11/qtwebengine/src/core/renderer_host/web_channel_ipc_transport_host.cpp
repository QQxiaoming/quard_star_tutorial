/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "web_channel_ipc_transport_host.h"

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"
#include "common/qt_messages.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <QtCore/private/qjson_p.h>

namespace QtWebEngineCore {

Q_LOGGING_CATEGORY(log, "qt.webengine.webchanneltransport");

inline QDebug operator<<(QDebug stream, content::RenderFrameHost *frame)
{
    return stream << "frame " << frame->GetRoutingID() << " in process " << frame->GetProcess()->GetID();
}

template <class T>
inline QDebug operator<<(QDebug stream, const base::Optional<T> &opt)
{
    if (opt)
        return stream << *opt;
    else
        return stream << "nullopt";
}

WebChannelIPCTransportHost::WebChannelIPCTransportHost(content::WebContents *contents, uint worldId, QObject *parent)
    : QWebChannelAbstractTransport(parent)
    , content::WebContentsObserver(contents)
    , m_worldId(worldId)
    , m_binding(contents, this)
{
    for (content::RenderFrameHost *frame : contents->GetAllFrames())
        setWorldId(frame, worldId);
}

WebChannelIPCTransportHost::~WebChannelIPCTransportHost()
{
    resetWorldId();
}

uint WebChannelIPCTransportHost::worldId() const
{
    return m_worldId;
}

void WebChannelIPCTransportHost::sendMessage(const QJsonObject &message)
{
    QJsonDocument doc(message);
    int size = 0;
    const char *rawData = doc.rawData(&size);
    content::RenderFrameHost *frame = web_contents()->GetMainFrame();
    qtwebchannel::mojom::WebChannelTransportRenderAssociatedPtr webChannelTransport;
    frame->GetRemoteAssociatedInterfaces()->GetInterface(&webChannelTransport);
    qCDebug(log).nospace() << "sending webchannel message to " << frame << ": " << doc;
    webChannelTransport->DispatchWebChannelMessage(std::vector<uint8_t>(rawData, rawData + size), m_worldId);
}

void WebChannelIPCTransportHost::setWorldId(uint32_t worldId)
{
    if (m_worldId == worldId)
        return;
    for (content::RenderFrameHost *frame : web_contents()->GetAllFrames())
        setWorldId(frame, worldId);
    m_worldId = worldId;
}

void WebChannelIPCTransportHost::setWorldId(content::RenderFrameHost *frame, uint32_t worldId)
{
    if (!frame->IsRenderFrameLive())
        return;
    qCDebug(log).nospace() << "sending setWorldId(" << worldId << ") message to " << frame;
    qtwebchannel::mojom::WebChannelTransportRenderAssociatedPtr webChannelTransport;
    frame->GetRemoteAssociatedInterfaces()->GetInterface(&webChannelTransport);
    webChannelTransport->SetWorldId(worldId);
}

void WebChannelIPCTransportHost::resetWorldId()
{
    for (content::RenderFrameHost *frame : web_contents()->GetAllFrames()) {
        if (!frame->IsRenderFrameLive())
            return;
        qtwebchannel::mojom::WebChannelTransportRenderAssociatedPtr webChannelTransport;
        frame->GetRemoteAssociatedInterfaces()->GetInterface(&webChannelTransport);
        webChannelTransport->ResetWorldId();
    }
}

void WebChannelIPCTransportHost::DispatchWebChannelMessage(const std::vector<uint8_t> &binaryJson)
{
    content::RenderFrameHost *frame = web_contents()->GetMainFrame();

    if (m_binding.GetCurrentTargetFrame() != frame) {
        return;
    }

    QJsonDocument doc;
    // QJsonDocument::fromRawData does not check the length before it starts
    // parsing the QJsonPrivate::Header and QJsonPrivate::Base structures.
    if (binaryJson.size() >= sizeof(QJsonPrivate::Header) + sizeof(QJsonPrivate::Base))
        doc = QJsonDocument::fromRawData(reinterpret_cast<const char *>(binaryJson.data()),
                                                                        binaryJson.size());

    if (!doc.isObject()) {
        qCCritical(log).nospace() << "received invalid webchannel message from " << frame;
        return;
    }

    qCDebug(log).nospace() << "received webchannel message from " << frame << ": " << doc;
    Q_EMIT messageReceived(doc.object(), this);
}

void WebChannelIPCTransportHost::RenderFrameCreated(content::RenderFrameHost *frame)
{
    setWorldId(frame, m_worldId);
}

} // namespace QtWebEngineCore
