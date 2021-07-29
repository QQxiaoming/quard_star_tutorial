// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/nux_google_apps/constants.h"

#include "base/feature_list.h"

namespace nux_google_apps {

extern const base::Feature kNuxGoogleAppsFeature{
    "NuxGoogleApps", base::FEATURE_DISABLED_BY_DEFAULT};

extern const char kNuxGoogleAppsUrl[] = "chrome://welcome/apps";

}  // namespace nux_google_apps