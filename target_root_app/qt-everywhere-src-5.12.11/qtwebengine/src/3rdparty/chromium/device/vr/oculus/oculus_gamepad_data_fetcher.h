// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_OCULUS_GAMEPAD_DATA_FETCHER_H_
#define DEVICE_VR_OCULUS_GAMEPAD_DATA_FETCHER_H_

#include "base/synchronization/lock.h"
#include "device/gamepad/gamepad_data_fetcher.h"
#include "third_party/libovr/src/Include/OVR_CAPI.h"

namespace device {

class OculusGamepadDataFetcher;

class OculusGamepadDataProvider {
 public:
  virtual void RegisterDataFetcher(OculusGamepadDataFetcher*) = 0;
};

struct OculusInputData {
  ovrInputState input_touch = {};
  ovrInputState input_remote = {};
  ovrTrackingState tracking = {};
  bool have_input_touch = false;
  bool have_input_remote = false;
};

class OculusGamepadDataFetcher : public GamepadDataFetcher {
 public:
  class Factory : public GamepadDataFetcherFactory {
   public:
    Factory(unsigned int display_id, OculusGamepadDataProvider* provider);
    ~Factory() override;
    std::unique_ptr<GamepadDataFetcher> CreateDataFetcher() override;
    GamepadSource source() override;

   private:
    unsigned int display_id_;
    OculusGamepadDataProvider* provider_;
  };

  OculusGamepadDataFetcher(unsigned int display_id,
                           OculusGamepadDataProvider* provider);
  ~OculusGamepadDataFetcher() override;

  GamepadSource source() override;

  void GetGamepadData(bool devices_changed_hint) override;
  void PauseHint(bool paused) override;
  void OnAddedToProvider() override;

  void UpdateGamepadData(OculusInputData data);  // Called on UI thread.

 private:
  unsigned int display_id_;

  // Protects access to data_, which is read/written on different threads.
  base::Lock lock_;

  // have_input_* are initialized to false, so we won't try to read uninialized
  // data from this if we read data_ before UpdateGamepadData is called.
  OculusInputData data_;

  base::WeakPtrFactory<OculusGamepadDataFetcher> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(OculusGamepadDataFetcher);
};

}  // namespace device

#endif  // DEVICE_VR_OCULUS_GAMEPAD_DATA_FETCHER_H_
