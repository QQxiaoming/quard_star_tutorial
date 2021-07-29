// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/window_delegate_impl.h"

#include "services/ui/ws2/test_window_service_delegate.h"
#include "services/ui/ws2/window_service_test_setup.h"
#include "services/ui/ws2/window_tree_test_helper.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/aura/window.h"
#include "ui/base/cursor/cursor_data.h"
#include "ui/base/cursor/cursor_type.h"
#include "ui/gfx/geometry/point.h"

namespace ui {
namespace ws2 {

TEST(WindowDeleteImplTest, GetCursorTopLevel) {
  WindowServiceTestSetup setup;
  // WindowDelegateImpl deletes itself when the window is deleted.
  WindowDelegateImpl* delegate = new WindowDelegateImpl();
  setup.delegate()->set_delegate_for_next_top_level(delegate);
  aura::Window* top_level =
      setup.window_tree_test_helper()->NewTopLevelWindow();
  delegate->set_window(top_level);
  // Verify no crash when no cursor has been set.
  top_level->GetCursor(gfx::Point());

  // Set a cursor and ensure we get it back.
  const ui::CursorData help_cursor(ui::CursorType::kHelp);
  setup.window_tree_test_helper()->SetCursor(top_level, help_cursor);
  EXPECT_EQ(help_cursor.cursor_type(),
            top_level->GetCursor(gfx::Point()).native_type());
}

}  // namespace ws2
}  // namespace ui
