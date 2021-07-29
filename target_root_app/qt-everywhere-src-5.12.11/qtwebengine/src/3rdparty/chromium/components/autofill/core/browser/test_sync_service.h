// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_TEST_SYNC_SERVICE_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_TEST_SYNC_SERVICE_H_

#include "components/sync/driver/fake_sync_service.h"
#include "components/sync/driver/sync_service.h"
#include "google_apis/gaia/google_service_auth_error.h"

namespace autofill {
class TestSyncService : public syncer::FakeSyncService {
 public:
  TestSyncService();
  ~TestSyncService() override;

  // FakeSyncService:
  int GetDisableReasons() const override;
  syncer::ModelTypeSet GetPreferredDataTypes() const override;
  syncer::ModelTypeSet GetActiveDataTypes() const override;
  bool IsEngineInitialized() const override;
  bool IsFirstSetupComplete() const override;
  bool IsUsingSecondaryPassphrase() const override;
  syncer::SyncCycleSnapshot GetLastCycleSnapshot() const override;
  const GoogleServiceAuthError& GetAuthError() const override;
  syncer::SyncTokenStatus GetSyncTokenStatus() const override;

  void SetDisableReasons(int disable_reasons) {
    disable_reasons_ = disable_reasons;
  }

  void SetDataTypes(syncer::ModelTypeSet data_types) {
    data_types_ = data_types;
  }

  void SetIsEngineInitialized(bool is_engine_initialized) {
    is_engine_initialized_ = is_engine_initialized;
  }

  void SetIsUsingSecondaryPassphrase(bool is_using_secondary_passphrase) {
    is_using_secondary_passphrase_ = is_using_secondary_passphrase;
  }

  void SetSyncCycleComplete(bool complete) { sync_cycle_complete_ = complete; }

  void SetInAuthError(bool is_in_auth_error);

 private:
  int disable_reasons_ = DISABLE_REASON_NONE;
  // Used as both "preferred" and "active" data types.
  syncer::ModelTypeSet data_types_;
  bool is_engine_initialized_ = true;
  bool is_using_secondary_passphrase_ = false;
  bool sync_cycle_complete_ = true;
  GoogleServiceAuthError auth_error_;
  bool is_in_auth_error_ = false;

  DISALLOW_COPY_AND_ASSIGN(TestSyncService);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_TEST_SYNC_SERVICE_H_
