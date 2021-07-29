// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/ntp_tiles/constants.h"

#include "base/feature_list.h"
#include "build/build_config.h"
#include "ui/base/ui_base_features.h"

namespace ntp_tiles {

const char kPopularSitesFieldTrialName[] = "NTPPopularSites";

const base::Feature kPopularSitesBakedInContentFeature{
    "NTPPopularSitesBakedInContent", base::FEATURE_ENABLED_BY_DEFAULT};

const base::Feature kNtpMostLikelyFaviconsFromServerFeature{
    "NTPMostLikelyFaviconsFromServer", base::FEATURE_ENABLED_BY_DEFAULT};

const base::Feature kSiteExplorationUiFeature{
    "SiteExplorationUi", base::FEATURE_DISABLED_BY_DEFAULT};

const base::Feature kUsePopularSitesSuggestions{
    "UsePopularSitesSuggestions", base::FEATURE_ENABLED_BY_DEFAULT};

const base::Feature kNtpIcons{"NewTabPageIcons",
                              base::FEATURE_DISABLED_BY_DEFAULT};

const base::Feature kNtpCustomLinks{"NewTabPageCustomLinks",
                                    base::FEATURE_DISABLED_BY_DEFAULT};

bool IsMDIconsEnabled() {
#if !defined(OS_ANDROID) && !defined(OS_IOS)
  return base::FeatureList::IsEnabled(kNtpIcons) ||
         base::FeatureList::IsEnabled(kNtpCustomLinks) ||
         base::FeatureList::IsEnabled(features::kExperimentalUi);
#else
  return false;
#endif
}

bool IsCustomLinksEnabled() {
#if !defined(OS_ANDROID) && !defined(OS_IOS)
  return base::FeatureList::IsEnabled(kNtpCustomLinks) ||
         base::FeatureList::IsEnabled(features::kExperimentalUi);
#else
  return false;
#endif
}

}  // namespace ntp_tiles
