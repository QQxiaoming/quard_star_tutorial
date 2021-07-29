// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webgpu/webgpu_adapter.h"

#include "third_party/blink/renderer/modules/webgpu/webgpu_device.h"

namespace blink {

// static
WebGPUAdapter* WebGPUAdapter::Create(const String& name) {
  return new WebGPUAdapter(name);
}

const String& WebGPUAdapter::name() const {
  return name_;
}

WebGPUDevice* WebGPUAdapter::createDevice() {
  return WebGPUDevice::Create(this);
}

WebGPUAdapter::WebGPUAdapter(const String& name) : name_(name) {}

}  // namespace blink
