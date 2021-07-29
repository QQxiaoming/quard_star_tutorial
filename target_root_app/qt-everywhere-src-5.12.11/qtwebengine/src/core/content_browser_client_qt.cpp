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

#include "content_browser_client_qt.h"

#include "base/json/json_reader.h"
#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#if QT_CONFIG(webengine_spellchecker)
#include "chrome/browser/spellchecker/spell_check_host_chrome_impl.h"
#endif
#include "components/network_hints/browser/network_hints_message_filter.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/common/url_schemes.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/client_certificate_delegate.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/service_manager_connection.h"
#include "content/public/common/service_names.mojom.h"
#include "content/public/common/url_constants.h"
#include "media/media_buildflags.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "printing/buildflags/buildflags.h"
#include "net/ssl/client_cert_identity.h"
#include "services/proxy_resolver/proxy_resolver_service.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/cpp/service.h"
#include "services/service_manager/sandbox/switches.h"
#include "third_party/blink/public/platform/modules/insecure_input/insecure_input_service.mojom.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_switches.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "ui/gl/gpu_timing.h"
#include "url/url_util_qt.h"

#include "qtwebengine/grit/qt_webengine_resources.h"

#include "profile_adapter.h"
#include "browser_main_parts_qt.h"
#include "browser_message_filter_qt.h"
#include "certificate_error_controller.h"
#include "certificate_error_controller_p.h"
#include "client_cert_select_controller.h"
#include "devtools_manager_delegate_qt.h"
#include "login_delegate_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "net/network_delegate_qt.h"
#include "net/qrc_protocol_handler_qt.h"
#include "net/url_request_context_getter_qt.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "printing/printing_message_filter_qt.h"
#endif
#include "profile_qt.h"
#include "quota_permission_context_qt.h"
#include "renderer_host/user_resource_controller_host.h"
#include "service/service_qt.h"
#include "type_conversion.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_context.h"
#include "web_engine_library_info.h"

#if defined(Q_OS_LINUX)
#include "global_descriptors_qt.h"
#include "ui/base/resource/resource_bundle.h"
#endif

#if defined(USE_NSS_CERTS)
#include "net/ssl/client_cert_store_nss.h"
#endif

#if defined(OS_WIN)
#include "net/ssl/client_cert_store_win.h"
#endif

#if defined(OS_MACOSX)
#include "net/ssl/client_cert_store_mac.h"
#endif

#if QT_CONFIG(webengine_pepper_plugins)
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "renderer_host/pepper/pepper_host_factory_qt.h"
#endif

#if QT_CONFIG(webengine_geolocation)
#include "location_provider_qt.h"
#endif

#include <QGuiApplication>
#include <QLocale>
#ifndef QT_NO_OPENGL
# include <QOpenGLContext>
#endif
#include <qpa/qplatformnativeinterface.h>

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE

namespace QtWebEngineCore {

class QtShareGLContext : public gl::GLContext {
public:
    QtShareGLContext(QOpenGLContext *qtContext)
        : gl::GLContext(0)
        , m_handle(0)
    {
        QString platform = qApp->platformName().toLower();
        QPlatformNativeInterface *pni = QGuiApplication::platformNativeInterface();
        if (platform == QLatin1String("xcb")) {
            if (gl::GetGLImplementation() == gl::kGLImplementationEGLGLES2)
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
            else
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("glxcontext"), qtContext);
        } else if (platform == QLatin1String("cocoa"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("cglcontextobj"), qtContext);
        else if (platform == QLatin1String("qnx"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QLatin1String("eglfs") || platform == QLatin1String("wayland")
                 || platform == QLatin1String("wayland-egl"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QLatin1String("windows")) {
            if (gl::GetGLImplementation() == gl::kGLImplementationEGLGLES2)
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglContext"), qtContext);
            else
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("renderingcontext"), qtContext);
        } else {
            qFatal("%s platform not yet supported", platform.toLatin1().constData());
            // Add missing platforms once they work.
            Q_UNREACHABLE();
        }
    }

    void* GetHandle() override { return m_handle; }
    // Qt currently never creates contexts using robustness attributes.
    bool WasAllocatedUsingRobustnessExtension() override { return false; }

    // We don't care about the rest, this context shouldn't be used except for its handle.
    bool Initialize(gl::GLSurface *, const gl::GLContextAttribs &) override { Q_UNREACHABLE(); return false; }
    bool MakeCurrent(gl::GLSurface *) override { Q_UNREACHABLE(); return false; }
    void ReleaseCurrent(gl::GLSurface *) override { Q_UNREACHABLE(); }
    bool IsCurrent(gl::GLSurface *) override { Q_UNREACHABLE(); return false; }
    scoped_refptr<gl::GPUTimingClient> CreateGPUTimingClient() override
    {
        return nullptr;
    }
    const gfx::ExtensionSet& GetExtensions() override
    {
        static const gfx::ExtensionSet s_emptySet;
        return s_emptySet;
    }
    void ResetExtensions() override
    {
    }

private:
    void *m_handle;
};

class ShareGroupQtQuick : public gl::GLShareGroup {
public:
    gl::GLContext* GetContext() override { return m_shareContextQtQuick.get(); }
    void AboutToAddFirstContext() override;

private:
    scoped_refptr<QtShareGLContext> m_shareContextQtQuick;
};

void ShareGroupQtQuick::AboutToAddFirstContext()
{
#ifndef QT_NO_OPENGL
    // This currently has to be setup by ::main in all applications using QQuickWebEngineView with delegated rendering.
    QOpenGLContext *shareContext = qt_gl_global_share_context();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to call QtWebEngine::initialize() in your main() function.");
    }
    m_shareContextQtQuick = new QtShareGLContext(shareContext);
#endif
}

ContentBrowserClientQt::ContentBrowserClientQt()
    : m_browserMainParts(0)
{
}

ContentBrowserClientQt::~ContentBrowserClientQt()
{
}

content::BrowserMainParts *ContentBrowserClientQt::CreateBrowserMainParts(const content::MainFunctionParams&)
{
    m_browserMainParts = new BrowserMainPartsQt();
    return m_browserMainParts;
}

void ContentBrowserClientQt::RenderProcessWillLaunch(content::RenderProcessHost* host,
                                                     service_manager::mojom::ServiceRequest *service_request)
{
    const int id = host->GetID();
    Profile *profile = Profile::FromBrowserContext(host->GetBrowserContext());
    content::BrowserThread::PostTaskAndReplyWithResult(
            content::BrowserThread::IO, FROM_HERE,
            base::Bind(&net::URLRequestContextGetter::GetURLRequestContext, base::Unretained(profile->GetRequestContext())),
            base::Bind(&ContentBrowserClientQt::AddNetworkHintsMessageFilter, base::Unretained(this), id));

    // FIXME: Add a settings variable to enable/disable the file scheme.
    content::ChildProcessSecurityPolicy::GetInstance()->GrantRequestScheme(id, url::kFileScheme);
    static_cast<ProfileQt*>(host->GetBrowserContext())->m_profileAdapter->userResourceController()->renderProcessStartedWithHost(host);
    host->AddFilter(new BrowserMessageFilterQt(id, profile));
#if QT_CONFIG(webengine_printing_and_pdf)
    host->AddFilter(new PrintingMessageFilterQt(host->GetID()));
#endif

    service_manager::mojom::ServicePtr service;
    *service_request = mojo::MakeRequest(&service);
    service_manager::mojom::PIDReceiverPtr pid_receiver;
    service_manager::Identity renderer_identity = host->GetChildIdentity();
    ServiceQt::GetInstance()->connector()->StartService(
                service_manager::Identity("qtwebengine_renderer",
                                          renderer_identity.user_id(),
                                          renderer_identity.instance()),
                std::move(service), mojo::MakeRequest(&pid_receiver));
}

void ContentBrowserClientQt::ResourceDispatcherHostCreated()
{
    m_resourceDispatcherHostDelegate.reset(new content::ResourceDispatcherHostDelegate);
    content::ResourceDispatcherHost::Get()->SetDelegate(m_resourceDispatcherHostDelegate.get());
}

gl::GLShareGroup *ContentBrowserClientQt::GetInProcessGpuShareGroup()
{
    if (!m_shareGroupQtQuick.get())
        m_shareGroupQtQuick = new ShareGroupQtQuick;
    return m_shareGroupQtQuick.get();
}

content::MediaObserver *ContentBrowserClientQt::GetMediaObserver()
{
    return MediaCaptureDevicesDispatcher::GetInstance();
}

void ContentBrowserClientQt::OverrideWebkitPrefs(content::RenderViewHost *rvh, content::WebPreferences *web_prefs)
{
    if (content::WebContents *webContents = rvh->GetDelegate()->GetAsWebContents()) {
        WebContentsDelegateQt* delegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
        if (delegate)
            delegate->overrideWebPreferences(webContents, web_prefs);
    }
}

content::QuotaPermissionContext *ContentBrowserClientQt::CreateQuotaPermissionContext()
{
    return new QuotaPermissionContextQt;
}

void ContentBrowserClientQt::GetQuotaSettings(content::BrowserContext* context,
                                              content::StoragePartition* partition,
                                              storage::OptionalQuotaSettingsCallback callback)
{
    storage::GetNominalDynamicSettings(partition->GetPath(), context->IsOffTheRecord(), std::move(callback));
}

// Copied from chrome/browser/ssl/ssl_error_handler.cc:
static int IsCertErrorFatal(int cert_error)
{
    switch (cert_error) {
    case net::ERR_CERT_COMMON_NAME_INVALID:
    case net::ERR_CERT_DATE_INVALID:
    case net::ERR_CERT_AUTHORITY_INVALID:
    case net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
    case net::ERR_CERT_WEAK_KEY:
    case net::ERR_CERT_NAME_CONSTRAINT_VIOLATION:
    case net::ERR_CERT_VALIDITY_TOO_LONG:
    case net::ERR_CERTIFICATE_TRANSPARENCY_REQUIRED:
    case net::ERR_CERT_SYMANTEC_LEGACY:
        return false;
    case net::ERR_CERT_CONTAINS_ERRORS:
    case net::ERR_CERT_REVOKED:
    case net::ERR_CERT_INVALID:
    case net::ERR_SSL_WEAK_SERVER_EPHEMERAL_DH_KEY:
    case net::ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN:
        return true;
    default:
        NOTREACHED();
    }
    return true;
}

void ContentBrowserClientQt::AllowCertificateError(content::WebContents *webContents,
                                                   int cert_error,
                                                   const net::SSLInfo &ssl_info,
                                                   const GURL &request_url,
                                                   content::ResourceType resource_type,
                                                   bool strict_enforcement,
                                                   bool expired_previous_decision,
                                                   const base::Callback<void(content::CertificateRequestResultType)> &callback)
{
    WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());

    QSharedPointer<CertificateErrorController> errorController(
            new CertificateErrorController(
                    new CertificateErrorControllerPrivate(
                            cert_error,
                            ssl_info,
                            request_url,
                            resource_type,
                            IsCertErrorFatal(cert_error),
                            strict_enforcement,
                            callback)));
    contentsDelegate->allowCertificateError(errorController);
}

void ContentBrowserClientQt::SelectClientCertificate(content::WebContents *webContents,
                                                     net::SSLCertRequestInfo *certRequestInfo,
                                                     net::ClientCertIdentityList clientCerts,
                                                     std::unique_ptr<content::ClientCertificateDelegate> delegate)
{
    if (!clientCerts.empty()) {
        WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());

        QSharedPointer<ClientCertSelectController> certSelectController(
                new ClientCertSelectController(certRequestInfo, std::move(clientCerts), std::move(delegate)));

        contentsDelegate->selectClientCert(certSelectController);
    } else {
        delegate->ContinueWithCertificate(nullptr, nullptr);
    }
}

std::unique_ptr<net::ClientCertStore> ContentBrowserClientQt::CreateClientCertStore(content::ResourceContext *resource_context)
{
    if (!resource_context)
        return nullptr;
#if defined(USE_NSS_CERTS)
    // FIXME: Give it a proper callback for a password delegate.
    return std::unique_ptr<net::ClientCertStore>(
                new net::ClientCertStoreNSS(net::ClientCertStoreNSS::PasswordDelegateFactory()));
#elif defined(OS_WIN)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreWin());
#elif defined(OS_MACOSX)
    return std::unique_ptr<net::ClientCertStore>(new net::ClientCertStoreMac());
#else
    return nullptr;
#endif
}

std::string ContentBrowserClientQt::GetApplicationLocale()
{
    return WebEngineLibraryInfo::getApplicationLocale();
}

std::string ContentBrowserClientQt::GetAcceptLangs(content::BrowserContext *context)
{
    return static_cast<ProfileQt*>(context)->profileAdapter()->httpAcceptLanguage().toStdString();
}

void ContentBrowserClientQt::AppendExtraCommandLineSwitches(base::CommandLine* command_line, int child_process_id)
{
    Q_UNUSED(child_process_id);

    url::CustomScheme::SaveSchemes(command_line);

    std::string processType = command_line->GetSwitchValueASCII(switches::kProcessType);
    if (processType == service_manager::switches::kZygoteProcess)
        command_line->AppendSwitchASCII(switches::kLang, GetApplicationLocale());
}

void ContentBrowserClientQt::GetAdditionalWebUISchemes(std::vector<std::string>* additional_schemes)
{
    additional_schemes->push_back(content::kChromeDevToolsScheme);
}

#if defined(Q_OS_LINUX)
void ContentBrowserClientQt::GetAdditionalMappedFilesForChildProcess(const base::CommandLine& command_line, int child_process_id, content::PosixFileDescriptorInfo* mappings)
{
    const std::string &locale = GetApplicationLocale();
    const base::FilePath &locale_file_path = ui::ResourceBundle::GetSharedInstance().GetLocaleFilePath(locale, true);
    if (locale_file_path.empty())
        return;

    // Open pak file of the current locale in the Browser process and pass its file descriptor to the sandboxed
    // Renderer Process. FileDescriptorInfo is responsible for closing the file descriptor.
    int flags = base::File::FLAG_OPEN | base::File::FLAG_READ;
    base::File locale_file = base::File(locale_file_path, flags);
    mappings->Transfer(kWebEngineLocale, base::ScopedFD(locale_file.TakePlatformFile()));
}
#endif

#if QT_CONFIG(webengine_pepper_plugins)
void ContentBrowserClientQt::DidCreatePpapiPlugin(content::BrowserPpapiHost* browser_host)
{
    browser_host->GetPpapiHost()->AddHostFactoryFilter(
                base::WrapUnique(new QtWebEngineCore::PepperHostFactoryQt(browser_host)));
}
#endif

content::DevToolsManagerDelegate* ContentBrowserClientQt::GetDevToolsManagerDelegate()
{
    return new DevToolsManagerDelegateQt;
}

// This is a really complicated way of doing absolutely nothing, but Mojo demands it:
class ServiceDriver
        : public blink::mojom::InsecureInputService
        , public content::WebContentsUserData<ServiceDriver>
{
public:
    static void CreateForRenderFrameHost(content::RenderFrameHost *renderFrameHost)
    {
        content::WebContents* web_contents = content::WebContents::FromRenderFrameHost(renderFrameHost);
        if (!web_contents)
            return;
        CreateForWebContents(web_contents);

    }
    static ServiceDriver* FromRenderFrameHost(content::RenderFrameHost *renderFrameHost)
    {
        content::WebContents* web_contents = content::WebContents::FromRenderFrameHost(renderFrameHost);
        if (!web_contents)
            return nullptr;
        return FromWebContents(web_contents);
    }
    static void BindInsecureInputService(blink::mojom::InsecureInputServiceRequest request, content::RenderFrameHost *render_frame_host)
    {
        CreateForRenderFrameHost(render_frame_host);
        ServiceDriver *driver = FromRenderFrameHost(render_frame_host);

        if (driver)
            driver->BindInsecureInputServiceRequest(std::move(request));
    }
    void BindInsecureInputServiceRequest(blink::mojom::InsecureInputServiceRequest request)
    {
        m_insecureInputServiceBindings.AddBinding(this, std::move(request));
    }

    // blink::mojom::InsecureInputService:
    void PasswordFieldVisibleInInsecureContext() override
    { }
    void AllPasswordFieldsInInsecureContextInvisible() override
    { }
    void DidEditFieldInInsecureContext() override
    { }

private:
    explicit ServiceDriver(content::WebContents* /*web_contents*/) { }
    friend class content::WebContentsUserData<ServiceDriver>;
    mojo::BindingSet<blink::mojom::InsecureInputService> m_insecureInputServiceBindings;
};

void ContentBrowserClientQt::InitFrameInterfaces()
{
    m_frameInterfaces = std::make_unique<service_manager::BinderRegistry>();
    m_frameInterfacesParameterized = std::make_unique<service_manager::BinderRegistryWithArgs<content::RenderFrameHost*>>();
    m_frameInterfacesParameterized->AddInterface(base::Bind(&ServiceDriver::BindInsecureInputService));
}

void ContentBrowserClientQt::BindInterfaceRequestFromFrame(content::RenderFrameHost* render_frame_host,
                                                           const std::string& interface_name,
                                                           mojo::ScopedMessagePipeHandle interface_pipe)
{
    if (!m_frameInterfaces.get() && !m_frameInterfacesParameterized.get())
        InitFrameInterfaces();

    if (!m_frameInterfacesParameterized->TryBindInterface(interface_name, &interface_pipe, render_frame_host))
        m_frameInterfaces->TryBindInterface(interface_name, &interface_pipe);
}

void ContentBrowserClientQt::RegisterInProcessServices(StaticServiceMap* services, content::ServiceManagerConnection* connection)
{
    service_manager::EmbeddedServiceInfo info;
    info.factory = ServiceQt::GetInstance()->CreateServiceQtFactory();
    services->insert(std::make_pair("qtwebengine", info));
}

void ContentBrowserClientQt::RegisterOutOfProcessServices(content::ContentBrowserClient::OutOfProcessServiceMap *services)
{
    (*services)[proxy_resolver::mojom::kProxyResolverServiceName] =
            base::BindRepeating(&base::ASCIIToUTF16, "V8 Proxy Resolver");
}

std::unique_ptr<base::Value> ContentBrowserClientQt::GetServiceManifestOverlay(base::StringPiece name)
{
    ui::ResourceBundle &rb = ui::ResourceBundle::GetSharedInstance();
    int id = -1;
    if (name == content::mojom::kPackagedServicesServiceName)
        id = IDR_QTWEBENGINE_CONTENT_PACKAGED_SERVICES_MANIFEST_OVERLAY;
    else if (name == content::mojom::kRendererServiceName)
        id = IDR_QTWEBENGINE_CONTENT_RENDERER_MANIFEST_OVERLAY;
    else if (name == content::mojom::kBrowserServiceName)
        id = IDR_QTWEBENGINE_CONTENT_BROWSER_MANIFEST_OVERLAY;
    if (id == -1)
        return nullptr;

    base::StringPiece manifest_contents =
        rb.GetRawDataResourceForScale(id, ui::ScaleFactor::SCALE_FACTOR_NONE);
    return base::JSONReader::Read(manifest_contents);
}

std::vector<content::ContentBrowserClient::ServiceManifestInfo> ContentBrowserClientQt::GetExtraServiceManifests()
{
    return std::vector<content::ContentBrowserClient::ServiceManifestInfo>({
        {"qtwebengine_renderer", IDR_QTWEBENGINE_RENDERER_SERVICE_MANIFEST},
    });
}

bool ContentBrowserClientQt::CanCreateWindow(
    content::RenderFrameHost* opener,
    const GURL& opener_url,
    const GURL& opener_top_level_frame_url,
    const GURL& source_origin,
    content::mojom::WindowContainerType container_type,
    const GURL& target_url,
    const content::Referrer& referrer,
    const std::string& frame_name,
    WindowOpenDisposition disposition,
    const blink::mojom::WindowFeatures& features,
    bool user_gesture,
    bool opener_suppressed,
    bool* no_javascript_access) {

    Q_UNUSED(opener_url);
    Q_UNUSED(opener_top_level_frame_url);
    Q_UNUSED(source_origin);
    Q_UNUSED(container_type);
    Q_UNUSED(target_url);
    Q_UNUSED(referrer);
    Q_UNUSED(frame_name);
    Q_UNUSED(disposition);
    Q_UNUSED(features);
    Q_UNUSED(opener_suppressed);

    if (no_javascript_access)
        *no_javascript_access = false;

    content::WebContents* webContents = content::WebContents::FromRenderFrameHost(opener);

    WebEngineSettings *settings = nullptr;
    if (webContents) {
        WebContentsDelegateQt* delegate =
                static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
        if (delegate)
            settings = delegate->webEngineSettings();
    }

    return (settings && settings->getJavaScriptCanOpenWindowsAutomatically()) || user_gesture;
}

#if QT_CONFIG(webengine_geolocation)
std::unique_ptr<device::LocationProvider> ContentBrowserClientQt::OverrideSystemLocationProvider()
{
    return base::WrapUnique(new LocationProviderQt());
}
#endif

scoped_refptr<net::URLRequestContextGetter> GetSystemRequestContextOnUIThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    return scoped_refptr<net::URLRequestContextGetter>(
                ProfileAdapter::createDefaultProfileAdapter()->profile()->GetRequestContext());
}

void ContentBrowserClientQt::AddNetworkHintsMessageFilter(int render_process_id, net::URLRequestContext *context)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    content::RenderProcessHost* host = content::RenderProcessHost::FromID(render_process_id);
    if (!host)
        return;

    content::BrowserMessageFilter *network_hints_message_filter(
                new network_hints::NetworkHintsMessageFilter(context->host_resolver()));
    host->AddFilter(network_hints_message_filter);
}

bool ContentBrowserClientQt::AllowGetCookie(const GURL &url,
                                            const GURL &first_party,
                                            const net::CookieList & /*cookie_list*/,
                                            content::ResourceContext *context,
                                            int /*render_process_id*/,
                                            int /*render_frame_id*/)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(context->GetRequestContext()->network_delegate());
    return networkDelegate->canGetCookies(first_party, url);
}

bool ContentBrowserClientQt::AllowSetCookie(const GURL &url,
                                            const GURL &first_party,
                                            const net::CanonicalCookie& /*cookie*/,
                                            content::ResourceContext *context,
                                            int /*render_process_id*/,
                                            int /*render_frame_id*/)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(context->GetRequestContext()->network_delegate());
    return networkDelegate->canSetCookies(first_party, url, std::string());
}

bool ContentBrowserClientQt::AllowAppCache(const GURL &manifest_url,
                                           const GURL &first_party,
                                           content::ResourceContext *context)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(context->GetRequestContext()->network_delegate());
    return networkDelegate->canGetCookies(first_party, manifest_url);
}

bool ContentBrowserClientQt::AllowServiceWorker(const GURL &scope,
                                                const GURL &first_party,
                                                content::ResourceContext *context,
                                                const base::Callback<content::WebContents*(void)> &/*wc_getter*/)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    // FIXME: Chrome also checks if javascript is enabled here to check if has been disabled since the service worker
    // was started.
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(context->GetRequestContext()->network_delegate());
    return networkDelegate->canGetCookies(first_party, scope);
}

// We control worker access to FS and indexed-db using cookie permissions, this is mirroring Chromium's logic.
void ContentBrowserClientQt::AllowWorkerFileSystem(const GURL &url,
                                                   content::ResourceContext *context,
                                                   const std::vector<std::pair<int, int> > &/*render_frames*/,
                                                   base::Callback<void(bool)> callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(context->GetRequestContext()->network_delegate());
    callback.Run(networkDelegate->canSetCookies(url, url, std::string()));
}

bool ContentBrowserClientQt::AllowWorkerIndexedDB(const GURL &url,
                                                  const base::string16 &/*name*/,
                                                  content::ResourceContext *context,
                                                  const std::vector<std::pair<int, int> > &/*render_frames*/)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(context->GetRequestContext()->network_delegate());
    return networkDelegate->canSetCookies(url, url, std::string());
}

static void LaunchURL(const GURL& url,
                      const content::ResourceRequestInfo::WebContentsGetter& web_contents_getter,
                      ui::PageTransition page_transition, bool is_main_frame, bool has_user_gesture)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::WebContents* webContents = web_contents_getter.Run();
    if (!webContents)
        return;
    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
    contentsDelegate->launchExternalURL(toQt(url), page_transition, is_main_frame, has_user_gesture);
}


bool ContentBrowserClientQt::HandleExternalProtocol(
        const GURL &url,
        content::ResourceRequestInfo::WebContentsGetter web_contents_getter,
        int child_id,
        content::NavigationUIData *navigation_data,
        bool is_main_frame,
        ui::PageTransition page_transition,
        bool has_user_gesture)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_UNUSED(child_id);
    Q_UNUSED(navigation_data);

    content::BrowserThread::PostTask(
            content::BrowserThread::UI,
            FROM_HERE,
            base::BindOnce(&LaunchURL,
                           url,
                           web_contents_getter,
                           page_transition,
                           is_main_frame,
                           has_user_gesture));
    return true;
}

scoped_refptr<content::LoginDelegate> ContentBrowserClientQt::CreateLoginDelegate(
        net::AuthChallengeInfo *authInfo,
        content::ResourceRequestInfo::WebContentsGetter web_contents_getter,
        const content::GlobalRequestID &request_id,
        bool /*is_main_frame*/,
        const GURL &url,
        scoped_refptr<net::HttpResponseHeaders> response_headers,
        bool first_auth_attempt,
        LoginAuthRequiredCallback auth_required_callback)
{
    return base::MakeRefCounted<LoginDelegateQt>(authInfo, web_contents_getter, url, first_auth_attempt, std::move(auth_required_callback));
}

} // namespace QtWebEngineCore

DEFINE_WEB_CONTENTS_USER_DATA_KEY(QtWebEngineCore::ServiceDriver);
