// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_MUS_SYSTEM_INPUT_INJECTOR_MUS_H_
#define UI_AURA_MUS_SYSTEM_INPUT_INJECTOR_MUS_H_

#include "services/ui/public/interfaces/remoting_event_injector.mojom.h"
#include "ui/aura/aura_export.h"
#include "ui/events/event_modifiers.h"
#include "ui/events/system_input_injector.h"

namespace service_manager {
class Connector;
}

namespace aura {

// Implementation of ui::SystemInputInjector that forwards all calls over a
// mojo pipe.
class AURA_EXPORT SystemInputInjectorMus : public ui::SystemInputInjector {
 public:
  explicit SystemInputInjectorMus(service_manager::Connector* connector);
  ~SystemInputInjectorMus() override;

  // Overridden from SystemInputInjector:
  void MoveCursorTo(const gfx::PointF& location) override;
  void InjectMouseButton(ui::EventFlags button, bool down) override;
  void InjectMouseWheel(int delta_x, int delta_y) override;
  void InjectKeyEvent(ui::DomCode physical_key,
                      bool down,
                      bool suppress_auto_repeat) override;

 private:
  // Forwards |event| to the display at |location|.
  void InjectEventAt(const ui::Event& event, const gfx::Point& location);

  // Updates |modifiers_| based on an incoming event.
  void UpdateModifier(unsigned int modifier, bool down);

  ui::mojom::RemotingEventInjectorPtr remoting_event_injector_;

  DISALLOW_COPY_AND_ASSIGN(SystemInputInjectorMus);
};

}  // namespace aura

#endif  // UI_AURA_MUS_SYSTEM_INPUT_INJECTOR_MUS_H_
