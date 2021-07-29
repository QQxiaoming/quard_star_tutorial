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

// Based on chrome/renderer/content_settings_observer.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content_settings_observer_qt.h"

#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/platform/web_content_setting_callbacks.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/web/web_plugin_document.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "url/origin.h"

#include "common/qt_messages.h"

using blink::WebContentSettingCallbacks;
using blink::WebSecurityOrigin;
using blink::WebString;

namespace {

bool IsUniqueFrame(blink::WebFrame *frame)
{
    return frame->GetSecurityOrigin().IsUnique() ||
           frame->Top()->GetSecurityOrigin().IsUnique();
}

}  // namespace

namespace QtWebEngineCore {

ContentSettingsObserverQt::ContentSettingsObserverQt(content::RenderFrame *render_frame)
        : content::RenderFrameObserver(render_frame)
        , content::RenderFrameObserverTracker<ContentSettingsObserverQt>(render_frame)
        , m_currentRequestId(0)
{
    ClearBlockedContentSettings();
    render_frame->GetWebFrame()->SetContentSettingsClient(this);
}

ContentSettingsObserverQt::~ContentSettingsObserverQt() {
}

bool ContentSettingsObserverQt::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ContentSettingsObserverQt, message)
        IPC_MESSAGE_HANDLER(QtWebEngineMsg_RequestFileSystemAccessAsyncResponse,
                            OnRequestFileSystemAccessAsyncResponse)
    IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()

    return handled;
}

void ContentSettingsObserverQt::DidCommitProvisionalLoad(bool /*is_new_navigation*/,
                                                         bool is_same_document_navigation)
{
    blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
    if (frame->Parent())
        return;  // Not a top-level navigation.

    if (!is_same_document_navigation)
        ClearBlockedContentSettings();

    GURL url = frame->GetDocument().Url();
    // If we start failing this DCHECK, please makes sure we don't regress
    // this bug: http://code.google.com/p/chromium/issues/detail?id=79304
    DCHECK(frame->GetDocument().GetSecurityOrigin().ToString() == "null" ||
           !url.SchemeIs(url::kDataScheme));
}

void ContentSettingsObserverQt::OnDestruct()
{
    delete this;
}

bool ContentSettingsObserverQt::AllowDatabase(const WebString &name,
                                              const WebString &display_name,
                                              unsigned /*estimated_size*/)
{
    blink::WebFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame))
        return false;

    bool result = false;
    Send(new QtWebEngineHostMsg_AllowDatabase(
             routing_id(), url::Origin(frame->GetSecurityOrigin()).GetURL(),
             url::Origin(frame->Top()->GetSecurityOrigin()).GetURL(), name.Utf16(),
             display_name.Utf16(), &result));
    return result;
}

void ContentSettingsObserverQt::RequestFileSystemAccessAsync(const WebContentSettingCallbacks &callbacks)
{
    blink::WebFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame)) {
        WebContentSettingCallbacks permissionCallbacks(callbacks);
        permissionCallbacks.DoDeny();
        return;
    }
    ++m_currentRequestId;
    bool inserted = m_permissionRequests.insert(std::make_pair(m_currentRequestId, callbacks)).second;

    // Verify there are no duplicate insertions.
    DCHECK(inserted);

    Send(new QtWebEngineHostMsg_RequestFileSystemAccessAsync(
             routing_id(), m_currentRequestId,
             url::Origin(frame->GetSecurityOrigin()).GetURL(),
             url::Origin(frame->Top()->GetSecurityOrigin()).GetURL()));
}

bool ContentSettingsObserverQt::AllowIndexedDB(const WebString &name,
                                               const WebSecurityOrigin &/*origin*/)
{
    blink::WebFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame))
        return false;

    bool result = false;
    Send(new QtWebEngineHostMsg_AllowIndexedDB(
             routing_id(), url::Origin(frame->GetSecurityOrigin()).GetURL(),
             url::Origin(frame->Top()->GetSecurityOrigin()).GetURL(), name.Utf16(),
             &result));
    return result;
}

bool ContentSettingsObserverQt::AllowStorage(bool local)
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame))
        return false;

    StoragePermissionsKey key(url::Origin(frame->GetDocument().GetSecurityOrigin()).GetURL(), local);
    const auto permissions = m_cachedStoragePermissions.find(key);
    if (permissions != m_cachedStoragePermissions.end())
        return permissions->second;

    bool result = false;
    Send(new QtWebEngineHostMsg_AllowDOMStorage(
             routing_id(), url::Origin(frame->GetSecurityOrigin()).GetURL(),
             url::Origin(frame->Top()->GetSecurityOrigin()).GetURL(), local, &result));
    m_cachedStoragePermissions[key] = result;
    return result;
}

void ContentSettingsObserverQt::OnRequestFileSystemAccessAsyncResponse(int request_id, bool allowed)
{
    auto it = m_permissionRequests.find(request_id);
    if (it == m_permissionRequests.end())
        return;

    WebContentSettingCallbacks callbacks = it->second;
    m_permissionRequests.erase(it);

    if (allowed) {
        callbacks.DoAllow();
        return;
    }
    callbacks.DoDeny();
}

void ContentSettingsObserverQt::ClearBlockedContentSettings()
{
    m_cachedStoragePermissions.clear();
}

} // namespace QtWebEngineCore
