// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_SCENIC_SCENIC_WINDOW_CANVAS_H_
#define UI_OZONE_PLATFORM_SCENIC_SCENIC_WINDOW_CANVAS_H_

#include "base/macros.h"
#include "base/memory/shared_memory.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "ui/gfx/geometry/size.h"
#include "ui/ozone/platform/scenic/scenic_session.h"
#include "ui/ozone/public/surface_ozone_canvas.h"

namespace ui {

class ScenicWindow;

// SurfaceOzoneCanvas implementation for ScenicWindow. It allows to draw on a
// ScenicWindow.
class ScenicWindowCanvas : public SurfaceOzoneCanvas {
 public:
  // |window| must outlive the surface. ScenicWindow owns the ScenicSession used
  // in this class for all drawing operations.
  explicit ScenicWindowCanvas(ScenicWindow* window);
  ~ScenicWindowCanvas() override;

  // SurfaceOzoneCanvas implementation.
  void ResizeCanvas(const gfx::Size& viewport_size) override;
  sk_sp<SkSurface> GetSurface() override;
  void PresentCanvas(const gfx::Rect& damage) override;
  std::unique_ptr<gfx::VSyncProvider> CreateVSyncProvider() override;

 private:
  // Use 2 buffers: one is shown on the screen while the other is used to render
  // the next frame.
  static const int kNumBuffers = 2;

  struct Frame {
    Frame();
    ~Frame();

    // Allocates and maps memory for a frame of |size| (in physical in pixels)
    // and then registers it with |scenic|.
    void Initialize(gfx::Size size, ScenicSession* scenic);

    // Copies pixels covered by |dirty_region| from another |frame|.
    void CopyDirtyRegionFrom(const Frame& frame);

    bool is_empty() { return memory.mapped_size() == 0; }

    // Shared memory for the buffer.
    base::SharedMemory memory;

    // SkSurface that wraps |memory|.
    sk_sp<SkSurface> surface;

    // Valid only when |memory| is set.
    ScenicSession::ResourceId memory_id;

    // Fence that will be release by Scenic when it stops using this frame.
    zx::event release_fence;

    // The region of the frame that's not up-to-date.
    SkRegion dirty_region;
  };

  ScenicWindow* const window_;

  // Shape and material resource ids for the view in the context of the scenic
  // session for the |window_| (window_->scenic()). They are used to set shape
  // and texture for the view node.
  ScenicSession::ResourceId shape_id_;
  ScenicSession::ResourceId material_id_;

  Frame frames_[kNumBuffers];

  // Buffer index in |frames_| for the frame that's currently being rendered.
  int current_frame_ = 0;

  // View size in device pixels.
  gfx::Size viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(ScenicWindowCanvas);
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_SCENIC_SCENIC_WINDOW_CANVAS_H_
