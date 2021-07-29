// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/unified_consent/unified_consent_service.h"

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/scoped_observer.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "components/sync/base/model_type.h"
#include "components/sync/base/sync_prefs.h"
#include "components/sync/driver/sync_service.h"
#include "components/unified_consent/pref_names.h"
#include "components/unified_consent/unified_consent_service_client.h"

namespace unified_consent {

namespace {

// Histogram recorded at startup to log which Google services are enabled.
const char kSyncAndGoogleServicesSettingsHistogram[] =
    "UnifiedConsent.SyncAndGoogleServicesSettings";

// Records a sample in the kSyncAndGoogleServicesSettingsHistogram. Wrapped in a
// function to avoid code size issues caused by histogram macros.
void RecordSettingsHistogramSample(SettingsHistogramValue value) {
  UMA_HISTOGRAM_ENUMERATION(kSyncAndGoogleServicesSettingsHistogram, value);
}

// Checks if a pref is enabled and if so, records a sample in the
// kSyncAndGoogleServicesSettingsHistogram. Returns true if a sample was
// recorded.
bool RecordSettingsHistogramFromPref(const char* pref_name,
                                     PrefService* pref_service,
                                     SettingsHistogramValue value) {
  if (!pref_service->GetBoolean(pref_name))
    return false;
  RecordSettingsHistogramSample(value);
  return true;
}

// Checks if a service is enabled and if so, records a sample in the
// kSyncAndGoogleServicesSettingsHistogram. Returns true if a sample was
// recorded.
bool RecordSettingsHistogramFromService(
    UnifiedConsentServiceClient* client,
    UnifiedConsentServiceClient::Service service,
    SettingsHistogramValue value) {
  if (client->GetServiceState(service) !=
      UnifiedConsentServiceClient::ServiceState::kEnabled) {
    return false;
  }

  RecordSettingsHistogramSample(value);
  return true;
}

// Used for observing the sync service and finishing the rollback once the sync
// engine is initialized.
// Note: This object is suicidal - it will kill itself after it finishes the
// rollback.
class RollbackHelper : public syncer::SyncServiceObserver {
 public:
  explicit RollbackHelper(syncer::SyncService* sync_service);
  ~RollbackHelper() override = default;

 private:
  // syncer::SyncServiceObserver:
  void OnStateChanged(syncer::SyncService* sync_service) override;

  void DoRollbackIfPossibleAndDie(syncer::SyncService* sync_service);

  ScopedObserver<syncer::SyncService, RollbackHelper> scoped_sync_observer_;
};

RollbackHelper::RollbackHelper(syncer::SyncService* sync_service)
    : scoped_sync_observer_(this) {
  if (sync_service->IsEngineInitialized())
    DoRollbackIfPossibleAndDie(sync_service);
  else
    scoped_sync_observer_.Add(sync_service);
}

void RollbackHelper::OnStateChanged(syncer::SyncService* sync_service) {
  if (!sync_service->IsEngineInitialized())
    return;

  scoped_sync_observer_.RemoveAll();
  DoRollbackIfPossibleAndDie(sync_service);
}

void RollbackHelper::DoRollbackIfPossibleAndDie(
    syncer::SyncService* sync_service) {
  DCHECK(!scoped_sync_observer_.IsObservingSources());

  syncer::ModelTypeSet user_types_without_user_events =
      syncer::UserSelectableTypes();
  user_types_without_user_events.Remove(syncer::USER_EVENTS);

  if (sync_service->GetPreferredDataTypes().HasAll(
          user_types_without_user_events)) {
    // As part of the migration of a profile to Unified Consent, sync everything
    // is disabled but sync continues to be enabled for all data types except
    // USER_EVENTS. Therefore it is desired to restore sync everything when
    // rolling back unified consent to leave sync in the same state as the one
    // before migration.
    sync_service->OnUserChoseDatatypes(true, syncer::UserSelectableTypes());
  }

  base::SequencedTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE, this);
}

}  // namespace

UnifiedConsentService::UnifiedConsentService(
    std::unique_ptr<UnifiedConsentServiceClient> service_client,
    PrefService* pref_service,
    identity::IdentityManager* identity_manager,
    syncer::SyncService* sync_service)
    : service_client_(std::move(service_client)),
      pref_service_(pref_service),
      identity_manager_(identity_manager),
      sync_service_(sync_service) {
  DCHECK(service_client_);
  DCHECK(pref_service_);
  DCHECK(identity_manager_);
  DCHECK(sync_service_);

  if (GetMigrationState() == MigrationState::kNotInitialized)
    MigrateProfileToUnifiedConsent();

  service_client_->AddObserver(this);
  identity_manager_->AddObserver(this);
  sync_service_->AddObserver(this);

  pref_change_registrar_ = std::make_unique<PrefChangeRegistrar>();
  pref_change_registrar_->Init(pref_service);
  pref_change_registrar_->Add(
      prefs::kUnifiedConsentGiven,
      base::BindRepeating(
          &UnifiedConsentService::OnUnifiedConsentGivenPrefChanged,
          base::Unretained(this)));

  // If somebody disabled any of the non-personalized services while Chrome
  // wasn't running, disable unified consent.
  if (!AreAllNonPersonalizedServicesEnabled() && IsUnifiedConsentGiven()) {
    SetUnifiedConsentGiven(false);
  }

  RecordSettingsHistogram();
}

UnifiedConsentService::~UnifiedConsentService() {}

// static
void UnifiedConsentService::RegisterPrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                                false);
  registry->RegisterBooleanPref(prefs::kUnifiedConsentGiven, false);
  registry->RegisterIntegerPref(
      prefs::kUnifiedConsentMigrationState,
      static_cast<int>(MigrationState::kNotInitialized));
  registry->RegisterBooleanPref(prefs::kShouldShowUnifiedConsentBump, false);
}

// static
void UnifiedConsentService::RollbackIfNeeded(
    PrefService* user_pref_service,
    syncer::SyncService* sync_service) {
  DCHECK(user_pref_service);

  if (user_pref_service->GetInteger(prefs::kUnifiedConsentMigrationState) ==
      static_cast<int>(MigrationState::kNotInitialized)) {
    // If there was no migration yet, nothing has to be rolled back.
    return;
  }

  if (user_pref_service->GetBoolean(prefs::kShouldShowUnifiedConsentBump) &&
      sync_service &&
      sync_service->GetDisableReasons() ==
          syncer::SyncService::DISABLE_REASON_NONE) {
    // This will wait until the sync engine is initialized and then enables the
    // sync-everything pref in case the user is syncing all data types.
    new RollbackHelper(sync_service);
  }

  // Clear all unified consent prefs.
  user_pref_service->ClearPref(prefs::kUrlKeyedAnonymizedDataCollectionEnabled);
  user_pref_service->ClearPref(prefs::kUnifiedConsentGiven);
  user_pref_service->ClearPref(prefs::kUnifiedConsentMigrationState);
  user_pref_service->ClearPref(prefs::kShouldShowUnifiedConsentBump);
}

void UnifiedConsentService::SetUnifiedConsentGiven(bool unified_consent_given) {
  // Unified consent cannot be enabled if the user is not signed in.
  DCHECK(!unified_consent_given || identity_manager_->HasPrimaryAccount());
  pref_service_->SetBoolean(prefs::kUnifiedConsentGiven, unified_consent_given);
}

bool UnifiedConsentService::IsUnifiedConsentGiven() {
  return pref_service_->GetBoolean(prefs::kUnifiedConsentGiven);
}

bool UnifiedConsentService::ShouldShowConsentBump() {
  return pref_service_->GetBoolean(prefs::kShouldShowUnifiedConsentBump);
}

void UnifiedConsentService::MarkConsentBumpShown() {
  // Record suppress reason kNone, which means that it was shown. This also sets
  // the |kShouldShowConsentBump| pref to false.
  RecordConsentBumpSuppressReason(ConsentBumpSuppressReason::kNone);
}

void UnifiedConsentService::RecordConsentBumpSuppressReason(
    ConsentBumpSuppressReason suppress_reason) {
  UMA_HISTOGRAM_ENUMERATION("UnifiedConsent.ConsentBump.SuppressReason",
                            suppress_reason);

  switch (suppress_reason) {
    case ConsentBumpSuppressReason::kNone:
    case ConsentBumpSuppressReason::kNotSignedIn:
    case ConsentBumpSuppressReason::kSyncEverythingOff:
    case ConsentBumpSuppressReason::kPrivacySettingOff:
    case ConsentBumpSuppressReason::kSettingsOptIn:
    case ConsentBumpSuppressReason::kUserSignedOut:
      pref_service_->SetBoolean(prefs::kShouldShowUnifiedConsentBump, false);
      break;
    case ConsentBumpSuppressReason::kSyncPaused:
      // Consent bump should be shown when sync is active again.
      DCHECK(ShouldShowConsentBump());
      break;
  }
}

void UnifiedConsentService::Shutdown() {
  service_client_->RemoveObserver(this);
  identity_manager_->RemoveObserver(this);
  sync_service_->RemoveObserver(this);
}

void UnifiedConsentService::OnServiceStateChanged(Service service) {
  // Unified consent is disabled when any of its dependent services gets
  // disabled.
  if (service_client_->GetServiceState(service) == ServiceState::kDisabled)
    SetUnifiedConsentGiven(false);
}

void UnifiedConsentService::OnPrimaryAccountCleared(
    const AccountInfo& account_info) {
  // When signing out, the unfied consent is revoked.
  pref_service_->SetBoolean(prefs::kUnifiedConsentGiven, false);

  // By design, signing out of Chrome automatically disables off-by-default
  // services.
  pref_service_->SetBoolean(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                            false);
  service_client_->SetServiceEnabled(Service::kSafeBrowsingExtendedReporting,
                                     false);
  service_client_->SetServiceEnabled(Service::kSpellCheck, false);

  if (GetMigrationState() != MigrationState::kCompleted) {
    // When the user signs out, the migration is complete.
    SetMigrationState(MigrationState::kCompleted);
  }

  if (ShouldShowConsentBump())
    RecordConsentBumpSuppressReason(ConsentBumpSuppressReason::kUserSignedOut);
}

void UnifiedConsentService::OnStateChanged(syncer::SyncService* sync) {
  if (sync_service_->GetDisableReasons() !=
          syncer::SyncService::DISABLE_REASON_NONE ||
      !sync_service_->IsEngineInitialized()) {
    return;
  }

  if (GetMigrationState() == MigrationState::kInProgressWaitForSyncInit)
    UpdateSettingsForMigration();

  if (sync_service_->IsUsingSecondaryPassphrase() && IsUnifiedConsentGiven()) {
    // Force off unified consent given when the user sets a custom passphrase.
    SetUnifiedConsentGiven(false);
  }

  syncer::SyncPrefs sync_prefs(pref_service_);
  if (IsUnifiedConsentGiven() != sync_prefs.HasKeepEverythingSynced()) {
    // Make sync-everything consistent with the |kUnifiedConsentGiven| pref.
    SetSyncEverythingIfPossible(IsUnifiedConsentGiven());
  }
}

void UnifiedConsentService::OnUnifiedConsentGivenPrefChanged() {
  bool enabled = pref_service_->GetBoolean(prefs::kUnifiedConsentGiven);

  if (!enabled) {
    if (identity_manager_->HasPrimaryAccount()) {
      // Sync-everything is set to false, so the user can select individual
      // sync data types.
      SetSyncEverythingIfPossible(false);
    }
    return;
  }

  DCHECK(sync_service_->IsSyncAllowed());
  DCHECK(identity_manager_->HasPrimaryAccount());
  DCHECK_LT(MigrationState::kNotInitialized, GetMigrationState());

  if (GetMigrationState() != MigrationState::kCompleted) {
    // If the user opted into unified consent, the migration is completed.
    SetMigrationState(MigrationState::kCompleted);
  }

  if (ShouldShowConsentBump())
    RecordConsentBumpSuppressReason(ConsentBumpSuppressReason::kSettingsOptIn);

  // Enable all sync data types if possible, otherwise they will be enabled with
  // |OnStateChanged| once sync is active;
  SetSyncEverythingIfPossible(true);

  // Enable all non-personalized services.
  pref_service_->SetBoolean(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                            true);
  // Inform client to enable non-personalized services.
  for (int i = 0; i <= static_cast<int>(Service::kLast); ++i) {
    Service service = static_cast<Service>(i);
    if (service_client_->GetServiceState(service) !=
        ServiceState::kNotSupported) {
      service_client_->SetServiceEnabled(service, true);
    }
  }
}

void UnifiedConsentService::SetSyncEverythingIfPossible(bool sync_everything) {
  syncer::SyncPrefs sync_prefs(pref_service_);
  if (sync_everything == sync_prefs.HasKeepEverythingSynced())
    return;

  if (!IsSyncConfigurable())
    return;

  if (sync_everything) {
    pref_service_->SetBoolean(autofill::prefs::kAutofillWalletImportEnabled,
                              true);
    sync_service_->OnUserChoseDatatypes(sync_everything,
                                        syncer::UserSelectableTypes());
  } else {
    syncer::ModelTypeSet preferred = sync_service_->GetPreferredDataTypes();
    preferred.RetainAll(syncer::UserSelectableTypes());
    sync_service_->OnUserChoseDatatypes(false, preferred);
  }
}

MigrationState UnifiedConsentService::GetMigrationState() {
  int migration_state_int =
      pref_service_->GetInteger(prefs::kUnifiedConsentMigrationState);
  DCHECK_LE(static_cast<int>(MigrationState::kNotInitialized),
            migration_state_int);
  DCHECK_GE(static_cast<int>(MigrationState::kCompleted), migration_state_int);
  return static_cast<MigrationState>(migration_state_int);
}

void UnifiedConsentService::SetMigrationState(MigrationState migration_state) {
  pref_service_->SetInteger(prefs::kUnifiedConsentMigrationState,
                            static_cast<int>(migration_state));
}

void UnifiedConsentService::MigrateProfileToUnifiedConsent() {
  DCHECK_EQ(GetMigrationState(), MigrationState::kNotInitialized);
  DCHECK(!IsUnifiedConsentGiven());

  if (!identity_manager_->HasPrimaryAccount()) {
    RecordConsentBumpSuppressReason(ConsentBumpSuppressReason::kNotSignedIn);
    SetMigrationState(MigrationState::kCompleted);
    return;
  }

  if (!syncer::SyncPrefs(pref_service_).HasKeepEverythingSynced()) {
    RecordConsentBumpSuppressReason(
        ConsentBumpSuppressReason::kSyncEverythingOff);
  } else if (!AreAllOnByDefaultPrivacySettingsOn()) {
    RecordConsentBumpSuppressReason(
        ConsentBumpSuppressReason::kPrivacySettingOff);
  } else {
    // When the user was syncing everything, and all on-by-default privacy
    // settings were on, the consent bump should be shown.
    pref_service_->SetBoolean(prefs::kShouldShowUnifiedConsentBump, true);
  }

  UpdateSettingsForMigration();
}

void UnifiedConsentService::UpdateSettingsForMigration() {
  if (!IsSyncConfigurable()) {
    SetMigrationState(MigrationState::kInProgressWaitForSyncInit);
    return;
  }

  if (IsUnifiedConsentGiven()) {
    // When the user opted into unified consent through the consent bump or the
    // settings page while waiting for sync initialization, the migration is
    // completed.
    SetMigrationState(MigrationState::kCompleted);
    return;
  }

  // Set URL-keyed anonymized metrics to the state it had before unified
  // consent.
  bool url_keyed_metrics_enabled = sync_service_->GetPreferredDataTypes().Has(
                                       syncer::HISTORY_DELETE_DIRECTIVES) &&
                                   !sync_service_->IsUsingSecondaryPassphrase();
  pref_service_->SetBoolean(prefs::kUrlKeyedAnonymizedDataCollectionEnabled,
                            url_keyed_metrics_enabled);

  // Disable the datatype user events for newly migrated users. Also set
  // sync-everything to false, so it matches unified consent given.
  syncer::ModelTypeSet preferred_types_without_user_events =
      sync_service_->GetPreferredDataTypes();
  preferred_types_without_user_events.RetainAll(syncer::UserSelectableTypes());
  preferred_types_without_user_events.Remove(syncer::USER_EVENTS);
  sync_service_->OnUserChoseDatatypes(false /*sync everything */,
                                      preferred_types_without_user_events);

  SetMigrationState(MigrationState::kCompleted);
}

bool UnifiedConsentService::AreAllNonPersonalizedServicesEnabled() {
  for (int i = 0; i <= static_cast<int>(Service::kLast); ++i) {
    Service service = static_cast<Service>(i);
    if (service_client_->GetServiceState(service) == ServiceState::kDisabled)
      return false;
  }
  if (!pref_service_->GetBoolean(
          prefs::kUrlKeyedAnonymizedDataCollectionEnabled))
    return false;

  return true;
}

bool UnifiedConsentService::AreAllOnByDefaultPrivacySettingsOn() {
  for (auto service : {Service::kAlternateErrorPages,
                       Service::kMetricsReporting, Service::kNetworkPrediction,
                       Service::kSafeBrowsing, Service::kSearchSuggest}) {
    if (service_client_->GetServiceState(service) == ServiceState::kDisabled)
      return false;
  }
  return true;
}

bool UnifiedConsentService::IsSyncConfigurable() {
  return sync_service_->GetState() == syncer::SyncService::State::ACTIVE;
}

void UnifiedConsentService::RecordSettingsHistogram() {
  bool metric_recorded = false;

  if (IsUnifiedConsentGiven()) {
    RecordSettingsHistogramSample(SettingsHistogramValue::kUnifiedConsentGiven);
    metric_recorded = true;
  }
  if (identity_manager_->HasPrimaryAccount() &&
      sync_service_->GetPreferredDataTypes().Has(syncer::USER_EVENTS)) {
    RecordSettingsHistogramSample(SettingsHistogramValue::kUserEvents);
    metric_recorded = true;
  }
  metric_recorded |= RecordSettingsHistogramFromPref(
      prefs::kUrlKeyedAnonymizedDataCollectionEnabled, pref_service_,
      SettingsHistogramValue::kUrlKeyedAnonymizedDataCollection);
  metric_recorded |= RecordSettingsHistogramFromService(
      service_client_.get(),
      UnifiedConsentServiceClient::Service::kSafeBrowsingExtendedReporting,
      SettingsHistogramValue::kSafeBrowsingExtendedReporting);
  metric_recorded |= RecordSettingsHistogramFromService(
      service_client_.get(), UnifiedConsentServiceClient::Service::kSpellCheck,
      SettingsHistogramValue::kSpellCheck);

  if (!metric_recorded)
    RecordSettingsHistogramSample(SettingsHistogramValue::kNone);
}

}  //  namespace unified_consent
