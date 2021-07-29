// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBRUNNER_APP_WEB_CONTENT_RUNNER_H_
#define WEBRUNNER_APP_WEB_CONTENT_RUNNER_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <memory>
#include <set>

#include "base/callback.h"
#include "base/containers/unique_ptr_adapters.h"
#include "base/macros.h"
#include "webrunner/fidl/chromium/web/cpp/fidl.h"

namespace webrunner {

class ComponentControllerImpl;

// An implementation of a sys::Runner that launches web content and renders it
// inside a View.
class WebContentRunner : public fuchsia::sys::Runner {
 public:
  // |content|: Context (e.g. persisted profile storage) under which all web
  //   content launched through this Runner instance will be run.
  explicit WebContentRunner(chromium::web::ContextPtr context);
  ~WebContentRunner() override;

  chromium::web::Context* context() { return context_.get(); }

  // Used by ComponentControllerImpls to signal that the controller's connection
  // was dropped, and therefore the controller should be destroyed.
  void DestroyComponent(ComponentControllerImpl* component);

  // fuchsia::sys::Runner implementation.
  void StartComponent(fuchsia::sys::Package package,
                      fuchsia::sys::StartupInfo startup_info,
                      fidl::InterfaceRequest<fuchsia::sys::ComponentController>
                          controller_request) override;

 private:
  chromium::web::ContextPtr context_;
  std::set<std::unique_ptr<ComponentControllerImpl>, base::UniquePtrComparator>
      controllers_;

  DISALLOW_COPY_AND_ASSIGN(WebContentRunner);
};

}  // namespace webrunner

#endif  // WEBRUNNER_APP_WEB_CONTENT_RUNNER_H_
