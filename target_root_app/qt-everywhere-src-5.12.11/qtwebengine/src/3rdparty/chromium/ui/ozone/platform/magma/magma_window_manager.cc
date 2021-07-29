// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/magma/magma_window_manager.h"

namespace ui {

MagmaWindowManager::MagmaWindowManager() = default;

MagmaWindowManager::~MagmaWindowManager() = default;

int32_t MagmaWindowManager::AddWindow(MagmaWindow* window) {
  return windows_.Add(window);
}

void MagmaWindowManager::RemoveWindow(int32_t window_id, MagmaWindow* window) {
  DCHECK_EQ(window, windows_.Lookup(window_id));
  windows_.Remove(window_id);
}

MagmaWindow* MagmaWindowManager::GetWindow(int32_t window_id) {
  return windows_.Lookup(window_id);
}

}  // namespace ui
