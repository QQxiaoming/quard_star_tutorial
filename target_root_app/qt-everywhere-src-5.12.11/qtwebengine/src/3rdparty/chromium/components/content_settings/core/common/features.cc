// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file defines all the public base::FeatureList features for the chrome
// module.

#include "components/content_settings/core/common/features.h"

namespace content_settings {
namespace features {

// Makes Flash plugin permissions persistent only through the current session.
const base::Feature kEnableEphemeralFlashPermission{
    "EnableEphemeralFlashPermission", base::FEATURE_ENABLED_BY_DEFAULT};

}  // namespace features
}  // namespace content_settings
