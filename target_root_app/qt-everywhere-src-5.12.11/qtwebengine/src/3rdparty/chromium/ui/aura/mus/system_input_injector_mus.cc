// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/mus/system_input_injector_mus.h"

#include "services/service_manager/public/cpp/connector.h"
#include "services/ui/public/interfaces/constants.mojom.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/dom/keycode_converter.h"

namespace aura {

namespace {

ui::mojom::InjectedMouseButtonType EventFlagsToInjectedMouseButtonType(
    ui::EventFlags flags) {
  switch (flags) {
    case ui::EF_LEFT_MOUSE_BUTTON:
      return ui::mojom::InjectedMouseButtonType::kLeft;
    case ui::EF_MIDDLE_MOUSE_BUTTON:
      return ui::mojom::InjectedMouseButtonType::kMiddle;
    case ui::EF_RIGHT_MOUSE_BUTTON:
      return ui::mojom::InjectedMouseButtonType::kRight;
    default:
      LOG(WARNING) << "Invalid flag: " << flags << " for the button parameter";
  }
  return ui::mojom::InjectedMouseButtonType::kLeft;
}

}  // namespace

SystemInputInjectorMus::SystemInputInjectorMus(
    service_manager::Connector* connector) {
  // Tests may use a null connector.
  if (connector)
    connector->BindInterface(ui::mojom::kServiceName,
                             &remoting_event_injector_);
}

SystemInputInjectorMus::~SystemInputInjectorMus() {}

void SystemInputInjectorMus::MoveCursorTo(const gfx::PointF& location) {
  if (remoting_event_injector_)
    remoting_event_injector_->MoveCursorToLocationInPixels(location);
}

void SystemInputInjectorMus::InjectMouseButton(ui::EventFlags button,
                                               bool down) {
  if (remoting_event_injector_) {
    remoting_event_injector_->InjectMousePressOrRelease(
        EventFlagsToInjectedMouseButtonType(button), down);
  }
}

void SystemInputInjectorMus::InjectMouseWheel(int delta_x, int delta_y) {
  if (remoting_event_injector_)
    remoting_event_injector_->InjectMouseWheelInPixels(delta_x, delta_y);
}

void SystemInputInjectorMus::InjectKeyEvent(ui::DomCode dom_code,
                                            bool down,
                                            bool suppress_auto_repeat) {
  if (remoting_event_injector_) {
    remoting_event_injector_->InjectKeyEvent(
        ui::KeycodeConverter::DomCodeToNativeKeycode(dom_code), down,
        suppress_auto_repeat);
  }
}

}  // namespace aura
