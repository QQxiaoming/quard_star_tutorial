// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_OPENVR_GAMEPAD_DATA_FETCHER_H_
#define DEVICE_VR_OPENVR_GAMEPAD_DATA_FETCHER_H_

#include "device/gamepad/gamepad_data_fetcher.h"
#include "third_party/openvr/src/headers/openvr.h"

namespace device {

class OpenVRGamepadDataFetcher;

class OpenVRGamepadDataProvider {
 public:
  virtual void RegisterDataFetcher(OpenVRGamepadDataFetcher*) = 0;
};

struct OpenVRGamepadState {
  int32_t axis_type[vr::k_unMaxTrackedDeviceCount]
                   [vr::k_unControllerStateAxisCount];
  int32_t ButtonMaskFromId[vr::k_unMaxTrackedDeviceCount]
                          [vr::k_unControllerStateAxisCount];

  vr::TrackedDevicePose_t tracked_devices_poses[vr::k_unMaxTrackedDeviceCount];
  vr::ETrackedDeviceClass device_class[vr::k_unMaxTrackedDeviceCount];
  vr::VRControllerState_t controller_state[vr::k_unMaxTrackedDeviceCount];
  vr::ETrackedControllerRole hand[vr::k_unMaxTrackedDeviceCount];
  uint64_t supported_buttons[vr::k_unMaxTrackedDeviceCount];

  bool have_controller_state[vr::k_unMaxTrackedDeviceCount] = {};
};

class OpenVRGamepadDataFetcher : public GamepadDataFetcher {
 public:
  class Factory : public GamepadDataFetcherFactory {
   public:
    Factory(unsigned int display_id, OpenVRGamepadDataProvider*);
    ~Factory() override;
    std::unique_ptr<GamepadDataFetcher> CreateDataFetcher() override;
    GamepadSource source() override;

   private:
    unsigned int display_id_;
    OpenVRGamepadDataProvider* provider_;
  };

  OpenVRGamepadDataFetcher(unsigned int display_id, OpenVRGamepadDataProvider*);
  ~OpenVRGamepadDataFetcher() override;

  GamepadSource source() override;

  void GetGamepadData(bool devices_changed_hint) override;
  void PauseHint(bool paused) override;
  void OnAddedToProvider() override;

  void UpdateGamepadData(OpenVRGamepadState);  // Called on UI thread.

 private:
  unsigned int display_id_;

  // Protects access to data_, which is read/written on different threads.
  base::Lock lock_;

  OpenVRGamepadState data_;

  DISALLOW_COPY_AND_ASSIGN(OpenVRGamepadDataFetcher);
};

}  // namespace device
#endif  // DEVICE_VR_OPENVR_GAMEPAD_DATA_FETCHER_H_
