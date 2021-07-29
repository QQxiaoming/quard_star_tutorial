// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_OPENVR_DEVICE_H
#define DEVICE_VR_OPENVR_DEVICE_H

#include <memory>

#include "base/macros.h"
#include "base/single_thread_task_runner.h"
#include "device/vr/openvr/openvr_api_wrapper.h"
#include "device/vr/openvr/openvr_gamepad_data_fetcher.h"
#include "device/vr/public/mojom/vr_service.mojom.h"
#include "device/vr/vr_device_base.h"
#include "mojo/public/cpp/bindings/binding.h"

namespace device {

class OpenVRRenderLoop;
struct OpenVRGamepadState;

class OpenVRDevice : public VRDeviceBase,
                     public mojom::XRSessionController,
                     public OpenVRGamepadDataProvider {
 public:
  OpenVRDevice();
  ~OpenVRDevice() override;

  void Shutdown();

  // VRDeviceBase
  void RequestSession(
      mojom::XRDeviceRuntimeSessionOptionsPtr options,
      mojom::XRRuntime::RequestSessionCallback callback) override;

  void OnPollingEvents();

  void OnRequestSessionResult(
      mojom::XRRuntime::RequestSessionCallback callback,
      bool result,
      mojom::VRSubmitFrameClientRequest request,
      mojom::VRPresentationProviderPtrInfo provider_info,
      mojom::VRDisplayFrameTransportOptionsPtr transport_options);

  bool IsInitialized() { return !!openvr_; }

 private:
  // VRDeviceBase
  void OnMagicWindowFrameDataRequest(
      mojom::VRMagicWindowProvider::GetFrameDataCallback callback) override;

  // XRSessionController
  void SetFrameDataRestricted(bool restricted) override;

  void OnPresentingControllerMojoConnectionError();
  void OnPresentationEnded();

  void RegisterDataFetcher(OpenVRGamepadDataFetcher*) override;
  void OnGamepadUpdated(OpenVRGamepadState state);

  std::unique_ptr<OpenVRRenderLoop> render_loop_;
  mojom::VRDisplayInfoPtr display_info_;
  std::unique_ptr<OpenVRWrapper> openvr_;
  scoped_refptr<base::SingleThreadTaskRunner> main_thread_task_runner_;

  mojo::Binding<mojom::XRSessionController> exclusive_controller_binding_;
  OpenVRGamepadDataFetcher* gamepad_data_fetcher_ = nullptr;

  base::WeakPtrFactory<OpenVRDevice> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(OpenVRDevice);
};

}  // namespace device

#endif  // DEVICE_VR_OPENVR_DEVICE_H
