// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/test_ws/test_gpu_interface_provider.h"

#include "base/bind.h"
#include "components/discardable_memory/public/interfaces/discardable_shared_memory_manager.mojom.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/ui/public/interfaces/gpu.mojom.h"

namespace {

// A placeholder to prevent test crashes on unbound requests.
void BindGpuRequest(ui::mojom::GpuRequest request) {
  NOTIMPLEMENTED_LOG_ONCE();
}

// A placeholder to prevent test crashes on unbound requests.
void BindDiscardableSharedMemoryManagerRequest(
    discardable_memory::mojom::DiscardableSharedMemoryManagerRequest request) {
  NOTIMPLEMENTED_LOG_ONCE();
}

}  // namespace

namespace ui {
namespace test {

TestGpuInterfaceProvider::TestGpuInterfaceProvider() = default;
TestGpuInterfaceProvider::~TestGpuInterfaceProvider() = default;

void TestGpuInterfaceProvider::RegisterGpuInterfaces(
    service_manager::BinderRegistry* registry) {
  registry->AddInterface(
      base::BindRepeating(&BindDiscardableSharedMemoryManagerRequest));
  registry->AddInterface(base::BindRepeating(BindGpuRequest));
}

#if defined(USE_OZONE)
void TestGpuInterfaceProvider::RegisterOzoneGpuInterfaces(
    service_manager::BinderRegistry* registry) {}
#endif

}  // namespace test
}  // namespace ui
