// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrunner/browser/webrunner_browser_main_parts.h"

#include "webrunner/browser/context_impl.h"
#include "webrunner/browser/webrunner_browser_context.h"
#include "webrunner/browser/webrunner_screen.h"

namespace webrunner {

WebRunnerBrowserMainParts::WebRunnerBrowserMainParts(
    zx::channel context_channel)
    : context_channel_(std::move(context_channel)) {}

WebRunnerBrowserMainParts::~WebRunnerBrowserMainParts() = default;

void WebRunnerBrowserMainParts::PreMainMessageLoopRun() {
  DCHECK(!screen_);
  screen_ = std::make_unique<WebRunnerScreen>();
  display::Screen::SetScreenInstance(screen_.get());

  DCHECK(!browser_context_);
  browser_context_ = std::make_unique<WebRunnerBrowserContext>();

  fidl::InterfaceRequest<chromium::web::Context> context_request(
      std::move(context_channel_));

  context_impl_ = std::make_unique<ContextImpl>(browser_context_.get());
  context_binding_ = std::make_unique<fidl::Binding<chromium::web::Context>>(
      context_impl_.get(), std::move(context_request));

  // Quit the context process as soon as context is disconnected.
  context_binding_->set_error_handler(
      [this]() { std::move(quit_closure_).Run(); });
}

void WebRunnerBrowserMainParts::PreDefaultMainMessageLoopRun(
    base::OnceClosure quit_closure) {
  quit_closure_ = std::move(quit_closure);
}

}  // namespace webrunner
