// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXO_INPUT_METHOD_SURFACE_H_
#define COMPONENTS_EXO_INPUT_METHOD_SURFACE_H_

#include "base/macros.h"
#include "components/exo/client_controlled_shell_surface.h"
#include "components/exo/surface_delegate.h"
#include "components/exo/surface_observer.h"

namespace exo {

class InputMethodSurfaceManager;

// Handles input method surface role of a given surface.
class InputMethodSurface : public ClientControlledShellSurface {
 public:
  InputMethodSurface(InputMethodSurfaceManager* manager,
                     Surface* surface,
                     double default_device_scale_factor);
  ~InputMethodSurface() override;

  // Overridden from SurfaceDelegate:
  void OnSurfaceCommit() override;

 private:
  InputMethodSurfaceManager* const manager_;
  bool added_to_manager_ = false;

  DISALLOW_COPY_AND_ASSIGN(InputMethodSurface);
};

}  // namespace exo

#endif  // COMPONENTS_EXO_INPUT_METHOD_SURFACE_H_
