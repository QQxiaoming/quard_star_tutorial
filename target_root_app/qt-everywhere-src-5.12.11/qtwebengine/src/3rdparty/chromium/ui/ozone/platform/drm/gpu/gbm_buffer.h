// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_DRM_GPU_GBM_BUFFER_H_
#define UI_OZONE_PLATFORM_DRM_GPU_GBM_BUFFER_H_

#include <vector>

#include "ui/gfx/native_pixmap.h"
#include "ui/ozone/common/linux/gbm_bo_wrapper.h"
#include "ui/ozone/platform/drm/gpu/scanout_buffer.h"

struct gbm_bo;

namespace ui {

class GbmDevice;
class GbmSurfaceFactory;

class GbmBuffer : public ScanoutBuffer {
 public:
  static constexpr uint32_t kFlagNoModifiers = 1U << 0;

  static scoped_refptr<GbmBuffer> CreateBuffer(
      const scoped_refptr<GbmDevice>& gbm,
      uint32_t format,
      const gfx::Size& size,
      uint32_t flags);
  static scoped_refptr<GbmBuffer> CreateBufferWithModifiers(
      const scoped_refptr<GbmDevice>& gbm,
      uint32_t format,
      const gfx::Size& size,
      uint32_t flags,
      const std::vector<uint64_t>& modifiers);
  static scoped_refptr<GbmBuffer> CreateBufferFromFds(
      const scoped_refptr<GbmDevice>& gbm,
      uint32_t format,
      const gfx::Size& size,
      std::vector<base::ScopedFD> fds,
      const std::vector<gfx::NativePixmapPlane>& planes);

  const GbmBoWrapper* gbm_bo() const { return &gbm_bo_; }

  // ScanoutBuffer:
  uint32_t GetFramebufferId() const override;
  uint32_t GetOpaqueFramebufferId() const override;
  uint32_t GetFramebufferPixelFormat() const override;
  uint32_t GetOpaqueFramebufferPixelFormat() const override;
  uint64_t GetFormatModifier() const override;
  uint32_t GetHandle() const override;
  gfx::Size GetSize() const override;
  const GbmDeviceLinux* GetGbmDeviceLinux() const override;
  bool RequiresGlFinish() const override;

 private:
  GbmBuffer(const scoped_refptr<GbmDevice>& gbm,
            struct gbm_bo* bo,
            uint32_t format,
            uint32_t flags,
            uint64_t modifier,
            std::vector<base::ScopedFD> fds,
            const gfx::Size& size,
            std::vector<gfx::NativePixmapPlane> planes);
  ~GbmBuffer() override;

  static scoped_refptr<GbmBuffer> CreateBufferForBO(
      const scoped_refptr<GbmDevice>& gbm,
      struct gbm_bo* bo,
      uint32_t format,
      const gfx::Size& size,
      uint32_t flags);

  scoped_refptr<GbmDevice> drm_;

  // Owned gbm_bo wrapper.
  GbmBoWrapper gbm_bo_;

  uint32_t framebuffer_ = 0;
  uint32_t framebuffer_pixel_format_ = 0;
  // If |opaque_framebuffer_pixel_format_| differs from
  // |framebuffer_pixel_format_| the following member is set to a valid fb,
  // otherwise it is set to 0.
  uint32_t opaque_framebuffer_ = 0;
  uint32_t opaque_framebuffer_pixel_format_ = 0;

  DISALLOW_COPY_AND_ASSIGN(GbmBuffer);
};

class GbmPixmap : public gfx::NativePixmap {
 public:
  GbmPixmap(GbmSurfaceFactory* surface_manager,
            const scoped_refptr<GbmBuffer>& buffer);

  // NativePixmap:
  bool AreDmaBufFdsValid() const override;
  size_t GetDmaBufFdCount() const override;
  int GetDmaBufFd(size_t plane) const override;
  int GetDmaBufPitch(size_t plane) const override;
  int GetDmaBufOffset(size_t plane) const override;
  uint64_t GetDmaBufModifier(size_t plane) const override;
  gfx::BufferFormat GetBufferFormat() const override;
  gfx::Size GetBufferSize() const override;
  uint32_t GetUniqueId() const override;
  bool ScheduleOverlayPlane(gfx::AcceleratedWidget widget,
                            int plane_z_order,
                            gfx::OverlayTransform plane_transform,
                            const gfx::Rect& display_bounds,
                            const gfx::RectF& crop_rect,
                            bool enable_blend,
                            std::unique_ptr<gfx::GpuFence> gpu_fence) override;
  gfx::NativePixmapHandle ExportHandle() override;

  scoped_refptr<GbmBuffer> buffer() { return buffer_; }

 private:
  ~GbmPixmap() override;
  scoped_refptr<ScanoutBuffer> ProcessBuffer(const gfx::Size& size,
                                             uint32_t format);

  GbmSurfaceFactory* surface_manager_;
  scoped_refptr<GbmBuffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(GbmPixmap);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_DRM_GPU_GBM_BUFFER_H_
