// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "components/consent_auditor/fake_consent_auditor.h"

namespace consent_auditor {

FakeConsentAuditor::FakeConsentAuditor() {}

FakeConsentAuditor::~FakeConsentAuditor() {}

void FakeConsentAuditor::RecordGaiaConsent(
    const std::string& account_id,
    consent_auditor::Feature feature,
    const std::vector<int>& description_grd_ids,
    int confirmation_grd_id,
    consent_auditor::ConsentStatus status) {
  account_id_ = account_id;
  recorded_id_vectors_.push_back(description_grd_ids);
  recorded_confirmation_ids_.push_back(confirmation_grd_id);
  recorded_features_.push_back(feature);
  recorded_statuses_.push_back(status);
}

void FakeConsentAuditor::RecordLocalConsent(
    const std::string& feature,
    const std::string& description_text,
    const std::string& confirmation_text) {
  NOTIMPLEMENTED();
}

base::WeakPtr<syncer::ModelTypeControllerDelegate>
FakeConsentAuditor::GetControllerDelegateOnUIThread() {
  NOTIMPLEMENTED();
  return nullptr;
}

}  // namespace consent_auditor
