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

#ifndef WEB_CHANNEL_IPC_TRANSPORT_H
#define WEB_CHANNEL_IPC_TRANSPORT_H

#include "qtwebenginecoreglobal.h"

#include "content/public/browser/web_contents_observer.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "content/public/browser/web_contents_binding_set.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

#include <QWebChannelAbstractTransport>

QT_FORWARD_DECLARE_CLASS(QString)

namespace QtWebEngineCore {

class WebChannelIPCTransportHost : public QWebChannelAbstractTransport
        , private content::WebContentsObserver
        , qtwebchannel::mojom::WebChannelTransportHost {
public:
    WebChannelIPCTransportHost(content::WebContents *webContents, uint32_t worldId = 0, QObject *parent = nullptr);
    ~WebChannelIPCTransportHost() override;

    void setWorldId(uint32_t worldId);
    uint32_t worldId() const;

    // QWebChannelAbstractTransport
    void sendMessage(const QJsonObject &message) override;

private:
    void setWorldId(content::RenderFrameHost *frame, uint32_t worldId);
    void resetWorldId();
    void onWebChannelMessage(const std::vector<char> &message);

    // WebContentsObserver
    void RenderFrameCreated(content::RenderFrameHost *frame) override;

    // qtwebchannel::mojom::WebChannelTransportHost
    void DispatchWebChannelMessage(const std::vector<uint8_t> &binaryJson) override;

    // Empty only during construction/destruction. Synchronized to all the
    // WebChannelIPCTransports/RenderFrames in the observed WebContents.
    uint32_t m_worldId;
    content::WebContentsFrameBindingSet<qtwebchannel::mojom::WebChannelTransportHost> m_binding;
};

} // namespace

#endif // WEB_CHANNEL_IPC_TRANSPORT_H
