// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_MAGMA_MAGMA_SURFACE_FACTORY_H_
#define UI_OZONE_PLATFORM_MAGMA_MAGMA_SURFACE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "ui/ozone/public/gl_ozone.h"
#include "ui/ozone/public/surface_factory_ozone.h"

namespace ui {

// Configure Magma platform to indicate that it does not support GL
class MagmaSurfaceFactory : public SurfaceFactoryOzone {
 public:
  MagmaSurfaceFactory();
  ~MagmaSurfaceFactory() override;

  // SurfaceFactoryOzone:
  std::vector<gl::GLImplementation> GetAllowedGLImplementations() override;
  GLOzone* GetGLOzone(gl::GLImplementation implementation) override;

 private:
  std::unique_ptr<GLOzone> egl_implementation_;

  DISALLOW_COPY_AND_ASSIGN(MagmaSurfaceFactory);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_MAGMA_MAGMA_SURFACE_FACTORY_H_
