// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/unified_consent/unified_consent_service.h"

#include <map>
#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/test/metrics/histogram_tester.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/sync/base/sync_prefs.h"
#include "components/sync/driver/fake_sync_service.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "components/unified_consent/pref_names.h"
#include "components/unified_consent/scoped_unified_consent.h"
#include "components/unified_consent/unified_consent_service_client.h"
#include "services/identity/public/cpp/identity_test_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace unified_consent {
namespace {

class TestSyncService : public syncer::FakeSyncService {
 public:
  explicit TestSyncService(PrefService* pref_service)
      : pref_service_(pref_service) {}

  int GetDisableReasons() const override { return DISABLE_REASON_NONE; }
  bool IsFirstSetupComplete() const override { return true; }
  bool IsEngineInitialized() const override { return engine_initialized_; }
  void AddObserver(syncer::SyncServiceObserver* observer) override {
    observer_ = observer;
  }
  void OnUserChoseDatatypes(bool sync_everything,
                            syncer::ModelTypeSet chosen_types) override {
    syncer::SyncPrefs(pref_service_).SetKeepEverythingSynced(sync_everything);
    chosen_types_ = chosen_types;
  }
  syncer::ModelTypeSet GetPreferredDataTypes() const override {
    syncer::ModelTypeSet preferred = chosen_types_;
    // Add this for the Migration_UpdateSettings test.
    preferred.Put(syncer::HISTORY_DELETE_DIRECTIVES);
    return preferred;
  }
  bool IsUsingSecondaryPassphrase() const override {
    return is_using_passphrase_;
  }
  SyncService::State GetState() const override { return state_; }

  void SetEngineInitialized(bool engine_initialized) {
    engine_initialized_ = engine_initialized;
  }
  void SetIsUsingPassphrase(bool using_passphrase) {
    is_using_passphrase_ = using_passphrase;
  }
  void SetState(SyncService::State state) { state_ = state; }

  void FireStateChanged() {
    if (observer_)
      observer_->OnStateChanged(this);
  }

 private:
  syncer::SyncServiceObserver* observer_ = nullptr;
  bool engine_initialized_ = true;
  syncer::ModelTypeSet chosen_types_ = syncer::UserSelectableTypes();
  SyncService::State state_ = SyncService::State::ACTIVE;
  bool is_using_passphrase_ = false;
  PrefService* pref_service_;
};

const char kSpellCheckDummyEnabled[] = "spell_check_dummy.enabled";

class FakeUnifiedConsentServiceClient : public UnifiedConsentServiceClient {
 public:
  FakeUnifiedConsentServiceClient(PrefService* pref_service)
      : pref_service_(pref_service) {
    // When the |kSpellCheckDummyEnabled| pref is changed, all observers should
    // be fired.
    ObserveServicePrefChange(Service::kSpellCheck, kSpellCheckDummyEnabled,
                             pref_service);
  }
  ~FakeUnifiedConsentServiceClient() override = default;

  // UnifiedConsentServiceClient:
  ServiceState GetServiceState(Service service) override {
    if (is_not_supported_[service])
      return ServiceState::kNotSupported;
    bool enabled;
    // Special treatment for spell check.
    if (service == Service::kSpellCheck) {
      enabled = pref_service_->GetBoolean(kSpellCheckDummyEnabled);
    } else {
      enabled = service_enabled_[service];
    }
    return enabled ? ServiceState::kEnabled : ServiceState::kDisabled;
  }
  void SetServiceEnabled(Service service, bool enabled) override {
    if (is_not_supported_[service])
      return;
    // Special treatment for spell check.
    if (service == Service::kSpellCheck) {
      pref_service_->SetBoolean(kSpellCheckDummyEnabled, enabled);
      return;
    }
    bool should_notify_observers = service_enabled_[service] != enabled;
    service_enabled_[service] = enabled;
    if (should_notify_observers)
      FireOnServiceStateChanged(service);
  }

  void SetServiceNotSupported(Service service) {
    is_not_supported_[service] = true;
  }

 private:
  std::map<Service, bool> service_enabled_;
  std::map<Service, bool> is_not_supported_;

  PrefService* pref_service_;
};

}  // namespace

class UnifiedConsentServiceTest : public testing::Test {
 public:
  UnifiedConsentServiceTest() : sync_service_(&pref_service_) {}

  // testing::Test:
  void SetUp() override {
    pref_service_.registry()->RegisterBooleanPref(
        autofill::prefs::kAutofillWalletImportEnabled, false);
    UnifiedConsentService::RegisterPrefs(pref_service_.registry());
    syncer::SyncPrefs::RegisterProfilePrefs(pref_service_.registry());
    pref_service_.registry()->RegisterBooleanPref(kSpellCheckDummyEnabled,
                                                  false);
  }

  void TearDown() override {
    if (consent_service_)
      consent_service_->Shutdown();
  }

  void CreateConsentService(bool client_services_on_by_default = false) {
    if (!scoped_unified_consent_) {
      SetUnifiedConsentFeatureState(
          unified_consent::UnifiedConsentFeatureState::kEnabledWithBump);
    }

    auto client =
        std::make_unique<FakeUnifiedConsentServiceClient>(&pref_service_);
    if (client_services_on_by_default) {
      for (int i = 0; i <= static_cast<int>(Service::kLast); ++i) {
        Service service = static_cast<Service>(i);
        client->SetServiceEnabled(service, true);
      }
      pref_service_.SetBoolean(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                               true);
    }
    consent_service_ = std::make_unique<UnifiedConsentService>(
        std::move(client), &pref_service_,
        identity_test_environment_.identity_manager(), &sync_service_);
    service_client_ = (FakeUnifiedConsentServiceClient*)
                          consent_service_->service_client_.get();
  }

  void SetUnifiedConsentFeatureState(
      unified_consent::UnifiedConsentFeatureState feature_state) {
    // First reset |scoped_unified_consent_| to nullptr in case it was set
    // before and then initialize it with the new value. This makes sure that
    // the old scoped object is deleted before the new one is created.
    scoped_unified_consent_.reset();
    scoped_unified_consent_.reset(
        new unified_consent::ScopedUnifiedConsent(feature_state));
  }

  bool AreAllNonPersonalizedServicesEnabled() {
    return consent_service_->AreAllNonPersonalizedServicesEnabled();
  }

  bool AreAllOnByDefaultPrivacySettingsOn() {
    return consent_service_->AreAllOnByDefaultPrivacySettingsOn();
  }

  unified_consent::MigrationState GetMigrationState() {
    int migration_state_int =
        pref_service_.GetInteger(prefs::kUnifiedConsentMigrationState);
    return static_cast<unified_consent::MigrationState>(migration_state_int);
  }

 protected:
  base::MessageLoop message_loop_;
  sync_preferences::TestingPrefServiceSyncable pref_service_;
  identity::IdentityTestEnvironment identity_test_environment_;
  TestSyncService sync_service_;
  std::unique_ptr<UnifiedConsentService> consent_service_;
  FakeUnifiedConsentServiceClient* service_client_ = nullptr;

  std::unique_ptr<ScopedUnifiedConsent> scoped_unified_consent_;
};

TEST_F(UnifiedConsentServiceTest, DefaultValuesWhenSignedOut) {
  CreateConsentService();
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
}

TEST_F(UnifiedConsentServiceTest, EnableUnfiedConsent) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());

  // Enable Unified Consent enables all non-personaized features
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, true);
  EXPECT_TRUE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());

  // Disable unified consent does not disable any of the non-personalized
  // features.
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, false);
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());
}

TEST_F(UnifiedConsentServiceTest, EnableUnfiedConsent_WithUnsupportedService) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());
  service_client_->SetServiceNotSupported(Service::kSpellCheck);
  EXPECT_EQ(service_client_->GetServiceState(Service::kSpellCheck),
            ServiceState::kNotSupported);
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());

  // Enable Unified Consent enables all supported non-personalized features
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, true);
  EXPECT_TRUE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());

  // Disable unified consent does not disable any of the supported
  // non-personalized features.
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, false);
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());
}

TEST_F(UnifiedConsentServiceTest, EnableUnfiedConsent_SyncNotActive) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  sync_service_.OnUserChoseDatatypes(false, syncer::UserSelectableTypes());
  syncer::SyncPrefs sync_prefs(&pref_service_);
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(consent_service_->IsUnifiedConsentGiven());

  // Make sure sync is not active.
  sync_service_.SetEngineInitialized(false);
  EXPECT_FALSE(sync_service_.IsEngineInitialized());
  sync_service_.SetState(syncer::SyncService::State::INITIALIZING);
  EXPECT_NE(sync_service_.GetState(), syncer::SyncService::State::ACTIVE);

  // Opt into unified consent.
  consent_service_->SetUnifiedConsentGiven(true);
  EXPECT_TRUE(consent_service_->IsUnifiedConsentGiven());

  // Couldn't sync everything because sync is not active.
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());

  // Initalize sync engine and therefore activate sync.
  sync_service_.SetEngineInitialized(true);
  sync_service_.SetState(syncer::SyncService::State::ACTIVE);
  EXPECT_EQ(sync_service_.GetState(), syncer::SyncService::State::ACTIVE);
  sync_service_.FireStateChanged();

  // UnifiedConsentService starts syncing everything.
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());
}

TEST_F(UnifiedConsentServiceTest, EnableUnfiedConsent_WithCustomPassphrase) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");
  EXPECT_FALSE(consent_service_->IsUnifiedConsentGiven());
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());

  // Enable Unified Consent.
  consent_service_->SetUnifiedConsentGiven(true);
  EXPECT_TRUE(consent_service_->IsUnifiedConsentGiven());
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());

  // Set custom passphrase.
  sync_service_.SetIsUsingPassphrase(true);
  sync_service_.FireStateChanged();

  // Setting a custom passphrase forces off unified consent given.
  EXPECT_FALSE(consent_service_->IsUnifiedConsentGiven());
}

// Test whether unified consent is disabled when any of its dependent services
// gets disabled.
TEST_F(UnifiedConsentServiceTest, DisableUnfiedConsentWhenServiceIsDisabled) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());

  // Enable Unified Consent enables all supported non-personalized features
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, true);
  EXPECT_TRUE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());

  // Disabling child service disables unified consent.
  pref_service_.SetBoolean(kSpellCheckDummyEnabled, false);
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
}

// Test whether unified consent is disabled when any of its dependent services
// gets disabled before startup.
TEST_F(UnifiedConsentServiceTest,
       DisableUnfiedConsentWhenServiceIsDisabled_OnStartup) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());

  // Enable Unified Consent enables all supported non-personalized features
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, true);
  EXPECT_TRUE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());

  // Simulate shutdown.
  consent_service_->Shutdown();
  consent_service_.reset();

  // Disable child service.
  pref_service_.SetBoolean(kSpellCheckDummyEnabled, false);

  // Unified Consent is disabled during creation of the consent service because
  // not all non-personalized services are enabled.
  CreateConsentService();
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
}

#if !defined(OS_CHROMEOS)
TEST_F(UnifiedConsentServiceTest, Migration_SyncingEverythingAndAllServicesOn) {
  base::HistogramTester histogram_tester;

  // Create inconsistent state.
  identity_test_environment_.SetPrimaryAccount("testaccount");
  sync_service_.OnUserChoseDatatypes(true, syncer::UserSelectableTypes());
  syncer::SyncPrefs sync_prefs(&pref_service_);
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  sync_service_.SetState(
      syncer::SyncService::State::PENDING_DESIRED_CONFIGURATION);
  EXPECT_FALSE(sync_service_.IsSyncActive());

  CreateConsentService(true /* client services on by default */);
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());
  // After the creation of the consent service, the profile started to migrate
  // (but waiting for sync init) and |ShouldShowConsentBump| should return true.
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_EQ(GetMigrationState(),
            unified_consent::MigrationState::kInProgressWaitForSyncInit);
  EXPECT_TRUE(consent_service_->ShouldShowConsentBump());
  // Sync-everything is still on because sync is not active yet.
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());

  // When sync is active, the migration should continue and finish.
  sync_service_.SetState(syncer::SyncService::State::ACTIVE);
  sync_service_.FireStateChanged();
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());

  // No metric for the consent bump suppress reason should have been recorded at
  // this point.
  histogram_tester.ExpectTotalCount("UnifiedConsent.ConsentBump.SuppressReason",
                                    0);

  // When the user signs out, the migration state changes to completed and the
  // consent bump doesn't need to be shown anymore.
  identity_test_environment_.ClearPrimaryAccount();
  EXPECT_EQ(GetMigrationState(), unified_consent::MigrationState::kCompleted);
  EXPECT_FALSE(consent_service_->ShouldShowConsentBump());
  // A metric for the consent bump suppress reason should have been recorded at
  // this point.
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.ConsentBump.SuppressReason",
      unified_consent::ConsentBumpSuppressReason::kUserSignedOut, 1);
}

TEST_F(UnifiedConsentServiceTest, Migration_SyncingEverythingAndServicesOff) {
  base::HistogramTester histogram_tester;

  // Create inconsistent state.
  identity_test_environment_.SetPrimaryAccount("testaccount");
  sync_service_.OnUserChoseDatatypes(true, syncer::UserSelectableTypes());
  syncer::SyncPrefs sync_prefs(&pref_service_);
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_TRUE(sync_service_.IsSyncActive());

  CreateConsentService();
  EXPECT_FALSE(AreAllOnByDefaultPrivacySettingsOn());
  // After the creation of the consent service, the profile is migrated and
  // |ShouldShowConsentBump| should return false.
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_EQ(GetMigrationState(), unified_consent::MigrationState::kCompleted);
  EXPECT_FALSE(consent_service_->ShouldShowConsentBump());

  // A metric for the consent bump suppress reason should have been recorded at
  // this point.
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.ConsentBump.SuppressReason",
      unified_consent::ConsentBumpSuppressReason::kPrivacySettingOff, 1);
}
#endif  // !defined(OS_CHROMEOS)

TEST_F(UnifiedConsentServiceTest, Migration_NotSyncingEverything) {
  base::HistogramTester histogram_tester;

  identity_test_environment_.SetPrimaryAccount("testaccount");
  sync_service_.OnUserChoseDatatypes(false, syncer::UserSelectableTypes());
  syncer::SyncPrefs sync_prefs(&pref_service_);
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());

  CreateConsentService();
  // When the user is not syncing everything the migration is completed after
  // the creation of the consent service.
  EXPECT_EQ(GetMigrationState(), unified_consent::MigrationState::kCompleted);
  // The suppress reason for not showing the consent bump should be recorded.
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.ConsentBump.SuppressReason",
      unified_consent::ConsentBumpSuppressReason::kSyncEverythingOff, 1);
}

TEST_F(UnifiedConsentServiceTest, Migration_UpdateSettings) {
  // Create user that syncs everything
  identity_test_environment_.SetPrimaryAccount("testaccount");
  sync_service_.OnUserChoseDatatypes(true, syncer::UserSelectableTypes());
  syncer::SyncPrefs sync_prefs(&pref_service_);
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_TRUE(sync_service_.IsSyncActive());
  EXPECT_TRUE(sync_service_.GetPreferredDataTypes().Has(syncer::USER_EVENTS));
  // Url keyed data collection is off before the migration.
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));

  CreateConsentService();
  EXPECT_EQ(GetMigrationState(), unified_consent::MigrationState::kCompleted);
  // During the migration USER_EVENTS is disabled and Url keyed data collection
  // is enabled.
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(sync_service_.GetPreferredDataTypes().Has(syncer::USER_EVENTS));
  EXPECT_TRUE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
}

#if !defined(OS_CHROMEOS)
TEST_F(UnifiedConsentServiceTest, ClearPrimaryAccountDisablesSomeServices) {
  CreateConsentService();
  identity_test_environment_.SetPrimaryAccount("testaccount");

  // Precondition: Enable unified consent.
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, true);
  EXPECT_TRUE(AreAllNonPersonalizedServicesEnabled());

  // Clearing primary account revokes unfied consent and a couple of other
  // non-personalized services.
  identity_test_environment_.ClearPrimaryAccount();
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_FALSE(AreAllNonPersonalizedServicesEnabled());
  EXPECT_FALSE(pref_service_.GetBoolean(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled));
  EXPECT_EQ(service_client_->GetServiceState(Service::kSpellCheck),
            ServiceState::kDisabled);
  EXPECT_EQ(
      service_client_->GetServiceState(Service::kSafeBrowsingExtendedReporting),
      ServiceState::kDisabled);

  // Consent is not revoked for the following services.
  EXPECT_EQ(service_client_->GetServiceState(Service::kAlternateErrorPages),
            ServiceState::kEnabled);
  EXPECT_EQ(service_client_->GetServiceState(Service::kMetricsReporting),
            ServiceState::kEnabled);
  EXPECT_EQ(service_client_->GetServiceState(Service::kNetworkPrediction),
            ServiceState::kEnabled);
  EXPECT_EQ(service_client_->GetServiceState(Service::kSearchSuggest),
            ServiceState::kEnabled);
  EXPECT_EQ(service_client_->GetServiceState(Service::kSafeBrowsing),
            ServiceState::kEnabled);
}

TEST_F(UnifiedConsentServiceTest, Migration_NotSignedIn) {
  base::HistogramTester histogram_tester;

  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));

  CreateConsentService();
  // Since there were not inconsistencies, the migration is completed after the
  // creation of the consent service.
  EXPECT_EQ(GetMigrationState(), unified_consent::MigrationState::kCompleted);
  // The suppress reason for not showing the consent bump should be recorded.
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.ConsentBump.SuppressReason",
      unified_consent::ConsentBumpSuppressReason::kNotSignedIn, 1);
}
#endif  // !defined(OS_CHROMEOS)

TEST_F(UnifiedConsentServiceTest, Rollback_WasSyncingEverything) {
  identity_test_environment_.SetPrimaryAccount("testaccount");
  syncer::SyncPrefs sync_prefs(&pref_service_);
  sync_service_.OnUserChoseDatatypes(true, syncer::UserSelectableTypes());
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());

  // Migrate
  CreateConsentService(true /* client services on by default */);
  // Check expectations after migration.
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_EQ(unified_consent::MigrationState::kCompleted, GetMigrationState());
  EXPECT_TRUE(consent_service_->ShouldShowConsentBump());

  consent_service_->Shutdown();
  consent_service_.reset();
  SetUnifiedConsentFeatureState(UnifiedConsentFeatureState::kDisabled);

  // Rollback
  UnifiedConsentService::RollbackIfNeeded(&pref_service_, &sync_service_);
  // Unified consent prefs should be cleared.
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_EQ(unified_consent::MigrationState::kNotInitialized,
            GetMigrationState());
  // Sync everything should be back on.
  EXPECT_TRUE(sync_prefs.HasKeepEverythingSynced());

  // Run until idle so the RollbackHelper is deleted.
  base::RunLoop().RunUntilIdle();
}

TEST_F(UnifiedConsentServiceTest, Rollback_WasNotSyncingEverything) {
  identity_test_environment_.SetPrimaryAccount("testaccount");
  syncer::SyncPrefs sync_prefs(&pref_service_);
  syncer::ModelTypeSet chosen_data_types = syncer::UserSelectableTypes();
  chosen_data_types.Remove(syncer::BOOKMARKS);
  sync_service_.OnUserChoseDatatypes(false, chosen_data_types);
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(sync_service_.GetPreferredDataTypes().HasAll(
      syncer::UserSelectableTypes()));

  // Migrate
  CreateConsentService();
  // Check expectations after migration.
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_EQ(unified_consent::MigrationState::kCompleted, GetMigrationState());

  consent_service_->Shutdown();
  consent_service_.reset();

  // Rollback
  UnifiedConsentService::RollbackIfNeeded(&pref_service_, &sync_service_);
  // Unified consent prefs should be cleared.
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kUnifiedConsentGiven));
  EXPECT_EQ(unified_consent::MigrationState::kNotInitialized,
            GetMigrationState());

  // Sync everything should be off because not all user types were on.
  EXPECT_FALSE(sync_prefs.HasKeepEverythingSynced());

  // Run until idle so the RollbackHelper is deleted.
  base::RunLoop().RunUntilIdle();
}

TEST_F(UnifiedConsentServiceTest, SettingsHistogram_None) {
  base::HistogramTester histogram_tester;
  // Disable all services.
  sync_service_.OnUserChoseDatatypes(false, syncer::ModelTypeSet());
  CreateConsentService();

  histogram_tester.ExpectUniqueSample(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kNone, 1);
}

TEST_F(UnifiedConsentServiceTest, SettingsHistogram_UnifiedConsentGiven) {
  base::HistogramTester histogram_tester;
  // Unified consent is given.
  identity_test_environment_.SetPrimaryAccount("testaccount");
  pref_service_.SetInteger(
      prefs::kUnifiedConsentMigrationState,
      static_cast<int>(unified_consent::MigrationState::kCompleted));
  pref_service_.SetBoolean(prefs::kUnifiedConsentGiven, true);
  CreateConsentService(true);

  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kNone, 0);
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kUnifiedConsentGiven, 1);
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kUserEvents, 1);
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kUrlKeyedAnonymizedDataCollection, 1);
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kSafeBrowsingExtendedReporting, 1);
  histogram_tester.ExpectBucketCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kSpellCheck, 1);
  histogram_tester.ExpectTotalCount(
      "UnifiedConsent.SyncAndGoogleServicesSettings", 5);
}

TEST_F(UnifiedConsentServiceTest, SettingsHistogram_NoUnifiedConsentGiven) {
  base::HistogramTester histogram_tester;
  // Unified consent is not given. Only spellcheck is enabled.
  pref_service_.SetBoolean(kSpellCheckDummyEnabled, true);
  CreateConsentService();

  // kUserEvents should have no sample even though the sync preference is set,
  // because the user is not signed in.
  histogram_tester.ExpectUniqueSample(
      "UnifiedConsent.SyncAndGoogleServicesSettings",
      SettingsHistogramValue::kSpellCheck, 1);
}

}  // namespace unified_consent
