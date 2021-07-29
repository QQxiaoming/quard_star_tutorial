// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_OCULUS_DEVICE_H
#define DEVICE_VR_OCULUS_DEVICE_H

#include <memory>

#include "base/macros.h"
#include "base/single_thread_task_runner.h"
#include "device/vr/oculus/oculus_gamepad_data_fetcher.h"
#include "device/vr/public/mojom/vr_service.mojom.h"
#include "device/vr/vr_device_base.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "third_party/libovr/src/Include/OVR_CAPI.h"

namespace device {

class OculusRenderLoop;

class OculusDevice : public VRDeviceBase,
                     public mojom::XRSessionController,
                     public OculusGamepadDataProvider {
 public:
  explicit OculusDevice();
  ~OculusDevice() override;

  // VRDeviceBase
  void RequestSession(
      mojom::XRDeviceRuntimeSessionOptionsPtr options,
      mojom::XRRuntime::RequestSessionCallback callback) override;
  void OnMagicWindowFrameDataRequest(
      mojom::VRMagicWindowProvider::GetFrameDataCallback callback) override;
  void OnRequestSessionResult(
      mojom::XRRuntime::RequestSessionCallback callback,
      bool result,
      mojom::VRSubmitFrameClientRequest request,
      mojom::VRPresentationProviderPtrInfo provider_info,
      mojom::VRDisplayFrameTransportOptionsPtr transport_options);

  bool IsInitialized() { return !!session_; }

 private:
  // XRSessionController
  void SetFrameDataRestricted(bool restricted) override;

  void OnPresentingControllerMojoConnectionError();

  // OculusGamepadDataProvider
  void RegisterDataFetcher(OculusGamepadDataFetcher*) override;

  void OnPresentationEnded();
  void StartOvrSession();
  void StopOvrSession();

  void OnControllerUpdated(ovrInputState input,
                           ovrInputState remote,
                           ovrTrackingState tracking,
                           bool has_touch,
                           bool has_remote);

  std::unique_ptr<OculusRenderLoop> render_loop_;
  OculusGamepadDataFetcher* data_fetcher_ = nullptr;
  mojom::VRDisplayInfoPtr display_info_;
  ovrSession session_ = nullptr;
  OculusGamepadDataFetcher::Factory* oculus_gamepad_factory_ = nullptr;
  scoped_refptr<base::SingleThreadTaskRunner> main_thread_task_runner_;

  mojo::Binding<mojom::XRSessionController> exclusive_controller_binding_;

  base::WeakPtrFactory<OculusDevice> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(OculusDevice);
};

}  // namespace device

#endif  // DEVICE_VR_OCULUS_DEVICE_H
