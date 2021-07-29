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

#include "qquickwebengineprofile.h"

#include "qquickwebenginedownloaditem_p.h"
#include "qquickwebenginedownloaditem_p_p.h"
#include "qquickwebengineprofile_p.h"
#include "qquickwebenginescript_p.h"
#include "qquickwebenginesettings_p.h"
#include "qquickwebengineview_p_p.h"
#include "qwebenginecookiestore.h"

#include <QQmlEngine>

#include "profile_adapter.h"
#include "renderer_host/user_resource_controller_host.h"
#include "web_engine_settings.h"

#include <QtWebEngineCore/qwebengineurlscheme.h>

using QtWebEngineCore::ProfileAdapter;

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(QQuickWebEngineDownloadItem::UnknownSaveFormat, QtWebEngineCore::ProfileAdapterClient::UnknownSavePageFormat)
ASSERT_ENUMS_MATCH(QQuickWebEngineDownloadItem::SingleHtmlSaveFormat, QtWebEngineCore::ProfileAdapterClient::SingleHtmlSaveFormat)
ASSERT_ENUMS_MATCH(QQuickWebEngineDownloadItem::CompleteHtmlSaveFormat, QtWebEngineCore::ProfileAdapterClient::CompleteHtmlSaveFormat)
ASSERT_ENUMS_MATCH(QQuickWebEngineDownloadItem::MimeHtmlSaveFormat, QtWebEngineCore::ProfileAdapterClient::MimeHtmlSaveFormat)

/*!
    \class QQuickWebEngineProfile
    \brief The QQuickWebEngineProfile class provides a web engine profile shared by multiple pages.
    \since 5.6

    \inmodule QtWebEngine

    A web engine profile contains settings, scripts, persistent cookie policy, and the list of
    visited links shared by all web engine pages that belong to the profile.

    Information about visited links is stored together with persistent cookies
    and other persistent data in a storage determined by the storageName
    property. Persistent data is stored in a subdirectory determined by the
    persistentStoragePath property and the cache in a subdirectory determined by
    the cachePath property. The httpCacheType property describes the type of the
    cache: \e in-memory or \e on-disk. If only the storageName property is set,
    the other values are generated automatically based on it. If you specify
    any of the values manually, you should do it before creating any pages that
    belong to the profile.

    Profiles can be used to isolate pages from each other. A typical use case is a dedicated
    \e {off-the-record profile} for a \e {private browsing} mode. An off-the-record profile forces
    cookies, the HTTP cache, and other normally persistent data to be stored only in memory. The
    offTheRecord property holds whether a profile is off-the-record.

    The default profile can be accessed by defaultProfile(). It is a built-in profile that all
    web pages not specifically created with another profile belong to.

    A WebEngineProfile instance can be created and accessed from C++ through the
    QQuickWebEngineProfile class, which exposes further functionality in C++. This allows Qt Quick
    applications to intercept URL requests (QQuickWebEngineProfile::setRequestInterceptor), or
    register custom URL schemes (QQuickWebEngineProfile::installUrlSchemeHandler).

    Spellchecking HTML form fields can be enabled per profile by setting the \l spellCheckEnabled
    property and the current languages used for spellchecking can be set by using the
    \l spellCheckLanguages property.
*/

/*!
    \enum QQuickWebEngineProfile::HttpCacheType

    This enum describes the HTTP cache type:

    \value MemoryHttpCache Use an in-memory cache. This is the default if
    \c off-the-record is set.
    \value DiskHttpCache Use a disk cache. This is the default if \c off-the-record
    is not set. Falls back to \c MemoryHttpCache if \c off-the-record is set.
    \value NoCache Disable both in-memory and disk caching. (Added in Qt 5.7)
*/

/*!
    \enum QQuickWebEngineProfile::PersistentCookiesPolicy

    This enum describes policy for cookie persistency:

    \value  NoPersistentCookies
            Both session and persistent cookies are stored in memory. This is the only setting
            possible if \c off-the-record is set or no persistent data path is available.
    \value  AllowPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are only stored to disk for crash recovery. This is the default setting.
    \value  ForcePersistentCookies
            Both session and persistent cookies are saved to and restored from disk.
*/

/*!
  \fn QQuickWebEngineProfile::downloadRequested(QQuickWebEngineDownloadItem *download)

  This signal is emitted whenever a download has been triggered.
  The \a download argument holds the state of the download.
  The download has to be explicitly accepted with
  \c{QQuickWebEngineDownloadItem::accept()} or it will be
  cancelled by default.
  The download item is parented by the profile. If it is not accepted, it
  will be deleted immediately after the signal emission.
  This signal cannot be used with a queued connection.
*/

/*!
  \fn QQuickWebEngineProfile::downloadFinished(QQuickWebEngineDownloadItem *download)

  This signal is emitted whenever downloading stops, because it finished successfully, was
  cancelled, or was interrupted (for example, because connectivity was lost).
  The \a download argument holds the state of the finished download instance.
*/

QQuickWebEngineProfilePrivate::QQuickWebEngineProfilePrivate(ProfileAdapter *profileAdapter)
        : m_settings(new QQuickWebEngineSettings())
        , m_profileAdapter(profileAdapter)
{
    profileAdapter->addClient(this);
    m_settings->d_ptr->initDefaults();
    // Fullscreen API was implemented before the supported setting, so we must
    // make it default true to avoid change in default API behavior.
    m_settings->d_ptr->setAttribute(QtWebEngineCore::WebEngineSettings::FullScreenSupportEnabled, true);
}

QQuickWebEngineProfilePrivate::~QQuickWebEngineProfilePrivate()
{
    if (m_profileAdapter) {
        // In the case the user sets this profile as the parent of the interceptor
        // it can be deleted before the browser-context still referencing it is.
        m_profileAdapter->setRequestInterceptor(nullptr);
        m_profileAdapter->removeClient(this);
    }

    if (m_profileAdapter != QtWebEngineCore::ProfileAdapter::defaultProfileAdapter())
        delete m_profileAdapter;
}

void QQuickWebEngineProfilePrivate::addWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient *adapter)
{
    Q_ASSERT(m_profileAdapter);
    m_profileAdapter->addWebContentsAdapterClient(adapter);
}

void QQuickWebEngineProfilePrivate::removeWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient*adapter)
{
    Q_ASSERT(m_profileAdapter);
    m_profileAdapter->removeWebContentsAdapterClient(adapter);
}

QtWebEngineCore::ProfileAdapter *QQuickWebEngineProfilePrivate::profileAdapter() const
{
    return m_profileAdapter;
}

QQuickWebEngineSettings *QQuickWebEngineProfilePrivate::settings() const
{
    return m_settings.data();
}

void QQuickWebEngineProfilePrivate::cancelDownload(quint32 downloadId)
{
    if (m_profileAdapter)
        m_profileAdapter->cancelDownload(downloadId);
}

void QQuickWebEngineProfilePrivate::downloadDestroyed(quint32 downloadId)
{
    m_ongoingDownloads.remove(downloadId);
    if (m_profileAdapter)
        m_profileAdapter->removeDownload(downloadId);
}

void QQuickWebEngineProfilePrivate::cleanDownloads()
{
    for (auto download : m_ongoingDownloads.values()) {
        if (!download)
            continue;

        if (!download->isFinished())
            download->cancel();

        if (m_profileAdapter)
            m_profileAdapter->removeDownload(download->id());
    }
    m_ongoingDownloads.clear();
}

void QQuickWebEngineProfilePrivate::downloadRequested(DownloadItemInfo &info)
{
    Q_Q(QQuickWebEngineProfile);

    Q_ASSERT(!m_ongoingDownloads.contains(info.id));
    QQuickWebEngineDownloadItemPrivate *itemPrivate = new QQuickWebEngineDownloadItemPrivate(q);
    itemPrivate->downloadId = info.id;
    itemPrivate->downloadState = QQuickWebEngineDownloadItem::DownloadRequested;
    itemPrivate->totalBytes = info.totalBytes;
    itemPrivate->mimeType = info.mimeType;
    itemPrivate->downloadPath = info.path;
    itemPrivate->savePageFormat = static_cast<QQuickWebEngineDownloadItem::SavePageFormat>(
                info.savePageFormat);
    itemPrivate->type = static_cast<QQuickWebEngineDownloadItem::DownloadType>(info.downloadType);
    if (info.page && info.page->clientType() == QtWebEngineCore::WebContentsAdapterClient::QmlClient)
        itemPrivate->view = static_cast<QQuickWebEngineViewPrivate *>(info.page)->q_ptr;
    else
        itemPrivate->view = nullptr;

    QQuickWebEngineDownloadItem *download = new QQuickWebEngineDownloadItem(itemPrivate, q);

    m_ongoingDownloads.insert(info.id, download);
    QObject::connect(download, &QQuickWebEngineDownloadItem::destroyed, q, [id = info.id, this] () { downloadDestroyed(id); });

    QQmlEngine::setObjectOwnership(download, QQmlEngine::JavaScriptOwnership);
    Q_EMIT q->downloadRequested(download);

    QQuickWebEngineDownloadItem::DownloadState state = download->state();
    info.path = download->path();
    info.savePageFormat = itemPrivate->savePageFormat;
    info.accepted = state != QQuickWebEngineDownloadItem::DownloadCancelled
                      && state != QQuickWebEngineDownloadItem::DownloadRequested;

    if (state == QQuickWebEngineDownloadItem::DownloadRequested) {
        // Delete unaccepted downloads.
        info.accepted = false;
        delete download;
    }
}

void QQuickWebEngineProfilePrivate::downloadUpdated(const DownloadItemInfo &info)
{
    if (!m_ongoingDownloads.contains(info.id))
        return;

    Q_Q(QQuickWebEngineProfile);

    QQuickWebEngineDownloadItem* download = m_ongoingDownloads.value(info.id).data();

    if (!download) {
        downloadDestroyed(info.id);
        return;
    }

    download->d_func()->update(info);

    if (info.state != ProfileAdapterClient::DownloadInProgress) {
        Q_EMIT q->downloadFinished(download);
    }
}

void QQuickWebEngineProfilePrivate::userScripts_append(QQmlListProperty<QQuickWebEngineScript> *p, QQuickWebEngineScript *script)
{
    Q_ASSERT(p && p->data);
    QQuickWebEngineProfilePrivate *d = static_cast<QQuickWebEngineProfilePrivate *>(p->data);
    QtWebEngineCore::UserResourceControllerHost *resourceController = d->profileAdapter()->userResourceController();
    d->m_userScripts.append(script);
    script->d_func()->bind(resourceController);
}

int QQuickWebEngineProfilePrivate::userScripts_count(QQmlListProperty<QQuickWebEngineScript> *p)
{
    Q_ASSERT(p && p->data);
    QQuickWebEngineProfilePrivate *d = static_cast<QQuickWebEngineProfilePrivate *>(p->data);
    return d->m_userScripts.count();
}

QQuickWebEngineScript *QQuickWebEngineProfilePrivate::userScripts_at(QQmlListProperty<QQuickWebEngineScript> *p, int idx)
{
    Q_ASSERT(p && p->data);
    QQuickWebEngineProfilePrivate *d = static_cast<QQuickWebEngineProfilePrivate *>(p->data);
    return d->m_userScripts.at(idx);
}

void QQuickWebEngineProfilePrivate::userScripts_clear(QQmlListProperty<QQuickWebEngineScript> *p)
{
    Q_ASSERT(p && p->data);
    QQuickWebEngineProfilePrivate *d = static_cast<QQuickWebEngineProfilePrivate *>(p->data);
    QtWebEngineCore::UserResourceControllerHost *resourceController = d->profileAdapter()->userResourceController();
    resourceController->clearAllScripts(NULL);
    d->m_userScripts.clear();
}

/*!
    \qmltype WebEngineProfile
    \instantiates QQuickWebEngineProfile
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.1
    \brief Contains settings, scripts, and visited links common to multiple web engine views.

    WebEngineProfile contains settings, scripts, and the list of visited links shared by all
    views that belong to the profile.

    Information about visited links is stored together with persistent cookies
    and other persistent data in a storage determined by the storageName
    property. Persistent data is stored in a subdirectory determined by the
    persistentStoragePath property and the cache in a subdirectory determined by
    the cachePath property. The httpCacheType property describes the type of the
    cache: \e in-memory or \e on-disk. If only the storageName property is set,
    the other values are generated automatically based on it. If you specify
    any of the values manually, you should do it before creating any pages that
    belong to the profile.

    Profiles can be used to isolate pages from each other. A typical use case is
    a dedicated \e {off-the-record profile} for a \e {private browsing} mode. An
    off-the-record profile forces cookies, the HTTP cache, and other normally
    persistent data to be stored only in memory. The offTheRecord property holds
    whether a profile is off-the-record.

    Each web engine view has an associated profile. Views that do not have a specific profile set
    share a common default one.
*/

/*!
    \qmlsignal WebEngineProfile::downloadRequested(WebEngineDownloadItem download)

    This signal is emitted whenever a download has been triggered.
    The \a download argument holds the state of the download.
    The download has to be explicitly accepted with WebEngineDownloadItem::accept() or the
    download will be cancelled by default.
*/

/*!
    \qmlsignal WebEngineProfile::downloadFinished(WebEngineDownloadItem download)

    This signal is emitted whenever downloading stops, because it finished successfully, was
    cancelled, or was interrupted (for example, because connectivity was lost).
    The \a download argument holds the state of the finished download instance.
*/

/*!
    Constructs a new profile with the parent \a parent.
*/
QQuickWebEngineProfile::QQuickWebEngineProfile(QObject *parent)
    : QObject(parent),
      d_ptr(new QQuickWebEngineProfilePrivate(new QtWebEngineCore::ProfileAdapter()))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineProfile::QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *privatePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(privatePtr)
{
    d_ptr->q_ptr = this;
}

/*!
   \internal
*/
QQuickWebEngineProfile::~QQuickWebEngineProfile()
{
    d_ptr->cleanDownloads();
}

/*!
    \qmlproperty string WebEngineProfile::storageName

    The storage name that is used to create separate subdirectories for each profile that uses
    the disk for storing persistent data and cache.

    \sa WebEngineProfile::persistentStoragePath, WebEngineProfile::cachePath
*/

/*!
    \property QQuickWebEngineProfile::storageName

    The storage name that is used to create separate subdirectories for each profile that uses
    the disk for storing persistent data and cache.

    \sa persistentStoragePath, cachePath
*/

QString QQuickWebEngineProfile::storageName() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->storageName();
}

void QQuickWebEngineProfile::setStorageName(const QString &name)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->storageName() == name)
        return;
    ProfileAdapter::HttpCacheType oldCacheType = d->profileAdapter()->httpCacheType();
    ProfileAdapter::PersistentCookiesPolicy oldPolicy = d->profileAdapter()->persistentCookiesPolicy();
    d->profileAdapter()->setStorageName(name);
    emit storageNameChanged();
    emit persistentStoragePathChanged();
    emit cachePathChanged();
    if (d->profileAdapter()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
    if (d->profileAdapter()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty bool WebEngineProfile::offTheRecord

    Whether the web engine profile is \e off-the-record.
    An off-the-record profile forces cookies, the HTTP cache, and other normally persistent data
    to be stored only in memory.
*/


/*!
    \property QQuickWebEngineProfile::offTheRecord

    Whether the web engine profile is \e off-the-record.
    An off-the-record profile forces cookies, the HTTP cache, and other normally persistent data
    to be stored only in memory.
*/

bool QQuickWebEngineProfile::isOffTheRecord() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->isOffTheRecord();
}

void QQuickWebEngineProfile::setOffTheRecord(bool offTheRecord)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->isOffTheRecord() == offTheRecord)
        return;
    ProfileAdapter::HttpCacheType oldCacheType = d->profileAdapter()->httpCacheType();
    ProfileAdapter::PersistentCookiesPolicy oldPolicy = d->profileAdapter()->persistentCookiesPolicy();
    d->profileAdapter()->setOffTheRecord(offTheRecord);
    emit offTheRecordChanged();
    if (d->profileAdapter()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
    if (d->profileAdapter()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty string WebEngineProfile::persistentStoragePath

    The path to the location where the persistent data for the browser and web content are
    stored. Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, the storage is located below
    QStandardPaths::writableLocation(QStandardPaths::DataLocation) in a directory named using
    storageName.
*/

/*!
    \property QQuickWebEngineProfile::persistentStoragePath

    The path to the location where the persistent data for the browser and web content are
    stored. Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, the storage is located below
    QStandardPaths::writableLocation(QStandardPaths::DataLocation) in a directory named using
    storageName.
*/

QString QQuickWebEngineProfile::persistentStoragePath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->dataPath();
}

void QQuickWebEngineProfile::setPersistentStoragePath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (persistentStoragePath() == path)
        return;
    d->profileAdapter()->setDataPath(path);
    emit persistentStoragePathChanged();
}

/*!
    \qmlproperty string WebEngineProfile::cachePath

    The path to the location where the profile's caches are stored, in particular the HTTP cache.

    By default, the caches are stored
    below QStandardPaths::writableLocation(QStandardPaths::CacheLocation) in a directory named using
    storageName.
*/

/*!
    \property QQuickWebEngineProfile::cachePath

    The path to the location where the profile's caches are stored, in particular the HTTP cache.

    By default, the caches are stored
    below QStandardPaths::writableLocation(QStandardPaths::CacheLocation) in a directory named using
    storageName.
*/

QString QQuickWebEngineProfile::cachePath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->cachePath();
}

void QQuickWebEngineProfile::setCachePath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (cachePath() == path)
        return;
    d->profileAdapter()->setCachePath(path);
    emit cachePathChanged();
}

/*!
    \qmlproperty string WebEngineProfile::httpUserAgent

    The user-agent string sent with HTTP to identify the browser.

    \note On Windows 8.1 and newer, the default user agent will always report
    "Windows NT 6.2" (Windows 8), unless the application does contain a manifest
    that declares newer Windows versions as supported.
*/

/*!
    \property QQuickWebEngineProfile::httpUserAgent

    The user-agent string sent with HTTP to identify the browser.
*/

QString QQuickWebEngineProfile::httpUserAgent() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->httpUserAgent();
}

void QQuickWebEngineProfile::setHttpUserAgent(const QString &userAgent)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->httpUserAgent() == userAgent)
        return;
    d->profileAdapter()->setHttpUserAgent(userAgent);
    emit httpUserAgentChanged();
}


/*!
    \qmlproperty enumeration WebEngineProfile::httpCacheType

    This enumeration describes the type of the HTTP cache:

    \value  WebEngineProfile.MemoryHttpCache
            Uses an in-memory cache. This is the only setting possible if offTheRecord is set or
            no persistentStoragePath is available.
    \value  WebEngineProfile.DiskHttpCache
            Uses a disk cache. This is the default value.
    \value  WebEngineProfile.NoCache
            Disables caching. (Added in 5.7)
*/

/*!
    \property QQuickWebEngineProfile::httpCacheType

    This enumeration describes the type of the HTTP cache.

    If the profile is off-the-record, MemoryHttpCache is returned.
*/

QQuickWebEngineProfile::HttpCacheType QQuickWebEngineProfile::httpCacheType() const
{
    const Q_D(QQuickWebEngineProfile);
    return QQuickWebEngineProfile::HttpCacheType(d->profileAdapter()->httpCacheType());
}

void QQuickWebEngineProfile::setHttpCacheType(QQuickWebEngineProfile::HttpCacheType httpCacheType)
{
    Q_D(QQuickWebEngineProfile);
    ProfileAdapter::HttpCacheType oldCacheType = d->profileAdapter()->httpCacheType();
    d->profileAdapter()->setHttpCacheType(ProfileAdapter::HttpCacheType(httpCacheType));
    if (d->profileAdapter()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
}

/*!
    \qmlproperty enumeration WebEngineProfile::persistentCookiesPolicy

    This enumeration describes the policy of cookie persistency:

    \value  WebEngineProfile.NoPersistentCookies
            Both session and persistent cookies are stored in memory. This is the only setting
            possible if offTheRecord is set or no persistentStoragePath is available.
    \value  WebEngineProfile.AllowPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are only stored to disk for crash recovery. This is the default setting.
    \value WebEngineProfile.ForcePersistentCookies
            Both session and persistent cookies are saved to and restored from disk.
*/

/*!
    \property QQuickWebEngineProfile::persistentCookiesPolicy

    This enumeration describes the policy of cookie persistency.
    If the profile is off-the-record, NoPersistentCookies is returned.
*/

QQuickWebEngineProfile::PersistentCookiesPolicy QQuickWebEngineProfile::persistentCookiesPolicy() const
{
    const Q_D(QQuickWebEngineProfile);
    return QQuickWebEngineProfile::PersistentCookiesPolicy(d->profileAdapter()->persistentCookiesPolicy());
}

void QQuickWebEngineProfile::setPersistentCookiesPolicy(QQuickWebEngineProfile::PersistentCookiesPolicy newPersistentCookiesPolicy)
{
    Q_D(QQuickWebEngineProfile);
    ProfileAdapter::PersistentCookiesPolicy oldPolicy = d->profileAdapter()->persistentCookiesPolicy();
    d->profileAdapter()->setPersistentCookiesPolicy(ProfileAdapter::PersistentCookiesPolicy(newPersistentCookiesPolicy));
    if (d->profileAdapter()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty int WebEngineProfile::httpCacheMaximumSize

    The maximum size of the HTTP cache. If \c 0, the size will be controlled automatically by
    QtWebEngine. The default value is \c 0.

    \sa httpCacheType
*/

/*!
    \property QQuickWebEngineProfile::httpCacheMaximumSize

    The maximum size of the HTTP cache. If \c 0, the size will be controlled automatically by
    QtWebEngine. The default value is \c 0.

    \sa httpCacheType
*/

int QQuickWebEngineProfile::httpCacheMaximumSize() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->httpCacheMaxSize();
}

void QQuickWebEngineProfile::setHttpCacheMaximumSize(int maximumSize)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->httpCacheMaxSize() == maximumSize)
        return;
    d->profileAdapter()->setHttpCacheMaxSize(maximumSize);
    emit httpCacheMaximumSizeChanged();
}

/*!
    \qmlproperty string WebEngineProfile::httpAcceptLanguage

    The value of the Accept-Language HTTP request-header field.

    \since QtWebEngine 1.2
*/

/*!
    \property QQuickWebEngineProfile::httpAcceptLanguage

    The value of the Accept-Language HTTP request-header field.
*/

QString QQuickWebEngineProfile::httpAcceptLanguage() const
{
    Q_D(const QQuickWebEngineProfile);
    return d->profileAdapter()->httpAcceptLanguage();
}

void QQuickWebEngineProfile::setHttpAcceptLanguage(const QString &httpAcceptLanguage)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->httpAcceptLanguage() == httpAcceptLanguage)
        return;
    d->profileAdapter()->setHttpAcceptLanguage(httpAcceptLanguage);
    emit httpAcceptLanguageChanged();
}

/*!
    Returns the default profile.

    The default profile uses the storage name "Default".

    \sa storageName()
*/
QQuickWebEngineProfile *QQuickWebEngineProfile::defaultProfile()
{
    static QQuickWebEngineProfile *profile = new QQuickWebEngineProfile(
                new QQuickWebEngineProfilePrivate(ProfileAdapter::createDefaultProfileAdapter()),
                ProfileAdapter::globalQObjectRoot());
    return profile;
}

/*!
    \property QQuickWebEngineProfile::spellCheckLanguages
    \brief The languages used by the spell checker.

    \since QtWebEngine 1.4
*/

/*!
    \qmlproperty list<string> WebEngineProfile::spellCheckLanguages

    This property holds the list of languages used by the spell checker.
    Each language should match the name of the \c .bdic dictionary.
    For example, the language \c en-US will load the \c en-US.bdic
    dictionary file.

    See the \l {Spellchecker}{Spellchecker feature documentation} for how
    dictionary files are searched.

    For more information about how to compile \c .bdic dictionaries, see the
    \l{WebEngine Widgets Spellchecker Example}{Spellchecker Example}.

    \since QtWebEngine 1.4
*/
void QQuickWebEngineProfile::setSpellCheckLanguages(const QStringList &languages)
{
    Q_D(QQuickWebEngineProfile);
    if (languages != d->profileAdapter()->spellCheckLanguages()) {
        d->profileAdapter()->setSpellCheckLanguages(languages);
        emit spellCheckLanguagesChanged();
    }
}

/*!
    \since 5.8

    Returns the list of languages used by the spell checker.
*/
QStringList QQuickWebEngineProfile::spellCheckLanguages() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->spellCheckLanguages();
}

/*!
    \property QQuickWebEngineProfile::spellCheckEnabled
    \brief whether the web engine spell checker is enabled.

    \since QtWebEngine 1.4
*/

/*!
    \qmlproperty bool WebEngineProfile::spellCheckEnabled

    This property holds whether the web engine spell checker is enabled.

    \since QtWebEngine 1.4
*/
void QQuickWebEngineProfile::setSpellCheckEnabled(bool enable)
{
     Q_D(QQuickWebEngineProfile);
    if (enable != isSpellCheckEnabled()) {
        d->profileAdapter()->setSpellCheckEnabled(enable);
        emit spellCheckEnabledChanged();
    }
}

bool QQuickWebEngineProfile::isSpellCheckEnabled() const
{
     const Q_D(QQuickWebEngineProfile);
     return d->profileAdapter()->isSpellCheckEnabled();
}

/*!

    Returns the cookie store for this profile.
*/
QWebEngineCookieStore *QQuickWebEngineProfile::cookieStore() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->cookieStore();
}

/*!
    \qmlmethod void WebEngineProfile::clearHttpCache()
    \since QtWebEngine 1.3

    Removes the profile's cache entries.

    \sa WebEngineProfile::cachePath
*/

/*!
    \since 5.7

    Removes the profile's cache entries.

    \sa WebEngineProfile::clearHttpCache
*/
void QQuickWebEngineProfile::clearHttpCache()
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->clearHttpCache();
}


/*!
    Registers a request interceptor singleton \a interceptor to intercept URL requests.

    The profile does not take ownership of the pointer.

    \sa QWebEngineUrlRequestInterceptor
*/
void QQuickWebEngineProfile::setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->setRequestInterceptor(interceptor);
}

/*!
    Returns the custom URL scheme handler register for the URL scheme \a scheme.
*/
const QWebEngineUrlSchemeHandler *QQuickWebEngineProfile::urlSchemeHandler(const QByteArray &scheme) const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->customUrlSchemeHandlers().value(scheme.toLower());
}

/*!
    Registers a handler \a handler for custom URL scheme \a scheme in the profile.

    It is necessary to first register the scheme with \l
    QWebEngineUrlScheme::registerScheme at application startup.
*/
void QQuickWebEngineProfile::installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *handler)
{
    Q_D(QQuickWebEngineProfile);
    Q_ASSERT(handler);
    if (!d->profileAdapter()->addCustomUrlSchemeHandler(scheme, handler))
        return;
    connect(handler, SIGNAL(_q_destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)), this, SLOT(destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)));
}

/*!
    Removes the custom URL scheme handler \a handler from the profile.

    \sa removeUrlScheme()
*/
void QQuickWebEngineProfile::removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler)
{
    Q_D(QQuickWebEngineProfile);
    Q_ASSERT(handler);
    if (!d->profileAdapter()->removeCustomUrlSchemeHandler(handler))
        return;
    disconnect(handler, SIGNAL(_q_destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)), this, SLOT(destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)));
}

/*!
    Removes the custom URL scheme \a scheme from the profile.

    \sa removeUrlSchemeHandler()
*/
void QQuickWebEngineProfile::removeUrlScheme(const QByteArray &scheme)
{
    Q_D(QQuickWebEngineProfile);
    QWebEngineUrlSchemeHandler *handler = d->profileAdapter()->takeCustomUrlSchemeHandler(scheme);
    if (!handler)
        return;
    disconnect(handler, SIGNAL(_q_destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)), this, SLOT(destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)));
}

/*!
    Removes all custom URL scheme handlers installed in the profile.
*/
void QQuickWebEngineProfile::removeAllUrlSchemeHandlers()
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->clearCustomUrlSchemeHandlers();
}

void QQuickWebEngineProfile::destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler *obj)
{
    removeUrlSchemeHandler(obj);
}

QQuickWebEngineSettings *QQuickWebEngineProfile::settings() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->settings();
}

/*!
    \qmlproperty list<WebEngineScript> WebEngineProfile::userScripts
    \since 1.5

    Returns the collection of WebEngineScripts that are injected into all pages that share
    this profile.

    \sa WebEngineScript
*/

/*!
    \property QQuickWebEngineProfile::userScripts
    \since 5.9

    \brief The collection of scripts that are injected into all pages that share
    this profile.

    \sa QQuickWebEngineScript, QQmlListReference
*/
QQmlListProperty<QQuickWebEngineScript> QQuickWebEngineProfile::userScripts()
{
    Q_D(QQuickWebEngineProfile);
    return QQmlListProperty<QQuickWebEngineScript>(this, d,
                                                   d->userScripts_append,
                                                   d->userScripts_count,
                                                   d->userScripts_at,
                                                   d->userScripts_clear);
}

QT_END_NAMESPACE
