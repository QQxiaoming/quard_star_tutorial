// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_VR_DEVICE_BASE_H
#define DEVICE_VR_VR_DEVICE_BASE_H

#include "base/callback.h"
#include "base/macros.h"
#include "device/vr/public/mojom/vr_service.mojom.h"
#include "device/vr/vr_device.h"
#include "device/vr/vr_export.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "ui/display/display.h"

namespace device {

class VRDisplayImpl;

// Represents one of the platform's VR devices. Owned by the respective
// VRDeviceProvider.
// TODO(mthiesse, crbug.com/769373): Remove DEVICE_VR_EXPORT.
class DEVICE_VR_EXPORT VRDeviceBase : public mojom::XRRuntime {
 public:
  explicit VRDeviceBase(VRDeviceId id);
  ~VRDeviceBase() override;

  // VRDevice Implementation
  void ListenToDeviceChanges(
      mojom::XRRuntimeEventListenerPtr listener,
      mojom::XRRuntime::ListenToDeviceChangesCallback callback) final;
  void SetListeningForActivate(bool is_listening) override;

  void GetFrameData(
      mojom::VRMagicWindowProvider::GetFrameDataCallback callback);

  virtual void RequestHitTest(
      mojom::XRRayPtr ray,
      mojom::VRMagicWindowProvider::RequestHitTestCallback callback);
  unsigned int GetId() const;

  bool HasExclusiveSession();
  void EndMagicWindowSession(VRDisplayImpl* session);

  // TODO(https://crbug.com/845283): This method is a temporary solution
  // until a XR related refactor lands. It allows to keep using the
  // existing PauseTracking/ResumeTracking while not changing the
  // existing VR functionality.
  virtual bool ShouldPauseTrackingWhenFrameDataRestricted();

  // Devices may be paused/resumed when focus changes by VRDisplayImpl or
  // GVR delegate.
  virtual void PauseTracking();
  virtual void ResumeTracking();
  void SetMagicWindowEnabled(bool enabled);

  mojom::VRDisplayInfoPtr GetVRDisplayInfo();

  // Used by providers to bind devices.
  mojom::XRRuntimePtr BindXRRuntimePtr();

  // TODO(mthiesse): The browser should handle browser-side exiting of
  // presentation before device/ is even aware presentation is being exited.
  // Then the browser should call StopSession() on Device, which does device/
  // exiting of presentation before notifying displays. This is currently messy
  // because browser-side notions of presentation are mostly Android-specific.
  virtual void OnExitPresent();

 protected:
  // Devices tell VRDeviceBase when they start presenting.  It will be paired
  // with an OnExitPresent when the device stops presenting.
  void OnStartPresenting();
  bool IsPresenting() { return presenting_; }  // Exposed for test.
  void SetVRDisplayInfo(mojom::VRDisplayInfoPtr display_info);
  void OnActivate(mojom::VRDisplayEventReason reason,
                  base::Callback<void(bool)> on_handled);

  std::vector<std::unique_ptr<VRDisplayImpl>> magic_window_sessions_;

 private:
  // TODO(https://crbug.com/842227): Rename methods to HandleOnXXX
  virtual void OnListeningForActivate(bool listening);
  virtual void OnMagicWindowFrameDataRequest(
      mojom::VRMagicWindowProvider::GetFrameDataCallback callback);

  // XRRuntime
  void RequestMagicWindowSession(
      mojom::XRRuntime::RequestMagicWindowSessionCallback callback) override;

  mojom::XRRuntimeEventListenerPtr listener_;

  mojom::VRDisplayInfoPtr display_info_;

  bool presenting_ = false;

  unsigned int id_;
  bool magic_window_enabled_ = true;

  mojo::Binding<mojom::XRRuntime> runtime_binding_;

  DISALLOW_COPY_AND_ASSIGN(VRDeviceBase);
};

}  // namespace device

#endif  // DEVICE_VR_VR_DEVICE_BASE_H
