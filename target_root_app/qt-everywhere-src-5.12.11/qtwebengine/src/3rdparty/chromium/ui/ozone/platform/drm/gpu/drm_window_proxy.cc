// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/drm/gpu/drm_window_proxy.h"

#include "ui/gfx/gpu_fence.h"
#include "ui/gfx/presentation_feedback.h"
#include "ui/ozone/platform/drm/gpu/drm_overlay_plane.h"
#include "ui/ozone/platform/drm/gpu/drm_thread.h"
#include "ui/ozone/platform/drm/gpu/proxy_helpers.h"
#include "ui/ozone/platform/drm/gpu/scanout_buffer.h"

namespace ui {

DrmWindowProxy::DrmWindowProxy(gfx::AcceleratedWidget widget,
                               DrmThread* drm_thread)
    : widget_(widget), drm_thread_(drm_thread) {}

DrmWindowProxy::~DrmWindowProxy() {}

void DrmWindowProxy::SchedulePageFlip(
    std::vector<DrmOverlayPlane> planes,
    SwapCompletionOnceCallback submission_callback,
    PresentationOnceCallback presentation_callback) {
  drm_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DrmThread::SchedulePageFlip,
                     base::Unretained(drm_thread_), widget_,
                     base::Passed(&planes),
                     CreateSafeOnceCallback(std::move(submission_callback)),
                     CreateSafeOnceCallback(std::move(presentation_callback))));
}

void DrmWindowProxy::GetVSyncParameters(
    const gfx::VSyncProvider::UpdateVSyncCallback& callback) {
  drm_thread_->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&DrmThread::GetVSyncParameters,
                                base::Unretained(drm_thread_), widget_,
                                CreateSafeCallback(callback)));
}

}  // namespace ui
