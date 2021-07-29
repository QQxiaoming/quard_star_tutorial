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

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SETTINGS_OBSERVER_QT_H
#define CONTENT_SETTINGS_OBSERVER_QT_H

#include "base/containers/flat_map.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "third_party/blink/public/platform/web_content_settings_client.h"
#include "url/gurl.h"

namespace blink {
class WebContentSettingCallbacks;
class WebSecurityOrigin;
}

namespace QtWebEngineCore {

// Handles blocking content per content settings for each RenderFrame.
class ContentSettingsObserverQt
        : public content::RenderFrameObserver
        , public content::RenderFrameObserverTracker<ContentSettingsObserverQt>
        , public blink::WebContentSettingsClient
{
public:
    ContentSettingsObserverQt(content::RenderFrame *render_frame);
    ~ContentSettingsObserverQt() override;

    // blink::WebContentSettingsClient:
    bool AllowDatabase(const blink::WebString &name,
                       const blink::WebString &display_name,
                       unsigned estimated_size) override;
    void RequestFileSystemAccessAsync(const blink::WebContentSettingCallbacks &callbacks) override;
    bool AllowIndexedDB(const blink::WebString &name,
                        const blink::WebSecurityOrigin &origin) override;
    bool AllowStorage(bool local) override;

private:

    // RenderFrameObserver implementation:
    bool OnMessageReceived(const IPC::Message &message) override;
    void DidCommitProvisionalLoad(bool is_new_navigation,
                                  bool is_same_document_navigation) override;
    void OnDestruct() override;

    // Message handlers.
    void OnRequestFileSystemAccessAsyncResponse(int request_id, bool allowed);

    // Clears m_cachedStoragePermissions
    void ClearBlockedContentSettings();

    // Caches the result of AllowStorage.
    using StoragePermissionsKey = std::pair<GURL, bool>;
    base::flat_map<StoragePermissionsKey, bool> m_cachedStoragePermissions;

    int m_currentRequestId;
    base::flat_map<int, blink::WebContentSettingCallbacks> m_permissionRequests;

    DISALLOW_COPY_AND_ASSIGN(ContentSettingsObserverQt);
};

} // namespace QtWebEngineCore

#endif  // RENDERER_CONTENT_SETTINGS_OBSERVER_QT_H
