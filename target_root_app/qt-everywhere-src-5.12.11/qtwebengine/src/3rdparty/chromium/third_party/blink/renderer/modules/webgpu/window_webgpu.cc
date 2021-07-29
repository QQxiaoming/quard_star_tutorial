// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webgpu/window_webgpu.h"

#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/modules/webgpu/webgpu.h"

namespace blink {

const char WindowWebGPU::kSupplementName[] = "WindowWebGPU";

// static
WindowWebGPU& WindowWebGPU::From(LocalDOMWindow& window) {
  WindowWebGPU* supplement =
      Supplement<LocalDOMWindow>::From<WindowWebGPU>(window);
  if (!supplement) {
    supplement = new WindowWebGPU(window);
    ProvideTo(window, supplement);
  }
  return *supplement;
}

// static
WebGPU* WindowWebGPU::webgpu(LocalDOMWindow& window) {
  return WindowWebGPU::From(window).webgpu();
}

WebGPU* WindowWebGPU::webgpu() const {
  if (!webgpu_) {
    webgpu_ = WebGPU::Create();
  }
  return webgpu_.Get();
}

void WindowWebGPU::Trace(blink::Visitor* visitor) {
  visitor->Trace(webgpu_);
  Supplement<LocalDOMWindow>::Trace(visitor);
}

WindowWebGPU::WindowWebGPU(LocalDOMWindow& window)
    : Supplement<LocalDOMWindow>(window) {}

}  // namespace blink
