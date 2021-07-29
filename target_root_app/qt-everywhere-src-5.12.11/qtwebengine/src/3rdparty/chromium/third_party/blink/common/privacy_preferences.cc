// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/privacy_preferences.h"

namespace blink {

PrivacyPreferences::PrivacyPreferences(bool enable_do_not_track,
                                       bool enable_referrers)
    : enable_do_not_track(enable_do_not_track),
      enable_referrers(enable_referrers) {}

}  // namespace blink
