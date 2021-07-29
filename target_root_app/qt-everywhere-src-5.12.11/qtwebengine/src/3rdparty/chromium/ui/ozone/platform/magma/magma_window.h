// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_MAGMA_MAGMA_WINDOW_H_
#define UI_OZONE_PLATFORM_MAGMA_MAGMA_WINDOW_H_

#include "base/macros.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/platform_window/stub/stub_window.h"

namespace ui {

class PlatformWindowDelegate;
class MagmaWindowManager;

class MagmaWindow : public StubWindow {
 public:
  MagmaWindow(PlatformWindowDelegate* delegate,
              MagmaWindowManager* manager,
              const gfx::Rect& bounds);
  ~MagmaWindow() override;

 private:
  MagmaWindowManager* manager_;
  gfx::AcceleratedWidget widget_;

  DISALLOW_COPY_AND_ASSIGN(MagmaWindow);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_MAGMA_MAGMA_WINDOW_H_
