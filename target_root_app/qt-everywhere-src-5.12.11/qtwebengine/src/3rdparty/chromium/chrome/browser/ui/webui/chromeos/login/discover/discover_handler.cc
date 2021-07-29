// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/discover/discover_handler.h"

namespace {

const char kDiscoverJsPrefix[] = "discover.";

}  // namespace

namespace chromeos {

DiscoverHandler::DiscoverHandler(const std::string& screen_name) {
  set_call_js_prefix(kDiscoverJsPrefix + screen_name);
}

}  // namespace chromeos
