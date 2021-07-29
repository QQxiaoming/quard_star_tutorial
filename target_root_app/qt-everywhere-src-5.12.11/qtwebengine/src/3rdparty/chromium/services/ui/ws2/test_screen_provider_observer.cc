// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/test_screen_provider_observer.h"

#include "base/strings/string_number_conversions.h"

namespace ui {
namespace ws2 {
namespace {

std::string DisplayIdsToString(
    const std::vector<mojom::WsDisplayPtr>& wm_displays) {
  std::string display_ids;
  for (const auto& wm_display : wm_displays) {
    if (!display_ids.empty())
      display_ids += " ";
    display_ids += base::NumberToString(wm_display->display.id());
  }
  return display_ids;
}

}  // namespace

TestScreenProviderObserver::TestScreenProviderObserver() = default;

TestScreenProviderObserver::~TestScreenProviderObserver() = default;

void TestScreenProviderObserver::OnDisplaysChanged(
    std::vector<mojom::WsDisplayPtr> displays,
    int64_t primary_display_id,
    int64_t internal_display_id) {
  displays_ = std::move(displays);
  display_ids_ = DisplayIdsToString(displays_);
  primary_display_id_ = primary_display_id;
  internal_display_id_ = internal_display_id;
}

}  // namespace ws2
}  // namespace ui
