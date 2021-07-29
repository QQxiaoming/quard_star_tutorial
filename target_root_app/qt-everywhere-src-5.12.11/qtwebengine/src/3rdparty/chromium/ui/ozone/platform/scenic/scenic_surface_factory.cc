// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/scenic/scenic_surface_factory.h"

#include <lib/zx/event.h>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "ui/gfx/native_pixmap.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/vsync_provider.h"
#include "ui/ozone/platform/scenic/scenic_window.h"
#include "ui/ozone/platform/scenic/scenic_window_canvas.h"
#include "ui/ozone/platform/scenic/scenic_window_manager.h"

namespace ui {

namespace {

// TODO(crbug.com/852011): Implement this class - currently it's just a stub.
class ScenicPixmap : public gfx::NativePixmap {
 public:
  explicit ScenicPixmap(gfx::AcceleratedWidget widget,
                        gfx::Size size,
                        gfx::BufferFormat format)
      : size_(size), format_(format) {
    NOTIMPLEMENTED();
  }

  bool AreDmaBufFdsValid() const override { return false; }
  size_t GetDmaBufFdCount() const override { return 0; }
  int GetDmaBufFd(size_t plane) const override { return -1; }
  int GetDmaBufPitch(size_t plane) const override { return 0; }
  int GetDmaBufOffset(size_t plane) const override { return 0; }
  uint64_t GetDmaBufModifier(size_t plane) const override { return 0; }
  gfx::BufferFormat GetBufferFormat() const override { return format_; }
  gfx::Size GetBufferSize() const override { return size_; }
  uint32_t GetUniqueId() const override { return 0; }
  bool ScheduleOverlayPlane(gfx::AcceleratedWidget widget,
                            int plane_z_order,
                            gfx::OverlayTransform plane_transform,
                            const gfx::Rect& display_bounds,
                            const gfx::RectF& crop_rect,
                            bool enable_blend,
                            std::unique_ptr<gfx::GpuFence> gpu_fence) override {
    NOTIMPLEMENTED();
    return false;
  }
  gfx::NativePixmapHandle ExportHandle() override {
    NOTIMPLEMENTED();
    return gfx::NativePixmapHandle();
  }

 private:
  ~ScenicPixmap() override {}

  gfx::Size size_;
  gfx::BufferFormat format_;

  DISALLOW_COPY_AND_ASSIGN(ScenicPixmap);
};

}  // namespace

ScenicSurfaceFactory::ScenicSurfaceFactory(ScenicWindowManager* window_manager)
    : window_manager_(window_manager) {}

ScenicSurfaceFactory::~ScenicSurfaceFactory() = default;

std::vector<gl::GLImplementation>
ScenicSurfaceFactory::GetAllowedGLImplementations() {
  return std::vector<gl::GLImplementation>{};
}

GLOzone* ScenicSurfaceFactory::GetGLOzone(gl::GLImplementation implementation) {
  NOTREACHED();
  return nullptr;
}

std::unique_ptr<SurfaceOzoneCanvas> ScenicSurfaceFactory::CreateCanvasForWidget(
    gfx::AcceleratedWidget widget) {
  ScenicWindow* window = window_manager_->GetWindow(widget);
  if (!window)
    return nullptr;
  return std::make_unique<ScenicWindowCanvas>(window);
}

std::vector<gfx::BufferFormat> ScenicSurfaceFactory::GetScanoutFormats(
    gfx::AcceleratedWidget widget) {
  return std::vector<gfx::BufferFormat>{gfx::BufferFormat::RGBA_8888};
}

scoped_refptr<gfx::NativePixmap> ScenicSurfaceFactory::CreateNativePixmap(
    gfx::AcceleratedWidget widget,
    gfx::Size size,
    gfx::BufferFormat format,
    gfx::BufferUsage usage) {
  return new ScenicPixmap(widget, size, format);
}

}  // namespace ui
