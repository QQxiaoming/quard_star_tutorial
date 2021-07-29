// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrunner/browser/webrunner_content_browser_client.h"

#include "webrunner/browser/webrunner_browser_main_parts.h"

namespace webrunner {

WebRunnerContentBrowserClient::WebRunnerContentBrowserClient(
    zx::channel context_channel)
    : context_channel_(std::move(context_channel)) {}

WebRunnerContentBrowserClient::~WebRunnerContentBrowserClient() = default;

content::BrowserMainParts*
WebRunnerContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  DCHECK(context_channel_);
  return new WebRunnerBrowserMainParts(std::move(context_channel_));
}

}  // namespace webrunner