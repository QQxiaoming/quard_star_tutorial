// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrunner/browser/context_impl.h"

#include <memory>
#include <utility>

#include "content/public/browser/web_contents.h"
#include "webrunner/browser/frame_impl.h"

namespace webrunner {

ContextImpl::ContextImpl(content::BrowserContext* browser_context)
    : browser_context_(browser_context) {}

ContextImpl::~ContextImpl() = default;

void ContextImpl::CreateFrame(
    fidl::InterfaceHandle<chromium::web::FrameObserver> observer,
    fidl::InterfaceRequest<chromium::web::Frame> frame_request) {
  auto web_contents = content::WebContents::Create(
      content::WebContents::CreateParams(browser_context_, nullptr));
  frame_bindings_.AddBinding(
      std::make_unique<FrameImpl>(std::move(web_contents), observer.Bind()),
      std::move(frame_request));
}

}  // namespace webrunner
