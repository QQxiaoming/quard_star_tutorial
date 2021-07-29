// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/exo/input_method_surface.h"

#include "ash/public/cpp/shell_window_ids.h"
#include "components/exo/input_method_surface_manager.h"

namespace exo {

InputMethodSurface::InputMethodSurface(InputMethodSurfaceManager* manager,
                                       Surface* surface,
                                       double default_device_scale_factor)
    : ClientControlledShellSurface(
          surface,
          true /* can_minimize */,
          ash::kShellWindowId_ArcVirtualKeyboardContainer),
      manager_(manager),
      added_to_manager_(false) {
  SetScale(default_device_scale_factor);
  host_window()->SetName("ExoInputMethodSurface");
}

InputMethodSurface::~InputMethodSurface() {
  if (added_to_manager_)
    manager_->RemoveSurface(this);
}

void InputMethodSurface::OnSurfaceCommit() {
  ClientControlledShellSurface::OnSurfaceCommit();

  if (!added_to_manager_) {
    added_to_manager_ = true;
    manager_->AddSurface(this);
  }
}

}  // namespace exo
