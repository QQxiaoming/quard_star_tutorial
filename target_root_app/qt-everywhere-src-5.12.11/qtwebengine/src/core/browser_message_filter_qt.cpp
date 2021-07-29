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

#include "browser_message_filter_qt.h"

#include "chrome/browser/profiles/profile.h"
#include "common/qt_messages.h"
#include "content/public/browser/plugin_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "type_conversion.h"

#include "net/network_delegate_qt.h"

namespace QtWebEngineCore {

BrowserMessageFilterQt::BrowserMessageFilterQt(int /*render_process_id*/, Profile *profile)
    : BrowserMessageFilter(QtMsgStart)
    , m_profile(profile)
{
}

// The following is based on chrome/browser/plugins/plugin_info_message_filter.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

bool BrowserMessageFilterQt::OnMessageReceived(const IPC::Message& message)
{
    IPC_BEGIN_MESSAGE_MAP(BrowserMessageFilterQt, message)
        IPC_MESSAGE_HANDLER(QtWebEngineHostMsg_AllowDatabase, OnAllowDatabase)
        IPC_MESSAGE_HANDLER(QtWebEngineHostMsg_AllowDOMStorage, OnAllowDOMStorage)
        IPC_MESSAGE_HANDLER_DELAY_REPLY(QtWebEngineHostMsg_RequestFileSystemAccessSync,
                                        OnRequestFileSystemAccessSync)
        IPC_MESSAGE_HANDLER(QtWebEngineHostMsg_RequestFileSystemAccessAsync,
                            OnRequestFileSystemAccessAsync)
        IPC_MESSAGE_HANDLER(QtWebEngineHostMsg_AllowIndexedDB, OnAllowIndexedDB)
        IPC_MESSAGE_UNHANDLED(return false)
    IPC_END_MESSAGE_MAP()
    return true;
}

void BrowserMessageFilterQt::OnAllowDatabase(int /*render_frame_id*/,
                                             const GURL &origin_url,
                                             const GURL &top_origin_url,
                                             const base::string16 &/*name*/,
                                             const base::string16 &/*display_name*/,
                                             bool* allowed)
{
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(m_profile->GetRequestContext()->GetURLRequestContext()->network_delegate());
    *allowed = networkDelegate->canGetCookies(top_origin_url, origin_url);
}

void BrowserMessageFilterQt::OnAllowDOMStorage(int /*render_frame_id*/,
                                               const GURL &origin_url,
                                               const GURL &top_origin_url,
                                               bool /*local*/,
                                               bool *allowed)
{
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(m_profile->GetRequestContext()->GetURLRequestContext()->network_delegate());
    *allowed = networkDelegate->canGetCookies(top_origin_url, origin_url);
}

void BrowserMessageFilterQt::OnAllowIndexedDB(int /*render_frame_id*/,
                                              const GURL &origin_url,
                                              const GURL &top_origin_url,
                                              const base::string16 &/*name*/,
                                              bool *allowed)
{
    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(m_profile->GetRequestContext()->GetURLRequestContext()->network_delegate());
    *allowed = networkDelegate->canGetCookies(top_origin_url, origin_url);
}

void BrowserMessageFilterQt::OnRequestFileSystemAccessSync(int render_frame_id,
                                                           const GURL& origin_url,
                                                           const GURL& top_origin_url,
                                                           IPC::Message* reply_msg)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    base::Callback<void(bool)> callback = base::Bind(
            &BrowserMessageFilterQt::OnRequestFileSystemAccessSyncResponse,
            base::WrapRefCounted(this), reply_msg);
    OnRequestFileSystemAccess(render_frame_id,
                              origin_url,
                              top_origin_url,
                              callback);
}

void BrowserMessageFilterQt::OnRequestFileSystemAccessSyncResponse(IPC::Message *reply_msg, bool allowed)
{
    QtWebEngineHostMsg_RequestFileSystemAccessSync::WriteReplyParams(reply_msg, allowed);
    Send(reply_msg);
}

void BrowserMessageFilterQt::OnRequestFileSystemAccessAsync(int render_frame_id,
                                                            int request_id,
                                                            const GURL& origin_url,
                                                            const GURL& top_origin_url)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    base::Callback<void(bool)> callback = base::Bind(
            &BrowserMessageFilterQt::OnRequestFileSystemAccessAsyncResponse,
            base::WrapRefCounted(this), render_frame_id, request_id);
    OnRequestFileSystemAccess(render_frame_id,
                              origin_url,
                              top_origin_url,
                              callback);
}

void BrowserMessageFilterQt::OnRequestFileSystemAccessAsyncResponse(int render_frame_id,
                                                                    int request_id,
                                                                    bool allowed)
{
    Send(new QtWebEngineMsg_RequestFileSystemAccessAsyncResponse(render_frame_id, request_id, allowed));
}

void BrowserMessageFilterQt::OnRequestFileSystemAccess(int /*render_frame_id*/,
                                                       const GURL &origin_url,
                                                       const GURL &top_origin_url,
                                                       base::Callback<void(bool)> callback)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    NetworkDelegateQt *networkDelegate = static_cast<NetworkDelegateQt *>(m_profile->GetRequestContext()->GetURLRequestContext()->network_delegate());
    bool allowed = networkDelegate->canGetCookies(top_origin_url, origin_url);

    callback.Run(allowed);
}

} // namespace QtWebEngineCore
