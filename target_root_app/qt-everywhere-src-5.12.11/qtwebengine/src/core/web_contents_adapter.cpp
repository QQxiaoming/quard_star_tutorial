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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "web_contents_adapter.h"

#include "browser_accessibility_qt.h"
#include "profile_adapter_client.h"
#include "profile_adapter.h"
#include "devtools_frontend_qt.h"
#include "download_manager_delegate_qt.h"
#include "media_capture_devices_dispatcher.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "printing/print_view_manager_qt.h"
#endif
#include "profile_qt.h"
#include "qwebenginecallback_p.h"
#include "render_view_observer_host_qt.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"
#include "web_engine_settings.h"

#include "base/command_line.h"
#include "base/run_loop.h"
#include "base/values.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_request_utils.h"
#include "content/public/browser/host_zoom_map.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/drop_data.h"
#include "content/public/common/page_state.h"
#include "content/public/common/page_zoom.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/web_preferences.h"
#include "content/public/common/webrtc_ip_handling_policy.h"
#include "third_party/blink/public/web/web_find_options.h"
#include "third_party/blink/public/web/web_media_player_action.h"
#include "printing/buildflags/buildflags.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/clipboard/custom_data_helper.h"
#include "ui/gfx/font_render_params.h"

#if QT_CONFIG(webengine_webchannel)
#include "renderer_host/web_channel_ipc_transport_host.h"
#include <QtWebChannel/QWebChannel>
#endif

#include <QDir>
#include <QGuiApplication>
#include <QPageLayout>
#include <QStringList>
#include <QStyleHints>
#include <QTimer>
#include <QVariant>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qmimedata.h>
#include <QtCore/qtemporarydir.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qdrag.h>
#include <QtGui/qpixmap.h>

namespace QtWebEngineCore {

#define CHECK_INITIALIZED(return_value)         \
    if (!isInitialized())                       \
        return return_value

#define CHECK_VALID_RENDER_WIDGET_HOST_VIEW(render_view_host) \
    if (!render_view_host->IsRenderViewLive() && render_view_host->GetWidget()->GetView()) { \
        qWarning("Ignore navigation due to terminated render process with invalid RenderWidgetHostView."); \
        return; \
    }

static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;
static const int kHistoryStreamVersion = 3;

static QVariant fromJSValue(const base::Value *result)
{
    QVariant ret;
    switch (result->type()) {
    case base::Value::Type::NONE:
        break;
    case base::Value::Type::BOOLEAN:
    {
        bool out;
        if (result->GetAsBoolean(&out))
            ret.setValue(out);
        break;
    }
    case base::Value::Type::INTEGER:
    {
        int out;
        if (result->GetAsInteger(&out))
            ret.setValue(out);
        break;
    }
    case base::Value::Type::DOUBLE:
    {
        double out;
        if (result->GetAsDouble(&out))
            ret.setValue(out);
        break;
    }
    case base::Value::Type::STRING:
    {
        base::string16 out;
        if (result->GetAsString(&out))
            ret.setValue(toQt(out));
        break;
    }
    case base::Value::Type::LIST:
    {
        const base::ListValue *out;
        if (result->GetAsList(&out)) {
            QVariantList list;
            list.reserve(out->GetSize());
            for (size_t i = 0; i < out->GetSize(); ++i) {
                const base::Value *outVal = 0;
                if (out->Get(i, &outVal) && outVal)
                    list.insert(i, fromJSValue(outVal));
            }
            ret.setValue(list);
        }
        break;
    }
    case base::Value::Type::DICTIONARY:
    {
        const base::DictionaryValue *out;
        if (result->GetAsDictionary(&out)) {
            QVariantMap map;
            base::DictionaryValue::Iterator it(*out);
            while (!it.IsAtEnd()) {
                map.insert(toQt(it.key()), fromJSValue(&it.value()));
                it.Advance();
            }
            ret.setValue(map);
        }
        break;
    }
    case base::Value::Type::BINARY:
    {
        QByteArray data(result->GetBlob().data(), result->GetBlob().size());
        ret.setValue(data);
        break;
    }
    default:
        Q_UNREACHABLE();
        break;
    }
    return ret;
}

static void callbackOnEvaluateJS(WebContentsAdapterClient *adapterClient, quint64 requestId, const base::Value *result)
{
    if (requestId)
        adapterClient->didRunJavaScript(requestId, fromJSValue(result));
}

#if QT_CONFIG(webengine_printing_and_pdf)
static void callbackOnPrintingFinished(WebContentsAdapterClient *adapterClient,
                                       int requestId,
                                       const std::vector<char>& result)
{
    if (requestId)
        adapterClient->didPrintPage(requestId, QByteArray(result.data(), result.size()));
}

static void callbackOnPdfSavingFinished(WebContentsAdapterClient *adapterClient,
                                        const QString& filePath,
                                        bool success)
{
    adapterClient->didPrintPageToPdf(filePath, success);
}
#endif

static std::unique_ptr<content::WebContents> createBlankWebContents(WebContentsAdapterClient *adapterClient, content::BrowserContext *browserContext)
{
    content::WebContents::CreateParams create_params(browserContext, NULL);
    create_params.routing_id = MSG_ROUTING_NONE;
    create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
    create_params.context = reinterpret_cast<gfx::NativeView>(adapterClient);
    create_params.initially_hidden = true;
    return content::WebContents::Create(create_params);
}

static void serializeNavigationHistory(const content::NavigationController &controller, QDataStream &output)
{
    const int currentIndex = controller.GetCurrentEntryIndex();
    const int count = controller.GetEntryCount();
    const int pendingIndex = controller.GetPendingEntryIndex();

    output << kHistoryStreamVersion;
    output << count;
    output << currentIndex;

    // Logic taken from SerializedNavigationEntry::WriteToPickle.
    for (int i = 0; i < count; ++i) {
        const content::NavigationEntry* entry = (i == pendingIndex)
            ? controller.GetPendingEntry()
            : controller.GetEntryAtIndex(i);
        if (entry->GetVirtualURL().is_valid()) {
            if (entry->GetHasPostData())
                entry->GetPageState().RemovePasswordData();
            std::string encodedPageState = entry->GetPageState().ToEncodedData();
            output << toQt(entry->GetVirtualURL());
            output << toQt(entry->GetTitle());
            output << QByteArray(encodedPageState.data(), encodedPageState.size());
            output << static_cast<qint32>(entry->GetTransitionType());
            output << entry->GetHasPostData();
            output << toQt(entry->GetReferrer().url);
            output << static_cast<qint32>(entry->GetReferrer().policy);
            output << toQt(entry->GetOriginalRequestURL());
            output << entry->GetIsOverridingUserAgent();
            output << static_cast<qint64>(entry->GetTimestamp().ToInternalValue());
            output << entry->GetHttpStatusCode();
        }
    }
}

static void deserializeNavigationHistory(QDataStream &input, int *currentIndex, std::vector<std::unique_ptr<content::NavigationEntry>> *entries, content::BrowserContext *browserContext)
{
    int version;
    input >> version;
    if (version != kHistoryStreamVersion) {
        // We do not try to decode previous history stream versions.
        // Make sure that our history is cleared and mark the rest of the stream as invalid.
        input.setStatus(QDataStream::ReadCorruptData);
        *currentIndex = -1;
        return;
    }

    int count;
    input >> count >> *currentIndex;

    entries->reserve(count);
    // Logic taken from SerializedNavigationEntry::ReadFromPickle and ToNavigationEntries.
    for (int i = 0; i < count; ++i) {
        QUrl virtualUrl, referrerUrl, originalRequestUrl;
        QString title;
        QByteArray pageState;
        qint32 transitionType, referrerPolicy;
        bool hasPostData, isOverridingUserAgent;
        qint64 timestamp;
        int httpStatusCode;
        input >> virtualUrl;
        input >> title;
        input >> pageState;
        input >> transitionType;
        input >> hasPostData;
        input >> referrerUrl;
        input >> referrerPolicy;
        input >> originalRequestUrl;
        input >> isOverridingUserAgent;
        input >> timestamp;
        input >> httpStatusCode;

        // If we couldn't unpack the entry successfully, abort everything.
        if (input.status() != QDataStream::Ok) {
            *currentIndex = -1;
            auto it = entries->begin();
            auto end = entries->end();
            for (; it != end; ++it)
                it->reset();
            entries->clear();
            return;
        }

        std::unique_ptr<content::NavigationEntry> entry = content::NavigationController::CreateNavigationEntry(
            toGurl(virtualUrl),
            content::Referrer(toGurl(referrerUrl), static_cast<blink::WebReferrerPolicy>(referrerPolicy)),
            // Use a transition type of reload so that we don't incorrectly
            // increase the typed count.
            ui::PAGE_TRANSITION_RELOAD,
            false,
            // The extra headers are not sync'ed across sessions.
            std::string(),
            browserContext,
            nullptr);

        entry->SetTitle(toString16(title));
        entry->SetPageState(content::PageState::CreateFromEncodedData(std::string(pageState.data(), pageState.size())));
        entry->SetHasPostData(hasPostData);
        entry->SetOriginalRequestURL(toGurl(originalRequestUrl));
        entry->SetIsOverridingUserAgent(isOverridingUserAgent);
        entry->SetTimestamp(base::Time::FromInternalValue(timestamp));
        entry->SetHttpStatusCode(httpStatusCode);
        entries->push_back(std::move(entry));
    }
}

namespace {
static QList<WebContentsAdapter *> recursive_guard_loading_adapters;

class LoadRecursionGuard {
    public:
    static bool isGuarded(WebContentsAdapter *adapter)
    {
        return recursive_guard_loading_adapters.contains(adapter);
    }
    LoadRecursionGuard(WebContentsAdapter *adapter)
        : m_adapter(adapter)
    {
        recursive_guard_loading_adapters.append(adapter);
    }

    ~LoadRecursionGuard() {
        recursive_guard_loading_adapters.removeOne(m_adapter);
    }

    private:
        WebContentsAdapter *m_adapter;
};
} // Anonymous namespace

QSharedPointer<WebContentsAdapter> WebContentsAdapter::createFromSerializedNavigationHistory(QDataStream &input, WebContentsAdapterClient *adapterClient)
{
    int currentIndex;
    std::vector<std::unique_ptr<content::NavigationEntry>> entries;
    deserializeNavigationHistory(input, &currentIndex, &entries, adapterClient->profileAdapter()->profile());

    if (currentIndex == -1)
        return QSharedPointer<WebContentsAdapter>();

    // Unlike WebCore, Chromium only supports Restoring to a new WebContents instance.
    std::unique_ptr<content::WebContents> newWebContents = createBlankWebContents(adapterClient, adapterClient->profileAdapter()->profile());
    content::NavigationController &controller = newWebContents->GetController();
    controller.Restore(currentIndex, content::RestoreType::LAST_SESSION_EXITED_CLEANLY, &entries);

    if (controller.GetActiveEntry()) {
        // Set up the file access rights for the selected navigation entry.
        // TODO(joth): This is duplicated from chrome/.../session_restore.cc and
        // should be shared e.g. in  NavigationController. http://crbug.com/68222
        const int id = newWebContents->GetMainFrame()->GetProcess()->GetID();
        const content::PageState& pageState = controller.GetActiveEntry()->GetPageState();
        const std::vector<base::FilePath>& filePaths = pageState.GetReferencedFiles();
        for (std::vector<base::FilePath>::const_iterator file = filePaths.begin(); file != filePaths.end(); ++file)
            content::ChildProcessSecurityPolicy::GetInstance()->GrantReadFile(id, *file);
    }

    return QSharedPointer<WebContentsAdapter>::create(std::move(newWebContents));
}

WebContentsAdapter::WebContentsAdapter()
  : m_profileAdapter(nullptr)
  , m_webContents(nullptr)
#if QT_CONFIG(webengine_webchannel)
  , m_webChannel(nullptr)
  , m_webChannelWorld(0)
#endif
  , m_adapterClient(nullptr)
  , m_nextRequestId(CallbackDirectory::ReservedCallbackIdsEnd)
  , m_lastFindRequestId(0)
  , m_currentDropAction(blink::kWebDragOperationNone)
  , m_devToolsFrontend(nullptr)
{
    // This has to be the first thing we create, and the last we destroy.
    WebEngineContext::current();
}

WebContentsAdapter::WebContentsAdapter(std::unique_ptr<content::WebContents> webContents)
  : m_profileAdapter(nullptr)
  , m_webContents(std::move(webContents))
#if QT_CONFIG(webengine_webchannel)
  , m_webChannel(nullptr)
  , m_webChannelWorld(0)
#endif
  , m_adapterClient(nullptr)
  , m_nextRequestId(CallbackDirectory::ReservedCallbackIdsEnd)
  , m_lastFindRequestId(0)
  , m_currentDropAction(blink::kWebDragOperationNone)
  , m_devToolsFrontend(nullptr)
{
    // This has to be the first thing we create, and the last we destroy.
    WebEngineContext::current();
}

WebContentsAdapter::~WebContentsAdapter()
{
    if (m_devToolsFrontend)
        closeDevToolsFrontend();
    Q_ASSERT(!m_devToolsFrontend);
}

void WebContentsAdapter::setClient(WebContentsAdapterClient *adapterClient)
{
    Q_ASSERT(!isInitialized());
    m_adapterClient = adapterClient;
    m_profileAdapter = adapterClient->profileAdapter();
    Q_ASSERT(m_profileAdapter);

    // This might replace any adapter that has been initialized with this WebEngineSettings.
    adapterClient->webEngineSettings()->setWebContentsAdapter(this);
}

bool WebContentsAdapter::isInitialized() const
{
    return (bool)m_webContentsDelegate;
}

void WebContentsAdapter::initialize(content::SiteInstance *site)
{
    Q_ASSERT(m_adapterClient);
    Q_ASSERT(!isInitialized());

    // Create our own if a WebContents wasn't provided at construction.
    if (!m_webContents) {
        content::WebContents::CreateParams create_params(m_profileAdapter->profile(), site);
        create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
        create_params.context = reinterpret_cast<gfx::NativeView>(m_adapterClient);
        create_params.initially_hidden = true;
        m_webContents = content::WebContents::Create(create_params);
    }

    content::RendererPreferences* rendererPrefs = m_webContents->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    const int qtCursorFlashTime = QGuiApplication::styleHints()->cursorFlashTime();
    rendererPrefs->caret_blink_interval = base::TimeDelta::FromMillisecondsD(0.5 * static_cast<double>(qtCursorFlashTime));
    rendererPrefs->user_agent_override = m_profileAdapter->httpUserAgent().toStdString();
    rendererPrefs->accept_languages = m_profileAdapter->httpAcceptLanguageWithoutQualities().toStdString();
#if QT_CONFIG(webengine_webrtc)
    base::CommandLine* commandLine = base::CommandLine::ForCurrentProcess();
    if (commandLine->HasSwitch(switches::kForceWebRtcIPHandlingPolicy))
        rendererPrefs->webrtc_ip_handling_policy = commandLine->GetSwitchValueASCII(switches::kForceWebRtcIPHandlingPolicy);
    else
        rendererPrefs->webrtc_ip_handling_policy = m_adapterClient->webEngineSettings()->testAttribute(WebEngineSettings::WebRTCPublicInterfacesOnly)
                                                    ? content::kWebRTCIPHandlingDefaultPublicInterfaceOnly
                                                    : content::kWebRTCIPHandlingDefault;
#endif
    // Set web-contents font settings to the default font settings as Chromium constantly overrides
    // the global font defaults with the font settings of the latest web-contents created.
    CR_DEFINE_STATIC_LOCAL(const gfx::FontRenderParams, params, (gfx::GetFontRenderParams(gfx::FontRenderParamsQuery(), NULL)));
    rendererPrefs->should_antialias_text = params.antialiasing;
    rendererPrefs->use_subpixel_positioning = params.subpixel_positioning;
    rendererPrefs->hinting = params.hinting;
    rendererPrefs->use_autohinter = params.autohinter;
    rendererPrefs->use_bitmaps = params.use_bitmaps;
    rendererPrefs->subpixel_rendering = params.subpixel_rendering;
    m_webContents->GetRenderViewHost()->SyncRendererPrefs();

    // Create and attach observers to the WebContents.
    m_webContentsDelegate.reset(new WebContentsDelegateQt(m_webContents.get(), m_adapterClient));
    m_renderViewObserverHost.reset(new RenderViewObserverHostQt(m_webContents.get(), m_adapterClient));

    // Let the WebContent's view know about the WebContentsAdapterClient.
    WebContentsViewQt* contentsView = static_cast<WebContentsViewQt*>(static_cast<content::WebContentsImpl*>(m_webContents.get())->GetView());
    contentsView->initialize(m_adapterClient);

    // This should only be necessary after having restored the history to a new WebContentsAdapter.
    m_webContents->GetController().LoadIfNecessary();

#if QT_CONFIG(webengine_printing_and_pdf)
    PrintViewManagerQt::CreateForWebContents(webContents());
#endif

    // Create an instance of WebEngineVisitedLinksManager to catch the first
    // content::NOTIFICATION_RENDERER_PROCESS_CREATED event. This event will
    // force to initialize visited links in VisitedLinkSlave.
    // It must be done before creating a RenderView.
    m_profileAdapter->visitedLinksManager();

    // Create a RenderView with the initial empty document
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    Q_ASSERT(rvh);
    if (!rvh->IsRenderViewLive())
        static_cast<content::WebContentsImpl*>(m_webContents.get())->CreateRenderViewForRenderManager(rvh, MSG_ROUTING_NONE, MSG_ROUTING_NONE, base::UnguessableToken::Create(), content::FrameReplicationState());

    m_webContentsDelegate->RenderViewHostChanged(nullptr, rvh);

    m_adapterClient->initializationFinished();
}

bool WebContentsAdapter::canGoBack() const
{
    CHECK_INITIALIZED(false);
    return m_webContents->GetController().CanGoBack();
}

bool WebContentsAdapter::canGoForward() const
{
    CHECK_INITIALIZED(false);
    return m_webContents->GetController().CanGoForward();
}

void WebContentsAdapter::stop()
{
    CHECK_INITIALIZED();
    content::NavigationController& controller = m_webContents->GetController();

    int index = controller.GetPendingEntryIndex();
    if (index != -1)
        controller.RemoveEntryAtIndex(index);

    m_webContents->Stop();
    focusIfNecessary();
}

void WebContentsAdapter::reload()
{
    CHECK_INITIALIZED();
    CHECK_VALID_RENDER_WIDGET_HOST_VIEW(m_webContents->GetRenderViewHost());
    m_webContents->GetController().Reload(content::ReloadType::NORMAL, /*checkRepost = */false);
    focusIfNecessary();
}

void WebContentsAdapter::reloadAndBypassCache()
{
    CHECK_INITIALIZED();
    CHECK_VALID_RENDER_WIDGET_HOST_VIEW(m_webContents->GetRenderViewHost());
    m_webContents->GetController().Reload(content::ReloadType::BYPASSING_CACHE, /*checkRepost = */false);
    focusIfNecessary();
}

void WebContentsAdapter::loadDefault()
{
    Q_ASSERT(!isInitialized());
    initialize(nullptr);
}

void WebContentsAdapter::load(const QUrl &url)
{
    QWebEngineHttpRequest request(url);
    load(request);
}

void WebContentsAdapter::load(const QWebEngineHttpRequest &request)
{
    GURL gurl = toGurl(request.url());
    if (!isInitialized()) {
        scoped_refptr<content::SiteInstance> site =
            content::SiteInstance::CreateForURL(m_profileAdapter->profile(), gurl);
        initialize(site.get());
    }

    CHECK_VALID_RENDER_WIDGET_HOST_VIEW(m_webContents->GetRenderViewHost());

    // The situation can occur when relying on the editingFinished signal in QML to set the url
    // of the WebView.
    // When enter is pressed, onEditingFinished fires and the url of the webview is set, which
    // calls into this and focuses the webview, taking the focus from the TextField/TextInput,
    // which in turn leads to editingFinished firing again. This scenario would cause a crash
    // down the line when unwinding as the first RenderWidgetHostViewQtDelegateQuick instance is
    // a dangling pointer by that time.

    if (LoadRecursionGuard::isGuarded(this))
        return;
    LoadRecursionGuard guard(this);
    Q_UNUSED(guard);

    // Add URL scheme if missing from view-source URL.
    if (request.url().scheme() == content::kViewSourceScheme) {
        QUrl pageUrl = QUrl(request.url().toString().remove(0,
                                                           strlen(content::kViewSourceScheme) + 1));
        if (pageUrl.scheme().isEmpty()) {
            QUrl extendedUrl = QUrl::fromUserInput(pageUrl.toString());
            extendedUrl = QUrl(QString("%1:%2").arg(content::kViewSourceScheme,
                                                    extendedUrl.toString()));
            gurl = toGurl(extendedUrl);
        }
    }

    content::NavigationController::LoadURLParams params(gurl);
    params.transition_type = ui::PageTransitionFromInt(ui::PAGE_TRANSITION_TYPED
                                                     | ui::PAGE_TRANSITION_FROM_ADDRESS_BAR);
    params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;

    switch (request.method()) {
    case QWebEngineHttpRequest::Get:
        params.load_type = content::NavigationController::LOAD_TYPE_DEFAULT;
        break;

    case QWebEngineHttpRequest::Post:
        params.load_type = content::NavigationController::LOAD_TYPE_HTTP_POST;
        // chromium accepts LOAD_TYPE_HTTP_POST only for the HTTP and HTTPS protocols
        if (!params.url.SchemeIsHTTPOrHTTPS()) {
            m_adapterClient->loadFinished(false, request.url(), false,
                                           net::ERR_DISALLOWED_URL_SCHEME,
                                           QCoreApplication::translate("WebContentsAdapter",
                                           "HTTP-POST data can only be sent over HTTP(S) protocol"));
            return;
        }

        params.post_data = network::ResourceRequestBody::CreateFromBytes(
                    (const char*)request.postData().constData(),
                    request.postData().length());
        break;
    }

    // convert the custom headers into the format that chromium expects
    QVector<QByteArray> headers = request.headers();
    for (QVector<QByteArray>::const_iterator it = headers.cbegin(); it != headers.cend(); ++it) {
        if (params.extra_headers.length() > 0)
            params.extra_headers += '\n';
        params.extra_headers += (*it).toStdString() + ": " + request.header(*it).toStdString();
    }

    bool resizeNeeded = false;
    if (request.url().hasFragment()) {
        if (content::RenderWidgetHostView *rwhv = webContents()->GetRenderWidgetHostView()) {
            const gfx::Size &viewportSize = rwhv->GetVisibleViewportSize();
            resizeNeeded = (viewportSize.width() == 0 || viewportSize.height() == 0);
        }
    }

    auto navigate = [](QWeakPointer<WebContentsAdapter> weakAdapter, const content::NavigationController::LoadURLParams &params) {
        WebContentsAdapter *adapter = weakAdapter.data();
        if (!adapter)
            return;
        adapter->webContents()->GetController().LoadURLWithParams(params);
        // Follow chrome::Navigate and invalidate the URL immediately.
        adapter->m_webContentsDelegate->NavigationStateChanged(adapter->webContents(), content::INVALIDATE_TYPE_URL);
        adapter->focusIfNecessary();
    };

    QWeakPointer<WebContentsAdapter> weakThis(sharedFromThis());
    if (resizeNeeded) {
        // Schedule navigation on the event loop.
        content::BrowserThread::PostTask(
            content::BrowserThread::UI, FROM_HERE, base::BindOnce(navigate, std::move(weakThis), std::move(params)));
    } else {
        navigate(std::move(weakThis), params);
    }
}

void WebContentsAdapter::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
    if (!isInitialized())
        loadDefault();

    CHECK_VALID_RENDER_WIDGET_HOST_VIEW(m_webContents->GetRenderViewHost());

    QByteArray encodedData = data.toPercentEncoding();
    std::string urlString;
    if (!mimeType.isEmpty())
        urlString = std::string("data:") + mimeType.toStdString() + std::string(",");
    else
        urlString = std::string("data:text/plain;charset=US-ASCII,");
    urlString.append(encodedData.constData(), encodedData.length());

    GURL dataUrlToLoad(urlString);
    if (dataUrlToLoad.spec().size() > url::kMaxURLChars) {
        m_adapterClient->loadFinished(false, baseUrl, false, net::ERR_ABORTED);
        return;
    }
    content::NavigationController::LoadURLParams params((dataUrlToLoad));
    params.load_type = content::NavigationController::LOAD_TYPE_DATA;
    params.base_url_for_data_url = toGurl(baseUrl);
    params.virtual_url_for_data_url = baseUrl.isEmpty() ? GURL(url::kAboutBlankURL) : toGurl(baseUrl);
    params.can_load_local_resources = true;
    params.transition_type = ui::PageTransitionFromInt(ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_FROM_API);
    params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;
    m_webContents->GetController().LoadURLWithParams(params);
    focusIfNecessary();
    m_webContents->CollapseSelection();
}

void WebContentsAdapter::save(const QString &filePath, int savePageFormat)
{
    CHECK_INITIALIZED();
    m_webContentsDelegate->setSavePageInfo(SavePageInfo(filePath, savePageFormat));
    m_webContents->OnSavePage();
}

QUrl WebContentsAdapter::activeUrl() const
{
    CHECK_INITIALIZED(QUrl());
    return m_webContentsDelegate->url();
}

QUrl WebContentsAdapter::requestedUrl() const
{
    CHECK_INITIALIZED(QUrl());
    content::NavigationEntry* entry = m_webContents->GetController().GetVisibleEntry();
    content::NavigationEntry* pendingEntry = m_webContents->GetController().GetPendingEntry();

    if (entry) {
        if (!entry->GetOriginalRequestURL().is_empty())
            return toQt(entry->GetOriginalRequestURL());

        if (pendingEntry && pendingEntry == entry)
            return toQt(entry->GetURL());
    }
    return QUrl();
}

QUrl WebContentsAdapter::iconUrl() const
{
    CHECK_INITIALIZED(QUrl());
    if (content::NavigationEntry* entry = m_webContents->GetController().GetVisibleEntry()) {
        content::FaviconStatus favicon = entry->GetFavicon();
        if (favicon.valid)
            return toQt(favicon.url);
    }
    return QUrl();
}

QString WebContentsAdapter::pageTitle() const
{
    CHECK_INITIALIZED(QString());
    return m_webContentsDelegate->title();
}

QString WebContentsAdapter::selectedText() const
{
    CHECK_INITIALIZED(QString());
    if (auto *rwhv = m_webContents->GetRenderWidgetHostView())
        return toQt(rwhv->GetSelectedText());
    return QString();
}

void WebContentsAdapter::undo()
{
    CHECK_INITIALIZED();
    m_webContents->Undo();
}

void WebContentsAdapter::redo()
{
    CHECK_INITIALIZED();
    m_webContents->Redo();
}

void WebContentsAdapter::cut()
{
    CHECK_INITIALIZED();
    m_webContents->Cut();
}

void WebContentsAdapter::copy()
{
    CHECK_INITIALIZED();
    m_webContents->Copy();
}

void WebContentsAdapter::paste()
{
    CHECK_INITIALIZED();
    m_webContents->Paste();
}

void WebContentsAdapter::pasteAndMatchStyle()
{
    CHECK_INITIALIZED();
    m_webContents->PasteAndMatchStyle();
}

void WebContentsAdapter::selectAll()
{
    CHECK_INITIALIZED();
    m_webContents->SelectAll();
}

void WebContentsAdapter::requestClose()
{
    CHECK_INITIALIZED();
    m_webContents->DispatchBeforeUnload();
}

void WebContentsAdapter::unselect()
{
    CHECK_INITIALIZED();
    m_webContents->CollapseSelection();
}

void WebContentsAdapter::navigateToIndex(int offset)
{
    CHECK_INITIALIZED();
    CHECK_VALID_RENDER_WIDGET_HOST_VIEW(m_webContents->GetRenderViewHost());
    m_webContents->GetController().GoToIndex(offset);
    focusIfNecessary();
}

void WebContentsAdapter::navigateToOffset(int offset)
{
    CHECK_INITIALIZED();
    CHECK_VALID_RENDER_WIDGET_HOST_VIEW(m_webContents->GetRenderViewHost());
    m_webContents->GetController().GoToOffset(offset);
    focusIfNecessary();
}

int WebContentsAdapter::navigationEntryCount()
{
    CHECK_INITIALIZED(0);
    return m_webContents->GetController().GetEntryCount();
}

int WebContentsAdapter::currentNavigationEntryIndex()
{
    CHECK_INITIALIZED(0);
    return m_webContents->GetController().GetCurrentEntryIndex();
}

QUrl WebContentsAdapter::getNavigationEntryOriginalUrl(int index)
{
    CHECK_INITIALIZED(QUrl());
    content::NavigationEntry *entry = m_webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetOriginalRequestURL()) : QUrl();
}

QUrl WebContentsAdapter::getNavigationEntryUrl(int index)
{
    CHECK_INITIALIZED(QUrl());
    content::NavigationEntry *entry = m_webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetURL()) : QUrl();
}

QString WebContentsAdapter::getNavigationEntryTitle(int index)
{
    CHECK_INITIALIZED(QString());
    content::NavigationEntry *entry = m_webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetTitle()) : QString();
}

QDateTime WebContentsAdapter::getNavigationEntryTimestamp(int index)
{
    CHECK_INITIALIZED(QDateTime());
    content::NavigationEntry *entry = m_webContents->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetTimestamp()) : QDateTime();
}

QUrl WebContentsAdapter::getNavigationEntryIconUrl(int index)
{
    CHECK_INITIALIZED(QUrl());
    content::NavigationEntry *entry = m_webContents->GetController().GetEntryAtIndex(index);
    if (!entry)
        return QUrl();
    content::FaviconStatus favicon = entry->GetFavicon();
    return favicon.valid ? toQt(favicon.url) : QUrl();
}

void WebContentsAdapter::clearNavigationHistory()
{
    CHECK_INITIALIZED();
    if (m_webContents->GetController().CanPruneAllButLastCommitted())
        m_webContents->GetController().PruneAllButLastCommitted();
}

void WebContentsAdapter::serializeNavigationHistory(QDataStream &output)
{
    CHECK_INITIALIZED();
    QtWebEngineCore::serializeNavigationHistory(m_webContents->GetController(), output);
}

void WebContentsAdapter::setZoomFactor(qreal factor)
{
    CHECK_INITIALIZED();
    if (factor < content::kMinimumZoomFactor || factor > content::kMaximumZoomFactor)
        return;

    double zoomLevel = content::ZoomFactorToZoomLevel(static_cast<double>(factor));
    content::HostZoomMap *zoomMap = content::HostZoomMap::GetForWebContents(m_webContents.get());

    if (zoomMap) {
        int render_process_id = m_webContents->GetMainFrame()->GetProcess()->GetID();
        int render_view_id = m_webContents->GetRenderViewHost()->GetRoutingID();
        zoomMap->SetTemporaryZoomLevel(render_process_id, render_view_id, zoomLevel);
    }
}

qreal WebContentsAdapter::currentZoomFactor() const
{
    CHECK_INITIALIZED(1);
    return content::ZoomLevelToZoomFactor(content::HostZoomMap::GetZoomLevel(m_webContents.get()));
}

ProfileQt* WebContentsAdapter::profile()
{
    return m_profileAdapter ? m_profileAdapter->profile() : m_webContents ?
                                         static_cast<ProfileQt*>(m_webContents->GetBrowserContext()) : nullptr;
}

ProfileAdapter* WebContentsAdapter::profileAdapter()
{
    return m_profileAdapter ? m_profileAdapter : m_webContents ?
                    static_cast<ProfileQt*>(m_webContents->GetBrowserContext())->profileAdapter() : nullptr;
}

#ifndef QT_NO_ACCESSIBILITY
QAccessibleInterface *WebContentsAdapter::browserAccessible()
{
    CHECK_INITIALIZED(nullptr);
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    Q_ASSERT(rvh);
    content::BrowserAccessibilityManager *manager = static_cast<content::RenderFrameHostImpl*>(rvh->GetMainFrame())->GetOrCreateBrowserAccessibilityManager();
    if (!manager) // FIXME!
        return nullptr;
    content::BrowserAccessibility *acc = manager->GetRoot();
    content::BrowserAccessibilityQt *accQt = static_cast<content::BrowserAccessibilityQt*>(acc);
    return accQt;
}
#endif // QT_NO_ACCESSIBILITY

void WebContentsAdapter::runJavaScript(const QString &javaScript, quint32 worldId)
{
    CHECK_INITIALIZED();
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    Q_ASSERT(rvh);
    if (worldId == 0) {
        rvh->GetMainFrame()->ExecuteJavaScript(toString16(javaScript));
        return;
    }

    content::RenderFrameHost::JavaScriptResultCallback callback = base::Bind(&callbackOnEvaluateJS, m_adapterClient, CallbackDirectory::NoCallbackId);
    rvh->GetMainFrame()->ExecuteJavaScriptInIsolatedWorld(toString16(javaScript), callback, worldId);
}

quint64 WebContentsAdapter::runJavaScriptCallbackResult(const QString &javaScript, quint32 worldId)
{
    CHECK_INITIALIZED(0);
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    Q_ASSERT(rvh);
    content::RenderFrameHost::JavaScriptResultCallback callback = base::Bind(&callbackOnEvaluateJS, m_adapterClient, m_nextRequestId);
    if (worldId == 0)
        rvh->GetMainFrame()->ExecuteJavaScript(toString16(javaScript), callback);
    else
        rvh->GetMainFrame()->ExecuteJavaScriptInIsolatedWorld(toString16(javaScript), callback, worldId);
    return m_nextRequestId++;
}

quint64 WebContentsAdapter::fetchDocumentMarkup()
{
    CHECK_INITIALIZED(0);
    m_renderViewObserverHost->fetchDocumentMarkup(m_nextRequestId);
    return m_nextRequestId++;
}

quint64 WebContentsAdapter::fetchDocumentInnerText()
{
    CHECK_INITIALIZED(0);
    m_renderViewObserverHost->fetchDocumentInnerText(m_nextRequestId);
    return m_nextRequestId++;
}

quint64 WebContentsAdapter::findText(const QString &subString, bool caseSensitively, bool findBackward)
{
    CHECK_INITIALIZED(0);
    if (m_lastFindRequestId > m_webContentsDelegate->lastReceivedFindReply()) {
        // There are cases where the render process will overwrite a previous request
        // with the new search and we'll have a dangling callback, leaving the application
        // waiting for it forever.
        // Assume that any unfinished find has been unsuccessful when a new one is started
        // to cover that case.
        m_webContentsDelegate->setLastReceivedFindReply(m_lastFindRequestId);
        m_adapterClient->didFindText(m_lastFindRequestId, 0);
    }

    blink::WebFindOptions options;
    options.forward = !findBackward;
    options.match_case = caseSensitively;
    options.find_next = subString == m_webContentsDelegate->lastSearchedString();
    m_webContentsDelegate->setLastSearchedString(subString);

    // Find already allows a request ID as input, but only as an int.
    // Use the same counter but mod it to MAX_INT, this keeps the same likeliness of request ID clashing.
    int shrunkRequestId = m_nextRequestId++ & 0x7fffffff;
    m_webContents->Find(shrunkRequestId, toString16(subString), options);
    m_lastFindRequestId = shrunkRequestId;
    return shrunkRequestId;
}

void WebContentsAdapter::stopFinding()
{
    CHECK_INITIALIZED();
    m_webContentsDelegate->setLastReceivedFindReply(m_lastFindRequestId);
    m_webContentsDelegate->setLastSearchedString(QString());
    m_webContents->StopFinding(content::STOP_FIND_ACTION_KEEP_SELECTION);
}

void WebContentsAdapter::updateWebPreferences(const content::WebPreferences & webPreferences)
{
    CHECK_INITIALIZED();
    m_webContents->GetRenderViewHost()->UpdateWebkitPreferences(webPreferences);

    // In case of updating preferences during navigation, there might be a pending RVH what will
    // be active on successful navigation.
    content::RenderFrameHost *pendingRFH = (static_cast<content::WebContentsImpl*>(m_webContents.get()))->GetPendingMainFrame();
    if (pendingRFH) {
        content::RenderViewHost *pendingRVH = pendingRFH->GetRenderViewHost();
        Q_ASSERT(pendingRVH);
        pendingRVH->UpdateWebkitPreferences(webPreferences);
    }
}

void WebContentsAdapter::download(const QUrl &url, const QString &suggestedFileName,
                                  const QUrl &referrerUrl,
                                  ReferrerPolicy referrerPolicy)
{
    CHECK_INITIALIZED();
    content::BrowserContext *bctx = m_webContents->GetBrowserContext();
    content::DownloadManager *dlm =  content::BrowserContext::GetDownloadManager(bctx);
    DownloadManagerDelegateQt *dlmd = m_profileAdapter->downloadManagerDelegate();

    if (!dlm)
        return;

    dlmd->markNextDownloadAsUserRequested();
    dlm->SetDelegate(dlmd);

    net::NetworkTrafficAnnotationTag traffic_annotation =
        net::DefineNetworkTrafficAnnotation(
            "WebContentsAdapter::download", R"(
            semantics {
              sender: "User"
              description:
                "User requested download"
              trigger: "User."
              data: "Anything."
              destination: OTHER
            }
            policy {
              cookies_allowed: YES
              cookies_store: "user"
              setting:
                "It's possible not to use this feature."
            })");
    GURL gurl = toGurl(url);
    std::unique_ptr<download::DownloadUrlParameters> params(
        content::DownloadRequestUtils::CreateDownloadForWebContentsMainFrame(webContents(), gurl, traffic_annotation));

    params->set_suggested_name(toString16(suggestedFileName));

    // referrer logic based on chrome/browser/renderer_context_menu/render_view_context_menu.cc:
    content::Referrer referrer = content::Referrer::SanitizeForRequest(
                gurl,
                content::Referrer(toGurl(referrerUrl).GetAsReferrer(),
                                  static_cast<blink::WebReferrerPolicy>(referrerPolicy)));

    params->set_referrer(referrer.url);
    params->set_referrer_policy(content::Referrer::ReferrerPolicyForUrlRequest(referrer.policy));

    dlm->DownloadUrl(std::move(params));
}

bool WebContentsAdapter::isAudioMuted() const
{
    CHECK_INITIALIZED(false);
    return m_webContents->IsAudioMuted();
}

void WebContentsAdapter::setAudioMuted(bool muted)
{
    CHECK_INITIALIZED();
    m_webContents->SetAudioMuted(muted);
}

bool WebContentsAdapter::recentlyAudible()
{
    CHECK_INITIALIZED(false);
    return m_webContents->IsCurrentlyAudible();
}

void WebContentsAdapter::copyImageAt(const QPoint &location)
{
    CHECK_INITIALIZED();
    m_webContents->GetRenderViewHost()->GetMainFrame()->CopyImageAt(location.x(), location.y());
}

ASSERT_ENUMS_MATCH(WebContentsAdapter::MediaPlayerNoAction, blink::WebMediaPlayerAction::kUnknown)
ASSERT_ENUMS_MATCH(WebContentsAdapter::MediaPlayerPlay, blink::WebMediaPlayerAction::kPlay)
ASSERT_ENUMS_MATCH(WebContentsAdapter::MediaPlayerMute, blink::WebMediaPlayerAction::kMute)
ASSERT_ENUMS_MATCH(WebContentsAdapter::MediaPlayerLoop,  blink::WebMediaPlayerAction::kLoop)
ASSERT_ENUMS_MATCH(WebContentsAdapter::MediaPlayerControls,  blink::WebMediaPlayerAction::kControls)

void WebContentsAdapter::executeMediaPlayerActionAt(const QPoint &location, MediaPlayerAction action, bool enable)
{
    CHECK_INITIALIZED();
    blink::WebMediaPlayerAction blinkAction((blink::WebMediaPlayerAction::Type)action, enable);
    m_webContents->GetRenderViewHost()->GetMainFrame()->ExecuteMediaPlayerActionAtLocation(toGfx(location), blinkAction);
}

void WebContentsAdapter::inspectElementAt(const QPoint &location)
{
    Q_ASSERT(isInitialized());
    if (m_devToolsFrontend) {
        m_devToolsFrontend->InspectElementAt(location.x(), location.y());
        return;
    }
    if (content::DevToolsAgentHost::HasFor(m_webContents.get()))
        content::DevToolsAgentHost::GetOrCreateFor(m_webContents.get())->InspectElement(
                    m_webContents->GetFocusedFrame(), location.x(), location.y());
}

bool WebContentsAdapter::hasInspector() const
{
    CHECK_INITIALIZED(false);
    if (m_devToolsFrontend)
        return true;
    if (content::DevToolsAgentHost::HasFor(m_webContents.get()))
        return content::DevToolsAgentHost::GetOrCreateFor(m_webContents.get())->IsAttached();
    return false;
}

void WebContentsAdapter::openDevToolsFrontend(QSharedPointer<WebContentsAdapter> frontendAdapter)
{
    Q_ASSERT(isInitialized());
    if (m_devToolsFrontend && frontendAdapter->webContents() &&
            m_devToolsFrontend->frontendDelegate() == frontendAdapter->webContents()->GetDelegate())
        return;

    if (m_devToolsFrontend) {
        m_devToolsFrontend->DisconnectFromTarget();
        m_devToolsFrontend->Close();
    }

    m_devToolsFrontend = DevToolsFrontendQt::Show(frontendAdapter, m_webContents.get());
}

void WebContentsAdapter::closeDevToolsFrontend()
{
    if (m_devToolsFrontend) {
        m_devToolsFrontend->DisconnectFromTarget();
        m_devToolsFrontend->Close();
    }
}

void WebContentsAdapter::devToolsFrontendDestroyed(DevToolsFrontendQt *frontend)
{
    Q_ASSERT(frontend == m_devToolsFrontend);
    Q_UNUSED(frontend);
    m_devToolsFrontend = nullptr;
}

void WebContentsAdapter::exitFullScreen()
{
    CHECK_INITIALIZED();
    m_webContents->ExitFullscreen(false);
}

void WebContentsAdapter::changedFullScreen()
{
    CHECK_INITIALIZED();
    m_webContents->NotifyFullscreenChanged(false);
}

void WebContentsAdapter::wasShown()
{
    CHECK_INITIALIZED();
    m_webContents->WasShown();
}

void WebContentsAdapter::wasHidden()
{
    CHECK_INITIALIZED();
    m_webContents->WasHidden();
}

void WebContentsAdapter::printToPDF(const QPageLayout &pageLayout, const QString &filePath)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    CHECK_INITIALIZED();
    PrintViewManagerQt::PrintToPDFFileCallback callback = base::Bind(&callbackOnPdfSavingFinished,
                                                                m_adapterClient,
                                                                filePath);
    PrintViewManagerQt::FromWebContents(m_webContents.get())->PrintToPDFFileWithCallback(pageLayout,
                                                                                   true,
                                                                                   filePath,
                                                                                   callback);
#endif // QT_CONFIG(webengine_printing_and_pdf)
}

quint64 WebContentsAdapter::printToPDFCallbackResult(const QPageLayout &pageLayout,
                                                     bool colorMode,
                                                     bool useCustomMargins)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    CHECK_INITIALIZED(0);
    PrintViewManagerQt::PrintToPDFCallback callback = base::Bind(&callbackOnPrintingFinished,
                                                                 m_adapterClient,
                                                                 m_nextRequestId);
    PrintViewManagerQt::FromWebContents(m_webContents.get())->PrintToPDFWithCallback(pageLayout,
                                                                               colorMode,
                                                                               useCustomMargins,
                                                                               callback);
    return m_nextRequestId++;
#else
    Q_UNUSED(pageLayout);
    Q_UNUSED(colorMode);
    return 0;
#endif // QT_CONFIG(webengine_printing_and_pdf)
}

QPointF WebContentsAdapter::lastScrollOffset() const
{
    CHECK_INITIALIZED(QPointF());
    if (RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView()))
        return toQt(rwhv->lastScrollOffset());
    return QPointF();
}

QSizeF WebContentsAdapter::lastContentsSize() const
{
    CHECK_INITIALIZED(QSizeF());
    if (RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView()))
        return toQt(rwhv->lastContentsSize());
    return QSizeF();
}

void WebContentsAdapter::grantMediaAccessPermission(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags flags)
{
    CHECK_INITIALIZED();
    // Let the permission manager remember the reply.
    if (flags & WebContentsAdapterClient::MediaAudioCapture)
        m_profileAdapter->permissionRequestReply(securityOrigin, ProfileAdapter::AudioCapturePermission, true);
    if (flags & WebContentsAdapterClient::MediaVideoCapture)
        m_profileAdapter->permissionRequestReply(securityOrigin, ProfileAdapter::VideoCapturePermission, true);
    MediaCaptureDevicesDispatcher::GetInstance()->handleMediaAccessPermissionResponse(m_webContents.get(), securityOrigin, flags);
}

void WebContentsAdapter::runGeolocationRequestCallback(const QUrl &securityOrigin, bool allowed)
{
    CHECK_INITIALIZED();
    m_profileAdapter->permissionRequestReply(securityOrigin, ProfileAdapter::GeolocationPermission, allowed);
}

void WebContentsAdapter::grantMouseLockPermission(bool granted)
{
    CHECK_INITIALIZED();

    if (granted) {
        if (RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView()))
            rwhv->Focus();
        else
            granted = false;
    }

    m_webContents->GotResponseToLockMouseRequest(granted);
}

void WebContentsAdapter::dpiScaleChanged()
{
    CHECK_INITIALIZED();
    content::RenderWidgetHostImpl* impl = NULL;
    if (m_webContents->GetRenderViewHost())
        impl = content::RenderWidgetHostImpl::From(m_webContents->GetRenderViewHost()->GetWidget());
    if (impl)
        impl->NotifyScreenInfoChanged();
}

void WebContentsAdapter::setBackgroundColor(const QColor &color)
{
    CHECK_INITIALIZED();
    if (content::RenderWidgetHostView *rwhv = m_webContents->GetRenderWidgetHostView())
        rwhv->SetBackgroundColor(toSk(color));
}

content::WebContents *WebContentsAdapter::webContents() const
{
    return m_webContents.get();
}

#if QT_CONFIG(webengine_webchannel)
QWebChannel *WebContentsAdapter::webChannel() const
{
    return m_webChannel;
}

void WebContentsAdapter::setWebChannel(QWebChannel *channel, uint worldId)
{
    CHECK_INITIALIZED();
    if (m_webChannel == channel && m_webChannelWorld == worldId)
        return;

    if (!m_webChannelTransport.get())
        m_webChannelTransport.reset(new WebChannelIPCTransportHost(m_webContents.get(), worldId));
    else {
        if (m_webChannel != channel)
            m_webChannel->disconnectFrom(m_webChannelTransport.get());
        if (m_webChannelWorld != worldId)
            m_webChannelTransport->setWorldId(worldId);
    }

    m_webChannel = channel;
    m_webChannelWorld = worldId;

    if (!channel) {
        m_webChannelTransport.reset();
        return;
    }
    channel->connectTo(m_webChannelTransport.get());
}
#endif

#if QT_CONFIG(draganddrop)
static QMimeData *mimeDataFromDropData(const content::DropData &dropData)
{
    QMimeData *mimeData = new QMimeData();
    if (!dropData.text.is_null())
        mimeData->setText(toQt(dropData.text.string()));
    if (!dropData.html.is_null())
        mimeData->setHtml(toQt(dropData.html.string()));
    if (dropData.url.is_valid())
        mimeData->setUrls(QList<QUrl>() << toQt(dropData.url));
    if (!dropData.custom_data.empty()) {
        base::Pickle pickle;
        ui::WriteCustomDataToPickle(dropData.custom_data, &pickle);
        mimeData->setData(toQt(ui::Clipboard::GetWebCustomDataFormatType().ToString()), QByteArray((const char*)pickle.data(), pickle.size()));
    }
    return mimeData;
}

static blink::WebDragOperationsMask toWeb(const Qt::DropActions action)
{
    int result = blink::kWebDragOperationNone;
    if (action & Qt::CopyAction)
        result |= blink::kWebDragOperationCopy;
    if (action & Qt::LinkAction)
        result |= blink::kWebDragOperationLink;
    if (action & Qt::MoveAction)
        result |= blink::kWebDragOperationMove;
    return static_cast<blink::WebDragOperationsMask>(result);
}

void WebContentsAdapter::startDragging(QObject *dragSource, const content::DropData &dropData,
                                       Qt::DropActions allowedActions, const QPixmap &pixmap,
                                       const QPoint &offset)
{
    CHECK_INITIALIZED();

    if (m_currentDropData)
        return;

    // Clear certain fields of the drop data to not run into DCHECKs
    // of DropDataToWebDragData in render_view_impl.cc.
    m_currentDropData.reset(new content::DropData(dropData));
    m_currentDropData->download_metadata.clear();
    m_currentDropData->file_contents.clear();
    m_currentDropData->file_contents_content_disposition.clear();

    m_currentDropAction = blink::kWebDragOperationNone;
    QDrag *drag = new QDrag(dragSource);    // will be deleted by Qt's DnD implementation
    bool dValid = true;
    QMetaObject::Connection onDestroyed = QObject::connect(dragSource, &QObject::destroyed, [&dValid](){
        dValid = false;
        QDrag::cancel();
    });

    QMimeData *mimeData = mimeDataFromDropData(*m_currentDropData);
    if (handleDropDataFileContents(dropData, mimeData))
        allowedActions = Qt::MoveAction;

    drag->setMimeData(mimeData);
    if (!pixmap.isNull()) {
        drag->setPixmap(pixmap);
        drag->setHotSpot(offset);
    }

    {
        base::MessageLoop::ScopedNestableTaskAllower allow;
        drag->exec(allowedActions);
    }

    QObject::disconnect(onDestroyed);
    if (dValid) {
        if (m_webContents) {
            content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
            if (rvh) {
                rvh->GetWidget()->DragSourceEndedAt(gfx::PointF(m_lastDragClientPos.x(), m_lastDragClientPos.y()),
                                                    gfx::PointF(m_lastDragScreenPos.x(), m_lastDragScreenPos.y()),
                                                    blink::WebDragOperation(m_currentDropAction));
                rvh->GetWidget()->DragSourceSystemDragEnded();
            }
        }
        m_currentDropData.reset();
    }
}

bool WebContentsAdapter::handleDropDataFileContents(const content::DropData &dropData,
                                                    QMimeData *mimeData)
{
    CHECK_INITIALIZED(false);
    if (dropData.file_contents.empty())
        return false;

    if (!m_dndTmpDir) {
        m_dndTmpDir.reset(new QTemporaryDir);
        if (!m_dndTmpDir->isValid()) {
            m_dndTmpDir.reset();
            return false;
        }
    }

    const auto maybeFilename = dropData.GetSafeFilenameForImageFileContents();
    const QString fileName = maybeFilename ? toQt(maybeFilename->AsUTF16Unsafe()) : QString();
    const QString &filePath = m_dndTmpDir->filePath(fileName);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Cannot write temporary file %s.", qUtf8Printable(filePath));
        return false;
    }
    file.write(QByteArray::fromStdString(dropData.file_contents));

    const QUrl &targetUrl = QUrl::fromLocalFile(filePath);
    mimeData->setUrls(QList<QUrl>{targetUrl});
    return true;
}

static void fillDropDataFromMimeData(content::DropData *dropData, const QMimeData *mimeData)
{
    Q_ASSERT(dropData->filenames.empty());
    const QList<QUrl> urls = mimeData->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            ui::FileInfo uifi;
            uifi.path = toFilePath(url.toLocalFile());
            dropData->filenames.push_back(uifi);
        }
    }
    if (!dropData->filenames.empty())
        return;
    if (mimeData->hasHtml())
        dropData->html = toNullableString16(mimeData->html());
    if (mimeData->hasText())
        dropData->text = toNullableString16(mimeData->text());
    if (mimeData->hasFormat(toQt(ui::Clipboard::GetWebCustomDataFormatType().ToString()))) {
        QByteArray customData = mimeData->data(toQt(ui::Clipboard::GetWebCustomDataFormatType().ToString()));
        ui::ReadCustomDataIntoMap(customData.constData(), customData.length(), &dropData->custom_data);
    }
}

Qt::DropAction toQt(blink::WebDragOperation op)
{
    if (op & blink::kWebDragOperationCopy)
        return Qt::CopyAction;
    if (op & blink::kWebDragOperationLink)
        return Qt::LinkAction;
    if (op & blink::kWebDragOperationMove || op & blink::kWebDragOperationDelete)
        return Qt::MoveAction;
    return Qt::IgnoreAction;
}

static int toWeb(Qt::MouseButtons buttons)
{
    int result = 0;
    if (buttons & Qt::LeftButton)
        result |= blink::WebInputEvent::kLeftButtonDown;
    if (buttons & Qt::RightButton)
        result |= blink::WebInputEvent::kRightButtonDown;
    if (buttons & Qt::MiddleButton)
        result |= blink::WebInputEvent::kMiddleButtonDown;
    return result;
}

static int toWeb(Qt::KeyboardModifiers modifiers)
{
    int result = 0;
    if (modifiers & Qt::ShiftModifier)
        result |= blink::WebInputEvent::kShiftKey;
    if (modifiers & Qt::ControlModifier)
        result |= blink::WebInputEvent::kControlKey;
    if (modifiers & Qt::AltModifier)
        result |= blink::WebInputEvent::kAltKey;
    if (modifiers & Qt::MetaModifier)
        result |= blink::WebInputEvent::kMetaKey;
    return result;
}

void WebContentsAdapter::enterDrag(QDragEnterEvent *e, const QPointF &screenPos)
{
    CHECK_INITIALIZED();

    if (!m_currentDropData) {
        // The drag originated outside the WebEngineView.
        m_currentDropData.reset(new content::DropData);
        fillDropDataFromMimeData(m_currentDropData.get(), e->mimeData());
    }

    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    rvh->GetWidget()->FilterDropData(m_currentDropData.get());
    rvh->GetWidget()->DragTargetDragEnter(*m_currentDropData, toGfx(e->posF()), toGfx(screenPos),
                                          toWeb(e->possibleActions()),
                                          toWeb(e->mouseButtons()) | toWeb(e->keyboardModifiers()));
}

Qt::DropAction WebContentsAdapter::updateDragPosition(QDragMoveEvent *e, const QPointF &screenPos)
{
    CHECK_INITIALIZED(Qt::DropAction());
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    m_lastDragClientPos = e->posF();
    m_lastDragScreenPos = screenPos;
    rvh->GetWidget()->DragTargetDragOver(toGfx(m_lastDragClientPos), toGfx(m_lastDragScreenPos), toWeb(e->possibleActions()),
                                         toWeb(e->mouseButtons()) | toWeb(e->keyboardModifiers()));
    waitForUpdateDragActionCalled();
    return toQt(blink::WebDragOperation(m_currentDropAction));
}

void WebContentsAdapter::waitForUpdateDragActionCalled()
{
    CHECK_INITIALIZED();
    const qint64 timeout = 3000;
    QElapsedTimer t;
    t.start();
    base::MessagePump::Delegate *delegate = base::MessageLoop::current();
    DCHECK(delegate);
    m_updateDragActionCalled = false;
    for (;;) {
        while (delegate->DoWork() && !m_updateDragActionCalled) {}
        if (m_updateDragActionCalled)
            break;
        if (t.hasExpired(timeout)) {
            qWarning("WebContentsAdapter::updateDragAction was not called within %d ms.",
                     static_cast<int>(timeout));
            return;
        }
        base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(1));
    }
}

void WebContentsAdapter::updateDragAction(int action)
{
    CHECK_INITIALIZED();
    m_updateDragActionCalled = true;
    m_currentDropAction = static_cast<blink::WebDragOperation>(action);
}

void WebContentsAdapter::endDragging(QDropEvent *e, const QPointF &screenPos)
{
    CHECK_INITIALIZED();
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    rvh->GetWidget()->FilterDropData(m_currentDropData.get());
    m_lastDragClientPos = e->posF();
    m_lastDragScreenPos = screenPos;
    rvh->GetWidget()->DragTargetDrop(*m_currentDropData, toGfx(m_lastDragClientPos), toGfx(m_lastDragScreenPos),
                                     toWeb(e->mouseButtons()) | toWeb(e->keyboardModifiers()));

    m_currentDropData.reset();
}

void WebContentsAdapter::leaveDrag()
{
    CHECK_INITIALIZED();
    content::RenderViewHost *rvh = m_webContents->GetRenderViewHost();
    rvh->GetWidget()->DragTargetDragLeave(toGfx(m_lastDragClientPos), toGfx(m_lastDragScreenPos));
    m_currentDropData.reset();
}
#endif // QT_CONFIG(draganddrop)

void WebContentsAdapter::replaceMisspelling(const QString &word)
{
#if QT_CONFIG(webengine_spellchecker)
    CHECK_INITIALIZED();
    m_webContents->ReplaceMisspelling(toString16(word));
#endif
}

void WebContentsAdapter::focusIfNecessary()
{
    CHECK_INITIALIZED();
    const WebEngineSettings *settings = m_adapterClient->webEngineSettings();
    bool focusOnNavigation = settings->testAttribute(WebEngineSettings::FocusOnNavigationEnabled);
    if (focusOnNavigation)
        m_webContents->Focus();
}

bool WebContentsAdapter::isFindTextInProgress() const
{
    CHECK_INITIALIZED(false);
    return m_lastFindRequestId != m_webContentsDelegate->lastReceivedFindReply();
}

bool WebContentsAdapter::hasFocusedFrame() const
{
    CHECK_INITIALIZED(false);
    return m_webContents->GetFocusedFrame() != nullptr;
}

WebContentsAdapterClient::RenderProcessTerminationStatus
WebContentsAdapterClient::renderProcessExitStatus(int terminationStatus) {
    auto status = WebContentsAdapterClient::RenderProcessTerminationStatus(-1);
    switch (terminationStatus) {
    case base::TERMINATION_STATUS_NORMAL_TERMINATION:
        status = WebContentsAdapterClient::NormalTerminationStatus;
        break;
    case base::TERMINATION_STATUS_ABNORMAL_TERMINATION:
        status = WebContentsAdapterClient::AbnormalTerminationStatus;
        break;
    case base::TERMINATION_STATUS_PROCESS_WAS_KILLED:
#if defined(OS_CHROMEOS)
    case base::TERMINATION_STATUS_PROCESS_WAS_KILLED_BY_OOM:
#endif
        status = WebContentsAdapterClient::KilledTerminationStatus;
        break;
    case base::TERMINATION_STATUS_PROCESS_CRASHED:
#if defined(OS_ANDROID)
    case base::TERMINATION_STATUS_OOM_PROTECTED:
#endif
        status = WebContentsAdapterClient::CrashedTerminationStatus;
        break;
    case base::TERMINATION_STATUS_STILL_RUNNING:
    case base::TERMINATION_STATUS_MAX_ENUM:
        // should be unreachable since Chromium asserts status != TERMINATION_STATUS_STILL_RUNNING
        // before calling this method
        break;
    }

    return status;
}

FaviconManager *WebContentsAdapter::faviconManager()
{
    CHECK_INITIALIZED(nullptr);
    return m_webContentsDelegate->faviconManager();
}

void WebContentsAdapter::viewSource()
{
    CHECK_INITIALIZED();
    m_webContents->GetMainFrame()->ViewSource();
}

bool WebContentsAdapter::canViewSource()
{
    CHECK_INITIALIZED(false);
    return m_webContents->GetController().CanViewSource();
}

ASSERT_ENUMS_MATCH(WebContentsAdapterClient::UnknownDisposition, WindowOpenDisposition::UNKNOWN)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::CurrentTabDisposition, WindowOpenDisposition::CURRENT_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::SingletonTabDisposition, WindowOpenDisposition::SINGLETON_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::NewForegroundTabDisposition, WindowOpenDisposition::NEW_FOREGROUND_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::NewBackgroundTabDisposition, WindowOpenDisposition::NEW_BACKGROUND_TAB)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::NewPopupDisposition, WindowOpenDisposition::NEW_POPUP)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::NewWindowDisposition, WindowOpenDisposition::NEW_WINDOW)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::SaveToDiskDisposition, WindowOpenDisposition::SAVE_TO_DISK)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::OffTheRecordDisposition, WindowOpenDisposition::OFF_THE_RECORD)
ASSERT_ENUMS_MATCH(WebContentsAdapterClient::IgnoreActionDisposition, WindowOpenDisposition::IGNORE_ACTION)

ASSERT_ENUMS_MATCH(ReferrerPolicy::Always, blink::kWebReferrerPolicyAlways)
ASSERT_ENUMS_MATCH(ReferrerPolicy::Default, blink::kWebReferrerPolicyDefault)
ASSERT_ENUMS_MATCH(ReferrerPolicy::NoReferrerWhenDowngrade, blink::kWebReferrerPolicyNoReferrerWhenDowngrade)
ASSERT_ENUMS_MATCH(ReferrerPolicy::Never, blink::kWebReferrerPolicyNever)
ASSERT_ENUMS_MATCH(ReferrerPolicy::Origin, blink::kWebReferrerPolicyOrigin)
ASSERT_ENUMS_MATCH(ReferrerPolicy::OriginWhenCrossOrigin, blink::kWebReferrerPolicyOriginWhenCrossOrigin)
ASSERT_ENUMS_MATCH(ReferrerPolicy::NoReferrerWhenDowngradeOriginWhenCrossOrigin, blink::kWebReferrerPolicyNoReferrerWhenDowngradeOriginWhenCrossOrigin)
ASSERT_ENUMS_MATCH(ReferrerPolicy::SameOrigin, blink::kWebReferrerPolicySameOrigin)
ASSERT_ENUMS_MATCH(ReferrerPolicy::StrictOrigin, blink::kWebReferrerPolicyStrictOrigin)
ASSERT_ENUMS_MATCH(ReferrerPolicy::Last, blink::kWebReferrerPolicyLast)

} // namespace QtWebEngineCore
