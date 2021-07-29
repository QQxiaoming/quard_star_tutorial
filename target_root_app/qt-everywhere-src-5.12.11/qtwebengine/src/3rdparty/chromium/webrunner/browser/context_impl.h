// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBRUNNER_BROWSER_CONTEXT_IMPL_H_
#define WEBRUNNER_BROWSER_CONTEXT_IMPL_H_

#include <lib/fidl/cpp/binding_set.h>
#include <memory>

#include "base/macros.h"
#include "webrunner/common/webrunner_export.h"
#include "webrunner/fidl/chromium/web/cpp/fidl.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace webrunner {

// Implementation of Context from //webrunner/fidl/context.fidl.
// Owns a BrowserContext instance and uses it to create new WebContents/Frames.
// All created Frames are owned by this object.
class WEBRUNNER_EXPORT ContextImpl : public chromium::web::Context {
 public:
  // |browser_context| must outlive ContextImpl.
  explicit ContextImpl(content::BrowserContext* browser_context);

  // Tears down the Context, destroying any active Frames in the process.
  ~ContextImpl() override;

  // chromium::web::Context implementation.
  void CreateFrame(fidl::InterfaceHandle<chromium::web::FrameObserver> observer,
                   fidl::InterfaceRequest<chromium::web::Frame> frame) override;

 private:
  content::BrowserContext* browser_context_;
  fidl::BindingSet<chromium::web::Frame, std::unique_ptr<chromium::web::Frame>>
      frame_bindings_;

  DISALLOW_COPY_AND_ASSIGN(ContextImpl);
};

}  // namespace webrunner

#endif  // WEBRUNNER_BROWSER_CONTEXT_IMPL_H_
