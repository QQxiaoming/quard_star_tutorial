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

#include "quota_permission_context_qt.h"

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "quota_request_controller_impl.h"
#include "qwebenginequotarequest.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"

using content::QuotaPermissionContext;
using content::RenderFrameHost;
using content::StorageQuotaParams;
using content::WebContents;

namespace QtWebEngineCore {

void QuotaPermissionContextQt::RequestQuotaPermission(const StorageQuotaParams &params, int render_process_id, const PermissionCallback &callback)
{
    if (params.storage_type != blink::mojom::StorageType::kPersistent) {
        // For now we only support requesting quota with this interface
        // for Persistent storage type.
        callback.Run(QUOTA_PERMISSION_RESPONSE_DISALLOW);
        return;
    }

    if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
        content::BrowserThread::PostTask(
            content::BrowserThread::UI, FROM_HERE,
            base::Bind(&QuotaPermissionContextQt::RequestQuotaPermission, this,
                       params, render_process_id, callback));
        return;
    }

    RenderFrameHost *renderFrameHost = RenderFrameHost::FromID(render_process_id, params.render_frame_id);
    if (!renderFrameHost)
        return;

    WebContents *webContents = WebContents::FromRenderFrameHost(renderFrameHost);
    if (!webContents)
        return;

    WebContentsAdapterClient *client = WebContentsViewQt::from(static_cast<content::WebContentsImpl *>(webContents)->GetView())->client();
    if (!client)
        return;

    QWebEngineQuotaRequest request(
        QSharedPointer<QuotaRequestControllerImpl>::create(this, params, callback));
    client->runQuotaRequest(std::move(request));
}

void QuotaPermissionContextQt::dispatchCallbackOnIOThread(const PermissionCallback &callback,
                                                          QuotaPermissionContext::QuotaPermissionResponse response)
{
    if (callback.is_null())
        return;

    if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
        content::BrowserThread::PostTask(
            content::BrowserThread::IO, FROM_HERE,
            base::Bind(&QuotaPermissionContextQt::dispatchCallbackOnIOThread,
                       this, callback, response));
        return;
    }

    callback.Run(response);
}

} // namespace QtWebEngineCore
