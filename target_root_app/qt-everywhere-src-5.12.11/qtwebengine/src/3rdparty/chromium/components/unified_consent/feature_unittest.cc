// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/unified_consent/feature.h"

#include <memory>

#include "components/sync/driver/sync_driver_switches.h"
#include "components/unified_consent/scoped_unified_consent.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace unified_consent {

TEST(UnifiedConsentFeatureTest, UnifiedConsent) {
  // Unified consent is disabled by default.
  EXPECT_EQ(UnifiedConsentFeatureState::kDisabled,
            internal::GetUnifiedConsentFeatureState());

  for (UnifiedConsentFeatureState state :
       {UnifiedConsentFeatureState::kDisabled,
        UnifiedConsentFeatureState::kEnabledNoBump,
        UnifiedConsentFeatureState::kEnabledWithBump}) {
    ScopedUnifiedConsent scoped_state(state);
    EXPECT_EQ(state, internal::GetUnifiedConsentFeatureState());
  }
}

TEST(UnifiedConsentFeatureTest, SyncUserConsentSeparateTypeDisabled) {
  // Enable kSyncUserConsentSeparateType
  base::test::ScopedFeatureList scoped_sync_user_consent_separate_type_feature;
  scoped_sync_user_consent_separate_type_feature.InitAndDisableFeature(
      switches::kSyncUserConsentSeparateType);

  {
    base::test::ScopedFeatureList unified_consent_feature_list_;
    unified_consent_feature_list_.InitAndEnableFeature(kUnifiedConsent);
    EXPECT_EQ(UnifiedConsentFeatureState::kDisabled,
              internal::GetUnifiedConsentFeatureState());
  }

  {
    std::map<std::string, std::string> feature_params;
    feature_params[kUnifiedConsentShowBumpParameter] = "true";
    base::test::ScopedFeatureList unified_consent_feature_list_;
    unified_consent_feature_list_.InitAndEnableFeatureWithParameters(
        kUnifiedConsent, feature_params);
    EXPECT_EQ(UnifiedConsentFeatureState::kDisabled,
              internal::GetUnifiedConsentFeatureState());
  }
}

}  // namespace unified_consent
