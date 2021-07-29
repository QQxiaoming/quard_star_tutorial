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

#include "network_delegate_qt.h"

#include "profile_adapter.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "cookie_monster_delegate_qt.h"
#include "ui/base/page_transition_types.h"
#include "profile_io_data_qt.h"
#include "net/base/load_flags.h"
#include "net/url_request/url_request.h"
#include "qwebengineurlrequestinfo.h"
#include "qwebengineurlrequestinfo_p.h"
#include "qwebengineurlrequestinterceptor.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

namespace QtWebEngineCore {

WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition)
{
    int32_t qualifier = ui::PageTransitionGetQualifier(transition);

    if (qualifier & ui::PAGE_TRANSITION_FORWARD_BACK)
        return WebContentsAdapterClient::BackForwardNavigation;
    // FIXME: Make redirects a separate type:
    if (qualifier & ui::PAGE_TRANSITION_CLIENT_REDIRECT)
        return WebContentsAdapterClient::OtherNavigation;

    ui::PageTransition strippedTransition = ui::PageTransitionStripQualifier(transition);

    switch (strippedTransition) {
    case ui::PAGE_TRANSITION_LINK:
        return WebContentsAdapterClient::LinkNavigation;
    case ui::PAGE_TRANSITION_TYPED:
        return WebContentsAdapterClient::TypedNavigation;
    case ui::PAGE_TRANSITION_FORM_SUBMIT:
        return WebContentsAdapterClient::FormSubmittedNavigation;
    case ui::PAGE_TRANSITION_RELOAD:
        return WebContentsAdapterClient::ReloadNavigation;
    default:
        return WebContentsAdapterClient::OtherNavigation;
    }
}

namespace {

QWebEngineUrlRequestInfo::ResourceType toQt(content::ResourceType resourceType)
{
    if (resourceType >= 0 && resourceType < content::ResourceType(QWebEngineUrlRequestInfo::ResourceTypeLast))
        return static_cast<QWebEngineUrlRequestInfo::ResourceType>(resourceType);
    return QWebEngineUrlRequestInfo::ResourceTypeUnknown;
}

QWebEngineUrlRequestInfo::NavigationType toQt(WebContentsAdapterClient::NavigationType navigationType)
{
    return static_cast<QWebEngineUrlRequestInfo::NavigationType>(navigationType);
}

// Notifies WebContentsAdapterClient of a new URLRequest.
class URLRequestNotification {
public:
    URLRequestNotification(net::URLRequest *request,
                           const QUrl &url,
                           bool isMainFrameRequest,
                           int navigationType,
                           int frameTreeNodeId,
                           net::CompletionOnceCallback callback)
        : m_request(request)
        , m_url(url)
        , m_isMainFrameRequest(isMainFrameRequest)
        , m_navigationType(navigationType)
        , m_frameTreeNodeId(frameTreeNodeId)
        , m_callback(std::move(callback))
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

        m_request->SetUserData(UserData::key, std::make_unique<UserData>(this));

        content::BrowserThread::PostTask(
            content::BrowserThread::UI,
            FROM_HERE,
            base::Bind(&URLRequestNotification::notify, base::Unretained(this)));
    }

private:
    // Calls cancel() when the URLRequest is destroyed.
    class UserData : public base::SupportsUserData::Data {
    public:
        UserData(URLRequestNotification *ptr) : m_ptr(ptr) {}
        ~UserData() { m_ptr->cancel(); }
        static const char key[];
    private:
        URLRequestNotification *m_ptr;
    };

    void cancel()
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

        // May run concurrently with notify() but we only touch m_request here.

        m_request = nullptr;
    }

    void notify()
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

        // May run concurrently with cancel() so no peeking at m_request here.

        int error = net::OK;
        content::WebContents *webContents = content::WebContents::FromFrameTreeNodeId(m_frameTreeNodeId);
        if (webContents) {
            int navigationRequestAction = WebContentsAdapterClient::AcceptRequest;
            WebContentsAdapterClient *client =
                WebContentsViewQt::from(static_cast<content::WebContentsImpl*>(webContents)->GetView())->client();
            client->navigationRequested(m_navigationType,
                                        m_url,
                                        navigationRequestAction,
                                        m_isMainFrameRequest);
            error = net::ERR_FAILED;
            switch (static_cast<WebContentsAdapterClient::NavigationRequestAction>(navigationRequestAction)) {
            case WebContentsAdapterClient::AcceptRequest:
                error = net::OK;
                break;
            case WebContentsAdapterClient::IgnoreRequest:
                error = net::ERR_ABORTED;
                break;
            }
            DCHECK(error != net::ERR_FAILED);
        }

        // Run the callback on the IO thread.
        content::BrowserThread::PostTask(
            content::BrowserThread::IO,
            FROM_HERE,
            base::BindOnce(&URLRequestNotification::complete, base::Unretained(this), error));
    }

    void complete(int error)
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

        if (m_request) {
            if (m_request->status().status() != net::URLRequestStatus::CANCELED)
                std::move(m_callback).Run(error);
            m_request->RemoveUserData(UserData::key);
        }

        delete this;
    }

    ~URLRequestNotification() {}

    net::URLRequest *m_request;
    QUrl m_url;
    bool m_isMainFrameRequest;
    int m_navigationType;
    int m_frameTreeNodeId;
    net::CompletionOnceCallback m_callback;
};

const char URLRequestNotification::UserData::key[] = "QtWebEngineCore::URLRequestNotification";

} // namespace

NetworkDelegateQt::NetworkDelegateQt(ProfileIODataQt *data)
    : m_profileIOData(data)
{
}

int NetworkDelegateQt::OnBeforeURLRequest(net::URLRequest *request, net::CompletionOnceCallback callback, GURL *newUrl)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    Q_ASSERT(m_profileIOData);

    const content::ResourceRequestInfo *resourceInfo = content::ResourceRequestInfo::ForRequest(request);

    content::ResourceType resourceType = content::RESOURCE_TYPE_LAST_TYPE;
    WebContentsAdapterClient::NavigationType navigationType = WebContentsAdapterClient::OtherNavigation;

    if (resourceInfo) {
        resourceType = resourceInfo->GetResourceType();
        navigationType = pageTransitionToNavigationType(resourceInfo->GetPageTransition());
    }

    const QUrl qUrl = toQt(request->url());

    QUrl firstPartyUrl = QUrl();
    if (resourceType == content::ResourceType::RESOURCE_TYPE_SUB_FRAME)
        firstPartyUrl = toQt(request->first_party_url());
    else
        firstPartyUrl = toQt(request->site_for_cookies());

    QWebEngineUrlRequestInterceptor* interceptor = m_profileIOData->acquireInterceptor();
    if (interceptor) {
        QWebEngineUrlRequestInfoPrivate *infoPrivate = new QWebEngineUrlRequestInfoPrivate(toQt(resourceType),
                                                                                           toQt(navigationType),
                                                                                           qUrl,
                                                                                           firstPartyUrl,
                                                                                           QByteArray::fromStdString(request->method()));
        QWebEngineUrlRequestInfo requestInfo(infoPrivate);
        interceptor->interceptRequest(requestInfo);
        m_profileIOData->releaseInterceptor();
        if (requestInfo.changed()) {
            int result = infoPrivate->shouldBlockRequest ? net::ERR_BLOCKED_BY_CLIENT : net::OK;

            if (qUrl != infoPrivate->url)
                *newUrl = toGurl(infoPrivate->url);

            if (!infoPrivate->extraHeaders.isEmpty()) {
                auto end = infoPrivate->extraHeaders.constEnd();
                for (auto header = infoPrivate->extraHeaders.constBegin(); header != end; ++header) {
                    std::string h = header.key().toStdString();
                    if (base::LowerCaseEqualsASCII(h, "referer")) {
                        request->SetReferrer(header.value().toStdString());
                    } else {
                        request->SetExtraRequestHeaderByName(h, header.value().toStdString(), /* overwrite */ true);
                    }
                }
            }

            if (result != net::OK)
                return result;
        }
    } else {
        m_profileIOData->releaseInterceptor();
    }

    if (!resourceInfo)
        return net::OK;

    int frameTreeNodeId = resourceInfo->GetFrameTreeNodeId();
    // Only intercept MAIN_FRAME and SUB_FRAME with an associated render frame.
    if (!content::IsResourceTypeFrame(resourceType) || frameTreeNodeId == -1)
        return net::OK;

    new URLRequestNotification(
        request,
        qUrl,
        resourceInfo->IsMainFrame(),
        navigationType,
        frameTreeNodeId,
        std::move(callback)
    );

    // We'll run the callback after we notified the UI thread.
    return net::ERR_IO_PENDING;
}

void NetworkDelegateQt::OnURLRequestDestroyed(net::URLRequest*)
{
}

void NetworkDelegateQt::OnCompleted(net::URLRequest */*request*/, bool /*started*/, int /*net_error*/)
{
}

bool NetworkDelegateQt::OnCanSetCookie(const net::URLRequest& request,
                                       const net::CanonicalCookie & /*cookie*/,
                                       net::CookieOptions*)
{
    return canSetCookies(request.site_for_cookies(), request.url(), std::string());
}

bool NetworkDelegateQt::OnCanGetCookies(const net::URLRequest& request, const net::CookieList&)
{
    return canGetCookies(request.site_for_cookies(), request.url());
}

bool NetworkDelegateQt::OnCanEnablePrivacyMode(const GURL &url, const GURL &site_for_cookies) const
{
    return !canGetCookies(site_for_cookies, url);
}

bool NetworkDelegateQt::canSetCookies(const GURL &first_party, const GURL &url, const std::string &cookie_line) const
{
    Q_ASSERT(m_profileIOData);
    return m_profileIOData->canSetCookie(toQt(first_party), QByteArray::fromStdString(cookie_line), toQt(url));
}

bool NetworkDelegateQt::canGetCookies(const GURL &first_party, const GURL &url) const
{
    Q_ASSERT(m_profileIOData);
    return m_profileIOData->canGetCookies(toQt(first_party), toQt(url));
}

int NetworkDelegateQt::OnBeforeStartTransaction(net::URLRequest *, net::CompletionOnceCallback, net::HttpRequestHeaders *)
{
    return net::OK;
}

void NetworkDelegateQt::OnBeforeSendHeaders(net::URLRequest* request, const net::ProxyInfo& proxy_info,
                                            const net::ProxyRetryInfoMap& proxy_retry_info, net::HttpRequestHeaders* headers)
{
}

void NetworkDelegateQt::OnStartTransaction(net::URLRequest *request, const net::HttpRequestHeaders &headers)
{
}

int NetworkDelegateQt::OnHeadersReceived(net::URLRequest*, net::CompletionOnceCallback, const net::HttpResponseHeaders*, scoped_refptr<net::HttpResponseHeaders>*, GURL*)
{
    return net::OK;
}

void NetworkDelegateQt::OnBeforeRedirect(net::URLRequest*, const GURL&)
{
}

void NetworkDelegateQt::OnResponseStarted(net::URLRequest*, int)
{
}

void NetworkDelegateQt::OnNetworkBytesReceived(net::URLRequest*, int64_t)
{
}

void NetworkDelegateQt::OnNetworkBytesSent(net::URLRequest*, int64_t)
{
}

void NetworkDelegateQt::OnPACScriptError(int, const base::string16&)
{
}

net::NetworkDelegate::AuthRequiredResponse NetworkDelegateQt::OnAuthRequired(net::URLRequest*, const net::AuthChallengeInfo&, AuthCallback, net::AuthCredentials*)
{
    return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool NetworkDelegateQt::OnCanAccessFile(const net::URLRequest&, const base::FilePath&, const base::FilePath&) const
{
    return true;
}

bool NetworkDelegateQt::OnAreExperimentalCookieFeaturesEnabled() const
{
    return false;
}

bool NetworkDelegateQt::OnCancelURLRequestWithPolicyViolatingReferrerHeader(const net::URLRequest&, const GURL&, const GURL&) const
{
    return false;
}

bool NetworkDelegateQt::OnCanQueueReportingReport(const url::Origin& origin) const
{
    return false;
}

void NetworkDelegateQt::OnCanSendReportingReports(std::set<url::Origin> origins, base::OnceCallback<void(std::set<url::Origin>)> result_callback) const
{
    std::move(result_callback).Run(std::set<url::Origin>());
}

bool NetworkDelegateQt::OnCanSetReportingClient(const url::Origin& origin, const GURL& endpoint) const
{
    return false;
}

bool NetworkDelegateQt::OnCanUseReportingClient(const url::Origin& origin, const GURL& endpoint) const
{
    return false;
}

} // namespace QtWebEngineCore
