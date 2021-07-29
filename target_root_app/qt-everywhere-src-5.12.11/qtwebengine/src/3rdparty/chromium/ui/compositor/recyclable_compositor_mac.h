// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_RECYCLABLE_COMPOSITOR_MAC_H_
#define UI_COMPOSITOR_RECYCLABLE_COMPOSITOR_MAC_H_

#include "base/no_destructor.h"
#include "components/viz/common/surfaces/local_surface_id.h"
#include "components/viz/common/surfaces/parent_local_surface_id_allocator.h"
#include "ui/accelerated_widget_mac/accelerated_widget_mac.h"
#include "ui/compositor/compositor.h"
#include "ui/compositor/compositor_export.h"
#include "ui/compositor/compositor_observer.h"

namespace ui {

////////////////////////////////////////////////////////////////////////////////
// RecyclableCompositorMac
//
// A ui::Compositor and a gfx::AcceleratedWidget (and helper) that it draws
// into. This structure is used to efficiently recycle these structures across
// tabs (because creating a new ui::Compositor for each tab would be expensive
// in terms of time and resources).
class COMPOSITOR_EXPORT RecyclableCompositorMac
    : public ui::CompositorObserver {
 public:
  RecyclableCompositorMac(ui::ContextFactory* context_factory,
                          ui::ContextFactoryPrivate* context_factory_private);
  ~RecyclableCompositorMac() override;

  ui::Compositor* compositor() { return &compositor_; }
  ui::AcceleratedWidgetMac* widget() { return accelerated_widget_mac_.get(); }
  const gfx::Size pixel_size() const { return size_pixels_; }
  float scale_factor() const { return scale_factor_; }

  // Suspend will prevent the compositor from producing new frames. This should
  // be called to avoid creating spurious frames while changing state.
  // Compositors are created as suspended.
  void Suspend();
  void Unsuspend();

  // Update the compositor's surface information, if needed.
  void UpdateSurface(const gfx::Size& size_pixels, float scale_factor);
  // Invalidate the compositor's surface information.
  void InvalidateSurface();

  // The viz::ParentLocalSurfaceIdAllocator for the ui::Compositor dispenses
  // viz::LocalSurfaceIds that are renderered into by the ui::Compositor.
  viz::ParentLocalSurfaceIdAllocator local_surface_id_allocator_;
  gfx::Size size_pixels_;
  float scale_factor_ = 1.f;

 private:
  friend class RecyclableCompositorMacFactory;

  // ui::CompositorObserver implementation:
  void OnCompositingDidCommit(ui::Compositor* compositor) override;
  void OnCompositingStarted(ui::Compositor* compositor,
                            base::TimeTicks start_time) override {}
  void OnCompositingEnded(ui::Compositor* compositor) override {}
  void OnCompositingLockStateChanged(ui::Compositor* compositor) override {}
  void OnCompositingChildResizing(ui::Compositor* compositor) override {}
  void OnCompositingShuttingDown(ui::Compositor* compositor) override {}

  std::unique_ptr<ui::AcceleratedWidgetMac> accelerated_widget_mac_;
  ui::Compositor compositor_;
  std::unique_ptr<ui::CompositorLock> compositor_suspended_lock_;

  DISALLOW_COPY_AND_ASSIGN(RecyclableCompositorMac);
};

////////////////////////////////////////////////////////////////////////////////
// RecyclableCompositorMacFactory
//
// The factory through which RecyclableCompositorMacs are created and recycled.

class COMPOSITOR_EXPORT RecyclableCompositorMacFactory {
 public:
  static RecyclableCompositorMacFactory* Get();

  // Create a compositor, or recycle a preexisting one.
  std::unique_ptr<RecyclableCompositorMac> CreateCompositor(
      ui::ContextFactory* context_factory,
      ui::ContextFactoryPrivate* context_factory_private,
      bool force_new_compositor = false);

  // Delete a compositor, or allow it to be recycled.
  void RecycleCompositor(std::unique_ptr<RecyclableCompositorMac> compositor);

  // Destroy any compositors that are being kept around for recycling.
  void DisableRecyclingForShutdown();

 private:
  friend class base::NoDestructor<ui::RecyclableCompositorMacFactory>;
  friend class RecyclableCompositorMac;
  RecyclableCompositorMacFactory();
  ~RecyclableCompositorMacFactory();
  void ReduceSpareCompositors();

  bool recycling_disabled_ = false;
  std::list<std::unique_ptr<RecyclableCompositorMac>> compositors_;
  base::WeakPtrFactory<RecyclableCompositorMacFactory> weak_factory_;
};

}  // namespace ui

#endif  // UI_COMPOSITOR_RECYCLABLE_COMPOSITOR_MAC_H_
