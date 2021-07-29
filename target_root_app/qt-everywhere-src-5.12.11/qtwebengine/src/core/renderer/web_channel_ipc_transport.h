/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "content/public/renderer/render_frame_observer.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "mojo/public/cpp/bindings/associated_binding_set.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

#include <QtCore/qglobal.h>

namespace QtWebEngineCore {

class WebChannelIPCTransport: private content::RenderFrameObserver,
        public qtwebchannel::mojom::WebChannelTransportRender {
public:
    WebChannelIPCTransport(content::RenderFrame *);

private:
    // qtwebchannel::mojom::WebChannelTransportRender
    void SetWorldId(uint32_t worldId) override;
    void ResetWorldId() override;
    void DispatchWebChannelMessage(const std::vector<uint8_t>& binaryJson, uint32_t worldId) override;

    // RenderFrameObserver
    void WillReleaseScriptContext(v8::Local<v8::Context> context, int worldId) override;
    void DidClearWindowObject() override;
    void OnDestruct() override;
    void BindRequest(qtwebchannel::mojom::WebChannelTransportRenderAssociatedRequest request);

private:
    // The worldId from our WebChannelIPCTransportHost or empty when there is no
    // WebChannelIPCTransportHost.
    uint32_t m_worldId;
    bool m_worldInitialized;
    // True means it's currently OK to manipulate the frame's script context.
    bool m_canUseContext = false;
    mojo::AssociatedBindingSet<qtwebchannel::mojom::WebChannelTransportRender> m_binding;
};

} // namespace

#endif // WEB_CHANNEL_IPC_TRANSPORT
