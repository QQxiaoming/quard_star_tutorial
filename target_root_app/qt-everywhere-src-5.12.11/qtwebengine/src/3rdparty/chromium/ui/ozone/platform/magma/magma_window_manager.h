// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_MAGMA_MAGMA_WINDOW_MANAGER_H_
#define UI_OZONE_PLATFORM_MAGMA_MAGMA_WINDOW_MANAGER_H_

#include <stdint.h>

#include <memory>

#include "base/containers/id_map.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/threading/thread_checker.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/ozone/public/surface_factory_ozone.h"

namespace ui {

class MagmaWindow;

class MagmaWindowManager {
 public:
  MagmaWindowManager();
  ~MagmaWindowManager();

  // Register a new window. Returns the window id.
  int32_t AddWindow(MagmaWindow* window);

  // Remove a window.
  void RemoveWindow(int32_t window_id, MagmaWindow* window);

  // Find a window object by id;
  MagmaWindow* GetWindow(int32_t window_id);

 private:
  base::IDMap<MagmaWindow*> windows_;

  DISALLOW_COPY_AND_ASSIGN(MagmaWindowManager);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_MAGMA_MAGMA_WINDOW_MANAGER_H_
