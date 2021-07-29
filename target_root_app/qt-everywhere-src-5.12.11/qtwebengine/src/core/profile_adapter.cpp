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

#include "profile_adapter.h"

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/download_manager.h"

#include "api/qwebengineurlscheme.h"
#include "content_client_qt.h"
#include "download_manager_delegate_qt.h"
#include "net/url_request_context_getter_qt.h"
#include "permission_manager_qt.h"
#include "profile_qt.h"
#include "renderer_host/user_resource_controller_host.h"
#include "type_conversion.h"
#include "visited_links_manager_qt.h"
#include "web_engine_context.h"
#include "web_contents_adapter_client.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QStandardPaths>

namespace {
inline QString buildLocationFromStandardPath(const QString &standardPath, const QString &name) {
    QString location = standardPath;
    if (location.isEmpty())
        location = QDir::homePath() % QLatin1String("/.") % QCoreApplication::applicationName();

    location.append(QLatin1String("/QtWebEngine/") % name);
    return location;
}
}

namespace QtWebEngineCore {

ProfileAdapter::ProfileAdapter(const QString &storageName):
      m_name(storageName)
    , m_offTheRecord(storageName.isEmpty())
    , m_httpCacheType(DiskHttpCache)
    , m_persistentCookiesPolicy(AllowPersistentCookies)
    , m_visitedLinksPolicy(TrackVisitedLinksOnDisk)
    , m_httpCacheMaxSize(0)
{
    WebEngineContext::current()->addProfileAdapter(this);
    // creation of profile requires webengine context
    m_profile.reset(new ProfileQt(this));
    content::BrowserContext::Initialize(m_profile.data(), toFilePath(dataPath()));
    // fixme: this should not be here
    m_profile->m_profileIOData->initializeOnUIThread();
}

ProfileAdapter::~ProfileAdapter()
{
    while (!m_webContentsAdapterClients.isEmpty()) {
       m_webContentsAdapterClients.first()->releaseProfile();
    }
    WebEngineContext::current()->removeProfileAdapter(this);
    if (m_downloadManagerDelegate) {
        m_profile->GetDownloadManager(m_profile.data())->Shutdown();
        m_downloadManagerDelegate.reset();
    }
}

void ProfileAdapter::setStorageName(const QString &storageName)
{
    if (storageName == m_name)
        return;
    m_name = storageName;
    if (!m_offTheRecord) {
        if (m_profile->m_urlRequestContextGetter.get())
            m_profile->m_profileIOData->updateStorageSettings();
        if (m_visitedLinksManager)
            resetVisitedLinksManager();
    }
}

void ProfileAdapter::setOffTheRecord(bool offTheRecord)
{
    if (offTheRecord == m_offTheRecord)
        return;
    m_offTheRecord = offTheRecord;
    if (m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateStorageSettings();
    if (m_visitedLinksManager)
        resetVisitedLinksManager();
}

ProfileQt *ProfileAdapter::profile()
{
    return m_profile.data();
}

VisitedLinksManagerQt *ProfileAdapter::visitedLinksManager()
{
    if (!m_visitedLinksManager)
        resetVisitedLinksManager();
    return m_visitedLinksManager.data();
}

DownloadManagerDelegateQt *ProfileAdapter::downloadManagerDelegate()
{
    if (!m_downloadManagerDelegate)
        m_downloadManagerDelegate.reset(new DownloadManagerDelegateQt(this));
    return m_downloadManagerDelegate.data();
}

QWebEngineCookieStore *ProfileAdapter::cookieStore()
{
    if (!m_cookieStore)
        m_cookieStore.reset(new QWebEngineCookieStore);
    return m_cookieStore.data();
}

QWebEngineUrlRequestInterceptor *ProfileAdapter::requestInterceptor()
{
    return m_requestInterceptor.data();
}

void ProfileAdapter::setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    if (m_requestInterceptor == interceptor)
        return;
    m_requestInterceptor = interceptor;
    if (m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateRequestInterceptor();
}

void ProfileAdapter::addClient(ProfileAdapterClient *adapterClient)
{
    m_clients.append(adapterClient);
}

void ProfileAdapter::removeClient(ProfileAdapterClient *adapterClient)
{
    m_clients.removeOne(adapterClient);
}

void ProfileAdapter::cancelDownload(quint32 downloadId)
{
    downloadManagerDelegate()->cancelDownload(downloadId);
}

void ProfileAdapter::pauseDownload(quint32 downloadId)
{
    downloadManagerDelegate()->pauseDownload(downloadId);
}

void ProfileAdapter::resumeDownload(quint32 downloadId)
{
    downloadManagerDelegate()->resumeDownload(downloadId);
}

void ProfileAdapter::removeDownload(quint32 downloadId)
{
    downloadManagerDelegate()->removeDownload(downloadId);
}

ProfileAdapter *ProfileAdapter::createDefaultProfileAdapter()
{
    return WebEngineContext::current()->createDefaultProfileAdapter();
}

ProfileAdapter *ProfileAdapter::defaultProfileAdapter()
{
    WebEngineContext *context = WebEngineContext::current();
    return context ? context->defaultProfileAdapter() : nullptr;
}

QObject* ProfileAdapter::globalQObjectRoot()
{
    return WebEngineContext::current()->globalQObject();
}

QString ProfileAdapter::dataPath() const
{
    if (m_offTheRecord)
        return QString();
    if (!m_dataPath.isEmpty())
        return m_dataPath;
    if (!m_name.isNull())
        return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation), m_name);
    return QString();
}

void ProfileAdapter::setDataPath(const QString &path)
{
    if (m_dataPath == path)
        return;
    m_dataPath = path;
    if (!m_offTheRecord) {
        if (m_profile->m_urlRequestContextGetter.get())
            m_profile->m_profileIOData->updateStorageSettings();
        if (m_visitedLinksManager)
            resetVisitedLinksManager();
    }
}

QString ProfileAdapter::cachePath() const
{
    if (m_offTheRecord)
        return QString();
    if (!m_cachePath.isEmpty())
        return m_cachePath;
    if (!m_name.isNull())
        return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation), m_name);
    return QString();
}

void ProfileAdapter::setCachePath(const QString &path)
{
    if (m_cachePath == path)
        return;
    m_cachePath = path;
    if (!m_offTheRecord && m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateHttpCache();
}

QString ProfileAdapter::cookiesPath() const
{
    if (m_offTheRecord)
        return QString();
    QString basePath = dataPath();
    if (!basePath.isEmpty()) {
        // This is a typo fix. We still need the old path in order to avoid breaking migration.
        QDir coookiesFolder(basePath % QLatin1String("/Coookies"));
        if (coookiesFolder.exists())
            return coookiesFolder.path();
        return basePath % QLatin1String("/Cookies");
    }
    return QString();
}

QString ProfileAdapter::channelIdPath() const
{
    if (m_offTheRecord)
        return QString();
    QString basePath = dataPath();
    if (!basePath.isEmpty())
        return basePath % QLatin1String("/Origin Bound Certs");
    return QString();
}

QString ProfileAdapter::httpCachePath() const
{
    if (m_offTheRecord)
        return QString();
    QString basePath = cachePath();
    if (!basePath.isEmpty())
        return basePath % QLatin1String("/Cache");
    return QString();
}

QString ProfileAdapter::httpUserAgent() const
{
    if (m_httpUserAgent.isNull())
        return QString::fromStdString(ContentClientQt::getUserAgent());
    return m_httpUserAgent;
}

void ProfileAdapter::setHttpUserAgent(const QString &userAgent)
{
    if (m_httpUserAgent == userAgent)
        return;
    m_httpUserAgent = userAgent.simplified();

    std::vector<content::WebContentsImpl *> list = content::WebContentsImpl::GetAllWebContents();
    for (content::WebContentsImpl *web_contents : list)
        if (web_contents->GetBrowserContext() == m_profile.data())
            web_contents->SetUserAgentOverride(m_httpUserAgent.toStdString(), true);

    if (m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateUserAgent();
}

ProfileAdapter::HttpCacheType ProfileAdapter::httpCacheType() const
{
    if (m_httpCacheType == NoCache)
        return NoCache;
    if (isOffTheRecord() || httpCachePath().isEmpty())
        return MemoryHttpCache;
    return m_httpCacheType;
}

void ProfileAdapter::setHttpCacheType(ProfileAdapter::HttpCacheType newhttpCacheType)
{
    ProfileAdapter::HttpCacheType oldCacheType = httpCacheType();
    m_httpCacheType = newhttpCacheType;
    if (oldCacheType == httpCacheType())
        return;
    if (!m_offTheRecord && m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateHttpCache();
}

ProfileAdapter::PersistentCookiesPolicy ProfileAdapter::persistentCookiesPolicy() const
{
    if (isOffTheRecord() || cookiesPath().isEmpty())
        return NoPersistentCookies;
    return m_persistentCookiesPolicy;
}

void ProfileAdapter::setPersistentCookiesPolicy(ProfileAdapter::PersistentCookiesPolicy newPersistentCookiesPolicy)
{
    ProfileAdapter::PersistentCookiesPolicy oldPolicy = persistentCookiesPolicy();
    m_persistentCookiesPolicy = newPersistentCookiesPolicy;
    if (oldPolicy == persistentCookiesPolicy())
        return;
    if (!m_offTheRecord && m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateCookieStore();
}

ProfileAdapter::VisitedLinksPolicy ProfileAdapter::visitedLinksPolicy() const
{
    if (isOffTheRecord() || m_visitedLinksPolicy == DoNotTrackVisitedLinks)
        return DoNotTrackVisitedLinks;
    if (dataPath().isEmpty())
        return TrackVisitedLinksInMemory;
    return m_visitedLinksPolicy;
}

bool ProfileAdapter::trackVisitedLinks() const
{
    switch (visitedLinksPolicy()) {
    case DoNotTrackVisitedLinks:
        return false;
    default:
        break;
    }
    return true;
}

bool ProfileAdapter::persistVisitedLinks() const
{
    switch (visitedLinksPolicy()) {
    case DoNotTrackVisitedLinks:
    case TrackVisitedLinksInMemory:
        return false;
    default:
        break;
    }
    return true;
}

void ProfileAdapter::setVisitedLinksPolicy(ProfileAdapter::VisitedLinksPolicy visitedLinksPolicy)
{
    if (m_visitedLinksPolicy == visitedLinksPolicy)
        return;
    m_visitedLinksPolicy = visitedLinksPolicy;
    if (m_visitedLinksManager)
        resetVisitedLinksManager();
}

int ProfileAdapter::httpCacheMaxSize() const
{
    return m_httpCacheMaxSize;
}

void ProfileAdapter::setHttpCacheMaxSize(int maxSize)
{
    if (m_httpCacheMaxSize == maxSize)
        return;
    m_httpCacheMaxSize = maxSize;
    if (!m_offTheRecord && m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateHttpCache();
}

const QHash<QByteArray, QWebEngineUrlSchemeHandler *> &ProfileAdapter::customUrlSchemeHandlers() const
{
    return m_customUrlSchemeHandlers;
}

const QList<QByteArray> ProfileAdapter::customUrlSchemes() const
{
    return m_customUrlSchemeHandlers.keys();
}

void ProfileAdapter::updateCustomUrlSchemeHandlers()
{
    if (m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateJobFactory();
}

bool ProfileAdapter::removeCustomUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler)
{
    bool removedOneOrMore = false;
    auto it = m_customUrlSchemeHandlers.begin();
    while (it != m_customUrlSchemeHandlers.end()) {
        if (it.value() == handler) {
            it = m_customUrlSchemeHandlers.erase(it);
            removedOneOrMore = true;
            continue;
        }
        ++it;
    }
    if (removedOneOrMore)
        updateCustomUrlSchemeHandlers();
    return removedOneOrMore;
}

QWebEngineUrlSchemeHandler *ProfileAdapter::takeCustomUrlSchemeHandler(const QByteArray &scheme)
{
    QWebEngineUrlSchemeHandler *handler = m_customUrlSchemeHandlers.take(scheme.toLower());
    if (handler)
        updateCustomUrlSchemeHandlers();
    return handler;
}

bool ProfileAdapter::addCustomUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *handler)
{
    static const QSet<QByteArray> blacklist{
        QByteArrayLiteral("about"),
        QByteArrayLiteral("blob"),
        QByteArrayLiteral("data"),
        QByteArrayLiteral("javascript"),
        QByteArrayLiteral("qrc"),
        // See also kStandardURLSchemes in url/url_util.cc (through url::IsStandard below)
    };

    static const QSet<QByteArray> whitelist{
        QByteArrayLiteral("gopher"),
    };

    const QByteArray canonicalScheme = scheme.toLower();
    bool standardSyntax = url::IsStandard(canonicalScheme.data(), url::Component(0, canonicalScheme.size()));
    bool customScheme = QWebEngineUrlScheme::schemeByName(canonicalScheme) != QWebEngineUrlScheme();
    bool blacklisted = blacklist.contains(canonicalScheme) || (standardSyntax && !customScheme);
    bool whitelisted = whitelist.contains(canonicalScheme);

    if (blacklisted && !whitelisted) {
        qWarning("Cannot install a URL scheme handler overriding internal scheme: %s", scheme.constData());
        return false;
    }

    if (m_customUrlSchemeHandlers.value(canonicalScheme, handler) != handler) {
        qWarning("URL scheme handler already installed for the scheme: %s", scheme.constData());
        return false;
    }

    if (!whitelisted && !customScheme)
        qWarning("Please register the custom scheme '%s' via QWebEngineUrlScheme::registerScheme() "
                 "before installing the custom scheme handler.", scheme.constData());

    m_customUrlSchemeHandlers.insert(canonicalScheme, handler);
    updateCustomUrlSchemeHandlers();
    return true;
}

void ProfileAdapter::clearCustomUrlSchemeHandlers()
{
    m_customUrlSchemeHandlers.clear();
    updateCustomUrlSchemeHandlers();
}

UserResourceControllerHost *ProfileAdapter::userResourceController()
{
    if (!m_userResourceController)
        m_userResourceController.reset(new UserResourceControllerHost);
    return m_userResourceController.data();
}

void ProfileAdapter::permissionRequestReply(const QUrl &origin, PermissionType type, bool reply)
{
    static_cast<PermissionManagerQt*>(profile()->GetPermissionControllerDelegate())->permissionRequestReply(origin, type, reply);
}

bool ProfileAdapter::checkPermission(const QUrl &origin, PermissionType type)
{
    return static_cast<PermissionManagerQt*>(profile()->GetPermissionControllerDelegate())->checkPermission(origin, type);
}

QString ProfileAdapter::httpAcceptLanguageWithoutQualities() const
{
    const QStringList list = m_httpAcceptLanguage.split(QLatin1Char(','));
    QString out;
    for (const QString &str : list) {
        if (!out.isEmpty())
            out.append(QLatin1Char(','));
        out.append(str.split(QLatin1Char(';')).first());
    }
    return out;
}

QString ProfileAdapter::httpAcceptLanguage() const
{
    return m_httpAcceptLanguage;
}

void ProfileAdapter::setHttpAcceptLanguage(const QString &httpAcceptLanguage)
{
    if (m_httpAcceptLanguage == httpAcceptLanguage)
        return;
    m_httpAcceptLanguage = httpAcceptLanguage;

    std::vector<content::WebContentsImpl *> list = content::WebContentsImpl::GetAllWebContents();
    for (content::WebContentsImpl *web_contents : list) {
        if (web_contents->GetBrowserContext() == m_profile.data()) {
            content::RendererPreferences* rendererPrefs = web_contents->GetMutableRendererPrefs();
            rendererPrefs->accept_languages = httpAcceptLanguageWithoutQualities().toStdString();
            web_contents->GetRenderViewHost()->SyncRendererPrefs();
        }
    }

    if (m_profile->m_urlRequestContextGetter.get())
        m_profile->m_profileIOData->updateUserAgent();
}

void ProfileAdapter::clearHttpCache()
{
    content::BrowsingDataRemover *remover = content::BrowserContext::GetBrowsingDataRemover(m_profile.data());
    remover->Remove(base::Time(), base::Time::Max(),
        content::BrowsingDataRemover::DATA_TYPE_CACHE,
        content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB | content::BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB);
}

void ProfileAdapter::setSpellCheckLanguages(const QStringList &languages)
{
#if QT_CONFIG(webengine_spellchecker)
    m_profile->setSpellCheckLanguages(languages);
#endif
}

QStringList ProfileAdapter::spellCheckLanguages() const
{
#if QT_CONFIG(webengine_spellchecker)
    return m_profile->spellCheckLanguages();
#else
    return QStringList();
#endif
}

void ProfileAdapter::setSpellCheckEnabled(bool enabled)
{
#if QT_CONFIG(webengine_spellchecker)
    m_profile->setSpellCheckEnabled(enabled);
#endif
}

bool ProfileAdapter::isSpellCheckEnabled() const
{
#if QT_CONFIG(webengine_spellchecker)
    return m_profile->isSpellCheckEnabled();
#else
    return false;
#endif
}

void ProfileAdapter::addWebContentsAdapterClient(WebContentsAdapterClient *client)
{
    m_webContentsAdapterClients.append(client);
}

void ProfileAdapter::removeWebContentsAdapterClient(WebContentsAdapterClient *client)
{
    m_webContentsAdapterClients.removeAll(client);
}

void ProfileAdapter::resetVisitedLinksManager()
{
    m_visitedLinksManager.reset(new VisitedLinksManagerQt(this));
}

} // namespace QtWebEngineCore
