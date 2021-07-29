// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/drm/gpu/crtc_controller.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "ui/gfx/presentation_feedback.h"
#include "ui/ozone/platform/drm/gpu/drm_device.h"
#include "ui/ozone/platform/drm/gpu/hardware_display_plane.h"
#include "ui/ozone/platform/drm/gpu/page_flip_request.h"
#include "ui/ozone/platform/drm/gpu/scanout_buffer.h"

namespace ui {

CrtcController::CrtcController(const scoped_refptr<DrmDevice>& drm,
                               uint32_t crtc,
                               uint32_t connector)
    : drm_(drm),
      crtc_(crtc),
      connector_(connector) {}

CrtcController::~CrtcController() {
  if (!is_disabled_) {
    const std::vector<std::unique_ptr<HardwareDisplayPlane>>& all_planes =
        drm_->plane_manager()->planes();
    for (const auto& plane : all_planes) {
      if (plane->owning_crtc() == crtc_) {
        plane->set_owning_crtc(0);
        plane->set_in_use(false);
      }
    }

    SetCursor(nullptr);
    drm_->DisableCrtc(crtc_);
  }
}

bool CrtcController::Modeset(const DrmOverlayPlane& plane,
                             drmModeModeInfo mode) {
  if (!drm_->SetCrtc(crtc_, plane.buffer->GetOpaqueFramebufferId(),
                     std::vector<uint32_t>(1, connector_), &mode)) {
    PLOG(ERROR) << "Failed to modeset: crtc=" << crtc_
                << " connector=" << connector_
                << " framebuffer_id=" << plane.buffer->GetOpaqueFramebufferId()
                << " mode=" << mode.hdisplay << "x" << mode.vdisplay << "@"
                << mode.vrefresh;
    return false;
  }

  mode_ = mode;
  is_disabled_ = false;

  ResetCursor();

  return true;
}

bool CrtcController::Disable() {
  if (is_disabled_)
    return true;

  is_disabled_ = true;
  return drm_->DisableCrtc(crtc_);
}

bool CrtcController::AssignOverlayPlanes(HardwareDisplayPlaneList* plane_list,
                                         const DrmOverlayPlaneList& overlays) {
  DCHECK(!is_disabled_);

  const DrmOverlayPlane* primary = DrmOverlayPlane::GetPrimaryPlane(overlays);
  if (primary && !drm_->plane_manager()->ValidatePrimarySize(*primary, mode_)) {
    VLOG(2) << "Trying to pageflip a buffer with the wrong size. Expected "
            << mode_.hdisplay << "x" << mode_.vdisplay << " got "
            << primary->buffer->GetSize().ToString() << " for"
            << " crtc=" << crtc_ << " connector=" << connector_;
    return true;
  }

  if (!drm_->plane_manager()->AssignOverlayPlanes(plane_list, overlays, crtc_,
                                                  this)) {
    PLOG(ERROR) << "Failed to assign overlay planes for crtc " << crtc_;
    return false;
  }

  return true;
}

std::vector<uint64_t> CrtcController::GetFormatModifiers(uint32_t format) {
  return drm_->plane_manager()->GetFormatModifiers(crtc_, format);
}

bool CrtcController::SetCursor(const scoped_refptr<ScanoutBuffer>& buffer) {
  DCHECK(!is_disabled_ || !buffer);
  cursor_buffer_ = buffer;

  return ResetCursor();
}

bool CrtcController::MoveCursor(const gfx::Point& location) {
  DCHECK(!is_disabled_);
  return drm_->MoveCursor(crtc_, location);
}

bool CrtcController::ResetCursor() {
  uint32_t handle = 0;
  gfx::Size size;

  if (cursor_buffer_) {
    handle = cursor_buffer_->GetHandle();
    size = cursor_buffer_->GetSize();
  }

  bool status = drm_->SetCursor(crtc_, handle, size);
  if (!status) {
    PLOG(ERROR) << "drmModeSetCursor: device " << drm_->device_path().value()
                << " crtc " << crtc_ << " handle " << handle << " size "
                << size.ToString();
  }

  return status;
}

}  // namespace ui
