// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/remoting_event_injector.h"

#include "ui/events/keycodes/dom/keycode_converter.h"
#include "ui/events/system_input_injector.h"

namespace ui {
namespace ws2 {
namespace {

// Converts an InjectedMouseButtonTypeToEventFlags to EventFlags, which is what
// SystemInputInjector expects.
EventFlags InjectedMouseButtonTypeToEventFlags(
    mojom::InjectedMouseButtonType type) {
  switch (type) {
    case mojom::InjectedMouseButtonType::kLeft:
      return EF_LEFT_MOUSE_BUTTON;
    case mojom::InjectedMouseButtonType::kMiddle:
      return EF_MIDDLE_MOUSE_BUTTON;
    case mojom::InjectedMouseButtonType::kRight:
      return EF_MIDDLE_MOUSE_BUTTON;
  }
  NOTREACHED();
  return EF_NONE;
}

}  // namespace

RemotingEventInjector::RemotingEventInjector(
    ui::SystemInputInjector* system_injector)
    : system_injector_(system_injector) {}

RemotingEventInjector::~RemotingEventInjector() = default;

void RemotingEventInjector::AddBinding(
    mojom::RemotingEventInjectorRequest request) {
  bindings_.AddBinding(this, std::move(request));
}

void RemotingEventInjector::MoveCursorToLocationInPixels(
    const gfx::PointF& location) {
  system_injector_->MoveCursorTo(location);
}

void RemotingEventInjector::InjectMousePressOrRelease(
    mojom::InjectedMouseButtonType button,
    bool down) {
  system_injector_->InjectMouseButton(
      InjectedMouseButtonTypeToEventFlags(button), down);
}

void RemotingEventInjector::InjectMouseWheelInPixels(int32_t delta_x,
                                                     int32_t delta_y) {
  system_injector_->InjectMouseWheel(delta_x, delta_y);
}

void RemotingEventInjector::InjectKeyEvent(int32_t native_key_code,
                                           bool down,
                                           bool suppress_auto_repeat) {
  system_injector_->InjectKeyEvent(
      ui::KeycodeConverter::NativeKeycodeToDomCode(native_key_code), down,
      suppress_auto_repeat);
}

}  // namespace ws2
}  // namespace ui
