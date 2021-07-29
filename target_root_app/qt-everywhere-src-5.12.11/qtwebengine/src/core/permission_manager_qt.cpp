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

#include "permission_manager_qt.h"

#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/permission_controller.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"

#include "type_conversion.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_settings.h"

namespace QtWebEngineCore {

ProfileAdapter::PermissionType toQt(content::PermissionType type)
{
    switch (type) {
    case content::PermissionType::GEOLOCATION:
        return ProfileAdapter::GeolocationPermission;
    case content::PermissionType::AUDIO_CAPTURE:
        return ProfileAdapter::AudioCapturePermission;
    case content::PermissionType::VIDEO_CAPTURE:
        return ProfileAdapter::VideoCapturePermission;
    case content::PermissionType::FLASH:
    case content::PermissionType::NOTIFICATIONS:
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
    case content::PermissionType::MIDI:
    case content::PermissionType::DURABLE_STORAGE:
    case content::PermissionType::BACKGROUND_SYNC:
    case content::PermissionType::SENSORS:
    case content::PermissionType::ACCESSIBILITY_EVENTS:
        break;
    case content::PermissionType::CLIPBOARD_READ:
        return ProfileAdapter::ClipboardRead;
    case content::PermissionType::CLIPBOARD_WRITE:
        return ProfileAdapter::ClipboardWrite;
    case content::PermissionType::PAYMENT_HANDLER:
    case content::PermissionType::NUM:
        break;
    }
    return ProfileAdapter::UnsupportedPermission;
}

PermissionManagerQt::PermissionManagerQt()
    : m_requestIdCount(0)
    , m_subscriberIdCount(0)
{
}

PermissionManagerQt::~PermissionManagerQt()
{
}

void PermissionManagerQt::permissionRequestReply(const QUrl &origin, ProfileAdapter::PermissionType type, bool reply)
{
    QPair<QUrl, ProfileAdapter::PermissionType> key(origin, type);
    m_permissions[key] = reply;
    blink::mojom::PermissionStatus status = reply ? blink::mojom::PermissionStatus::GRANTED : blink::mojom::PermissionStatus::DENIED;
    {
        auto it = m_requests.begin();
        while (it != m_requests.end()) {
            if (it->origin == origin && it->type == type) {
                it->callback.Run(status);
                it = m_requests.erase(it);
            } else
                ++it;
        }
    }
    for (const RequestOrSubscription &subscriber : qAsConst(m_subscribers)) {
        if (subscriber.origin == origin && subscriber.type == type)
            subscriber.callback.Run(status);
    }

    auto it = m_multiRequests.begin();
    while (it != m_multiRequests.end()) {
        if (it->origin == origin) {
            bool answerable = true;
            std::vector<blink::mojom::PermissionStatus> result;
            result.reserve(it->types.size());
            for (content::PermissionType permission : it->types) {
                const ProfileAdapter::PermissionType permissionType = toQt(permission);
                if (permissionType == ProfileAdapter::UnsupportedPermission) {
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
                    continue;
                }

                QPair<QUrl, ProfileAdapter::PermissionType> key(origin, permissionType);
                if (!m_permissions.contains(key)) {
                    answerable = false;
                    break;
                }
                if (m_permissions[key])
                    result.push_back(blink::mojom::PermissionStatus::GRANTED);
                else
                    result.push_back(blink::mojom::PermissionStatus::DENIED);
            }
            if (answerable) {
                it->callback.Run(result);
                it = m_multiRequests.erase(it);
                continue;
            }
        }
        ++it;
    }
}

bool PermissionManagerQt::checkPermission(const QUrl &origin, ProfileAdapter::PermissionType type)
{
    QPair<QUrl, ProfileAdapter::PermissionType> key(origin, type);
    return m_permissions.contains(key) && m_permissions[key];
}

int PermissionManagerQt::RequestPermission(content::PermissionType permission,
                                            content::RenderFrameHost *frameHost,
                                            const GURL& requesting_origin,
                                            bool /*user_gesture*/,
                                            const base::Callback<void(blink::mojom::PermissionStatus)>& callback)
{
    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
        content::WebContents::FromRenderFrameHost(frameHost)->GetDelegate());
    Q_ASSERT(contentsDelegate);

    ProfileAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == ProfileAdapter::UnsupportedPermission) {
        callback.Run(blink::mojom::PermissionStatus::DENIED);
        return content::PermissionController::kNoPendingOperation;
    } else if (permissionType == ProfileAdapter::ClipboardRead) {
        WebEngineSettings *settings = contentsDelegate->webEngineSettings();
        if (settings->testAttribute(WebEngineSettings::JavascriptCanAccessClipboard)
            && settings->testAttribute(WebEngineSettings::JavascriptCanPaste))
            callback.Run(blink::mojom::PermissionStatus::GRANTED);
        else
            callback.Run(blink::mojom::PermissionStatus::DENIED);
        return content::PermissionController::kNoPendingOperation;
    }
    // Audio and video-capture should not come this way currently
    Q_ASSERT(permissionType != ProfileAdapter::AudioCapturePermission
          && permissionType != ProfileAdapter::VideoCapturePermission);

    int request_id = ++m_requestIdCount;
    RequestOrSubscription request = {
        permissionType,
        toQt(requesting_origin),
        callback
    };
    m_requests.insert(request_id, request);
    if (permissionType == ProfileAdapter::GeolocationPermission)
        contentsDelegate->requestGeolocationPermission(request.origin);
    return request_id;
}

int PermissionManagerQt::RequestPermissions(const std::vector<content::PermissionType>& permissions,
                                            content::RenderFrameHost* frameHost,
                                            const GURL& requesting_origin,
                                            bool /*user_gesture*/,
                                            const base::Callback<void(const std::vector<blink::mojom::PermissionStatus>&)>& callback)
{
    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt *>(
        content::WebContents::FromRenderFrameHost(frameHost)->GetDelegate());
    Q_ASSERT(contentsDelegate);

    bool answerable = true;
    std::vector<blink::mojom::PermissionStatus> result;
    result.reserve(permissions.size());
    for (content::PermissionType permission : permissions) {
        const ProfileAdapter::PermissionType permissionType = toQt(permission);
        if (permissionType == ProfileAdapter::UnsupportedPermission)
            result.push_back(blink::mojom::PermissionStatus::DENIED);
        else if (permissionType == ProfileAdapter::ClipboardRead) {
            WebEngineSettings *settings = contentsDelegate->webEngineSettings();
            if (settings->testAttribute(WebEngineSettings::JavascriptCanAccessClipboard)
                && settings->testAttribute(WebEngineSettings::JavascriptCanPaste))
                result.push_back(blink::mojom::PermissionStatus::GRANTED);
            else
                result.push_back(blink::mojom::PermissionStatus::DENIED);
        } else {
            answerable = false;
            break;
        }
    }
    if (answerable) {
        callback.Run(result);
        return content::PermissionController::kNoPendingOperation;
    }

    int request_id = ++m_requestIdCount;
    MultiRequest request = {
        permissions,
        toQt(requesting_origin),
        callback
    };
    m_multiRequests.insert(request_id, request);
    for (content::PermissionType permission : permissions) {
        const ProfileAdapter::PermissionType permissionType = toQt(permission);
        if (permissionType == ProfileAdapter::GeolocationPermission)
            contentsDelegate->requestGeolocationPermission(request.origin);
    }
    return request_id;
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const ProfileAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == ProfileAdapter::UnsupportedPermission)
        return blink::mojom::PermissionStatus::DENIED;

    QPair<QUrl, ProfileAdapter::PermissionType> key(toQt(requesting_origin), permissionType);
    if (!m_permissions.contains(key))
        return blink::mojom::PermissionStatus::ASK;
    if (m_permissions[key])
        return blink::mojom::PermissionStatus::GRANTED;
    return blink::mojom::PermissionStatus::DENIED;
}

blink::mojom::PermissionStatus PermissionManagerQt::GetPermissionStatusForFrame(
        content::PermissionType permission,
        content::RenderFrameHost *render_frame_host,
        const GURL &requesting_origin)
{
    if (permission == content::PermissionType::CLIPBOARD_READ ||
            permission == content::PermissionType::CLIPBOARD_WRITE) {
        WebContentsDelegateQt *delegate = static_cast<WebContentsDelegateQt *>(
                content::WebContents::FromRenderFrameHost(render_frame_host)->GetDelegate());
        if (!delegate->webEngineSettings()->testAttribute(WebEngineSettings::JavascriptCanAccessClipboard))
            return blink::mojom::PermissionStatus::DENIED;
        if (permission == content::PermissionType::CLIPBOARD_READ &&
                !delegate->webEngineSettings()->testAttribute(WebEngineSettings::JavascriptCanPaste))
            return blink::mojom::PermissionStatus::DENIED;
        return blink::mojom::PermissionStatus::GRANTED;
    }

    return GetPermissionStatus(
                permission,
                requesting_origin,
                content::WebContents::FromRenderFrameHost(render_frame_host)->GetLastCommittedURL().GetOrigin());
}

void PermissionManagerQt::ResetPermission(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/)
{
    const ProfileAdapter::PermissionType permissionType = toQt(permission);
    if (permissionType == ProfileAdapter::UnsupportedPermission)
        return;

    QPair<QUrl, ProfileAdapter::PermissionType> key(toQt(requesting_origin), permissionType);
    m_permissions.remove(key);
}

int PermissionManagerQt::SubscribePermissionStatusChange(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& /*embedding_origin*/,
    const base::Callback<void(blink::mojom::PermissionStatus)>& callback)
{
    int subscriber_id = ++m_subscriberIdCount;
    RequestOrSubscription subscriber = {
        toQt(permission),
        toQt(requesting_origin),
        callback
    };
    m_subscribers.insert(subscriber_id, subscriber);
    return subscriber_id;
}

void PermissionManagerQt::UnsubscribePermissionStatusChange(int subscription_id)
{
    if (!m_subscribers.remove(subscription_id))
        qWarning() << "PermissionManagerQt::UnsubscribePermissionStatusChange called on unknown subscription id" << subscription_id;
}

} // namespace QtWebEngineCore
