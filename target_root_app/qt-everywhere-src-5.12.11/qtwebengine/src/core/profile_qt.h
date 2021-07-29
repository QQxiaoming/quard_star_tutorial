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

#ifndef PROFILE_QT_H
#define PROFILE_QT_H

#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "net/url_request/url_request_context.h"
#include "profile_io_data_qt.h"
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE
class InMemoryPrefStore;
class PrefService;

namespace QtWebEngineCore {

class BrowsingDataRemoverDelegateQt;
class ProfileAdapter;
class PermissionManagerQt;
class SSLHostStateDelegateQt;

class ProfileQt : public Profile
{
public:
    explicit ProfileQt(ProfileAdapter *profileAdapter);

    virtual ~ProfileQt();

    // BrowserContext implementation:
    base::FilePath GetPath() const override;
    bool IsOffTheRecord() const override;

    net::URLRequestContextGetter *CreateMediaRequestContext() override;
    net::URLRequestContextGetter *CreateMediaRequestContextForStoragePartition(
            const base::FilePath &partition_path,
            bool in_memory) override;
    content::ResourceContext *GetResourceContext() override;
    content::DownloadManagerDelegate *GetDownloadManagerDelegate() override;
    content::BrowserPluginGuestManager *GetGuestManager() override;
    storage::SpecialStoragePolicy *GetSpecialStoragePolicy() override;
    content::PushMessagingService *GetPushMessagingService() override;
    content::SSLHostStateDelegate *GetSSLHostStateDelegate() override;
    net::URLRequestContextGetter *CreateRequestContext(
            content::ProtocolHandlerMap *protocol_handlers,
            content::URLRequestInterceptorScopedVector request_interceptors) override;
    net::URLRequestContextGetter *CreateRequestContextForStoragePartition(
            const base::FilePath &partition_path, bool in_memory,
            content::ProtocolHandlerMap *protocol_handlers,
            content::URLRequestInterceptorScopedVector request_interceptors) override;
    std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
            const base::FilePath &partition_path) override;
    content::PermissionControllerDelegate * GetPermissionControllerDelegate() override;
    content::BackgroundFetchDelegate *GetBackgroundFetchDelegate() override;
    content::BackgroundSyncController *GetBackgroundSyncController() override;
    content::BrowsingDataRemoverDelegate *GetBrowsingDataRemoverDelegate() override;

    // Profile implementation:
    PrefService *GetPrefs() override;
    const PrefService *GetPrefs() const override;
    net::URLRequestContextGetter *GetRequestContext() override;

    ProfileAdapter *profileAdapter() { return m_profileAdapter; }

#if QT_CONFIG(webengine_spellchecker)
    void FailedToLoadDictionary(const std::string &language) override;
    void setSpellCheckLanguages(const QStringList &languages);
    QStringList spellCheckLanguages() const;
    void setSpellCheckEnabled(bool enabled);
    bool isSpellCheckEnabled() const;
#endif

private:
    friend class ContentBrowserClientQt;
    friend class WebContentsAdapter;
    scoped_refptr<net::URLRequestContextGetter> m_urlRequestContextGetter;
    std::unique_ptr<BrowsingDataRemoverDelegateQt> m_removerDelegate;
    std::unique_ptr<PermissionManagerQt> m_permissionManager;
    std::unique_ptr<SSLHostStateDelegateQt> m_sslHostStateDelegate;
    std::unique_ptr<PrefService> m_prefService;
    std::unique_ptr<ProfileIODataQt> m_profileIOData;
    ProfileAdapter *m_profileAdapter;
    friend class ProfileAdapter;

    DISALLOW_COPY_AND_ASSIGN(ProfileQt);
};

} // namespace QtWebEngineCore

#endif // PROFILE_QT_H
