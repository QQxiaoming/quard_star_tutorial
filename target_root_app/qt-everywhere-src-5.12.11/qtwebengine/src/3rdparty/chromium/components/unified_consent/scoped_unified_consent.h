// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_UNIFIED_CONSENT_SCOPED_UNIFIED_CONSENT_H_
#define COMPONENTS_UNIFIED_CONSENT_SCOPED_UNIFIED_CONSENT_H_

#include <memory>

#include "base/macros.h"
#include "base/test/scoped_feature_list.h"
#include "components/unified_consent/feature.h"

namespace unified_consent {

// Changes the unified consent feature state while it is in scope. Useful for
// tests.
// Also enables the feature |switches::kSyncUserConsentSeparateType| as
// unified consent depends on its.
class ScopedUnifiedConsent {
 public:
  explicit ScopedUnifiedConsent(UnifiedConsentFeatureState state);
  ~ScopedUnifiedConsent();

 private:
  base::test::ScopedFeatureList sync_user_consent_separate_type_feature_list_;
  base::test::ScopedFeatureList unified_consent_feature_list_;

  DISALLOW_COPY_AND_ASSIGN(ScopedUnifiedConsent);
};

}  // namespace unified_consent

#endif  // COMPONENTS_UNIFIED_CONSENT_SCOPED_UNIFIED_CONSENT_H_
