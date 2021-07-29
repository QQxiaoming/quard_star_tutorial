// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/vr/vr_display_impl.h"

#include <utility>

#include "base/bind.h"
#include "device/vr/vr_device_base.h"

namespace {
constexpr int kMaxImageHeightOrWidth = 8000;
}  // namespace

namespace device {

VRDisplayImpl::VRDisplayImpl(
    VRDeviceBase* device,
    mojom::VRMagicWindowProviderRequest magic_window_request,
    mojom::XRSessionControllerRequest session_request)
    : magic_window_binding_(this),
      session_controller_binding_(this),
      device_(device) {
  magic_window_binding_.Bind(std::move(magic_window_request));
  session_controller_binding_.Bind(std::move(session_request));

  // Unretained is safe because the binding will close when we are destroyed,
  // so we won't receive any more callbacks after that.
  session_controller_binding_.set_connection_error_handler(base::BindOnce(
      &VRDisplayImpl::OnMojoConnectionError, base::Unretained(this)));
}

VRDisplayImpl::~VRDisplayImpl() = default;

// Gets frame data for sessions.
void VRDisplayImpl::GetFrameData(
    mojom::VRMagicWindowProvider::GetFrameDataCallback callback) {
  if (device_->HasExclusiveSession() || restrict_frame_data_) {
    std::move(callback).Run(nullptr);
    return;
  }

  device_->GetFrameData(std::move(callback));
}

void VRDisplayImpl::UpdateSessionGeometry(const gfx::Size& frame_size,
                                          display::Display::Rotation rotation) {
  // Check for a valid frame size.

  // TODO(https://crbug.com/841062, https://crbug.com/836496): Reconsider how we
  // check the sizes.
  if (frame_size.width() <= 0 || frame_size.height() <= 0 ||
      frame_size.width() > kMaxImageHeightOrWidth ||
      frame_size.height() > kMaxImageHeightOrWidth) {
    DLOG(ERROR) << "Invalid frame size passed to UpdateSessionGeometry.";
    return;
  }

  session_frame_size_ = frame_size;
  session_rotation_ = rotation;
}

void VRDisplayImpl::RequestHitTest(
    mojom::XRRayPtr ray,
    mojom::VRMagicWindowProvider::RequestHitTestCallback callback) {
  if (restrict_frame_data_) {
    std::move(callback).Run(base::nullopt);
    return;
  }
  device_->RequestHitTest(std::move(ray), std::move(callback));
}

// XRSessionController
void VRDisplayImpl::SetFrameDataRestricted(bool frame_data_restricted) {
  restrict_frame_data_ = frame_data_restricted;
  if (device_->ShouldPauseTrackingWhenFrameDataRestricted()) {
    if (restrict_frame_data_) {
      device_->PauseTracking();
    } else {
      device_->ResumeTracking();
    }
  }
}

void VRDisplayImpl::OnMojoConnectionError() {
  magic_window_binding_.Close();
  session_controller_binding_.Close();
  device_->EndMagicWindowSession(this);  // This call will destroy us.
}

}  // namespace device
