/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

// Based on chrome/browser/ui/webui/chrome_web_ui_controller_factory.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webui_controller_factory_qt.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/devtools_ui.h"
#include "chrome/browser/ui/webui/quota_internals/quota_internals_ui.h"
#include "chrome/browser/ui/webui/task_scheduler_internals/task_scheduler_internals_ui.h"
#include "chrome/common/url_constants.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/common/content_client.h"
#include "content/public/common/url_utils.h"
#include "extensions/buildflags/buildflags.h"
#include "media/media_buildflags.h"
#include "ppapi/buildflags/buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "ui/web_dialogs/web_dialog_ui.h"
#include "url/gurl.h"

#if defined(OS_LINUX) || defined(OS_ANDROID)
#include "chrome/browser/ui/webui/sandbox_internals_ui.h"
#endif

// The Following WebUIs are disabled because they currently doesn't build
// or doesn't work, but would be interesting for us if they did:

// #include "chrome/browser/ui/webui/inspect_ui.h"
// #include "chrome/browser/ui/webui/user_actions/user_actions_ui.h"

// #if BUILDFLAG(ENABLE_WEBRTC)
// #include "chrome/browser/ui/webui/media/webrtc_logs_ui.h"
// #endif

// #if BUILDFLAG(ENABLE_PRINT_PREVIEW)
// #include "chrome/browser/ui/webui/print_preview/print_preview_ui.h"
// #endif

// #if defined(USE_NSS_CERTS) && defined(USE_AURA)
// #include "chrome/browser/ui/webui/certificate_viewer_ui.h"
// #endif

// #if BUILDFLAG(ENABLE_EXTENSIONS)
// #include "chrome/browser/extensions/extension_web_ui.h"
// #include "chrome/browser/ui/webui/extensions/extensions_ui.h"
// #include "chrome/common/extensions/extension_constants.h"
// #include "extensions/browser/extension_registry.h"
// #include "extensions/browser/extension_system.h"
// #include "extensions/common/constants.h"
// #include "extensions/common/extension.h"
// #include "extensions/common/feature_switch.h"
// #include "extensions/common/manifest.h"
// #endif

using content::WebUI;
using content::WebUIController;

namespace {

// A function for creating a new WebUI. The caller owns the return value, which
// may be NULL (for example, if the URL refers to an non-existent extension).
typedef std::unique_ptr<WebUIController> (*WebUIFactoryFunction)(WebUI *web_ui, const GURL &url);

// Template for defining WebUIFactoryFunction.
template<class T>
std::unique_ptr<WebUIController> NewWebUI(WebUI *web_ui, const GURL &/*url*/)
{
    return std::unique_ptr<WebUIController>(new T(web_ui));
}

// Returns a function that can be used to create the right type of WebUI for a
// tab, based on its URL. Returns NULL if the URL doesn't have WebUI associated
// with it.
WebUIFactoryFunction GetWebUIFactoryFunction(WebUI *web_ui, Profile *profile, const GURL &url)
{
    // This will get called a lot to check all URLs, so do a quick check of other
    // schemes to filter out most URLs.
    if (!content::HasWebUIScheme(url))
        return NULL;

    // We must compare hosts only since some of the Web UIs append extra stuff
    // after the host name.
    if (url.host() == chrome::kChromeUIQuotaInternalsHost)
        return &NewWebUI<QuotaInternalsUI>;
    if (url.host_piece() == chrome::kChromeUITaskSchedulerInternalsHost)
        return &NewWebUI<TaskSchedulerInternalsUI>;

    if (url.SchemeIs(content::kChromeDevToolsScheme)) {
//        if (!DevToolsUIBindings::IsValidFrontendURL(url))
//            return nullptr;
        return &NewWebUI<DevToolsUI>;
    }

//    if (url.host_piece() == chrome::kChromeUIUserActionsHost)
//        return &NewWebUI<UserActionsUI>;
//    if (url.host_piece() == chrome::kChromeUIInspectHost)
//        return &NewWebUI<InspectUI>;
//
//#if defined(USE_NSS_CERTS) && defined(USE_AURA)
//    if (url.host_piece() == chrome::kChromeUICertificateViewerHost)
//        return &NewWebUI<CertificateViewerUI>;
//#endif  // USE_NSS_CERTS && USE_AURA
//#if BUILDFLAG(ENABLE_EXTENSIONS)
//    if (url.host_piece() == chrome::kChromeUIExtensionsFrameHost)
//        return &NewWebUI<extensions::ExtensionsUI>;
//#endif
//#if BUILDFLAG(ENABLE_PLUGINS)
//    if (url.host_piece() == chrome::kChromeUIFlashHost)
//        return &NewWebUI<FlashUI>;
//#endif
//#if BUILDFLAG(ENABLE_PRINT_PREVIEW)
//    if (url.host_piece() == chrome::kChromeUIPrintHost &&
//        !profile->GetPrefs()->GetBoolean(prefs::kPrintPreviewDisabled)) {
//        return &NewWebUI<PrintPreviewUI>;
//    }
//#endif
//#if BUILDFLAG(ENABLE_WEBRTC)
//    if (url.host_piece() == chrome::kChromeUIWebRtcLogsHost)
//        return &NewWebUI<WebRtcLogsUI>;
//#endif
#if defined(OS_LINUX) || defined(OS_ANDROID)
    if (url.host_piece() == chrome::kChromeUISandboxHost)
        return &NewWebUI<SandboxInternalsUI>;
#endif
    return nullptr;
}

} // namespace

namespace QtWebEngineCore {

WebUI::TypeID WebUIControllerFactoryQt::GetWebUIType(content::BrowserContext *browser_context, const GURL &url) const
{
    Profile *profile = Profile::FromBrowserContext(browser_context);
    WebUIFactoryFunction function = GetWebUIFactoryFunction(nullptr, profile, url);
    return function ? reinterpret_cast<WebUI::TypeID>(function) : WebUI::kNoWebUI;
}

bool WebUIControllerFactoryQt::UseWebUIForURL(content::BrowserContext *browser_context, const GURL &url) const
{
    return GetWebUIType(browser_context, url) != WebUI::kNoWebUI;
}

bool WebUIControllerFactoryQt::UseWebUIBindingsForURL(content::BrowserContext *browser_context, const GURL &url) const
{
    return UseWebUIForURL(browser_context, url);
}

std::unique_ptr<WebUIController> WebUIControllerFactoryQt::CreateWebUIControllerForURL(WebUI *web_ui, const GURL &url) const
{
    Profile *profile = Profile::FromWebUI(web_ui);
    WebUIFactoryFunction function = GetWebUIFactoryFunction(web_ui, profile, url);
    if (!function)
        return nullptr;

    return (*function)(web_ui, url);
}

// static
WebUIControllerFactoryQt *WebUIControllerFactoryQt::GetInstance()
{
    return base::Singleton<WebUIControllerFactoryQt>::get();
}

WebUIControllerFactoryQt::WebUIControllerFactoryQt()
{
}

WebUIControllerFactoryQt::~WebUIControllerFactoryQt()
{
}

} // namespace QtWebEngineCore
