// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/client_root.h"

#include "components/viz/common/surfaces/surface_info.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "services/ui/ws2/client_change.h"
#include "services/ui/ws2/client_change_tracker.h"
#include "services/ui/ws2/server_window.h"
#include "services/ui/ws2/window_service.h"
#include "services/ui/ws2/window_tree.h"
#include "ui/aura/env.h"
#include "ui/aura/mus/client_surface_embedder.h"
#include "ui/aura/mus/property_converter.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#include "ui/compositor/compositor.h"
#include "ui/compositor/dip_util.h"
#include "ui/compositor/property_change_reason.h"

namespace ui {
namespace ws2 {

ClientRoot::ClientRoot(WindowTree* window_tree,
                       aura::Window* window,
                       bool is_top_level)
    : window_tree_(window_tree), window_(window), is_top_level_(is_top_level) {
  window_->AddObserver(this);
  if (window_->GetHost())
    window->GetHost()->AddObserver(this);
  // TODO: wire up gfx::Insets() correctly below. See usage in
  // aura::ClientSurfaceEmbedder for details. Insets here are used for
  // guttering.
  client_surface_embedder_ = std::make_unique<aura::ClientSurfaceEmbedder>(
      window_, is_top_level, gfx::Insets());
  // Ensure there is a valid LocalSurfaceId (if necessary).
  UpdateLocalSurfaceIdIfNecessary();
}

ClientRoot::~ClientRoot() {
  ServerWindow* server_window = ServerWindow::GetMayBeNull(window_);
  window_->RemoveObserver(this);
  if (window_->GetHost())
    window_->GetHost()->RemoveObserver(this);

  viz::HostFrameSinkManager* host_frame_sink_manager =
      aura::Env::GetInstance()
          ->context_factory_private()
          ->GetHostFrameSinkManager();
  host_frame_sink_manager->InvalidateFrameSinkId(
      server_window->frame_sink_id());
}

void ClientRoot::RegisterVizEmbeddingSupport() {
  // This function should only be called once.
  viz::HostFrameSinkManager* host_frame_sink_manager =
      aura::Env::GetInstance()
          ->context_factory_private()
          ->GetHostFrameSinkManager();
  viz::FrameSinkId frame_sink_id =
      ServerWindow::GetMayBeNull(window_)->frame_sink_id();
  host_frame_sink_manager->RegisterFrameSinkId(frame_sink_id, this);
  window_->SetEmbedFrameSinkId(frame_sink_id);

  UpdatePrimarySurfaceId();
}

bool ClientRoot::ShouldAssignLocalSurfaceId() {
  // First level embeddings have their LocalSurfaceId assigned by the
  // WindowService. First level embeddings have no embeddings above them.
  if (is_top_level_)
    return true;
  ServerWindow* server_window = ServerWindow::GetMayBeNull(window_);
  return server_window->owning_window_tree() == nullptr;
}

void ClientRoot::UpdateLocalSurfaceIdIfNecessary() {
  if (!ShouldAssignLocalSurfaceId())
    return;

  gfx::Size size_in_pixels =
      ui::ConvertSizeToPixel(window_->layer(), window_->bounds().size());
  ServerWindow* server_window = ServerWindow::GetMayBeNull(window_);
  // It's expected by cc code that any time the size changes a new
  // LocalSurfaceId is used.
  if (last_surface_size_in_pixels_ != size_in_pixels ||
      !server_window->local_surface_id().has_value() ||
      !server_window->local_surface_id()->is_valid() ||
      last_device_scale_factor_ != window_->layer()->device_scale_factor()) {
    server_window->set_local_surface_id(
        parent_local_surface_id_allocator_.GenerateId());
    last_surface_size_in_pixels_ = size_in_pixels;
    last_device_scale_factor_ = window_->layer()->device_scale_factor();
  }
}

void ClientRoot::UpdatePrimarySurfaceId() {
  UpdateLocalSurfaceIdIfNecessary();
  ServerWindow* server_window = ServerWindow::GetMayBeNull(window_);
  if (server_window->local_surface_id().has_value()) {
    client_surface_embedder_->SetPrimarySurfaceId(viz::SurfaceId(
        window_->GetFrameSinkId(), *server_window->local_surface_id()));
    if (fallback_surface_info_) {
      client_surface_embedder_->SetFallbackSurfaceInfo(*fallback_surface_info_);
      fallback_surface_info_.reset();
    }
  }
}

void ClientRoot::CheckForScaleFactorChange() {
  if (!ShouldAssignLocalSurfaceId() ||
      last_device_scale_factor_ == window_->layer()->device_scale_factor()) {
    return;
  }

  HandleBoundsOrScaleFactorChange(window_->bounds());
}

void ClientRoot::HandleBoundsOrScaleFactorChange(const gfx::Rect& old_bounds) {
  UpdatePrimarySurfaceId();
  client_surface_embedder_->UpdateSizeAndGutters();
  // See comments in WindowTree::SetWindowBoundsImpl() for details on
  // why this always notifies the client.
  window_tree_->window_tree_client_->OnWindowBoundsChanged(
      window_tree_->TransportIdForWindow(window_), old_bounds,
      window_->bounds(),
      ServerWindow::GetMayBeNull(window_)->local_surface_id());
}

void ClientRoot::OnWindowPropertyChanged(aura::Window* window,
                                         const void* key,
                                         intptr_t old) {
  if (window_tree_->property_change_tracker_->IsProcessingChangeForWindow(
          window, ClientChangeType::kProperty)) {
    // Do not send notifications for changes intiated by the client.
    return;
  }
  std::string transport_name;
  std::unique_ptr<std::vector<uint8_t>> transport_value;
  if (window_tree_->window_service()
          ->property_converter()
          ->ConvertPropertyForTransport(window, key, &transport_name,
                                        &transport_value)) {
    base::Optional<std::vector<uint8_t>> transport_value_mojo;
    if (transport_value)
      transport_value_mojo.emplace(std::move(*transport_value));
    window_tree_->window_tree_client_->OnWindowSharedPropertyChanged(
        window_tree_->TransportIdForWindow(window), transport_name,
        transport_value_mojo);
  }
}

void ClientRoot::OnWindowBoundsChanged(aura::Window* window,
                                       const gfx::Rect& old_bounds,
                                       const gfx::Rect& new_bounds,
                                       ui::PropertyChangeReason reason) {
  HandleBoundsOrScaleFactorChange(old_bounds);
}

void ClientRoot::OnWindowAddedToRootWindow(aura::Window* window) {
  DCHECK_EQ(window, window_);
  DCHECK(window->GetHost());
  window->GetHost()->AddObserver(this);
  CheckForScaleFactorChange();
}

void ClientRoot::OnWindowRemovingFromRootWindow(aura::Window* window,
                                                aura::Window* new_root) {
  DCHECK_EQ(window, window_);
  DCHECK(window->GetHost());
  window->GetHost()->RemoveObserver(this);
}

void ClientRoot::OnHostResized(aura::WindowTreeHost* host) {
  // This function is also called when the device-scale-factor changes too.
  CheckForScaleFactorChange();
}

void ClientRoot::OnFirstSurfaceActivation(
    const viz::SurfaceInfo& surface_info) {
  ServerWindow* server_window = ServerWindow::GetMayBeNull(window_);
  if (server_window->local_surface_id().has_value()) {
    DCHECK(!fallback_surface_info_);
    client_surface_embedder_->SetFallbackSurfaceInfo(surface_info);
  } else {
    fallback_surface_info_ = std::make_unique<viz::SurfaceInfo>(surface_info);
  }
  if (!window_tree_->client_name().empty()) {
    // OnFirstSurfaceActivation() should only be called after
    // AttachCompositorFrameSink().
    DCHECK(server_window->attached_compositor_frame_sink());
    window_tree_->window_service()->OnFirstSurfaceActivation(
        window_tree_->client_name());
  }
}

void ClientRoot::OnFrameTokenChanged(uint32_t frame_token) {
  // TODO: this needs to call through to WindowTreeClient.
  // https://crbug.com/848022.
}

}  // namespace ws2
}  // namespace ui
