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

#include "profile_io_data_qt.h"

#include "base/task_scheduler/post_task.h"
#include "components/certificate_transparency/ct_known_logs.h"
#include "components/network_session_configurator/common/network_features.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/content_features.h"
#include "chrome/browser/custom_handlers/protocol_handler_registry_factory.h"
#include "chrome/browser/net/chrome_mojo_proxy_resolver_factory.h"
#include "chrome/common/chrome_switches.h"
#include "net/cert/cert_verifier.h"
#include "net/cert/ct_log_verifier.h"
#include "net/cert/ct_policy_enforcer.h"
#include "net/cert/multi_log_ct_verifier.h"
#include "net/extras/sqlite/sqlite_channel_id_store.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_auth_scheme.h"
#include "net/http/http_auth_preferences.h"
#include "net/http/http_cache.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/http_network_session.h"
#include "net/http/transport_security_persister.h"
#include "net/proxy_resolution/dhcp_pac_file_fetcher_factory.h"
#include "net/proxy_resolution/pac_file_fetcher_impl.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "net/proxy_resolution/proxy_resolution_service.h"
#include "net/ssl/channel_id_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "net/url_request/url_request_intercepting_job_factory.h"
#include "services/file/user_id_map.h"
#include "services/network/proxy_service_mojo.h"

#include "net/cookie_monster_delegate_qt.h"
#include "net/custom_protocol_handler.h"
#include "net/network_delegate_qt.h"
#include "net/proxy_config_service_qt.h"
#include "net/qrc_protocol_handler_qt.h"
#include "net/url_request_context_getter_qt.h"
#include "profile_qt.h"
#include "resource_context_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

static bool doNetworkSessionParamsMatch(const net::HttpNetworkSession::Params &first,
                                        const net::HttpNetworkSession::Params &second)
{
    if (first.ignore_certificate_errors != second.ignore_certificate_errors)
        return false;
    if (first.enable_channel_id != second.enable_channel_id)
        return false;
    if (first.enable_token_binding != second.enable_token_binding)
        return false;
    return true;
}

static bool doNetworkSessionContextMatch(const net::HttpNetworkSession::Context &first,
                                         const net::HttpNetworkSession::Context &second)
{
    if (first.transport_security_state != second.transport_security_state)
        return false;
    if (first.cert_verifier != second.cert_verifier)
        return false;
    if (first.channel_id_service != second.channel_id_service)
        return false;
    if (first.proxy_resolution_service != second.proxy_resolution_service)
        return false;
    if (first.ssl_config_service != second.ssl_config_service)
        return false;
    if (first.http_auth_handler_factory != second.http_auth_handler_factory)
        return false;
    if (first.http_server_properties != second.http_server_properties)
        return false;
    if (first.host_resolver != second.host_resolver)
        return false;
    if (first.cert_transparency_verifier != second.cert_transparency_verifier)
        return false;
    if (first.ct_policy_enforcer != second.ct_policy_enforcer)
        return false;
    return true;
}

static net::HttpNetworkSession::Context generateNetworkSessionContext(net::URLRequestContext *urlRequestContext)
{
    net::HttpNetworkSession::Context network_session_context;
    network_session_context.transport_security_state = urlRequestContext->transport_security_state();
    network_session_context.cert_verifier = urlRequestContext->cert_verifier();
    network_session_context.channel_id_service = urlRequestContext->channel_id_service();
    network_session_context.proxy_resolution_service = urlRequestContext->proxy_resolution_service();
    network_session_context.ssl_config_service = urlRequestContext->ssl_config_service();
    network_session_context.http_auth_handler_factory = urlRequestContext->http_auth_handler_factory();
    network_session_context.http_server_properties = urlRequestContext->http_server_properties();
    network_session_context.host_resolver = urlRequestContext->host_resolver();
    network_session_context.cert_transparency_verifier = urlRequestContext->cert_transparency_verifier();
    network_session_context.ct_policy_enforcer = urlRequestContext->ct_policy_enforcer();
    return network_session_context;
}

static net::HttpNetworkSession::Params generateNetworkSessionParams(bool ignoreCertificateErrors)
{
    net::HttpNetworkSession::Params network_session_params;
    network_session_params.ignore_certificate_errors = ignoreCertificateErrors;
    network_session_params.enable_token_binding = base::FeatureList::IsEnabled(features::kTokenBinding);
    network_session_params.enable_channel_id = base::FeatureList::IsEnabled(features::kChannelID);
    return network_session_params;
}

ProfileIODataQt::ProfileIODataQt(ProfileQt *profile)
    : m_profile(profile),
      m_mutex(QMutex::Recursive),
      m_weakPtrFactory(this)
{
    if (content::BrowserThread::IsThreadInitialized(content::BrowserThread::UI))
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

ProfileIODataQt::~ProfileIODataQt()
{
    if (content::BrowserThread::IsThreadInitialized(content::BrowserThread::IO))
        DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_urlRequestContext && m_urlRequestContext->proxy_resolution_service())
        m_urlRequestContext->proxy_resolution_service()->OnShutdown();
    m_resourceContext.reset();
    if (m_cookieDelegate)
        m_cookieDelegate->setCookieMonster(0); // this will let CookieMonsterDelegateQt be deleted
    m_networkDelegate.reset();
    delete m_proxyConfigService.fetchAndStoreAcquire(0);
}

void ProfileIODataQt::shutdownOnUIThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    bool posted = content::BrowserThread::DeleteSoon(content::BrowserThread::IO, FROM_HERE, this);
    if (!posted) {
        qWarning() << "Could not delete ProfileIODataQt on io thread !";
        delete this;
    }
}

net::URLRequestContext *ProfileIODataQt::urlRequestContext()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_initialized)
        initializeOnIOThread();
    return m_urlRequestContext.get();
}

content::ResourceContext *ProfileIODataQt::resourceContext()
{
    return m_resourceContext.get();
}

void ProfileIODataQt::initializeOnIOThread()
{
    m_networkDelegate.reset(new NetworkDelegateQt(this));
    m_hostResolver = net::HostResolver::CreateDefaultResolver(NULL);
    m_urlRequestContext.reset(new net::URLRequestContext());
    m_urlRequestContext->set_network_delegate(m_networkDelegate.get());
    m_urlRequestContext->set_enable_brotli(base::FeatureList::IsEnabled(features::kBrotliEncoding));
    m_urlRequestContext->set_host_resolver(m_hostResolver.get());
    // this binds factory to io thread
    m_weakPtr = m_weakPtrFactory.GetWeakPtr();
    QMutexLocker lock(&m_mutex);
    generateAllStorage();
    generateJobFactory();
    m_initialized = true;
}

void ProfileIODataQt::initializeOnUIThread()
{
    m_profileAdapter = m_profile->profileAdapter();
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_resourceContext.reset(new ResourceContextQt(this));
    ProtocolHandlerRegistry* protocolHandlerRegistry =
        ProtocolHandlerRegistryFactory::GetForBrowserContext(m_profile);
    DCHECK(protocolHandlerRegistry);
    m_protocolHandlerInterceptor =
        protocolHandlerRegistry->CreateJobInterceptorFactory();
    m_cookieDelegate = new CookieMonsterDelegateQt();
    m_cookieDelegate->setClient(m_profile->profileAdapter()->cookieStore());
    createProxyConfig();
}

void ProfileIODataQt::cancelAllUrlRequests()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);

    const std::set<const net::URLRequest*> *url_requests = m_urlRequestContext->url_requests();
    std::set<const net::URLRequest*>::const_iterator it = url_requests->begin();
    std::set<const net::URLRequest*>::const_iterator end = url_requests->end();
    for ( ; it != end; ++it) {
        net::URLRequest* request = const_cast<net::URLRequest*>(*it);
        if (request)
            request->Cancel();
    }
}

void ProfileIODataQt::generateAllStorage()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    QMutexLocker lock(&m_mutex);
    generateStorage();
    generateCookieStore();
    generateUserAgent();
    generateHttpCache();
    m_updateAllStorage = false;
}

void ProfileIODataQt::generateStorage()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);

    // We must stop all requests before deleting their backends.
    if (m_storage) {
        m_urlRequestContext->proxy_resolution_service()->OnShutdown();
        m_cookieDelegate->setCookieMonster(nullptr);
        m_storage->set_cookie_store(nullptr);
        cancelAllUrlRequests();
        // we need to get rid of dangling pointer due to coming storage deletion
        m_urlRequestContext->set_http_transaction_factory(nullptr);
        m_httpNetworkSession.reset();
        m_transportSecurityPersister.reset();
    }

    m_storage.reset(new net::URLRequestContextStorage(m_urlRequestContext.get()));

    net::ProxyConfigService *proxyConfigService = m_proxyConfigService.fetchAndStoreAcquire(0);
    Q_ASSERT(proxyConfigService);

    m_storage->set_cert_verifier(net::CertVerifier::CreateDefault());
    std::unique_ptr<net::MultiLogCTVerifier> ct_verifier(new net::MultiLogCTVerifier());
    std::vector<scoped_refptr<const net::CTLogVerifier>> ct_logs;
    for (const auto &ct_log : certificate_transparency::GetKnownLogs()) {
        scoped_refptr<const net::CTLogVerifier> log_verifier =
                net::CTLogVerifier::Create(std::string(ct_log.log_key, ct_log.log_key_length),
                                           ct_log.log_name,
                                           ct_log.log_dns_domain);
        if (!log_verifier)
            continue;
        ct_logs.push_back(std::move(log_verifier));
    }
    ct_verifier->AddLogs(ct_logs);
    m_storage->set_cert_transparency_verifier(std::move(ct_verifier));
    m_storage->set_ct_policy_enforcer(base::WrapUnique(new net::DefaultCTPolicyEnforcer()));
    m_storage->set_ssl_config_service(std::make_unique<net::SSLConfigServiceDefaults>());
    if (!m_httpAuthPreferences) {
        m_httpAuthPreferences.reset(new net::HttpAuthPreferences());
        std::string serverWhitelist = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kAuthServerWhitelist);
        m_httpAuthPreferences->SetServerWhitelist(serverWhitelist);
    }
    m_storage->set_http_auth_handler_factory(net::HttpAuthHandlerFactory::CreateDefault(
                                                 m_urlRequestContext->host_resolver(),
                                                 m_httpAuthPreferences.get()));
    m_storage->set_transport_security_state(std::make_unique<net::TransportSecurityState>());

    if (!m_dataPath.isEmpty()) {
        scoped_refptr<base::SequencedTaskRunner> background_task_runner(
                    base::CreateSequencedTaskRunnerWithTraits(
        {base::MayBlock(),
         base::TaskPriority::BACKGROUND,
         base::TaskShutdownBehavior::BLOCK_SHUTDOWN}));
        m_transportSecurityPersister =
                std::make_unique<net::TransportSecurityPersister>(
                    m_urlRequestContext->transport_security_state(),
                    toFilePath(m_dataPath),
                    background_task_runner);
    };

    m_storage->set_http_server_properties(std::unique_ptr<net::HttpServerProperties>(
                                              new net::HttpServerPropertiesImpl));

    // The System Proxy Resolver has issues on Windows with unconfigured network cards,
    // which is why we want to use the v8 one
    if (!m_dhcpPacFileFetcherFactory)
        m_dhcpPacFileFetcherFactory.reset(new net::DhcpPacFileFetcherFactory);

    proxy_resolver::mojom::ProxyResolverFactoryPtr proxyResolver(std::move(m_proxyResolverFactoryInterface));
    m_storage->set_proxy_resolution_service(network::CreateProxyResolutionServiceUsingMojoFactory(
                                                std::move(proxyResolver),
                                                std::unique_ptr<net::ProxyConfigService>(proxyConfigService),
                                                net::PacFileFetcherImpl::CreateWithFileUrlSupport(m_urlRequestContext.get()),
                                                m_dhcpPacFileFetcherFactory->Create(m_urlRequestContext.get()),
                                                m_urlRequestContext->host_resolver(),
                                                nullptr /* NetLog */,
                                                m_urlRequestContext->network_delegate()));
}


void ProfileIODataQt::generateCookieStore()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);

    QMutexLocker lock(&m_mutex);

    scoped_refptr<net::SQLiteChannelIDStore> channel_id_db;
    if (!m_channelIdPath.isEmpty() && m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies) {
        channel_id_db = new net::SQLiteChannelIDStore(
                toFilePath(m_channelIdPath),
                base::CreateSequencedTaskRunnerWithTraits(
                                {base::MayBlock(), base::TaskPriority::BACKGROUND}));
    }

    m_storage->set_channel_id_service(
            base::WrapUnique(new net::ChannelIDService(
                    new net::DefaultChannelIDStore(channel_id_db.get()))));

    std::unique_ptr<net::CookieStore> cookieStore;
    switch (m_persistentCookiesPolicy) {
    case ProfileAdapter::NoPersistentCookies:
        cookieStore = content::CreateCookieStore(
            content::CookieStoreConfig(
                base::FilePath(),
                false,
                false,
                nullptr)
        );
        break;
    case ProfileAdapter::AllowPersistentCookies:
        cookieStore = content::CreateCookieStore(
            content::CookieStoreConfig(
                toFilePath(m_cookiesPath),
                false,
                true,
                nullptr)
            );
        break;
    case ProfileAdapter::ForcePersistentCookies:
        cookieStore = content::CreateCookieStore(
            content::CookieStoreConfig(
                toFilePath(m_cookiesPath),
                true,
                true,
                nullptr)
            );
        break;
    }

    net::CookieMonster * const cookieMonster = static_cast<net::CookieMonster*>(cookieStore.get());
    cookieStore->SetChannelIDServiceID(m_urlRequestContext->channel_id_service()->GetUniqueID());
    m_cookieDelegate->setCookieMonster(cookieMonster);
    m_storage->set_cookie_store(std::move(cookieStore));

    const std::vector<std::string> cookieableSchemes(kCookieableSchemes,
                                                     kCookieableSchemes + arraysize(kCookieableSchemes));
    cookieMonster->SetCookieableSchemes(cookieableSchemes);
}

void ProfileIODataQt::generateUserAgent()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(m_storage);

    QMutexLocker lock(&m_mutex);
    m_storage->set_http_user_agent_settings(std::unique_ptr<net::HttpUserAgentSettings>(
        new net::StaticHttpUserAgentSettings(m_httpAcceptLanguage.toStdString(),
                                             m_httpUserAgent.toStdString())));
}

void ProfileIODataQt::generateHttpCache()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(m_storage);

    QMutexLocker lock(&m_mutex);

    net::HttpCache::DefaultBackend* main_backend = 0;
    switch (m_httpCacheType) {
    case ProfileAdapter::MemoryHttpCache:
        main_backend =
            new net::HttpCache::DefaultBackend(
                net::MEMORY_CACHE,
                net::CACHE_BACKEND_DEFAULT,
                base::FilePath(),
                m_httpCacheMaxSize
            );
        break;
    case ProfileAdapter::DiskHttpCache:
        main_backend =
            new net::HttpCache::DefaultBackend(
                net::DISK_CACHE,
                net::CACHE_BACKEND_DEFAULT,
                toFilePath(m_httpCachePath),
                m_httpCacheMaxSize
            );
        break;
    case ProfileAdapter::NoCache:
        // It's safe to not create BackendFactory.
        break;
    }

    net::HttpCache *cache = 0;
    net::HttpNetworkSession::Context network_session_context =
            generateNetworkSessionContext(m_urlRequestContext.get());
    net::HttpNetworkSession::Params network_session_params =
            generateNetworkSessionParams(m_ignoreCertificateErrors);

    if (!m_httpNetworkSession
            || !doNetworkSessionParamsMatch(network_session_params, m_httpNetworkSession->params())
            || !doNetworkSessionContextMatch(network_session_context, m_httpNetworkSession->context())) {
        cancelAllUrlRequests();
        m_httpNetworkSession.reset(new net::HttpNetworkSession(network_session_params,
                                                               network_session_context));
    }

    cache = new net::HttpCache(m_httpNetworkSession.get(),
                               std::unique_ptr<net::HttpCache::DefaultBackend>(main_backend), false);

    m_storage->set_http_transaction_factory(std::unique_ptr<net::HttpCache>(cache));
}

void ProfileIODataQt::generateJobFactory()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(!m_jobFactory);

    QMutexLocker lock(&m_mutex);
    m_updateJobFactory = false;

    std::unique_ptr<net::URLRequestJobFactoryImpl> jobFactory(new net::URLRequestJobFactoryImpl());
    for (auto &it : m_protocolHandlers)
        jobFactory->SetProtocolHandler(it.first, base::WrapUnique(it.second.release()));
    m_protocolHandlers.clear();

    jobFactory->SetProtocolHandler(url::kDataScheme,
                                   std::unique_ptr<net::URLRequestJobFactory::ProtocolHandler>(
                                       new net::DataProtocolHandler()));
    scoped_refptr<base::TaskRunner> taskRunner(base::CreateTaskRunnerWithTraits({base::MayBlock(),
                                      base::TaskPriority::BACKGROUND,
                                      base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN}));
    jobFactory->SetProtocolHandler(url::kFileScheme,
                                   std::make_unique<net::FileProtocolHandler>(taskRunner));
    jobFactory->SetProtocolHandler(kQrcSchemeQt,
                                   std::unique_ptr<net::URLRequestJobFactory::ProtocolHandler>(
                                       new QrcProtocolHandlerQt()));
    jobFactory->SetProtocolHandler(url::kFtpScheme,
            net::FtpProtocolHandler::Create(m_urlRequestContext->host_resolver()));

    m_installedCustomSchemes = m_customUrlSchemes;
    for (const QByteArray &scheme : qAsConst(m_installedCustomSchemes)) {
        jobFactory->SetProtocolHandler(scheme.toStdString(),
                                       std::unique_ptr<net::URLRequestJobFactory::ProtocolHandler>(
                                           new CustomProtocolHandler(m_profileAdapter)));
    }

    m_baseJobFactory = jobFactory.get();

    // Set up interceptors in the reverse order.
    std::unique_ptr<net::URLRequestJobFactory> topJobFactory = std::move(jobFactory);
    content::URLRequestInterceptorScopedVector::reverse_iterator i;
    for (i = m_requestInterceptors.rbegin(); i != m_requestInterceptors.rend(); ++i) {
        topJobFactory.reset(new net::URLRequestInterceptingJobFactory(std::move(topJobFactory),
                                                                      std::move(*i)));
    }

    m_requestInterceptors.clear();

    if (m_protocolHandlerInterceptor) {
        m_protocolHandlerInterceptor->Chain(std::move(topJobFactory));
        topJobFactory = std::move(m_protocolHandlerInterceptor);
    }

    m_jobFactory = std::move(topJobFactory);

    m_urlRequestContext->set_job_factory(m_jobFactory.get());
}

void ProfileIODataQt::regenerateJobFactory()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(m_jobFactory);
    Q_ASSERT(m_baseJobFactory);

    QMutexLocker lock(&m_mutex);
    m_updateJobFactory = false;

    if (m_customUrlSchemes == m_installedCustomSchemes)
        return;

    for (const QByteArray &scheme : qAsConst(m_installedCustomSchemes)) {
        m_baseJobFactory->SetProtocolHandler(scheme.toStdString(), nullptr);
    }

    m_installedCustomSchemes = m_customUrlSchemes;
    for (const QByteArray &scheme : qAsConst(m_installedCustomSchemes)) {
        m_baseJobFactory->SetProtocolHandler(scheme.toStdString(),
                                             std::unique_ptr<net::URLRequestJobFactory::ProtocolHandler>(
                                                 new CustomProtocolHandler(m_profileAdapter)));
    }
}

void ProfileIODataQt::setRequestContextData(content::ProtocolHandlerMap *protocolHandlers,
                                            content::URLRequestInterceptorScopedVector request_interceptors)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    Q_ASSERT(!m_initialized);
    m_requestInterceptors = std::move(request_interceptors);
    std::swap(m_protocolHandlers, *protocolHandlers);
}

void ProfileIODataQt::setFullConfiguration()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    m_requestInterceptor = m_profileAdapter->requestInterceptor();
    m_persistentCookiesPolicy = m_profileAdapter->persistentCookiesPolicy();
    m_cookiesPath = m_profileAdapter->cookiesPath();
    m_channelIdPath = m_profileAdapter->channelIdPath();
    m_httpAcceptLanguage = m_profileAdapter->httpAcceptLanguage();
    m_httpUserAgent = m_profileAdapter->httpUserAgent();
    m_httpCacheType = m_profileAdapter->httpCacheType();
    m_httpCachePath = m_profileAdapter->httpCachePath();
    m_httpCacheMaxSize = m_profileAdapter->httpCacheMaxSize();
    m_customUrlSchemes = m_profileAdapter->customUrlSchemes();
    m_dataPath = m_profileAdapter->dataPath();
}

void ProfileIODataQt::requestStorageGeneration() {
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);
    if (m_initialized && !m_updateAllStorage) {
        m_updateAllStorage = true;
        createProxyConfig();
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                         base::Bind(&ProfileIODataQt::generateAllStorage, m_weakPtr));
    }
}

// TODO(miklocek): mojofy ProxyConfigServiceQt
void ProfileIODataQt::createProxyConfig()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);
    // We must create the proxy config service on the UI loop on Linux because it
    // must synchronously run on the glib message loop. This will be passed to
    // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
    Q_ASSERT(m_proxyConfigService == 0);
    m_proxyConfigService =
            new ProxyConfigServiceQt(
                net::ProxyResolutionService::CreateSystemProxyConfigService(
                    content::BrowserThread::GetTaskRunnerForThread(content::BrowserThread::IO)));
    //pass interface to io thread
    m_proxyResolverFactoryInterface = ChromeMojoProxyResolverFactory::CreateWithStrongBinding().PassInterface();
}

void ProfileIODataQt::updateStorageSettings()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    QMutexLocker lock(&m_mutex);
    setFullConfiguration();

    std::string userId = content::BrowserContext::GetServiceUserIdFor(m_profile);
    if (file::GetUserDirForUserId(userId) != toFilePath(m_profileAdapter->dataPath())) {
        file::ForgetServiceUserIdUserDirAssociation(userId);
        file::AssociateServiceUserIdWithUserDir(userId, toFilePath(m_profileAdapter->dataPath()));
    }
    requestStorageGeneration();
}

void ProfileIODataQt::updateCookieStore()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);
    m_persistentCookiesPolicy = m_profileAdapter->persistentCookiesPolicy();
    m_cookiesPath = m_profileAdapter->cookiesPath();
    m_channelIdPath = m_profileAdapter->channelIdPath();
    requestStorageGeneration();
}

void ProfileIODataQt::updateUserAgent()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);
    m_httpAcceptLanguage = m_profileAdapter->httpAcceptLanguage();
    m_httpUserAgent = m_profileAdapter->httpUserAgent();
    requestStorageGeneration();
}

void ProfileIODataQt::updateHttpCache()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);
    m_httpCacheType = m_profileAdapter->httpCacheType();
    m_httpCachePath = m_profileAdapter->httpCachePath();
    m_httpCacheMaxSize = m_profileAdapter->httpCacheMaxSize();

    if (m_httpCacheType == ProfileAdapter::NoCache) {
        content::BrowsingDataRemover *remover =
                content::BrowserContext::GetBrowsingDataRemover(m_profileAdapter->profile());
        remover->Remove(base::Time(), base::Time::Max(),
            content::BrowsingDataRemover::DATA_TYPE_CACHE,
            content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB |
                        content::BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB);
    }
    requestStorageGeneration();
}

void ProfileIODataQt::updateJobFactory()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);

    m_customUrlSchemes = m_profileAdapter->customUrlSchemes();

    if (m_initialized && !m_updateJobFactory) {
        m_updateJobFactory = true;
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                         base::Bind(&ProfileIODataQt::regenerateJobFactory, m_weakPtr));
    }
}

void ProfileIODataQt::updateRequestInterceptor()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    QMutexLocker lock(&m_mutex);
    m_requestInterceptor = m_profileAdapter->requestInterceptor();
    // We in this case do not need to regenerate any Chromium classes.
}

QWebEngineUrlRequestInterceptor *ProfileIODataQt::acquireInterceptor()
{
    m_mutex.lock();
    return m_requestInterceptor;
}

void ProfileIODataQt::releaseInterceptor()
{
    m_mutex.unlock();
}

bool ProfileIODataQt::canSetCookie(const QUrl &firstPartyUrl, const QByteArray &cookieLine, const QUrl &url) const
{
    return m_cookieDelegate->canSetCookie(firstPartyUrl,cookieLine, url);
}

bool ProfileIODataQt::canGetCookies(const QUrl &firstPartyUrl, const QUrl &url) const
{
    return m_cookieDelegate->canGetCookies(firstPartyUrl, url);
}

} // namespace QtWebEngineCore
