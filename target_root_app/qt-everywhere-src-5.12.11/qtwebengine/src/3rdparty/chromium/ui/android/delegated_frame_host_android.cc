// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/android/delegated_frame_host_android.h"

#include "base/android/build_info.h"
#include "base/bind.h"
#include "base/logging.h"
#include "cc/layers/solid_color_layer.h"
#include "cc/layers/surface_layer.h"
#include "components/viz/common/features.h"
#include "components/viz/common/frame_sinks/copy_output_result.h"
#include "components/viz/common/quads/compositor_frame.h"
#include "components/viz/common/surfaces/surface_id.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/service/surfaces/surface.h"
#include "ui/android/view_android.h"
#include "ui/android/window_android.h"
#include "ui/android/window_android_compositor.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/gfx/geometry/dip_util.h"

namespace ui {

namespace {

scoped_refptr<cc::SurfaceLayer> CreateSurfaceLayer(
    const viz::SurfaceId& primary_surface_id,
    const viz::SurfaceId& fallback_surface_id,
    const gfx::Size& size_in_pixels,
    const cc::DeadlinePolicy& deadline_policy,
    bool surface_opaque) {
  // manager must outlive compositors using it.
  auto layer = cc::SurfaceLayer::Create();
  layer->SetPrimarySurfaceId(primary_surface_id, deadline_policy);
  layer->SetFallbackSurfaceId(fallback_surface_id);
  layer->SetBounds(size_in_pixels);
  layer->SetIsDrawable(true);
  layer->SetContentsOpaque(surface_opaque);
  layer->SetSurfaceHitTestable(true);

  return layer;
}

}  // namespace

DelegatedFrameHostAndroid::DelegatedFrameHostAndroid(
    ui::ViewAndroid* view,
    viz::HostFrameSinkManager* host_frame_sink_manager,
    Client* client,
    const viz::FrameSinkId& frame_sink_id,
    bool enable_surface_synchronization)
    : frame_sink_id_(frame_sink_id),
      view_(view),
      host_frame_sink_manager_(host_frame_sink_manager),
      client_(client),
      begin_frame_source_(this),
      enable_surface_synchronization_(enable_surface_synchronization),
      enable_viz_(
          base::FeatureList::IsEnabled(features::kVizDisplayCompositor)),
      frame_evictor_(std::make_unique<viz::FrameEvictor>(this)) {
  DCHECK(view_);
  DCHECK(client_);

  if (enable_surface_synchronization_) {
    constexpr bool is_transparent = false;
    content_layer_ = CreateSurfaceLayer(
        viz::SurfaceId(), viz::SurfaceId(), gfx::Size(),
        cc::DeadlinePolicy::UseDefaultDeadline(), is_transparent);
    view_->GetLayer()->AddChild(content_layer_);
  }

  host_frame_sink_manager_->RegisterFrameSinkId(frame_sink_id_, this);
  host_frame_sink_manager_->SetFrameSinkDebugLabel(frame_sink_id_,
                                                   "DelegatedFrameHostAndroid");
  CreateNewCompositorFrameSinkSupport();
}

DelegatedFrameHostAndroid::~DelegatedFrameHostAndroid() {
  EvictDelegatedFrame();
  DetachFromCompositor();
  support_.reset();
  host_frame_sink_manager_->InvalidateFrameSinkId(frame_sink_id_);
}

void DelegatedFrameHostAndroid::SubmitCompositorFrame(
    const viz::LocalSurfaceId& local_surface_id,
    viz::CompositorFrame frame,
    base::Optional<viz::HitTestRegionList> hit_test_region_list) {
  DCHECK(!enable_viz_);

  viz::RenderPass* root_pass = frame.render_pass_list.back().get();
  const bool has_transparent_background = root_pass->has_transparent_background;
  const gfx::Size pending_surface_size_in_pixels = frame.size_in_pixels();
  // Reset |content_layer_| only if surface-sync is not used. When surface-sync
  // is turned on, |content_layer_| is updated with the appropriate states (see
  // in EmbedSurface()) instead of being recreated.
  if (!enable_surface_synchronization_ &&
      SurfaceId().local_surface_id() != local_surface_id) {
    EvictDelegatedFrame();
  }
  support_->SubmitCompositorFrame(local_surface_id, std::move(frame),
                                  std::move(hit_test_region_list));
  if (enable_surface_synchronization_) {
    DCHECK(content_layer_);
    return;
  }

  if (!content_layer_) {
    pending_local_surface_id_ = local_surface_id;
    pending_surface_size_in_pixels_ = pending_surface_size_in_pixels;
    has_transparent_background_ = has_transparent_background;
    content_layer_ = CreateSurfaceLayer(
        viz::SurfaceId(frame_sink_id_, local_surface_id),
        viz::SurfaceId(frame_sink_id_, local_surface_id),
        pending_surface_size_in_pixels_,
        cc::DeadlinePolicy::UseDefaultDeadline(), !has_transparent_background_);
    view_->GetLayer()->AddChild(content_layer_);
  }
  content_layer_->SetContentsOpaque(!has_transparent_background_);

  compositor_attach_until_frame_lock_.reset();

  // If surface synchronization is disabled, SubmitCompositorFrame immediately
  // activates the CompositorFrame and issues OnFirstSurfaceActivation if the
  // |local_surface_id| has changed since the last submission.
  if (content_layer_->bounds() == expected_pixel_size_)
    compositor_pending_resize_lock_.reset();

  frame_evictor_->SwappedFrame(frame_evictor_->visible());
}

void DelegatedFrameHostAndroid::DidNotProduceFrame(
    const viz::BeginFrameAck& ack) {
  DCHECK(!enable_viz_);
  support_->DidNotProduceFrame(ack);
}

const viz::FrameSinkId& DelegatedFrameHostAndroid::GetFrameSinkId() const {
  return frame_sink_id_;
}

void DelegatedFrameHostAndroid::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& output_size,
    base::OnceCallback<void(const SkBitmap&)> callback) {
  // TODO(vmpstr): We should defer this request until such time that this
  // returns true. https://crbug.com/826097.
  if (!CanCopyFromCompositingSurface()) {
    std::move(callback).Run(SkBitmap());
    return;
  }

  // TODO(samans): We shouldn't need a readback layer. https://crbug.com/841734
  scoped_refptr<cc::Layer> readback_layer = CreateSurfaceLayer(
      content_layer_->fallback_surface_id(),
      content_layer_->fallback_surface_id(), content_layer_->bounds(),
      cc::DeadlinePolicy::UseDefaultDeadline(), !has_transparent_background_);
  readback_layer->SetHideLayerAndSubtree(true);
  view_->GetWindowAndroid()->GetCompositor()->AttachLayerForReadback(
      readback_layer);
  std::unique_ptr<viz::CopyOutputRequest> request =
      std::make_unique<viz::CopyOutputRequest>(
          viz::CopyOutputRequest::ResultFormat::RGBA_BITMAP,
          base::BindOnce(
              [](base::OnceCallback<void(const SkBitmap&)> callback,
                 scoped_refptr<cc::Layer> readback_layer,
                 std::unique_ptr<viz::CopyOutputResult> result) {
                readback_layer->RemoveFromParent();
                std::move(callback).Run(result->AsSkBitmap());
              },
              std::move(callback), std::move(readback_layer)));

  if (src_subrect.IsEmpty()) {
    request->set_area(gfx::Rect(content_layer_->bounds()));
  } else {
    request->set_area(
        gfx::ConvertRectToPixel(view_->GetDipScale(), src_subrect));
  }

  if (!output_size.IsEmpty()) {
    if (request->area().IsEmpty()) {
      // Viz would normally return an empty result for an empty source area.
      // However, this guard here is still necessary to protect against setting
      // an illegal scaling ratio.
      return;
    }
    request->set_result_selection(gfx::Rect(output_size));
    request->SetScaleRatio(
        gfx::Vector2d(request->area().width(), request->area().height()),
        gfx::Vector2d(output_size.width(), output_size.height()));
  }
  host_frame_sink_manager_->RequestCopyOfOutput(
      content_layer_->fallback_surface_id(), std::move(request));
}

bool DelegatedFrameHostAndroid::CanCopyFromCompositingSurface() const {
  return content_layer_ && content_layer_->fallback_surface_id().is_valid() &&
         view_->GetWindowAndroid() &&
         view_->GetWindowAndroid()->GetCompositor();
}

void DelegatedFrameHostAndroid::EvictDelegatedFrame() {
  if (!content_layer_)
    return;
  viz::SurfaceId surface_id = content_layer_->fallback_surface_id();
  content_layer_->SetFallbackSurfaceId(viz::SurfaceId());
  content_layer_->SetPrimarySurfaceId(viz::SurfaceId(),
                                      cc::DeadlinePolicy::UseDefaultDeadline());
  if (!enable_surface_synchronization_) {
    content_layer_->RemoveFromParent();
    content_layer_ = nullptr;
  }
  if (!surface_id.is_valid())
    return;
  std::vector<viz::SurfaceId> surface_ids = {surface_id};
  host_frame_sink_manager_->EvictSurfaces(surface_ids);
  frame_evictor_->DiscardedFrame();
}

bool DelegatedFrameHostAndroid::HasDelegatedContent() const {
  return content_layer_ && content_layer_->primary_surface_id().is_valid();
}

void DelegatedFrameHostAndroid::CompositorFrameSinkChanged() {
  EvictDelegatedFrame();
  CreateNewCompositorFrameSinkSupport();
  if (registered_parent_compositor_)
    AttachToCompositor(registered_parent_compositor_);
}

void DelegatedFrameHostAndroid::AttachToCompositor(
    WindowAndroidCompositor* compositor) {
  if (registered_parent_compositor_)
    DetachFromCompositor();
  // If this is the first frame after the compositor became visible, we want to
  // take the compositor lock, preventing compositor frames from being produced
  // until all delegated frames are ready. This improves the resume transition,
  // preventing flashes. Set a 5 second timeout to prevent locking up the
  // browser in cases where the renderer hangs or another factor prevents a
  // frame from being produced. If we already have delegated content, no need
  // to take the lock.
  // If surface synchronization is enabled, then it will block browser UI until
  // a renderer frame is available instead.
  if (!enable_surface_synchronization_ &&
      compositor->IsDrawingFirstVisibleFrame() && !HasDelegatedContent()) {
    compositor_attach_until_frame_lock_ =
        compositor->GetCompositorLock(this, FirstFrameTimeout());
  }
  compositor->AddChildFrameSink(frame_sink_id_);
  if (!enable_viz_)
    client_->SetBeginFrameSource(&begin_frame_source_);
  registered_parent_compositor_ = compositor;
}

void DelegatedFrameHostAndroid::DetachFromCompositor() {
  if (!registered_parent_compositor_)
    return;
  compositor_attach_until_frame_lock_.reset();
  compositor_pending_resize_lock_.reset();
  if (!enable_viz_) {
    client_->SetBeginFrameSource(nullptr);
    support_->SetNeedsBeginFrame(false);
  }
  registered_parent_compositor_->RemoveChildFrameSink(frame_sink_id_);
  registered_parent_compositor_ = nullptr;
}

bool DelegatedFrameHostAndroid::IsPrimarySurfaceEvicted() const {
  return !content_layer_ || !content_layer_->primary_surface_id().is_valid();
}

bool DelegatedFrameHostAndroid::HasSavedFrame() const {
  return frame_evictor_->HasFrame();
}

void DelegatedFrameHostAndroid::WasHidden() {
  frame_evictor_->SetVisible(false);
}

void DelegatedFrameHostAndroid::WasShown(
    const viz::LocalSurfaceId& new_pending_local_surface_id,
    const gfx::Size& new_pending_size_in_pixels) {
  frame_evictor_->SetVisible(true);

  if (!enable_surface_synchronization_)
    return;

  EmbedSurface(
      new_pending_local_surface_id, new_pending_size_in_pixels,
      cc::DeadlinePolicy::UseSpecifiedDeadline(FirstFrameTimeoutFrames()));
}

void DelegatedFrameHostAndroid::EmbedSurface(
    const viz::LocalSurfaceId& new_pending_local_surface_id,
    const gfx::Size& new_pending_size_in_pixels,
    cc::DeadlinePolicy deadline_policy) {
  if (!enable_surface_synchronization_)
    return;

  pending_local_surface_id_ = new_pending_local_surface_id;
  pending_surface_size_in_pixels_ = new_pending_size_in_pixels;
  viz::SurfaceId current_primary_surface_id =
      content_layer_->primary_surface_id();

  if (!frame_evictor_->visible()) {
    // If the tab is resized while hidden, reset the fallback so that the next
    // time user switches back to it the page is blank. This is preferred to
    // showing contents of old size. Don't call EvictDelegatedFrame to avoid
    // races when dragging tabs across displays. See https://crbug.com/813157.
    if (pending_surface_size_in_pixels_ != content_layer_->bounds() &&
        content_layer_->fallback_surface_id().is_valid()) {
      content_layer_->SetFallbackSurfaceId(viz::SurfaceId());
    }
    // Don't update the SurfaceLayer when invisible to avoid blocking on
    // renderers that do not submit CompositorFrames. Next time the renderer
    // is visible, EmbedSurface will be called again. See WasShown.
    return;
  }
  if (!current_primary_surface_id.is_valid() ||
      current_primary_surface_id.local_surface_id() !=
          pending_local_surface_id_) {
    if (base::android::BuildInfo::GetInstance()->sdk_int() <
        base::android::SDK_VERSION_OREO) {
      // On version of Android earlier than Oreo, we would like to produce new
      // content as soon as possible or the OS will create an additional black
      // gutter. We only reset the deadline on the first frame (no bounds yet
      // specified) or on resize, and only if the deadline policy is not
      // infinite.
      if (deadline_policy.policy_type() !=
              cc::DeadlinePolicy::kUseInfiniteDeadline &&
          (content_layer_->bounds().IsEmpty() ||
           content_layer_->bounds() != pending_surface_size_in_pixels_)) {
        deadline_policy = cc::DeadlinePolicy::UseSpecifiedDeadline(0u);
      }
    }
    viz::SurfaceId primary_surface_id(frame_sink_id_,
                                      pending_local_surface_id_);
    content_layer_->SetPrimarySurfaceId(primary_surface_id, deadline_policy);
    content_layer_->SetBounds(new_pending_size_in_pixels);
  }
}

void DelegatedFrameHostAndroid::PixelSizeWillChange(
    const gfx::Size& pixel_size) {
  if (enable_surface_synchronization_)
    return;

  // We never take the resize lock unless we're on O+, as previous versions of
  // Android won't wait for us to produce the correct sized frame and will end
  // up looking worse.
  if (base::android::BuildInfo::GetInstance()->sdk_int() <
      base::android::SDK_VERSION_OREO) {
    return;
  }

  expected_pixel_size_ = pixel_size;
  if (content_layer_ && registered_parent_compositor_) {
    if (content_layer_->bounds() != expected_pixel_size_) {
      compositor_pending_resize_lock_ =
          registered_parent_compositor_->GetCompositorLock(this,
                                                           ResizeTimeout());
    }
  }
}

void DelegatedFrameHostAndroid::DidReceiveCompositorFrameAck(
    const std::vector<viz::ReturnedResource>& resources) {
  client_->DidReceiveCompositorFrameAck(resources);
}

void DelegatedFrameHostAndroid::DidPresentCompositorFrame(
    uint32_t presentation_token,
    const gfx::PresentationFeedback& feedback) {
  client_->DidPresentCompositorFrame(presentation_token, feedback);
}

void DelegatedFrameHostAndroid::OnBeginFrame(const viz::BeginFrameArgs& args) {
  if (enable_viz_) {
    NOTREACHED();
    return;
  }
  begin_frame_source_.OnBeginFrame(args);
}

void DelegatedFrameHostAndroid::ReclaimResources(
    const std::vector<viz::ReturnedResource>& resources) {
  client_->ReclaimResources(resources);
}

void DelegatedFrameHostAndroid::OnBeginFramePausedChanged(bool paused) {
  begin_frame_source_.OnSetBeginFrameSourcePaused(paused);
}

void DelegatedFrameHostAndroid::OnNeedsBeginFrames(bool needs_begin_frames) {
  DCHECK(!enable_viz_);
  support_->SetNeedsBeginFrame(needs_begin_frames);
}

void DelegatedFrameHostAndroid::OnFirstSurfaceActivation(
    const viz::SurfaceInfo& surface_info) {
  if (!enable_surface_synchronization_)
    return;

  uint32_t active_parent_sequence_number =
      surface_info.id().local_surface_id().parent_sequence_number();
  uint32_t pending_parent_sequence_number =
      pending_local_surface_id_.parent_sequence_number();

  // If |pending_parent_sequence_number| is less than
  // |first_parent_sequence_number_after_navigation_|, then the parent id has
  // wrapped around. Make sure that case is covered.
  bool sequence_wrapped_around =
      pending_parent_sequence_number <
          first_parent_sequence_number_after_navigation_ &&
      active_parent_sequence_number <= pending_parent_sequence_number;

  if (active_parent_sequence_number >=
          first_parent_sequence_number_after_navigation_ ||
      sequence_wrapped_around) {
    if (!received_frame_after_navigation_) {
      received_frame_after_navigation_ = true;
      client_->DidReceiveFirstFrameAfterNavigation();
    }
  } else {
    host_frame_sink_manager_->DropTemporaryReference(surface_info.id());
  }

  // If there's no primary surface, then we don't wish to display content at
  // this time (e.g. the view is hidden) and so we don't need a fallback
  // surface either. Since we won't use the fallback surface, we drop the
  // temporary reference here to save resources.
  if (!content_layer_->primary_surface_id().is_valid()) {
    host_frame_sink_manager_->DropTemporaryReference(surface_info.id());
    return;
  }

  content_layer_->SetFallbackSurfaceId(surface_info.id());

  // TODO(fsamuel): "SwappedFrame" is a bad name. Also, this method doesn't
  // really need to take in visiblity. FrameEvictor already has the latest
  // visibility state.
  frame_evictor_->SwappedFrame(frame_evictor_->visible());
  // Note: the frame may have been evicted immediately.
}

void DelegatedFrameHostAndroid::OnFrameTokenChanged(uint32_t frame_token) {
  client_->OnFrameTokenChanged(frame_token);
}

void DelegatedFrameHostAndroid::CompositorLockTimedOut() {}

void DelegatedFrameHostAndroid::CreateNewCompositorFrameSinkSupport() {
  if (enable_viz_)
    return;

  constexpr bool is_root = false;
  constexpr bool needs_sync_points = true;
  support_.reset();
  support_ = host_frame_sink_manager_->CreateCompositorFrameSinkSupport(
      this, frame_sink_id_, is_root, needs_sync_points);
}

const viz::SurfaceId& DelegatedFrameHostAndroid::SurfaceId() const {
  static constexpr viz::SurfaceId kNullSurface;
  return content_layer_ ? content_layer_->fallback_surface_id() : kNullSurface;
}

void DelegatedFrameHostAndroid::TakeFallbackContentFrom(
    DelegatedFrameHostAndroid* other) {
  if (!content_layer_ || !other->content_layer_)
    return;
  if (content_layer_->fallback_surface_id().is_valid() ||
      !other->content_layer_->fallback_surface_id().is_valid()) {
    return;
  }

  if (!enable_surface_synchronization_) {
    if (content_layer_) {
      content_layer_->SetPrimarySurfaceId(
          other->content_layer_->fallback_surface_id(),
          cc::DeadlinePolicy::UseDefaultDeadline());
    } else {
      const auto& surface_id = other->SurfaceId();
      pending_local_surface_id_ = surface_id.local_surface_id();
      pending_surface_size_in_pixels_ = other->pending_surface_size_in_pixels_;
      has_transparent_background_ = other->has_transparent_background_;
      content_layer_ = CreateSurfaceLayer(
          surface_id, surface_id, other->content_layer_->bounds(),
          cc::DeadlinePolicy::UseDefaultDeadline(),
          other->content_layer_->contents_opaque());
      view_->GetLayer()->AddChild(content_layer_);
    }
  }
  content_layer_->SetFallbackSurfaceId(
      other->content_layer_->fallback_surface_id());
}

void DelegatedFrameHostAndroid::DidNavigate() {
  if (!enable_surface_synchronization_)
    return;

  first_parent_sequence_number_after_navigation_ =
      pending_local_surface_id_.parent_sequence_number();
  received_frame_after_navigation_ = false;
}

}  // namespace ui
