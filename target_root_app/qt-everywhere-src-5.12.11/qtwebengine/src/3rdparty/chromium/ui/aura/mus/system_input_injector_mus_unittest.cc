// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/mus/system_input_injector_mus.h"

#include <vector>

#include "ui/aura/mus/window_manager_delegate.h"
#include "ui/aura/mus/window_tree_host_mus_init_params.h"
#include "ui/aura/test/aura_test_base.h"
#include "ui/events/event.h"
#include "ui/events/keycodes/dom/dom_code.h"
#include "ui/events/keycodes/keyboard_codes.h"

namespace aura {

class TestDispaterchWindowManagerClient : public aura::WindowManagerClient {
 public:
  TestDispaterchWindowManagerClient() {}
  ~TestDispaterchWindowManagerClient() override {}

  const std::vector<std::unique_ptr<ui::Event>>& events() const {
    return events_;
  }

  // This is the one method in this interface that we're using to grab data.
  void InjectEvent(const ui::Event& event, int64_t display_id) override {
    events_.push_back(ui::Event::Clone(event));
  }

  // The rest of these interfaces aren't relevant to testing
  // SystemInputInjectorMus and are left empty.
  void SetFrameDecorationValues(
      ui::mojom::FrameDecorationValuesPtr values) override {}
  void SetNonClientCursor(Window* window,
                          const ui::CursorData& non_client_cursor) override {}
  void AddAccelerators(std::vector<ui::mojom::WmAcceleratorPtr> accelerators,
                       const base::Callback<void(bool)>& callback) override {}
  void RemoveAccelerator(uint32_t id) override {}
  void AddActivationParent(Window* window) override {}
  void RemoveActivationParent(Window* window) override {}
  void SetExtendedHitRegionForChildren(Window* window,
                                       const gfx::Insets& mouse_area,
                                       const gfx::Insets& touch_area) override {
  }
  void LockCursor() override {}
  void UnlockCursor() override {}
  void SetCursorVisible(bool visible) override {}
  void SetCursorSize(ui::CursorSize cursor_size) override {}
  void SetGlobalOverrideCursor(base::Optional<ui::CursorData> cursor) override {
  }
  void SetCursorTouchVisible(bool enabled) override {}
  void SetKeyEventsThatDontHideCursor(
      std::vector<ui::mojom::EventMatcherPtr> cursor_key_list) override {}
  void RequestClose(Window* window) override {}
  void SetBlockingContainers(
      const std::vector<BlockingContainers>& all_blocking_containers) override {
  }
  bool WaitForInitialDisplays() override { return false; }
  WindowTreeHostMusInitParams CreateInitParamsForNewDisplay() override {
    return WindowTreeHostMusInitParams();
  }
  void SetDisplayConfiguration(
      const std::vector<display::Display>& displays,
      std::vector<ui::mojom::WmViewportMetricsPtr> viewport_metrics,
      int64_t primary_display_id,
      const std::vector<display::Display>& mirrors) override {}
  void AddDisplayReusingWindowTreeHost(
      WindowTreeHostMus* window_tree_host,
      const display::Display& display,
      ui::mojom::WmViewportMetricsPtr viewport_metrics) override {}
  void SwapDisplayRoots(WindowTreeHostMus* window_tree_host1,
                        WindowTreeHostMus* window_tree_host2) override {}

 private:
  std::vector<std::unique_ptr<ui::Event>> events_;

  DISALLOW_COPY_AND_ASSIGN(TestDispaterchWindowManagerClient);
};

}  // namespace aura
