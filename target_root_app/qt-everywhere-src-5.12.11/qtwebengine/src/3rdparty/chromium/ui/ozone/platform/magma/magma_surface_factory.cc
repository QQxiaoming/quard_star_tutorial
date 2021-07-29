// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/magma/magma_surface_factory.h"

#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/task_scheduler/post_task.h"
#include "build/build_config.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/vsync_provider.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/vsync_provider_win.h"
#include "ui/ozone/common/egl_util.h"
#include "ui/ozone/common/gl_ozone_egl.h"
#include "ui/ozone/platform/magma/magma_window.h"
#include "ui/ozone/platform/magma/magma_window_manager.h"

namespace ui {

MagmaSurfaceFactory::MagmaSurfaceFactory() = default;

MagmaSurfaceFactory::~MagmaSurfaceFactory() = default;

std::vector<gl::GLImplementation>
MagmaSurfaceFactory::GetAllowedGLImplementations() {
  return std::vector<gl::GLImplementation>{};
}

GLOzone* MagmaSurfaceFactory::GetGLOzone(gl::GLImplementation implementation) {
  NOTREACHED();
  return nullptr;
}

}  // namespace ui
