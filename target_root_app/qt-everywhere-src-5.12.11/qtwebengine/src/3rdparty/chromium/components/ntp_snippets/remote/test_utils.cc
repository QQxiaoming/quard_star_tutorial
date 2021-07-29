// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/ntp_snippets/remote/test_utils.h"

#include <memory>

#include "components/prefs/testing_pref_service.h"
#include "components/sync/driver/fake_sync_service.h"

namespace ntp_snippets {

namespace test {

FakeSyncService::FakeSyncService()
    : is_encrypt_everything_enabled_(false),
      active_data_types_(syncer::HISTORY_DELETE_DIRECTIVES) {}

FakeSyncService::~FakeSyncService() = default;

int FakeSyncService::GetDisableReasons() const {
  return DISABLE_REASON_NONE;
}

bool FakeSyncService::IsEncryptEverythingEnabled() const {
  return is_encrypt_everything_enabled_;
}

syncer::ModelTypeSet FakeSyncService::GetActiveDataTypes() const {
  return active_data_types_;
}

RemoteSuggestionsTestUtils::RemoteSuggestionsTestUtils()
    : pref_service_(std::make_unique<TestingPrefServiceSyncable>()) {
  fake_sync_service_ = std::make_unique<FakeSyncService>();
}

RemoteSuggestionsTestUtils::~RemoteSuggestionsTestUtils() = default;

}  // namespace test

}  // namespace ntp_snippets
