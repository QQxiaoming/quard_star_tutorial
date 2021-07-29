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

#include "web_contents_delegate_qt.h"

#include "profile_adapter.h"
#include "color_chooser_controller.h"
#include "color_chooser_qt.h"
#include "favicon_manager.h"
#include "file_picker_controller.h"
#include "media_capture_devices_dispatcher.h"
#include "net/network_delegate_qt.h"
#include "profile_qt.h"
#include "qwebengineregisterprotocolhandlerrequest.h"
#include "register_protocol_handler_request_controller_impl.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "visited_links_manager_qt.h"
#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"
#include "web_engine_context.h"
#include "web_engine_settings.h"

#include "chrome/browser/custom_handlers/protocol_handler_registry_factory.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/frame_navigate_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/web_preferences.h"
#include "net/base/data_url.h"
#include "net/base/url_util.h"

#include <QDesktopServices>
#include <QTimer>
#include <QWindow>

namespace QtWebEngineCore {

// Maps the LogSeverity defines in base/logging.h to the web engines message levels.
static WebContentsAdapterClient::JavaScriptConsoleMessageLevel mapToJavascriptConsoleMessageLevel(int32_t messageLevel)
{
    if (messageLevel < 1)
        return WebContentsAdapterClient::Info;
    else if (messageLevel > 1)
        return WebContentsAdapterClient::Error;

    return WebContentsAdapterClient::Warning;
}

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents *webContents, WebContentsAdapterClient *adapterClient)
    : m_viewClient(adapterClient)
    , m_lastReceivedFindReply(0)
    , m_faviconManager(new FaviconManager(webContents, adapterClient))
    , m_lastLoadProgress(-1)
    , m_frameFocusedObserver(adapterClient)
{
    webContents->SetDelegate(this);
    Observe(webContents);
}

WebContentsDelegateQt::~WebContentsDelegateQt()
{
    // The destruction of this object should take place before
    // WebContents destruction since WebContentsAdapterClient
    // might be already deleted.
}

content::WebContents *WebContentsDelegateQt::OpenURLFromTab(content::WebContents *source, const content::OpenURLParams &params)
{
    content::WebContents *target = source;
    content::SiteInstance *target_site_instance = params.source_site_instance.get();
    content::Referrer referrer = params.referrer;
    if (params.disposition != WindowOpenDisposition::CURRENT_TAB) {
        QSharedPointer<WebContentsAdapter> targetAdapter = createWindow(0, params.disposition, gfx::Rect(), params.user_gesture);
        if (targetAdapter) {
            if (targetAdapter->profile() != source->GetBrowserContext()) {
                target_site_instance = nullptr;
                referrer = content::Referrer();
            }
            if (!targetAdapter->isInitialized())
                targetAdapter->initialize(target_site_instance);
            target = targetAdapter->webContents();
        }
    }
    Q_ASSERT(target);

    content::NavigationController::LoadURLParams load_url_params(params.url);
    load_url_params.source_site_instance = target_site_instance;
    load_url_params.referrer = referrer;
    load_url_params.frame_tree_node_id = params.frame_tree_node_id;
    load_url_params.redirect_chain = params.redirect_chain;
    load_url_params.transition_type = params.transition;
    load_url_params.extra_headers = params.extra_headers;
    load_url_params.should_replace_current_entry = params.should_replace_current_entry;
    load_url_params.is_renderer_initiated = params.is_renderer_initiated;
    load_url_params.started_from_context_menu = params.started_from_context_menu;
    load_url_params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;
    if (params.uses_post) {
        load_url_params.load_type = content::NavigationController::LOAD_TYPE_HTTP_POST;
        load_url_params.post_data = params.post_data;
    }

    target->GetController().LoadURLWithParams(load_url_params);
    return target;
}

static bool shouldUseActualURL(const content::NavigationEntry *entry)
{
    Q_ASSERT(entry);

    // Show actual URL for data URLs only
    if (!entry->GetURL().SchemeIs(url::kDataScheme))
        return false;

    // Do not show data URL of interstitial and error pages
    if (entry->GetPageType() != content::PAGE_TYPE_NORMAL)
        return false;

    // Show invalid data URL
    std::string mime_type, charset, data;
    if (!net::DataURL::Parse(entry->GetURL(), &mime_type, &charset, &data))
        return false;

    // Do not show empty data URL
    return !data.empty();
}

void WebContentsDelegateQt::NavigationStateChanged(content::WebContents* source, content::InvalidateTypes changed_flags)
{
    if (changed_flags & content::INVALIDATE_TYPE_URL) {
        content::NavigationEntry *entry = source->GetController().GetVisibleEntry();

        QUrl newUrl;
        if (source->GetVisibleURL().SchemeIs(content::kViewSourceScheme)) {
            Q_ASSERT(entry);
            GURL url = entry->GetURL();

            // Strip user name, password and reference section from view-source URLs
            if (url.has_password() || url.has_username() || url.has_ref()) {
                GURL strippedUrl = net::SimplifyUrlForRequest(entry->GetURL());
                newUrl = QUrl(QString("%1:%2").arg(content::kViewSourceScheme, QString::fromStdString(strippedUrl.spec())));
            }
        }

        // If there is a visible entry there are special cases when we dont wan't to use the actual URL
        if (entry && newUrl.isEmpty())
            newUrl = shouldUseActualURL(entry) ? toQt(entry->GetURL()) : toQt(entry->GetVirtualURL());

        if (m_url != newUrl) {
            m_url = newUrl;
            m_viewClient->urlChanged(m_url);
        }
    }

    if (changed_flags & content::INVALIDATE_TYPE_TITLE) {
        QString newTitle = toQt(source->GetTitle());
        if (m_title != newTitle) {
            m_title = newTitle;
            m_viewClient->titleChanged(m_title);
        }
    }

    // NavigationStateChanged gets called with INVALIDATE_TYPE_TAB by AudioStateProvider::Notify,
    // whenever an audio sound gets played or stopped, this is the only way to actually figure out
    // if there was a recently played audio sound.
    // Make sure to only emit the signal when loading isn't in progress, because it causes multiple
    // false signals to be emitted.
    if ((changed_flags & content::INVALIDATE_TYPE_TAB) && !(changed_flags & content::INVALIDATE_TYPE_LOAD)) {
        m_viewClient->recentlyAudibleChanged(source->IsCurrentlyAudible());
    }
}

void WebContentsDelegateQt::AddNewContents(content::WebContents* source, std::unique_ptr<content::WebContents> new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture, bool* was_blocked)
{
    Q_UNUSED(source)
    QSharedPointer<WebContentsAdapter> newAdapter = createWindow(std::move(new_contents), disposition, initial_pos, user_gesture);
    // Chromium can forget to pass user-agent override settings to new windows (see QTBUG-61774 and QTBUG-76249),
    // so set it here. Note the actual value doesn't really matter here. Only the second value does, but we try
    // to give the correct user-agent anyway.
    if (newAdapter)
        newAdapter->webContents()->SetUserAgentOverride(newAdapter->profileAdapter()->httpUserAgent().toStdString(), true);
    if (newAdapter && !newAdapter->isInitialized())
        newAdapter->loadDefault();
    if (was_blocked)
        *was_blocked = !newAdapter;
}

void WebContentsDelegateQt::CloseContents(content::WebContents *source)
{
    m_viewClient->close();
    GetJavaScriptDialogManager(source)->CancelDialogs(source, /* whatever?: */false);
}

void WebContentsDelegateQt::LoadProgressChanged(content::WebContents */*source*/, double progress)
{
    if (!m_loadingErrorFrameList.isEmpty())
        return;
    if (m_lastLoadProgress < 0) // suppress signals that aren't between loadStarted and loadFinished
        return;
    m_lastLoadProgress = qMax(m_lastLoadProgress, qRound(progress * 100)); // ensure monotonicity
    m_lastLoadProgress = qMin(m_lastLoadProgress, 100);
    m_viewClient->loadProgressChanged(m_lastLoadProgress);
}

void WebContentsDelegateQt::HandleKeyboardEvent(content::WebContents *, const content::NativeWebKeyboardEvent &event)
{
    Q_ASSERT(!event.skip_in_browser);
    if (event.os_event)
        m_viewClient->unhandledKeyEvent(reinterpret_cast<QKeyEvent *>(event.os_event));
}

void WebContentsDelegateQt::RenderFrameCreated(content::RenderFrameHost *render_frame_host)
{
    content::FrameTreeNode *node = static_cast<content::RenderFrameHostImpl *>(render_frame_host)->frame_tree_node();
    m_frameFocusedObserver.addNode(node);
}

void WebContentsDelegateQt::RenderFrameDeleted(content::RenderFrameHost *render_frame_host)
{
    m_loadingErrorFrameList.removeOne(render_frame_host->GetRoutingID());
}

void WebContentsDelegateQt::RenderFrameHostChanged(content::RenderFrameHost *old_host, content::RenderFrameHost *new_host)
{
    if (old_host) {
        content::FrameTreeNode *old_node = static_cast<content::RenderFrameHostImpl *>(old_host)->frame_tree_node();
        m_frameFocusedObserver.removeNode(old_node);
    }

    if (new_host) {
        content::FrameTreeNode *new_node = static_cast<content::RenderFrameHostImpl *>(new_host)->frame_tree_node();
        m_frameFocusedObserver.addNode(new_node);
    }
}

void WebContentsDelegateQt::RenderViewHostChanged(content::RenderViewHost *, content::RenderViewHost *newHost)
{
    if (newHost && newHost->GetWidget() && newHost->GetWidget()->GetView()) {
        auto rwhv = static_cast<RenderWidgetHostViewQt *>(newHost->GetWidget()->GetView());
        m_viewClient->widgetChanged(rwhv->delegate());
    }
}

void WebContentsDelegateQt::EmitLoadStarted(const QUrl &url, bool isErrorPage)
{
    if (m_lastLoadProgress >= 0 && m_lastLoadProgress < 100) // already running
        return;
    m_viewClient->loadStarted(url, isErrorPage);
    m_viewClient->updateNavigationActions();
    m_viewClient->loadProgressChanged(0);
    m_lastLoadProgress = 0;
}

void WebContentsDelegateQt::DidStartNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame())
        return;

    // Error-pages are not reported as separate started navigations.
    Q_ASSERT(!navigation_handle->IsErrorPage());

    m_loadingErrorFrameList.clear();
    m_faviconManager->resetCandidates();
    EmitLoadStarted(toQt(navigation_handle->GetURL()));
}

void WebContentsDelegateQt::EmitLoadFinished(bool success, const QUrl &url, bool isErrorPage, int errorCode, const QString &errorDescription)
{
    if (m_lastLoadProgress < 0) // not currently running
        return;
    m_lastLoadProgress = -1;
    m_viewClient->loadProgressChanged(100);
    m_viewClient->loadFinished(success, url, isErrorPage, errorCode, errorDescription);
    m_viewClient->updateNavigationActions();
}

void WebContentsDelegateQt::EmitLoadCommitted()
{
    // Make sure that we don't set the findNext WebFindOptions on a new frame.
    m_lastSearchedString = QString();

    m_viewClient->loadCommitted();
    m_viewClient->updateNavigationActions();
}

void WebContentsDelegateQt::DidFinishNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame())
        return;

    if (navigation_handle->HasCommitted() && !navigation_handle->IsErrorPage()) {
        ProfileAdapter *profileAdapter = m_viewClient->profileAdapter();
        // VisistedLinksMaster asserts !IsOffTheRecord().
        if (navigation_handle->ShouldUpdateHistory() && profileAdapter->trackVisitedLinks()) {
            for (const GURL &url : navigation_handle->GetRedirectChain())
                profileAdapter->visitedLinksManager()->addUrl(url);
        }

        EmitLoadCommitted();
    }
    // Success is reported by DidFinishLoad, but DidFailLoad is now dead code and needs to be handled below
    if (navigation_handle->GetNetErrorCode() == net::OK)
        return;

    // WebContentsObserver::DidFailLoad is not called any longer so we have to report the failure here.
    const net::Error error_code = navigation_handle->GetNetErrorCode();
    const std::string error_description = net::ErrorToString(error_code);
    didFailLoad(toQt(navigation_handle->GetURL()), error_code, toQt(error_description));

    // The load will succede as an error-page load later, and we reported the original error above
    if (navigation_handle->IsErrorPage()) {
        // Now report we are starting to load an error-page.
        m_loadingErrorFrameList.append(navigation_handle->GetRenderFrameHost()->GetRoutingID());
        m_faviconManager->resetCandidates();
        EmitLoadStarted(toQt(GURL(content::kUnreachableWebDataURL)), true);

        // If it is already committed we will not see another DidFinishNavigation call or a DidFinishLoad call.
        if (navigation_handle->HasCommitted())
            EmitLoadCommitted();
    }
}

void WebContentsDelegateQt::didFailLoad(const QUrl &url, int errorCode, const QString &errorDescription)
{
    m_viewClient->iconChanged(QUrl());
    EmitLoadFinished(false /* success */ , url, false /* isErrorPage */, errorCode, errorDescription);
}

void WebContentsDelegateQt::DidFailLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url, int error_code, const base::string16& error_description)
{
    if (render_frame_host != web_contents()->GetMainFrame())
        return;

    if (validated_url.spec() == content::kUnreachableWebDataURL) {
        // error-pages should only ever fail due to abort:
        Q_ASSERT(error_code == -3 /* ERR_ABORTED */);
        m_loadingErrorFrameList.removeOne(render_frame_host->GetRoutingID());
        m_viewClient->iconChanged(QUrl());

        EmitLoadFinished(false /* success */, toQt(validated_url), true /* isErrorPage */);
        return;
    }

    didFailLoad(toQt(validated_url), error_code, toQt(error_description));
}

void WebContentsDelegateQt::DidFinishLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url)
{
    Q_ASSERT(validated_url.is_valid());
    if (validated_url.spec() == content::kUnreachableWebDataURL) {
        m_loadingErrorFrameList.removeOne(render_frame_host->GetRoutingID());
        m_viewClient->iconChanged(QUrl());

        // Trigger LoadFinished signal for main frame's error page only.
        if (!render_frame_host->GetParent())
            EmitLoadFinished(true /* success */, toQt(validated_url), true /* isErrorPage */);

        return;
    }

    if (render_frame_host->GetParent()) {
        m_viewClient->updateNavigationActions();
        return;
    }

    if (!m_faviconManager->hasCandidate())
        m_viewClient->iconChanged(QUrl());

    content::NavigationEntry *entry = web_contents()->GetController().GetActiveEntry();
    int http_statuscode = 0;
    if (entry)
       http_statuscode = entry->GetHttpStatusCode();
    EmitLoadFinished(true /* success */ , toQt(validated_url), false /* isErrorPage */, http_statuscode);
}

void WebContentsDelegateQt::DidUpdateFaviconURL(const std::vector<content::FaviconURL> &candidates)
{
    QList<FaviconInfo> faviconCandidates;
    faviconCandidates.reserve(static_cast<int>(candidates.size()));
    for (const content::FaviconURL &candidate : candidates) {
        // Store invalid candidates too for later debugging via API
        faviconCandidates.append(toFaviconInfo(candidate));
    }

    // Favicon URL can be changed from JavaScript too. Thus we need to reset
    // the current candidate icon list to not handle previous icon as a candidate.
    m_faviconManager->resetCandidates();
    m_faviconManager->update(faviconCandidates);
}

void WebContentsDelegateQt::WebContentsCreated(content::WebContents */*source_contents*/,
                                               int /*opener_render_process_id*/, int /*opener_render_frame_id*/,
                                               const std::string &/*frame_name*/,
                                               const GURL &target_url, content::WebContents */*new_contents*/)
{
    m_initialTargetUrl = toQt(target_url);
}

content::ColorChooser *WebContentsDelegateQt::OpenColorChooser(content::WebContents *source, SkColor color, const std::vector<blink::mojom::ColorSuggestionPtr> &suggestion)
{
    Q_UNUSED(suggestion);
    ColorChooserQt *colorChooser = new ColorChooserQt(source, toQt(color));
    m_viewClient->showColorDialog(colorChooser->controller());

    return colorChooser;
}
content::JavaScriptDialogManager *WebContentsDelegateQt::GetJavaScriptDialogManager(content::WebContents *)
{
    return JavaScriptDialogManagerQt::GetInstance();
}

void WebContentsDelegateQt::EnterFullscreenModeForTab(content::WebContents *web_contents, const GURL& origin, const blink::WebFullscreenOptions &)
{
    Q_UNUSED(web_contents);
    if (!m_viewClient->isFullScreenMode())
        m_viewClient->requestFullScreenMode(toQt(origin), true);
}

void WebContentsDelegateQt::ExitFullscreenModeForTab(content::WebContents *web_contents)
{
    if (m_viewClient->isFullScreenMode())
        m_viewClient->requestFullScreenMode(toQt(web_contents->GetLastCommittedURL().GetOrigin()), false);
}

bool WebContentsDelegateQt::IsFullscreenForTabOrPending(const content::WebContents* web_contents) const
{
    Q_UNUSED(web_contents);
    return m_viewClient->isFullScreenMode();
}

ASSERT_ENUMS_MATCH(FilePickerController::Open, content::FileChooserParams::Open)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, content::FileChooserParams::OpenMultiple)
ASSERT_ENUMS_MATCH(FilePickerController::UploadFolder, content::FileChooserParams::UploadFolder)
ASSERT_ENUMS_MATCH(FilePickerController::Save, content::FileChooserParams::Save)

void WebContentsDelegateQt::RunFileChooser(content::RenderFrameHost *frameHost, const content::FileChooserParams &params)
{
    QStringList acceptedMimeTypes;
    acceptedMimeTypes.reserve(params.accept_types.size());
    for (std::vector<base::string16>::const_iterator it = params.accept_types.begin(); it < params.accept_types.end(); ++it)
        acceptedMimeTypes.append(toQt(*it));

    m_filePickerController.reset(new FilePickerController(static_cast<FilePickerController::FileChooserMode>(params.mode),
                                                          frameHost, toQt(params.default_file_name.value()), acceptedMimeTypes));

    // Defer the call to not block base::MessageLoop::RunTask with modal dialogs.
    QTimer::singleShot(0, [this] () {
        m_viewClient->runFileChooser(m_filePickerController);
    });
}

bool WebContentsDelegateQt::DidAddMessageToConsole(content::WebContents *source, int32_t level, const base::string16 &message, int32_t line_no, const base::string16 &source_id)
{
    Q_UNUSED(source)
    m_viewClient->javaScriptConsoleMessage(mapToJavascriptConsoleMessageLevel(level), toQt(message), static_cast<int>(line_no), toQt(source_id));
    return false;
}

void WebContentsDelegateQt::FindReply(content::WebContents *source, int request_id, int number_of_matches, const gfx::Rect& selection_rect, int active_match_ordinal, bool final_update)
{
    Q_UNUSED(source)
    Q_UNUSED(selection_rect)
    Q_UNUSED(active_match_ordinal)
    if (final_update && request_id > m_lastReceivedFindReply) {
        m_lastReceivedFindReply = request_id;
        m_viewClient->didFindText(request_id, number_of_matches);
    }
}

void WebContentsDelegateQt::RequestMediaAccessPermission(content::WebContents *web_contents, const content::MediaStreamRequest &request,  content::MediaResponseCallback callback)
{
    MediaCaptureDevicesDispatcher::GetInstance()->processMediaAccessRequest(m_viewClient, web_contents, request, std::move(callback));
}

void WebContentsDelegateQt::SetContentsBounds(content::WebContents *source, const gfx::Rect &bounds)
{
    if (!source->HasOpener()) // is popup
        return;

    QRect frameGeometry(toQt(bounds));
    QRect geometry;
    if (RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt*>(web_contents()->GetRenderWidgetHostView())) {
        if (rwhv->delegate() && rwhv->delegate()->window())
            geometry = frameGeometry.marginsRemoved(rwhv->delegate()->window()->frameMargins());
    }
    m_viewClient->requestGeometryChange(geometry, frameGeometry);
}

void WebContentsDelegateQt::UpdateTargetURL(content::WebContents* source, const GURL& url)
{
    Q_UNUSED(source)
    m_viewClient->didUpdateTargetURL(toQt(url));
}

void WebContentsDelegateQt::OnVisibilityChanged(content::Visibility visibility)
{
    if (visibility != content::Visibility::HIDDEN)
        web_cache::WebCacheManager::GetInstance()->ObserveActivity(web_contents()->GetMainFrame()->GetProcess()->GetID());
}

void WebContentsDelegateQt::DidFirstVisuallyNonEmptyPaint()
{
    RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt*>(web_contents()->GetRenderWidgetHostView());
    if (!rwhv)
        return;

    RenderWidgetHostViewQt::LoadVisuallyCommittedState loadVisuallyCommittedState = rwhv->getLoadVisuallyCommittedState();
    if (loadVisuallyCommittedState == RenderWidgetHostViewQt::NotCommitted) {
        rwhv->setLoadVisuallyCommittedState(RenderWidgetHostViewQt::DidFirstVisuallyNonEmptyPaint);
    } else if (loadVisuallyCommittedState == RenderWidgetHostViewQt::DidFirstCompositorFrameSwap) {
        m_viewClient->loadVisuallyCommitted();
        rwhv->setLoadVisuallyCommittedState(RenderWidgetHostViewQt::NotCommitted);
    }
}

void WebContentsDelegateQt::ActivateContents(content::WebContents* contents)
{
    WebEngineSettings *settings = m_viewClient->webEngineSettings();
    if (settings->testAttribute(settings->Attribute::AllowWindowActivationFromJavaScript))
        contents->Focus();
}

void WebContentsDelegateQt::RequestToLockMouse(content::WebContents *web_contents, bool user_gesture, bool last_unlocked_by_target)
{
    Q_UNUSED(user_gesture);

    if (last_unlocked_by_target)
        web_contents->GotResponseToLockMouseRequest(true);
    else
        m_viewClient->runMouseLockPermissionRequest(toQt(web_contents->GetVisibleURL()));
}

void WebContentsDelegateQt::overrideWebPreferences(content::WebContents *webContents, content::WebPreferences *webPreferences)
{
    m_viewClient->webEngineSettings()->overrideWebPreferences(webContents, webPreferences);
}

QWeakPointer<WebContentsAdapter> WebContentsDelegateQt::createWindow(std::unique_ptr<content::WebContents> new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture)
{
    QSharedPointer<WebContentsAdapter> newAdapter = QSharedPointer<WebContentsAdapter>::create(std::move(new_contents));

    m_viewClient->adoptNewWindow(newAdapter, static_cast<WebContentsAdapterClient::WindowOpenDisposition>(disposition), user_gesture, toQt(initial_pos), m_initialTargetUrl);

    // If the client didn't reference the adapter, it will be deleted now, and the weak pointer zeroed.
    return newAdapter;
}

void WebContentsDelegateQt::allowCertificateError(const QSharedPointer<CertificateErrorController> &errorController)
{
    m_viewClient->allowCertificateError(errorController);
}

void WebContentsDelegateQt::selectClientCert(const QSharedPointer<ClientCertSelectController> &selectController)
{
    m_viewClient->selectClientCert(selectController);
}

void WebContentsDelegateQt::requestGeolocationPermission(const QUrl &requestingOrigin)
{
    m_viewClient->runGeolocationPermissionRequest(requestingOrigin);
}

extern WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition);

void WebContentsDelegateQt::launchExternalURL(const QUrl &url, ui::PageTransition page_transition, bool is_main_frame, bool has_user_gesture)
{
    WebEngineSettings *settings = m_viewClient->webEngineSettings();
    bool navigationAllowedByPolicy = false;
    bool navigationRequestAccepted = true;

    switch (settings->unknownUrlSchemePolicy()) {
    case WebEngineSettings::DisallowUnknownUrlSchemes:
        break;
    case WebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction:
        navigationAllowedByPolicy = has_user_gesture;
        break;
    case WebEngineSettings::AllowAllUnknownUrlSchemes:
        navigationAllowedByPolicy = true;
        break;
    default:
        Q_UNREACHABLE();
    }

    if (navigationAllowedByPolicy) {
        int navigationRequestAction = WebContentsAdapterClient::AcceptRequest;
        m_viewClient->navigationRequested(pageTransitionToNavigationType(page_transition), url, navigationRequestAction, is_main_frame);
        navigationRequestAccepted = navigationRequestAction == WebContentsAdapterClient::AcceptRequest;
#ifndef QT_NO_DESKTOPSERVICES
        if (navigationRequestAccepted)
            QDesktopServices::openUrl(url);
#endif
    }

    if (!navigationAllowedByPolicy || !navigationRequestAccepted) {
        QString errorDescription;
        if (!navigationAllowedByPolicy)
            errorDescription = QStringLiteral("Launching external protocol forbidden by WebEngineSettings::UnknownUrlSchemePolicy");
        else
            errorDescription = QStringLiteral("Launching external protocol suppressed by WebContentsAdapterClient::navigationRequested");
        didFailLoad(url, net::Error::ERR_ABORTED, errorDescription);
    }
}

void WebContentsDelegateQt::BeforeUnloadFired(content::WebContents *tab, bool proceed, bool *proceed_to_fire_unload)
{
    Q_UNUSED(tab);
    Q_ASSERT(proceed_to_fire_unload);
    *proceed_to_fire_unload = proceed;
    if (!proceed)
        m_viewClient->windowCloseRejected();
}

void WebContentsDelegateQt::BeforeUnloadFired(const base::TimeTicks &proceed_time) {
    Q_UNUSED(proceed_time);
}

bool WebContentsDelegateQt::CheckMediaAccessPermission(content::RenderFrameHost *, const GURL& security_origin, content::MediaStreamType type)
{
    switch (type) {
    case content::MEDIA_DEVICE_AUDIO_CAPTURE:
        return m_viewClient->profileAdapter()->checkPermission(toQt(security_origin), ProfileAdapter::AudioCapturePermission);
    case content::MEDIA_DEVICE_VIDEO_CAPTURE:
        return m_viewClient->profileAdapter()->checkPermission(toQt(security_origin), ProfileAdapter::VideoCapturePermission);
    default:
        LOG(INFO) << "WebContentsDelegateQt::CheckMediaAccessPermission: "
                  << "Unsupported media stream type checked" << type;
        return false;
    }
}

void WebContentsDelegateQt::RegisterProtocolHandler(content::WebContents *webContents, const std::string &protocol, const GURL &url, bool)
{
    content::BrowserContext *context = webContents->GetBrowserContext();
    if (context->IsOffTheRecord())
        return;

    ProtocolHandler handler =
        ProtocolHandler::CreateProtocolHandler(protocol, url);

    ProtocolHandlerRegistry *registry =
        ProtocolHandlerRegistryFactory::GetForBrowserContext(context);
    if (registry->SilentlyHandleRegisterHandlerRequest(handler))
        return;

    QWebEngineRegisterProtocolHandlerRequest request(
        QSharedPointer<RegisterProtocolHandlerRequestControllerImpl>::create(webContents, handler));
    m_viewClient->runRegisterProtocolHandlerRequest(std::move(request));
}

void WebContentsDelegateQt::UnregisterProtocolHandler(content::WebContents *webContents, const std::string &protocol, const GURL &url, bool)
{
    content::BrowserContext* context = webContents->GetBrowserContext();
    if (context->IsOffTheRecord())
        return;

    ProtocolHandler handler =
        ProtocolHandler::CreateProtocolHandler(protocol, url);

    ProtocolHandlerRegistry* registry =
        ProtocolHandlerRegistryFactory::GetForBrowserContext(context);
    registry->RemoveHandler(handler);
}

bool WebContentsDelegateQt::TakeFocus(content::WebContents *source, bool reverse)
{
    Q_UNUSED(source);
    return m_viewClient->passOnFocus(reverse);
}

FaviconManager *WebContentsDelegateQt::faviconManager()
{
    return m_faviconManager.data();
}

WebEngineSettings *WebContentsDelegateQt::webEngineSettings() const {
    return m_viewClient->webEngineSettings();
}

WebContentsAdapter *WebContentsDelegateQt::webContentsAdapter() const
{
    return m_viewClient->webContentsAdapter();
}


FrameFocusedObserver::FrameFocusedObserver(WebContentsAdapterClient *adapterClient)
    : m_viewClient(adapterClient)
{}

void FrameFocusedObserver::addNode(content::FrameTreeNode *node)
{
   if (m_observedNodes.contains(node))
       return;

   node->AddObserver(this);
   m_observedNodes.append(node);
}

void FrameFocusedObserver::removeNode(content::FrameTreeNode *node)
{
    node->RemoveObserver(this);
    m_observedNodes.removeOne(node);
}

void FrameFocusedObserver::OnFrameTreeNodeFocused(content::FrameTreeNode *node)
{
    Q_UNUSED(node);
    m_viewClient->updateEditActions();
}

void FrameFocusedObserver::OnFrameTreeNodeDestroyed(content::FrameTreeNode *node)
{
    m_observedNodes.removeOne(node);
    m_viewClient->updateEditActions();
}

FrameFocusedObserver::~FrameFocusedObserver()
{
    for (content::FrameTreeNode *node : m_observedNodes)
        node->RemoveObserver(this);
}

} // namespace QtWebEngineCore
