// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/frame_sinks/root_compositor_frame_sink_impl.h"

#include <utility>

#include "base/compiler_specific.h"
#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"
#include "components/viz/common/frame_sinks/begin_frame_source.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display_embedder/display_provider.h"
#include "components/viz/service/frame_sinks/external_begin_frame_source_mojo.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "components/viz/service/hit_test/hit_test_aggregator.h"

#if defined(OS_ANDROID)
#include "components/viz/service/frame_sinks/external_begin_frame_source_android.h"
#endif

namespace viz {

// static
std::unique_ptr<RootCompositorFrameSinkImpl>
RootCompositorFrameSinkImpl::Create(
    mojom::RootCompositorFrameSinkParamsPtr params,
    FrameSinkManagerImpl* frame_sink_manager,
    DisplayProvider* display_provider) {
  // First create some sort of a BeginFrameSource, depending on the platform
  // and |params|.
  std::unique_ptr<ExternalBeginFrameSource> external_begin_frame_source;
  std::unique_ptr<SyntheticBeginFrameSource> synthetic_begin_frame_source;
  ExternalBeginFrameSourceMojo* external_begin_frame_source_mojo = nullptr;

  if (params->external_begin_frame_controller.is_pending() &&
      params->external_begin_frame_controller_client) {
    auto owned_external_begin_frame_source_mojo =
        std::make_unique<ExternalBeginFrameSourceMojo>(
            std::move(params->external_begin_frame_controller),
            mojom::ExternalBeginFrameControllerClientPtr(
                std::move(params->external_begin_frame_controller_client)));
    external_begin_frame_source_mojo =
        owned_external_begin_frame_source_mojo.get();
    external_begin_frame_source =
        std::move(owned_external_begin_frame_source_mojo);
  } else {
#if defined(OS_ANDROID)
    external_begin_frame_source =
        std::make_unique<ExternalBeginFrameSourceAndroid>();
#else
    synthetic_begin_frame_source = std::make_unique<DelayBasedBeginFrameSource>(
        std::make_unique<DelayBasedTimeSource>(
            base::ThreadTaskRunnerHandle::Get().get()),
        display_provider->GetRestartId());
#endif
  }

  // |impl| isn't ready to use until after a display has been created for it and
  // Initialize() has been called.
  auto impl = base::WrapUnique(new RootCompositorFrameSinkImpl(
      frame_sink_manager, params->frame_sink_id,
      std::move(params->compositor_frame_sink),
      mojom::CompositorFrameSinkClientPtr(
          std::move(params->compositor_frame_sink_client)),
      std::move(params->display_private),
      mojom::DisplayClientPtr(std::move(params->display_client)),
      std::move(synthetic_begin_frame_source),
      std::move(external_begin_frame_source)));

  auto display = display_provider->CreateDisplay(
      params->frame_sink_id, params->widget, params->gpu_compositing,
      impl->display_client_.get(), impl->external_begin_frame_source_.get(),
      impl->synthetic_begin_frame_source_.get(), params->renderer_settings,
      params->send_swap_size_notifications);

  // Creating a display failed. Destroy |impl| which will close the message
  // pipes. The host can send a new request, potential with a different
  // compositing mode.
  if (!display)
    return nullptr;

  if (external_begin_frame_source_mojo)
    external_begin_frame_source_mojo->SetDisplay(display.get());

  impl->Initialize(std::move(display));

  return impl;
}

RootCompositorFrameSinkImpl::~RootCompositorFrameSinkImpl() {
  support_->frame_sink_manager()->UnregisterBeginFrameSource(
      begin_frame_source());
}

void RootCompositorFrameSinkImpl::SetDisplayVisible(bool visible) {
  display_->SetVisible(visible);
}

void RootCompositorFrameSinkImpl::DisableSwapUntilResize(
    DisableSwapUntilResizeCallback callback) {
  display_->Resize(gfx::Size());
  std::move(callback).Run();
}

void RootCompositorFrameSinkImpl::Resize(const gfx::Size& size) {
  display_->Resize(size);
}

void RootCompositorFrameSinkImpl::SetDisplayColorMatrix(
    const gfx::Transform& color_matrix) {
  display_->SetColorMatrix(color_matrix.matrix());
}

void RootCompositorFrameSinkImpl::SetDisplayColorSpace(
    const gfx::ColorSpace& blending_color_space,
    const gfx::ColorSpace& device_color_space) {
  display_->SetColorSpace(blending_color_space, device_color_space);
}

void RootCompositorFrameSinkImpl::SetOutputIsSecure(bool secure) {
  display_->SetOutputIsSecure(secure);
}

void RootCompositorFrameSinkImpl::SetAuthoritativeVSyncInterval(
    base::TimeDelta interval) {
  if (synthetic_begin_frame_source_)
    synthetic_begin_frame_source_->SetAuthoritativeVSyncInterval(interval);
}

void RootCompositorFrameSinkImpl::SetDisplayVSyncParameters(
    base::TimeTicks timebase,
    base::TimeDelta interval) {
  if (synthetic_begin_frame_source_)
    synthetic_begin_frame_source_->OnUpdateVSyncParameters(timebase, interval);
}

void RootCompositorFrameSinkImpl::SetNeedsBeginFrame(bool needs_begin_frame) {
  support_->SetNeedsBeginFrame(needs_begin_frame);
}

void RootCompositorFrameSinkImpl::SetWantsAnimateOnlyBeginFrames() {
  support_->SetWantsAnimateOnlyBeginFrames();
}

void RootCompositorFrameSinkImpl::SubmitCompositorFrame(
    const LocalSurfaceId& local_surface_id,
    CompositorFrame frame,
    base::Optional<HitTestRegionList> hit_test_region_list,
    uint64_t submit_time) {
  if (support_->last_activated_local_surface_id() != local_surface_id)
    display_->SetLocalSurfaceId(local_surface_id, frame.device_scale_factor());

  const auto result = support_->MaybeSubmitCompositorFrame(
      local_surface_id, std::move(frame), std::move(hit_test_region_list),
      submit_time, SubmitCompositorFrameSyncCallback());
  if (result == SubmitResult::ACCEPTED)
    return;

  const char* reason =
      CompositorFrameSinkSupport::GetSubmitResultAsString(result);
  DLOG(ERROR) << "SubmitCompositorFrame failed for " << local_surface_id
              << " because " << reason;
  compositor_frame_sink_binding_.CloseWithReason(static_cast<uint32_t>(result),
                                                 reason);
}

void RootCompositorFrameSinkImpl::SubmitCompositorFrameSync(
    const LocalSurfaceId& local_surface_id,
    CompositorFrame frame,
    base::Optional<HitTestRegionList> hit_test_region_list,
    uint64_t submit_time,
    SubmitCompositorFrameSyncCallback callback) {
  NOTIMPLEMENTED();
}

void RootCompositorFrameSinkImpl::DidNotProduceFrame(
    const BeginFrameAck& begin_frame_ack) {
  support_->DidNotProduceFrame(begin_frame_ack);
}

void RootCompositorFrameSinkImpl::DidAllocateSharedBitmap(
    mojo::ScopedSharedBufferHandle buffer,
    const SharedBitmapId& id) {
  if (!support_->DidAllocateSharedBitmap(std::move(buffer), id)) {
    DLOG(ERROR) << "DidAllocateSharedBitmap failed for duplicate "
                << "SharedBitmapId";
    compositor_frame_sink_binding_.Close();
  }
}

void RootCompositorFrameSinkImpl::DidDeleteSharedBitmap(
    const SharedBitmapId& id) {
  support_->DidDeleteSharedBitmap(id);
}

RootCompositorFrameSinkImpl::RootCompositorFrameSinkImpl(
    FrameSinkManagerImpl* frame_sink_manager,
    const FrameSinkId& frame_sink_id,
    mojom::CompositorFrameSinkAssociatedRequest frame_sink_request,
    mojom::CompositorFrameSinkClientPtr frame_sink_client,
    mojom::DisplayPrivateAssociatedRequest display_request,
    mojom::DisplayClientPtr display_client,
    std::unique_ptr<SyntheticBeginFrameSource> synthetic_begin_frame_source,
    std::unique_ptr<ExternalBeginFrameSource> external_begin_frame_source)
    : compositor_frame_sink_client_(std::move(frame_sink_client)),
      compositor_frame_sink_binding_(this, std::move(frame_sink_request)),
      display_client_(std::move(display_client)),
      display_private_binding_(this, std::move(display_request)),
      support_(std::make_unique<CompositorFrameSinkSupport>(
          compositor_frame_sink_client_.get(),
          frame_sink_manager,
          frame_sink_id,
          /*is_root=*/true,
          /*needs_sync_points=*/true)),
      synthetic_begin_frame_source_(std::move(synthetic_begin_frame_source)),
      external_begin_frame_source_(std::move(external_begin_frame_source)) {
  DCHECK(begin_frame_source());

  frame_sink_manager->RegisterBeginFrameSource(begin_frame_source(),
                                               support_->frame_sink_id());
}

void RootCompositorFrameSinkImpl::Initialize(std::unique_ptr<Display> display) {
  display_ = std::move(display);
  DCHECK(display_);

  display_->Initialize(this, support_->frame_sink_manager()->surface_manager());
  support_->SetUpHitTest(display_.get());
}

void RootCompositorFrameSinkImpl::DisplayOutputSurfaceLost() {
  // |display_| has encountered an error and needs to be recreated. Close
  // message pipes from the client, the client will see the connection error and
  // recreate the CompositorFrameSink+Display.
  compositor_frame_sink_binding_.Close();
  display_private_binding_.Close();
}

void RootCompositorFrameSinkImpl::DisplayWillDrawAndSwap(
    bool will_draw_and_swap,
    const RenderPassList& render_pass) {
  DCHECK(support_->GetHitTestAggregator());
  support_->GetHitTestAggregator()->Aggregate(display_->CurrentSurfaceId());
}

void RootCompositorFrameSinkImpl::DisplayDidReceiveCALayerParams(
    const gfx::CALayerParams& ca_layer_params) {
#if defined(OS_MACOSX)
  // If |ca_layer_params| should have content only when there exists a client
  // to send it to.
  DCHECK(ca_layer_params.is_empty || display_client_);
  if (display_client_)
    display_client_->OnDisplayReceivedCALayerParams(ca_layer_params);
#else
  NOTREACHED();
  ALLOW_UNUSED_LOCAL(display_client_);
#endif
}

void RootCompositorFrameSinkImpl::DisplayDidCompleteSwapWithSize(
    const gfx::Size& pixel_size) {
#if defined(OS_ANDROID)
  if (display_client_)
    display_client_->DidCompleteSwapWithSize(pixel_size);
#else
  NOTREACHED();
  ALLOW_UNUSED_LOCAL(display_client_);
#endif
}

void RootCompositorFrameSinkImpl::DidSwapAfterSnapshotRequestReceived(
    const std::vector<ui::LatencyInfo>& latency_info) {
  display_client_->DidSwapAfterSnapshotRequestReceived(latency_info);
}

void RootCompositorFrameSinkImpl::DisplayDidDrawAndSwap() {}

BeginFrameSource* RootCompositorFrameSinkImpl::begin_frame_source() {
  if (external_begin_frame_source_)
    return external_begin_frame_source_.get();
  return synthetic_begin_frame_source_.get();
}

}  // namespace viz
