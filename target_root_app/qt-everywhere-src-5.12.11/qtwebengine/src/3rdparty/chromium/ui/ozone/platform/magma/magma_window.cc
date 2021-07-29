// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/magma/magma_window.h"

#include <string>

#include "build/build_config.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/ozone/platform/magma/magma_window_manager.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

MagmaWindow::MagmaWindow(PlatformWindowDelegate* delegate,
                         MagmaWindowManager* manager,
                         const gfx::Rect& bounds)
    : StubWindow(delegate, false, bounds), manager_(manager) {
  widget_ = manager_->AddWindow(this);
  delegate->OnAcceleratedWidgetAvailable(widget_, 1.f);
}

MagmaWindow::~MagmaWindow() {
  manager_->RemoveWindow(widget_, this);
}

}  // namespace ui
