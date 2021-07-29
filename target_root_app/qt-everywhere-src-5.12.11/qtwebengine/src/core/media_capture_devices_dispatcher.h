/****************************************************************************
**
** Copyright (c) 2012 The Chromium Authors. All rights reserved.
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

#ifndef MEDIA_CAPTURE_DEVICES_DISPATCHER_H
#define MEDIA_CAPTURE_DEVICES_DISPATCHER_H

#include <deque>
#include <list>
#include <map>

#include "web_contents_adapter_client.h"

#include "base/callback.h"
#include "base/containers/circular_deque.h"
#include "base/memory/singleton.h"
#include "base/observer_list.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/media_stream_request.h"

class DesktopStreamsRegistry;

namespace QtWebEngineCore {

// This singleton is used to receive updates about media events from the content
// layer. Based on Chrome's implementation.
class MediaCaptureDevicesDispatcher : public content::MediaObserver,
                                      public content::NotificationObserver
{
public:
    static MediaCaptureDevicesDispatcher *GetInstance();

    void processMediaAccessRequest(WebContentsAdapterClient *, content::WebContents *, const content::MediaStreamRequest &, content::MediaResponseCallback);

    // Called back from our WebContentsAdapter to grant the requested permission.
    void handleMediaAccessPermissionResponse(content::WebContents *, const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags);

private:
    void getDefaultDevices(const std::string &audioDeviceId, const std::string &videoDeviceId, bool audio, bool video, content::MediaStreamDevices *);

    // Overridden from content::MediaObserver:
    void OnAudioCaptureDevicesChanged() override {}
    void OnVideoCaptureDevicesChanged() override {}
    void OnMediaRequestStateChanged(int render_process_id,
                                    int render_frame_id,
                                    int page_request_id,
                                    const GURL &security_origin,
                                    content::MediaStreamType stream_type,
                                    content::MediaRequestState state) override;

    void OnCreatingAudioStream(int /*render_process_id*/, int /*render_frame_id*/) override {}
    void OnSetCapturingLinkSecured(int /*render_process_id*/,
                                   int /*render_frame_id*/,
                                   int /*page_request_id*/,
                                   content::MediaStreamType /*stream_type*/,
                                   bool /*is_secure*/) override {}

    DesktopStreamsRegistry *getDesktopStreamsRegistry();

    friend struct base::DefaultSingletonTraits<MediaCaptureDevicesDispatcher>;

    typedef base::RepeatingCallback<void(const content::MediaStreamDevices &devices,
                                         content::MediaStreamRequestResult result,
                                         std::unique_ptr<content::MediaStreamUI> ui)>
            RepeatingMediaResponseCallback;

    struct PendingAccessRequest {
        PendingAccessRequest(const content::MediaStreamRequest &request, const RepeatingMediaResponseCallback &callback);
        ~PendingAccessRequest();

        content::MediaStreamRequest request;
        RepeatingMediaResponseCallback callback;
    };
    typedef base::circular_deque<PendingAccessRequest> RequestsQueue;
    typedef std::map<content::WebContents *, RequestsQueue> RequestsQueues;

    MediaCaptureDevicesDispatcher();
    virtual ~MediaCaptureDevicesDispatcher();

    // content::NotificationObserver implementation.
    void Observe(int type, const content::NotificationSource &source, const content::NotificationDetails &details) override;

    // Helpers for ProcessMediaAccessRequest().
    void processDesktopCaptureAccessRequest(content::WebContents *, const content::MediaStreamRequest &, content::MediaResponseCallback);
    void enqueueMediaAccessRequest(content::WebContents *, const content::MediaStreamRequest &, content::MediaResponseCallback);
    void ProcessQueuedAccessRequest(content::WebContents *);

    // Called by the MediaObserver() functions, executed on UI thread.
    void updateMediaRequestStateOnUIThread(int render_process_id, int render_frame_id, int page_request_id, const GURL &security_origin, content::MediaStreamType stream_type, content::MediaRequestState state);

    RequestsQueues m_pendingRequests;

    std::unique_ptr<DesktopStreamsRegistry> m_desktopStreamsRegistry;

    content::NotificationRegistrar m_notificationsRegistrar;

    DISALLOW_COPY_AND_ASSIGN(MediaCaptureDevicesDispatcher);
};

} // namespace QtWebEngineCore

#endif // MEDIA_CAPTURE_DEVICES_DISPATCHER_H
