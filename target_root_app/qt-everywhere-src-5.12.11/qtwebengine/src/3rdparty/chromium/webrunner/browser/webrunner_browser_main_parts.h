// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBRUNNER_BROWSER_WEBRUNNER_BROWSER_MAIN_PARTS_H_
#define WEBRUNNER_BROWSER_WEBRUNNER_BROWSER_MAIN_PARTS_H_

#include <lib/fidl/cpp/binding.h>
#include <memory>

#include "base/macros.h"
#include "content/public/browser/browser_main_parts.h"
#include "webrunner/fidl/chromium/web/cpp/fidl.h"

namespace webrunner {

class ContextImpl;
class WebRunnerBrowserContext;
class WebRunnerScreen;

class WebRunnerBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit WebRunnerBrowserMainParts(zx::channel context_channel);
  ~WebRunnerBrowserMainParts() override;

  // content::BrowserMainParts overrides.
  void PreMainMessageLoopRun() override;
  void PreDefaultMainMessageLoopRun(base::OnceClosure quit_closure) override;

 private:
  zx::channel context_channel_;

  std::unique_ptr<WebRunnerScreen> screen_;
  std::unique_ptr<WebRunnerBrowserContext> browser_context_;

  std::unique_ptr<ContextImpl> context_impl_;
  std::unique_ptr<fidl::Binding<chromium::web::Context>> context_binding_;

  base::OnceClosure quit_closure_;

  DISALLOW_COPY_AND_ASSIGN(WebRunnerBrowserMainParts);
};

}  // namespace webrunner

#endif  // WEBRUNNER_BROWSER_WEBRUNNER_BROWSER_MAIN_PARTS_H_