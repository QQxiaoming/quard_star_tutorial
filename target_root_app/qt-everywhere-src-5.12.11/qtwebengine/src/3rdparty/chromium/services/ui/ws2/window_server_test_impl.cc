// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/window_server_test_impl.h"

#include "services/ui/public/interfaces/window_tree.mojom.h"
#include "services/ui/ws2/window_service.h"
#include "services/ui/ws2/window_tree.h"

namespace ui {
namespace ws2 {

WindowServerTestImpl::WindowServerTestImpl(WindowService* window_service)
    : window_service_(window_service) {}

WindowServerTestImpl::~WindowServerTestImpl() {}

WindowTree* WindowServerTestImpl::GetWindowTreeWithClientName(
    const std::string& name) {
  for (WindowTree* window_tree : window_service_->window_trees()) {
    if (window_tree->client_name() == name)
      return window_tree;
  }
  return nullptr;
}

void WindowServerTestImpl::OnSurfaceActivated(
    const std::string& desired_client_name,
    EnsureClientHasDrawnWindowCallback cb,
    const std::string& actual_client_name) {
  if (desired_client_name == actual_client_name) {
    std::move(cb).Run(true);
  } else {
    // No tree with the given name, or it hasn't painted yet. Install a callback
    // for the next time a client creates a CompositorFramesink.
    InstallCallback(desired_client_name, std::move(cb));
  }
}

void WindowServerTestImpl::InstallCallback(
    const std::string& client_name,
    EnsureClientHasDrawnWindowCallback cb) {
  window_service_->SetSurfaceActivationCallback(
      base::BindOnce(&WindowServerTestImpl::OnSurfaceActivated,
                     base::Unretained(this), client_name, std::move(cb)));
}

void WindowServerTestImpl::EnsureClientHasDrawnWindow(
    const std::string& client_name,
    EnsureClientHasDrawnWindowCallback callback) {
  WindowTree* tree = GetWindowTreeWithClientName(client_name);
  if (tree && tree->HasAtLeastOneRootWithCompositorFrameSink()) {
    std::move(callback).Run(true);
    return;
  }
  InstallCallback(client_name, std::move(callback));
}

}  // namespace ws2
}  // namespace ui
