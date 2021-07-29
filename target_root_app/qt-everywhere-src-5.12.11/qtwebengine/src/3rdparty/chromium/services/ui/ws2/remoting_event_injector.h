// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_WS2_REMOTING_EVENT_INJECTOR_H_
#define SERVICES_UI_WS2_REMOTING_EVENT_INJECTOR_H_

#include <vector>

#include "base/macros.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/ui/public/interfaces/remoting_event_injector.mojom.h"

namespace ui {

class SystemInputInjector;

namespace ws2 {

// See description in mojom for details on this. This trivially forwards to
// SystemInputInjector.
class RemotingEventInjector : public mojom::RemotingEventInjector {
 public:
  explicit RemotingEventInjector(SystemInputInjector* system_injector);
  ~RemotingEventInjector() override;

  void AddBinding(mojom::RemotingEventInjectorRequest request);

 private:
  // mojom::RemotingEventInjector:
  void MoveCursorToLocationInPixels(const gfx::PointF& location) override;
  void InjectMousePressOrRelease(mojom::InjectedMouseButtonType button,
                                 bool down) override;
  void InjectMouseWheelInPixels(int32_t delta_x, int32_t delta_y) override;
  void InjectKeyEvent(int32_t native_key_code,
                      bool down,
                      bool suppress_auto_repeat) override;

  ui::SystemInputInjector* system_injector_;

  mojo::BindingSet<mojom::RemotingEventInjector> bindings_;

  DISALLOW_COPY_AND_ASSIGN(RemotingEventInjector);
};

}  // namespace ws2
}  // namespace ui

#endif  // SERVICES_UI_WS2_REMOTING_EVENT_INJECTOR_H_
