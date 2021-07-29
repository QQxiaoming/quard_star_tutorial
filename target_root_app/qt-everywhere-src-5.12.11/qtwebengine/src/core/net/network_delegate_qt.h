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

#ifndef NETWORK_DELEGATE_QT_H
#define NETWORK_DELEGATE_QT_H

#include "net/base/network_delegate.h"
#include "net/base/net_errors.h"

#include <QUrl>
#include <QSet>

namespace content {
class WebContents;
}

namespace QtWebEngineCore {

class ProfileIODataQt;

class NetworkDelegateQt : public net::NetworkDelegate {
    ProfileIODataQt *m_profileIOData;
public:
    NetworkDelegateQt(ProfileIODataQt *data);

    // net::NetworkDelegate implementation
    int OnBeforeURLRequest(net::URLRequest* request, net::CompletionOnceCallback callback, GURL* new_url) override;
    void OnURLRequestDestroyed(net::URLRequest* request) override;
    bool OnCanSetCookie(const net::URLRequest& request, const net::CanonicalCookie& cookie, net::CookieOptions* options) override;
    int OnBeforeStartTransaction(net::URLRequest *request, const net::CompletionOnceCallback callback, net::HttpRequestHeaders *headers) override;
    void OnBeforeSendHeaders(net::URLRequest* request, const net::ProxyInfo& proxy_info,
                             const net::ProxyRetryInfoMap& proxy_retry_info, net::HttpRequestHeaders* headers) override;
    void OnStartTransaction(net::URLRequest *request, const net::HttpRequestHeaders &headers) override;
    int OnHeadersReceived(net::URLRequest*, net::CompletionOnceCallback, const net::HttpResponseHeaders*, scoped_refptr<net::HttpResponseHeaders>*, GURL*) override;
    void OnBeforeRedirect(net::URLRequest*, const GURL&) override;
    void OnResponseStarted(net::URLRequest*, int) override;
    void OnNetworkBytesReceived(net::URLRequest*, int64_t) override;
    void OnNetworkBytesSent(net::URLRequest *, int64_t) override;
    void OnCompleted(net::URLRequest *request, bool started, int net_error) override;
    void OnPACScriptError(int, const base::string16&) override;
    net::NetworkDelegate::AuthRequiredResponse OnAuthRequired(net::URLRequest*, const net::AuthChallengeInfo&, AuthCallback, net::AuthCredentials*) override;
    bool OnCanGetCookies(const net::URLRequest&, const net::CookieList&) override;
    bool OnCanAccessFile(const net::URLRequest&, const base::FilePath&, const base::FilePath&) const override;
    bool OnCanEnablePrivacyMode(const GURL&, const GURL&) const override;
    bool OnAreExperimentalCookieFeaturesEnabled() const override;
    bool OnCancelURLRequestWithPolicyViolatingReferrerHeader(const net::URLRequest&, const GURL&, const GURL&) const override;

    bool OnCanQueueReportingReport(const url::Origin& origin) const override;
    void OnCanSendReportingReports(std::set<url::Origin> origins, base::OnceCallback<void(std::set<url::Origin>)> result_callback) const override;
    bool OnCanSetReportingClient(const url::Origin& origin, const GURL& endpoint) const override;
    bool OnCanUseReportingClient(const url::Origin& origin, const GURL& endpoint) const override;

    bool canSetCookies(const GURL &first_party, const GURL &url, const std::string &cookie_line) const;
    bool canGetCookies(const GURL &first_party, const GURL &url) const;
};

} // namespace QtWebEngineCore

#endif // NETWORK_DELEGATE_QT_H
