// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_TEST_WS_TEST_GPU_INTERFACE_PROVIDER_H_
#define SERVICES_UI_TEST_WS_TEST_GPU_INTERFACE_PROVIDER_H_

#include "services/ui/ws2/gpu_interface_provider.h"

namespace ui {
namespace test {

// TestGpuInterfaceProvider binds Gpu and DiscardableSharedMemoryManager
// requests with no-op stub functions to avoid test crashes.
class TestGpuInterfaceProvider : public ui::ws2::GpuInterfaceProvider {
 public:
  TestGpuInterfaceProvider();
  ~TestGpuInterfaceProvider() override;

  // ui::ws2::GpuInterfaceProvider:
  void RegisterGpuInterfaces(
      service_manager::BinderRegistry* registry) override;
#if defined(USE_OZONE)
  void RegisterOzoneGpuInterfaces(
      service_manager::BinderRegistry* registry) override;
#endif

 private:
  DISALLOW_COPY_AND_ASSIGN(TestGpuInterfaceProvider);
};

}  // namespace test
}  // namespace ui

#endif  // SERVICES_UI_TEST_WS_TEST_GPU_INTERFACE_PROVIDER_H_
