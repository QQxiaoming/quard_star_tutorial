// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_UNIFIED_CONSENT_FEATURE_H_
#define COMPONENTS_UNIFIED_CONSENT_FEATURE_H_

#include "base/feature_list.h"

namespace unified_consent {

// State of the "Unified Consent" feature.
enum class UnifiedConsentFeatureState {
  // Unified consent is disabled.
  kDisabled,
  // Unified consent is enabled, but the bump is not shown.
  kEnabledNoBump,
  // Unified consent is enabled and the bump is shown.
  kEnabledWithBump
};

// Improved and unified consent for privacy-related features.
extern const base::Feature kUnifiedConsent;
extern const char kUnifiedConsentShowBumpParameter[];

namespace internal {
// Returns the state of the "Unified Consent" feature.
//
// WARNING: Do not call this method directly to check whether unfied consent
// is enabled. Please use the per-platfome functions defined in
// * chrome/browser/signin/unified_consent_helper.h
// * ios/chrome/browser/unified_consent/feature.h
unified_consent::UnifiedConsentFeatureState GetUnifiedConsentFeatureState();
}  // namespace internal

}  // namespace unified_consent

#endif  // COMPONENTS_UNIFIED_CONSENT_FEATURE_H_
