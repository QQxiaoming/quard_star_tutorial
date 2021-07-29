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
#ifndef CONTENT_RENDERER_CLIENT_QT_H
#define CONTENT_RENDERER_CLIENT_QT_H

#include "qtwebenginecoreglobal_p.h"
#include "content/public/renderer/content_renderer_client.h"
#include "components/spellcheck/spellcheck_buildflags.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/cpp/local_interface_provider.h"
#include "services/service_manager/public/cpp/service.h"

#include <QScopedPointer>

namespace error_page {
class Error;
}

namespace network_hints {
class PrescientNetworkingDispatcher;
}

namespace visitedlink {
class VisitedLinkSlave;
}

namespace web_cache {
class WebCacheImpl;
}

#if QT_CONFIG(webengine_spellchecker)
class SpellCheck;
#endif

namespace QtWebEngineCore {

class ContentRendererClientQt : public content::ContentRendererClient
                              , public service_manager::Service
                              , public service_manager::LocalInterfaceProvider
{
public:
    ContentRendererClientQt();
    ~ContentRendererClientQt();

    // content::ContentRendererClient:
    void RenderThreadStarted() override;
    void RenderViewCreated(content::RenderView *render_view) override;
    void RenderFrameCreated(content::RenderFrame* render_frame) override;
    bool ShouldSuppressErrorPage(content::RenderFrame *, const GURL &) override;
    bool HasErrorPage(int http_status_code) override;
    void PrepareErrorPage(content::RenderFrame* renderFrame, const blink::WebURLRequest& failedRequest,
                          const blink::WebURLError& error, std::string* errorHtml, base::string16* errorDescription) override;
    void PrepareErrorPageForHttpStatusError(content::RenderFrame* render_frame, const blink::WebURLRequest& failed_request,
                                            const GURL& unreachable_url, int http_status,
                                            std::string* error_html, base::string16* error_description) override;

    unsigned long long VisitedLinkHash(const char *canonicalUrl, size_t length) override;
    bool IsLinkVisited(unsigned long long linkHash) override;
    blink::WebPrescientNetworking* GetPrescientNetworking() override;
    void AddSupportedKeySystems(std::vector<std::unique_ptr<media::KeySystemProperties>>* key_systems) override;

    void RunScriptsAtDocumentEnd(content::RenderFrame* render_frame) override;

    void CreateRendererService(service_manager::mojom::ServiceRequest service_request) override;

private:
#if BUILDFLAG(ENABLE_SPELLCHECK)
    void InitSpellCheck();
#endif
    service_manager::Connector *GetConnector();

    // service_manager::Service:
    void OnStart() override;
    void OnBindInterface(const service_manager::BindSourceInfo &remote_info,
                         const std::string &name,
                         mojo::ScopedMessagePipeHandle handle) override;

    // service_manager::LocalInterfaceProvider:
    void GetInterface(const std::string& name, mojo::ScopedMessagePipeHandle request_handle) override;

    void GetNavigationErrorStringsInternal(content::RenderFrame* renderFrame, const blink::WebURLRequest& failedRequest,
                                           const error_page::Error& error, std::string* errorHtml, base::string16* errorDescription);

    QScopedPointer<visitedlink::VisitedLinkSlave> m_visitedLinkSlave;
    QScopedPointer<web_cache::WebCacheImpl> m_webCacheImpl;
#if QT_CONFIG(webengine_spellchecker)
    QScopedPointer<SpellCheck> m_spellCheck;
#endif

    std::unique_ptr<service_manager::Connector> m_connector;
    service_manager::mojom::ConnectorRequest m_connectorRequest;
    std::unique_ptr<service_manager::ServiceContext> m_serviceContext;
    service_manager::BinderRegistry m_registry;
    std::unique_ptr<network_hints::PrescientNetworkingDispatcher> m_prescientNetworkingDispatcher;

    DISALLOW_COPY_AND_ASSIGN(ContentRendererClientQt);
};

} // namespace

#endif // CONTENT_RENDERER_CLIENT_QT_H
