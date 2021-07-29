// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_OPENVR_OPENVR_API_WRAPPER_H_
#define DEVICE_VR_OPENVR_OPENVR_API_WRAPPER_H_

#include "base/memory/scoped_refptr.h"
#include "third_party/openvr/src/headers/openvr.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace device {
class OpenVRTestHook;
class TestHookRegistration;

class OpenVRWrapper {
 public:
  OpenVRWrapper(bool for_rendering);
  ~OpenVRWrapper();

  bool IsInitialized() { return initialized_; }

  // Gets the OpenVR API objects.
  // Ensures that they are used in a single thread at a time.
  vr::IVRCompositor* GetCompositor();
  vr::IVRSystem* GetSystem();

  static void SetTestHook(OpenVRTestHook* hook);

 private:
  bool Initialize(bool for_rendering);
  void Uninitialize();

  vr::IVRSystem* system_ = nullptr;
  vr::IVRCompositor* compositor_ = nullptr;
  scoped_refptr<base::SingleThreadTaskRunner> current_task_runner_;
  bool initialized_ = false;

  static TestHookRegistration* test_hook_registration_;
  static OpenVRTestHook* test_hook_;
  static bool any_initialized_;
};

std::string GetOpenVRString(vr::IVRSystem* vr_system,
                            vr::TrackedDeviceProperty prop);

}  // namespace device

#endif  // DEVICE_VR_OPENVR_OPENVR_API_WRAPPER_H_