// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webgpu/webgpu.h"

#include "third_party/blink/renderer/modules/webgpu/webgpu_adapter.h"
#include "third_party/blink/renderer/modules/webgpu/webgpu_adapter_descriptor.h"

namespace blink {

// static
WebGPU* WebGPU::Create() {
  return new WebGPU();
}

WebGPUAdapter* WebGPU::getAdapter(const WebGPUAdapterDescriptor& descriptor) {
  return WebGPUAdapter::Create(descriptor.powerPreference());
}

WebGPU::WebGPU() {}

}  // namespace blink
