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

#include "renderer/content_renderer_client_qt.h"

#include "common/qt_messages.h"
#include "renderer/content_settings_observer_qt.h"
#include "base/strings/string_split.h"
#if QT_CONFIG(webengine_spellchecker)
#include "components/spellcheck/renderer/spellcheck.h"
#include "components/spellcheck/renderer/spellcheck_provider.h"
#endif
#include "components/cdm/renderer/external_clear_key_key_system_properties.h"
#include "components/cdm/renderer/widevine_key_system_properties.h"
#include "components/error_page/common/error.h"
#include "components/error_page/common/error_page_params.h"
#include "components/error_page/common/localized_error.h"
#include "components/network_hints/renderer/prescient_networking_dispatcher.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "components/printing/renderer/print_render_frame_helper.h"
#endif
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "components/web_cache/renderer/web_cache_impl.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/child/child_thread.h"
#include "content/public/common/service_manager_connection.h"
#include "content/public/common/simple_connection_filter.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "media/base/key_system_properties.h"
#include "media/media_buildflags.h"
#include "net/base/net_errors.h"
#include "services/service_manager/public/cpp/service_context.h"
#include "third_party/blink/public/platform/web_url_error.h"
#include "third_party/blink/public/platform/web_url_request.h"
#include "third_party/blink/public/web/web_security_policy.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "content/public/common/web_preferences.h"

#if QT_CONFIG(webengine_printing_and_pdf)
#include "renderer/print_web_view_helper_delegate_qt.h"
#endif

#include "renderer/render_frame_observer_qt.h"
#include "renderer/render_view_observer_qt.h"
#include "renderer/user_resource_controller.h"
#if QT_CONFIG(webengine_webchannel)
#include "renderer/web_channel_ipc_transport.h"
#endif
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/connector.h"

#include "components/grit/components_resources.h"

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
#include "base/feature_list.h"
#include "content/public/renderer/key_system_support.h"
#include "media/base/media_switches.h"
#include "media/base/video_codecs.h"
#include "third_party/widevine/cdm/widevine_cdm_common.h"

#include "widevine_cdm_version.h" // In SHARED_INTERMEDIATE_DIR.
#endif

namespace QtWebEngineCore {

static const char kHttpErrorDomain[] = "http";

ContentRendererClientQt::ContentRendererClientQt()
{
}

ContentRendererClientQt::~ContentRendererClientQt()
{
}

void ContentRendererClientQt::RenderThreadStarted()
{
    content::RenderThread *renderThread = content::RenderThread::Get();
    (void)GetConnector();
    m_visitedLinkSlave.reset(new visitedlink::VisitedLinkSlave);
    m_webCacheImpl.reset(new web_cache::WebCacheImpl());

    m_prescientNetworkingDispatcher.reset(new network_hints::PrescientNetworkingDispatcher());

    auto registry = std::make_unique<service_manager::BinderRegistry>();
    registry->AddInterface(m_visitedLinkSlave->GetBindCallback(),
                           base::ThreadTaskRunnerHandle::Get());
    content::ChildThread::Get()->GetServiceManagerConnection()->AddConnectionFilter(
                std::make_unique<content::SimpleConnectionFilter>(std::move(registry)));

    renderThread->AddObserver(UserResourceController::instance());

#if QT_CONFIG(webengine_spellchecker)
    if (!m_spellCheck)
        InitSpellCheck();
#endif

    // Allow XMLHttpRequests from qrc to file.
    blink::WebURL qrc(blink::KURL("qrc:"));
    blink::WebString file(blink::WebString::FromASCII("file"));
    blink::WebSecurityPolicy::AddOriginAccessWhitelistEntry(qrc, file, blink::WebString(), true);
}

void ContentRendererClientQt::RenderViewCreated(content::RenderView* render_view)
{
    // RenderViewObservers destroy themselves with their RenderView.
    new RenderViewObserverQt(render_view, m_webCacheImpl.data());
    UserResourceController::instance()->renderViewCreated(render_view);
}

void ContentRendererClientQt::RenderFrameCreated(content::RenderFrame* render_frame)
{
    new QtWebEngineCore::RenderFrameObserverQt(render_frame);
#if QT_CONFIG(webengine_webchannel)
    if (render_frame->IsMainFrame())
        new WebChannelIPCTransport(render_frame);
#endif
    UserResourceController::instance()->renderFrameCreated(render_frame);

    new QtWebEngineCore::ContentSettingsObserverQt(render_frame);

#if QT_CONFIG(webengine_spellchecker)
    new SpellCheckProvider(render_frame, m_spellCheck.data(), this);
#endif
#if QT_CONFIG(webengine_printing_and_pdf)
    new printing::PrintRenderFrameHelper(
                render_frame,
                base::WrapUnique(new PrintWebViewHelperDelegateQt()));
#endif // QT_CONFIG(webengine_printing_and_pdf)
}

void ContentRendererClientQt::RunScriptsAtDocumentEnd(content::RenderFrame* render_frame)
{
    // Check whether the render_frame has been created and has not been detached yet.
    // Otherwise the WebFrame is not available.
    RenderFrameObserverQt *render_frame_observer = RenderFrameObserverQt::Get(render_frame);
    if (!render_frame_observer || render_frame_observer->isFrameDetached())
        return; // The frame is invisible to scripts.

    UserResourceController::instance()->RunScriptsAtDocumentEnd(render_frame);
}

bool ContentRendererClientQt::HasErrorPage(int httpStatusCode)
{
    // Use an internal error page, if we have one for the status code.
    if (!error_page::LocalizedError::HasStrings(error_page::Error::kHttpErrorDomain, httpStatusCode)) {
        return false;
    }

    return true;
}

bool ContentRendererClientQt::ShouldSuppressErrorPage(content::RenderFrame *frame, const GURL &)
{
    return !(frame->GetWebkitPreferences().enable_error_page);
}

// To tap into the chromium localized strings. Ripped from the chrome layer (highly simplified).
void ContentRendererClientQt::PrepareErrorPage(content::RenderFrame* renderFrame, const blink::WebURLRequest &failedRequest,
                                               const blink::WebURLError &web_error,
                                               std::string *errorHtml, base::string16 *errorDescription)
{
    GetNavigationErrorStringsInternal(renderFrame, failedRequest,
                                      error_page::Error::NetError(web_error.url(), web_error.reason(), web_error.has_copy_in_cache()),
                                      errorHtml, errorDescription);
}

void ContentRendererClientQt::PrepareErrorPageForHttpStatusError(content::RenderFrame* renderFrame, const blink::WebURLRequest& failedRequest,
                                                                 const GURL& unreachable_url, int http_status,
                                                                 std::string* errorHtml, base::string16* errorDescription)
{
    GetNavigationErrorStringsInternal(renderFrame, failedRequest,
                                      error_page::Error::HttpError(unreachable_url, http_status),
                                      errorHtml, errorDescription);
}

void ContentRendererClientQt::GetNavigationErrorStringsInternal(content::RenderFrame *renderFrame, const blink::WebURLRequest &failedRequest, const error_page::Error &error, std::string *errorHtml, base::string16 *errorDescription)
{
    Q_UNUSED(renderFrame)
    const bool isPost = QByteArray::fromStdString(failedRequest.HttpMethod().Utf8()) == QByteArrayLiteral("POST");

    if (errorHtml) {
        // Use a local error page.
        int resourceId;
        base::DictionaryValue errorStrings;

        const std::string locale = content::RenderThread::Get()->GetLocale();
        // TODO(elproxy): We could potentially get better diagnostics here by first calling
        // NetErrorHelper::GetErrorStringsForDnsProbe, but that one is harder to untangle.

        error_page::LocalizedError::GetStrings(
            error.reason(), error.domain(), error.url(), isPost,
            error.stale_copy_in_cache(), false, false,
            locale, std::unique_ptr<error_page::ErrorPageParams>(), &errorStrings);
        resourceId = IDR_NET_ERROR_HTML;

        const base::StringPiece template_html(ui::ResourceBundle::GetSharedInstance().GetRawDataResource(resourceId));
        if (template_html.empty())
            NOTREACHED() << "unable to load template. ID: " << resourceId;
        else // "t" is the id of the templates root node.
            *errorHtml = webui::GetTemplatesHtml(template_html, &errorStrings, "t");
    }

    if (errorDescription)
        *errorDescription = error_page::LocalizedError::GetErrorDetails(error.domain(), error.reason(), isPost);
}

unsigned long long ContentRendererClientQt::VisitedLinkHash(const char *canonicalUrl, size_t length)
{
    return m_visitedLinkSlave->ComputeURLFingerprint(canonicalUrl, length);
}

bool ContentRendererClientQt::IsLinkVisited(unsigned long long linkHash)
{
    return m_visitedLinkSlave->IsVisited(linkHash);
}

blink::WebPrescientNetworking *ContentRendererClientQt::GetPrescientNetworking()
{
    return m_prescientNetworkingDispatcher.get();
}

void ContentRendererClientQt::OnStart()
{
    context()->connector()->BindConnectorRequest(std::move(m_connectorRequest));
}

void ContentRendererClientQt::OnBindInterface(const service_manager::BindSourceInfo &remote_info,
                                              const std::string& name,
                                              mojo::ScopedMessagePipeHandle handle)
{
    Q_UNUSED(remote_info);
    m_registry.TryBindInterface(name, &handle);
}

void ContentRendererClientQt::GetInterface(const std::string &interface_name, mojo::ScopedMessagePipeHandle interface_pipe)
{
    if (!m_connector)
        return;
    m_connector->BindInterface(service_manager::Identity("qtwebengine"),
                               interface_name,
                               std::move(interface_pipe));
}

// The following is based on chrome/renderer/media/chrome_key_systems.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
// External Clear Key (used for testing).
static void AddExternalClearKey(std::vector<std::unique_ptr<media::KeySystemProperties>>* concrete_key_systems)
{
    // TODO(xhwang): Move these into an array so we can use a for loop to add
    // supported key systems below.
    static const char kExternalClearKeyKeySystem[] =
            "org.chromium.externalclearkey";
    static const char kExternalClearKeyDecryptOnlyKeySystem[] =
            "org.chromium.externalclearkey.decryptonly";
    static const char kExternalClearKeyMessageTypeTestKeySystem[] =
            "org.chromium.externalclearkey.messagetypetest";
    static const char kExternalClearKeyFileIOTestKeySystem[] =
            "org.chromium.externalclearkey.fileiotest";
    static const char kExternalClearKeyOutputProtectionTestKeySystem[] =
            "org.chromium.externalclearkey.outputprotectiontest";
    static const char kExternalClearKeyPlatformVerificationTestKeySystem[] =
            "org.chromium.externalclearkey.platformverificationtest";
    static const char kExternalClearKeyInitializeFailKeySystem[] =
            "org.chromium.externalclearkey.initializefail";
    static const char kExternalClearKeyCrashKeySystem[] =
            "org.chromium.externalclearkey.crash";
    static const char kExternalClearKeyVerifyCdmHostTestKeySystem[] =
            "org.chromium.externalclearkey.verifycdmhosttest";
    static const char kExternalClearKeyStorageIdTestKeySystem[] =
            "org.chromium.externalclearkey.storageidtest";
    static const char kExternalClearKeyDifferentGuidTestKeySystem[] =
            "org.chromium.externalclearkey.differentguid";
    static const char kExternalClearKeyCdmProxyTestKeySystem[] =
            "org.chromium.externalclearkey.cdmproxytest";

    media::mojom::KeySystemCapabilityPtr capability;
    if (!content::IsKeySystemSupported(kExternalClearKeyKeySystem, &capability)) {
        DVLOG(1) << "External Clear Key not supported";
        return;
    }

    concrete_key_systems->emplace_back(
                new cdm::ExternalClearKeyProperties(kExternalClearKeyKeySystem));

    // Add support of decrypt-only mode in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyDecryptOnlyKeySystem));

    // A key system that triggers various types of messages in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyMessageTypeTestKeySystem));

    // A key system that triggers the FileIO test in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyFileIOTestKeySystem));

    // A key system that triggers the output protection test in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyOutputProtectionTestKeySystem));

    // A key system that triggers the platform verification test in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyPlatformVerificationTestKeySystem));

    // A key system that Chrome thinks is supported by ClearKeyCdm, but actually
    // will be refused by ClearKeyCdm. This is to test the CDM initialization
    // failure case.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyInitializeFailKeySystem));

    // A key system that triggers a crash in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyCrashKeySystem));

    // A key system that triggers the verify host files test in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyVerifyCdmHostTestKeySystem));

    // A key system that fetches the Storage ID in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyStorageIdTestKeySystem));

    // A key system that is registered with a different CDM GUID.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyDifferentGuidTestKeySystem));

    // A key system that triggers CDM Proxy test in ClearKeyCdm.
    concrete_key_systems->emplace_back(new cdm::ExternalClearKeyProperties(
                                           kExternalClearKeyCdmProxyTestKeySystem));
}

#if defined(WIDEVINE_CDM_AVAILABLE)
static media::SupportedCodecs GetSupportedCodecs(const std::vector<media::VideoCodec> &supported_video_codecs, bool is_secure)
{
    media::SupportedCodecs supported_codecs = media::EME_CODEC_NONE;

    // Audio codecs are always supported because the CDM only does decrypt-only
    // for audio. The only exception is when |is_secure| is true and there's no
    // secure video decoder available, which is a signal that secure hardware
    // decryption is not available either.
    // TODO(sandersd): Distinguish these from those that are directly supported,
    // as those may offer a higher level of protection.
    if (!supported_video_codecs.empty() || !is_secure) {
        supported_codecs |= media::EME_CODEC_WEBM_OPUS;
        supported_codecs |= media::EME_CODEC_WEBM_VORBIS;
        supported_codecs |= media::EME_CODEC_MP4_FLAC;
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
        supported_codecs |= media::EME_CODEC_MP4_AAC;
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)
    }

    // Video codecs are determined by what was registered for the CDM.
    for (const auto &codec : supported_video_codecs) {
        switch (codec) {
        case media::VideoCodec::kCodecVP8:
            supported_codecs |= media::EME_CODEC_WEBM_VP8;
            break;
        case media::VideoCodec::kCodecVP9:
            supported_codecs |= media::EME_CODEC_WEBM_VP9;
            supported_codecs |= media::EME_CODEC_COMMON_VP9;
            break;
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
        case media::VideoCodec::kCodecH264:
            supported_codecs |= media::EME_CODEC_MP4_AVC1;
            break;
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)
        default:
            DVLOG(1) << "Unexpected supported codec: " << GetCodecName(codec);
            break;
        }
    }

    return supported_codecs;
}

static void AddWidevine(std::vector<std::unique_ptr<media::KeySystemProperties>> *concrete_key_systems)
{
    media::mojom::KeySystemCapabilityPtr capability;
    if (!content::IsKeySystemSupported(kWidevineKeySystem, &capability)) {
        DVLOG(1) << "Widevine CDM is not currently available.";
        return;
    }

    // Codecs and encryption schemes.
    auto codecs =
        GetSupportedCodecs(capability->video_codecs, /*is_secure=*/false);
    const auto& encryption_schemes = capability->encryption_schemes;
    auto hw_secure_codecs = GetSupportedCodecs(capability->hw_secure_video_codecs,
                                               /*is_secure=*/true);
    const auto& hw_secure_encryption_schemes =
        capability->hw_secure_encryption_schemes;

    // Robustness.
    using Robustness = cdm::WidevineKeySystemProperties::Robustness;
    auto max_audio_robustness = Robustness::SW_SECURE_CRYPTO;
    auto max_video_robustness = Robustness::SW_SECURE_DECODE;

    if (base::FeatureList::IsEnabled(media::kHardwareSecureDecryption)) {
        max_audio_robustness = Robustness::HW_SECURE_CRYPTO;
        max_video_robustness = Robustness::HW_SECURE_ALL;
    }

    // Session types.
    bool cdm_supports_temporary_session = base::ContainsValue(capability->session_types, media::CdmSessionType::kTemporary);
    if (!cdm_supports_temporary_session) {
        DVLOG(1) << "Temporary session must be supported.";
        return;
    }

    auto persistent_license_support = media::EmeSessionTypeSupport::NOT_SUPPORTED;
    auto persistent_usage_record_support = media::EmeSessionTypeSupport::NOT_SUPPORTED;

    // Others.
    auto persistent_state_support = media::EmeFeatureSupport::REQUESTABLE;
    auto distinctive_identifier_support = media::EmeFeatureSupport::NOT_SUPPORTED;

    concrete_key_systems->emplace_back(new cdm::WidevineKeySystemProperties(
                                           codecs, encryption_schemes, hw_secure_codecs,
                                           hw_secure_encryption_schemes, max_audio_robustness, max_video_robustness,
                                           persistent_license_support, persistent_usage_record_support,
                                           persistent_state_support, distinctive_identifier_support));
}
#endif // defined(WIDEVINE_CDM_AVAILABLE)
#endif // BUILDFLAG(ENABLE_LIBRARY_CDMS)

void ContentRendererClientQt::AddSupportedKeySystems(std::vector<std::unique_ptr<media::KeySystemProperties>> *key_systems)
{
#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
    if (base::FeatureList::IsEnabled(media::kExternalClearKeyForTesting))
        AddExternalClearKey(key_systems);

#if defined(WIDEVINE_CDM_AVAILABLE)
    AddWidevine(key_systems);
#endif // defined(WIDEVINE_CDM_AVAILABLE)

#endif // BUILDFLAG(ENABLE_LIBRARY_CDMS)
}

#if QT_CONFIG(webengine_spellchecker)
void ContentRendererClientQt::InitSpellCheck()
{
    m_spellCheck.reset(new SpellCheck(&m_registry, this));
}
#endif

void ContentRendererClientQt::CreateRendererService(service_manager::mojom::ServiceRequest service_request)
{
    m_serviceContext = std::make_unique<service_manager::ServiceContext>(
                std::make_unique<service_manager::ForwardingService>(this),
                std::move(service_request));
}

service_manager::Connector* ContentRendererClientQt::GetConnector()
{
    if (!m_connector)
        m_connector = service_manager::Connector::Create(&m_connectorRequest);
    return m_connector.get();
}

} // namespace
