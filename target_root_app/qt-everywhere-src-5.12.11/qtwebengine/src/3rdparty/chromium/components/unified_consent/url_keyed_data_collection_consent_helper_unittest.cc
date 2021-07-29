// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/unified_consent/url_keyed_data_collection_consent_helper.h"

#include <vector>

#include "components/sync/driver/fake_sync_service.h"
#include "components/sync/engine/cycle/sync_cycle_snapshot.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "components/unified_consent/pref_names.h"
#include "components/unified_consent/unified_consent_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace unified_consent {
namespace {

class TestSyncService : public syncer::FakeSyncService {
 public:
  void set_sync_initialized(bool sync_initialized) {
    sync_initialized_ = sync_initialized;
  }
  void AddActiveDataType(syncer::ModelType type) {
    sync_active_data_types_.Put(type);
  }
  void ClearActiveDataTypes() { sync_active_data_types_.Clear(); }
  void FireOnStateChangeOnAllObservers() {
    for (auto& observer : observers_)
      observer.OnStateChanged(this);
  }

  // syncer::FakeSyncService:
  int GetDisableReasons() const override { return DISABLE_REASON_NONE; }
  syncer::ModelTypeSet GetPreferredDataTypes() const override {
    return syncer::ModelTypeSet(syncer::ModelType::HISTORY_DELETE_DIRECTIVES,
                                syncer::ModelType::USER_EVENTS,
                                syncer::ModelType::EXTENSIONS);
  }
  bool IsFirstSetupComplete() const override { return true; }
  bool IsEngineInitialized() const override { return true; }

  syncer::SyncCycleSnapshot GetLastCycleSnapshot() const override {
    if (!sync_initialized_)
      return syncer::SyncCycleSnapshot();
    return syncer::SyncCycleSnapshot(
        syncer::ModelNeutralState(), syncer::ProgressMarkerMap(), false, 5, 2,
        7, false, 0, base::Time::Now(), base::Time::Now(),
        std::vector<int>(syncer::MODEL_TYPE_COUNT, 0),
        std::vector<int>(syncer::MODEL_TYPE_COUNT, 0),
        sync_pb::SyncEnums::UNKNOWN_ORIGIN,
        /*short_poll_interval=*/base::TimeDelta::FromMinutes(30),
        /*long_poll_interval=*/base::TimeDelta::FromMinutes(180),
        /*has_remaining_local_changes=*/false);
  }

  syncer::ModelTypeSet GetActiveDataTypes() const override {
    return sync_active_data_types_;
  }

  void AddObserver(syncer::SyncServiceObserver* observer) override {
    observers_.AddObserver(observer);
  }
  void RemoveObserver(syncer::SyncServiceObserver* observer) override {
    observers_.RemoveObserver(observer);
  }

 private:
  bool sync_initialized_ = false;
  syncer::ModelTypeSet sync_active_data_types_;
  base::ObserverList<syncer::SyncServiceObserver> observers_;
};

class UrlKeyedDataCollectionConsentHelperTest
    : public testing::Test,
      public UrlKeyedDataCollectionConsentHelper::Observer {
 public:
  // testing::Test:
  void SetUp() override {
    UnifiedConsentService::RegisterPrefs(pref_service_.registry());
  }

  void OnUrlKeyedDataCollectionConsentStateChanged(
      UrlKeyedDataCollectionConsentHelper* consent_helper) override {
    state_changed_notifications.push_back(consent_helper->IsEnabled());
  }

 protected:
  sync_preferences::TestingPrefServiceSyncable pref_service_;
  std::vector<bool> state_changed_notifications;
  TestSyncService sync_service_;
};

TEST_F(UrlKeyedDataCollectionConsentHelperTest,
       AnonymizedDataCollection_UnifiedConsentEnabled) {
  std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
      UrlKeyedDataCollectionConsentHelper::
          NewAnonymizedDataCollectionConsentHelper(true, &pref_service_,
                                                   &sync_service_);
  helper->AddObserver(this);
  EXPECT_FALSE(helper->IsEnabled());
  EXPECT_TRUE(state_changed_notifications.empty());

  pref_service_.SetBoolean(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                           true);
  EXPECT_TRUE(helper->IsEnabled());
  ASSERT_EQ(1U, state_changed_notifications.size());
  EXPECT_TRUE(state_changed_notifications[0]);

  state_changed_notifications.clear();
  pref_service_.SetBoolean(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                           false);
  EXPECT_FALSE(helper->IsEnabled());
  ASSERT_EQ(1U, state_changed_notifications.size());
  EXPECT_FALSE(state_changed_notifications[0]);
  helper->RemoveObserver(this);
}

TEST_F(UrlKeyedDataCollectionConsentHelperTest,
       AnonymizedDataCollection_UnifiedConsentDisabled) {
  std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
      UrlKeyedDataCollectionConsentHelper::
          NewAnonymizedDataCollectionConsentHelper(false, &pref_service_,
                                                   &sync_service_);
  helper->AddObserver(this);
  EXPECT_FALSE(helper->IsEnabled());
  EXPECT_TRUE(state_changed_notifications.empty());

  sync_service_.set_sync_initialized(true);
  sync_service_.AddActiveDataType(syncer::ModelType::HISTORY_DELETE_DIRECTIVES);
  sync_service_.FireOnStateChangeOnAllObservers();
  EXPECT_TRUE(helper->IsEnabled());
  EXPECT_EQ(1U, state_changed_notifications.size());
  helper->RemoveObserver(this);
}

TEST_F(UrlKeyedDataCollectionConsentHelperTest,
       AnonymizedDataCollection_UnifiedConsentDisabled_NullSyncService) {
  std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
      UrlKeyedDataCollectionConsentHelper::
          NewAnonymizedDataCollectionConsentHelper(
              false /* is_unified_consent_enabled */, &pref_service_,
              nullptr /* sync_service */);
  EXPECT_FALSE(helper->IsEnabled());
}

TEST_F(UrlKeyedDataCollectionConsentHelperTest,
       PersonalizeddDataCollection_UnifiedConsentEnabled) {
  std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
      UrlKeyedDataCollectionConsentHelper::
          NewPersonalizedDataCollectionConsentHelper(true, &sync_service_);
  helper->AddObserver(this);
  EXPECT_FALSE(helper->IsEnabled());
  EXPECT_TRUE(state_changed_notifications.empty());
  sync_service_.set_sync_initialized(true);

  // Peronalized data collection is disabled when only USER_EVENTS are enabled.
  sync_service_.AddActiveDataType(syncer::ModelType::USER_EVENTS);
  sync_service_.FireOnStateChangeOnAllObservers();
  EXPECT_FALSE(helper->IsEnabled());
  EXPECT_TRUE(state_changed_notifications.empty());

  // Peronalized data collection is disabled when only HISTORY_DELETE_DIRECTIVES
  // are enabled.
  sync_service_.ClearActiveDataTypes();
  sync_service_.AddActiveDataType(syncer::ModelType::HISTORY_DELETE_DIRECTIVES);
  sync_service_.FireOnStateChangeOnAllObservers();
  EXPECT_FALSE(helper->IsEnabled());
  EXPECT_TRUE(state_changed_notifications.empty());

  // Personalized data collection is enabled iff USER_EVENTS and
  // HISTORY_DELETE_DIRECTIVES are enabled.
  sync_service_.ClearActiveDataTypes();
  sync_service_.AddActiveDataType(syncer::ModelType::HISTORY_DELETE_DIRECTIVES);
  sync_service_.AddActiveDataType(syncer::ModelType::USER_EVENTS);
  sync_service_.FireOnStateChangeOnAllObservers();
  EXPECT_TRUE(helper->IsEnabled());
  EXPECT_EQ(1U, state_changed_notifications.size());
  helper->RemoveObserver(this);
}

TEST_F(UrlKeyedDataCollectionConsentHelperTest,
       PersonalizedDataCollection_UnifiedConsentDisabled) {
  std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
      UrlKeyedDataCollectionConsentHelper::
          NewPersonalizedDataCollectionConsentHelper(false, &sync_service_);
  helper->AddObserver(this);
  EXPECT_FALSE(helper->IsEnabled());
  EXPECT_TRUE(state_changed_notifications.empty());

  sync_service_.set_sync_initialized(true);
  sync_service_.AddActiveDataType(syncer::ModelType::HISTORY_DELETE_DIRECTIVES);
  sync_service_.FireOnStateChangeOnAllObservers();
  EXPECT_TRUE(helper->IsEnabled());
  EXPECT_EQ(1U, state_changed_notifications.size());
  helper->RemoveObserver(this);
}

TEST_F(UrlKeyedDataCollectionConsentHelperTest,
       PersonalizedDataCollection_NullSyncService) {
  {
    std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
        UrlKeyedDataCollectionConsentHelper::
            NewPersonalizedDataCollectionConsentHelper(
                false /* is_unified_consent_enabled */,
                nullptr /* sync_service */);
    EXPECT_FALSE(helper->IsEnabled());
  }
  {
    std::unique_ptr<UrlKeyedDataCollectionConsentHelper> helper =
        UrlKeyedDataCollectionConsentHelper::
            NewPersonalizedDataCollectionConsentHelper(
                true /* is_unified_consent_enabled */,
                nullptr /* sync_service */);
    EXPECT_FALSE(helper->IsEnabled());
  }
}

}  // namespace
}  // namespace unified_consent
