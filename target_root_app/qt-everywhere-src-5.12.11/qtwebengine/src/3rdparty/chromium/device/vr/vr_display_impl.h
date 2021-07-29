// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_VR_DISPLAY_IMPL_H
#define DEVICE_VR_VR_DISPLAY_IMPL_H

#include <memory>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "device/vr/public/mojom/isolated_xr_service.mojom.h"
#include "device/vr/public/mojom/vr_service.mojom.h"
#include "device/vr/vr_device.h"
#include "device/vr/vr_export.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "ui/display/display.h"

namespace device {

class VRDeviceBase;

// VR device process implementation of a VRMagicWindowProvider within a WebVR
// or WebXR site session.
// VRDisplayImpl objects are owned by their respective XRRuntime instances.
// TODO(offenwanger): Rename this.
class DEVICE_VR_EXPORT VRDisplayImpl : public mojom::VRMagicWindowProvider,
                                       public mojom::XRSessionController {
 public:
  VRDisplayImpl(VRDeviceBase* device,
                mojom::VRMagicWindowProviderRequest,
                mojom::XRSessionControllerRequest);
  ~VRDisplayImpl() override;

  gfx::Size sessionFrameSize() { return session_frame_size_; };
  display::Display::Rotation sessionRotation() { return session_rotation_; };

  device::VRDeviceBase* device() { return device_; };

  // Accessible to tests.
 protected:
  // mojom::VRMagicWindowProvider
  void GetFrameData(GetFrameDataCallback callback) override;
  void UpdateSessionGeometry(const gfx::Size& frame_size,
                             display::Display::Rotation rotation) override;
  void RequestHitTest(mojom::XRRayPtr ray,
                      RequestHitTestCallback callback) override;

  // mojom::XRSessionController
  void SetFrameDataRestricted(bool paused) override;

  void OnMojoConnectionError();

  mojo::Binding<mojom::VRMagicWindowProvider> magic_window_binding_;
  mojo::Binding<mojom::XRSessionController> session_controller_binding_;
  device::VRDeviceBase* device_;
  bool restrict_frame_data_ = true;

  gfx::Size session_frame_size_ = gfx::Size(0, 0);
  display::Display::Rotation session_rotation_ = display::Display::ROTATE_0;
};

}  // namespace device

#endif  //  DEVICE_VR_VR_DISPLAY_IMPL_H
