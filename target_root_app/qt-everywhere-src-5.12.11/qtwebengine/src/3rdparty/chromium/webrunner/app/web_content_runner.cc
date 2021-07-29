// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrunner/app/web_content_runner.h"

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fidl/cpp/binding_set.h>
#include <utility>

#include "base/fuchsia/scoped_service_binding.h"
#include "base/fuchsia/service_directory.h"
#include "base/logging.h"
#include "webrunner/app/component_controller_impl.h"
#include "webrunner/fidl/chromium/web/cpp/fidl.h"

namespace webrunner {

WebContentRunner::WebContentRunner(chromium::web::ContextPtr context)
    : context_(std::move(context)) {
  DCHECK(context_);
}

WebContentRunner::~WebContentRunner() = default;

void WebContentRunner::StartComponent(
    fuchsia::sys::Package package,
    fuchsia::sys::StartupInfo startup_info,
    fidl::InterfaceRequest<fuchsia::sys::ComponentController>
        controller_request) {
  auto controller = ComponentControllerImpl::CreateForRequest(
      this, std::move(package), std::move(startup_info),
      std::move(controller_request));
  controller->BindToRequest(std::move(package), std::move(startup_info),
                            std::move(controller_request));
  controllers_.insert(std::move(controller));
}

void WebContentRunner::DestroyComponent(ComponentControllerImpl* component) {
  controllers_.erase(controllers_.find(component));
}

}  // namespace webrunner
