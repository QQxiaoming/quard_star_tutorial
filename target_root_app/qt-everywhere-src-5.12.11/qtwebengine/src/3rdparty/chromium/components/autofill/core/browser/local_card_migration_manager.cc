// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/local_card_migration_manager.h"

#include <stddef.h>

#include <algorithm>
#include <vector>

#include "base/bind.h"
#include "components/autofill/core/browser/autofill_client.h"
#include "components/autofill/core/browser/autofill_experiments.h"
#include "components/autofill/core/browser/autofill_metrics.h"
#include "components/autofill/core/browser/credit_card.h"
#include "components/autofill/core/browser/form_data_importer.h"
#include "components/autofill/core/browser/payments/payments_client.h"
#include "components/autofill/core/browser/personal_data_manager.h"
#include "services/identity/public/cpp/identity_manager.h"

namespace autofill {

LocalCardMigrationManager::LocalCardMigrationManager(
    AutofillClient* client,
    payments::PaymentsClient* payments_client,
    const std::string& app_locale,
    PersonalDataManager* personal_data_manager)
    : client_(client),
      payments_client_(payments_client),
      app_locale_(app_locale),
      personal_data_manager_(personal_data_manager) {
  if (payments_client_)
    payments_client_->SetSaveDelegate(this);
}

LocalCardMigrationManager::~LocalCardMigrationManager() {}

bool LocalCardMigrationManager::ShouldOfferLocalCardMigration(
    int imported_credit_card_record_type) {
  // Must be an existing card. New cards always get Upstream or local save.
  if (imported_credit_card_record_type !=
          FormDataImporter::ImportedCreditCardRecordType::LOCAL_CARD &&
      imported_credit_card_record_type !=
          FormDataImporter::ImportedCreditCardRecordType::SERVER_CARD) {
    return false;
  }

  if (!IsCreditCardMigrationEnabled())
    return false;

  std::vector<CreditCard*> local_credit_cards =
      personal_data_manager_->GetLocalCreditCards();

  // Empty previous state.
  migratable_credit_cards_.clear();

  // Initialize the local credit card list and queue for showing and uploading.
  for (CreditCard* card : local_credit_cards) {
    // If the card is valid (has a valid card number, expiration date, and is
    // not expired) and is not a server card, add it to the list of migratable
    // cards.
    if (card->IsValid() && !IsServerCard(card))
      migratable_credit_cards_.push_back(*card);
  }

  // If the form was submitted with a local card, only offer migration instead
  // of Upstream if there are other local cards to migrate as well. If the form
  // was submitted with a server card, offer migration if ANY local cards can be
  // migrated.
  return (imported_credit_card_record_type ==
              FormDataImporter::ImportedCreditCardRecordType::LOCAL_CARD &&
          migratable_credit_cards_.size() > 1) ||
         (imported_credit_card_record_type ==
              FormDataImporter::ImportedCreditCardRecordType::SERVER_CARD &&
          !migratable_credit_cards_.empty());
}

void LocalCardMigrationManager::AttemptToOfferLocalCardMigration() {
  // Abort the migration if |payments_client_| is nullptr.
  if (!payments_client_)
    return;
  payments_client_->SetSaveDelegate(this);
  upload_request_ = payments::PaymentsClient::UploadRequestDetails();

  // Payments server determines which version of the legal message to show based
  // on the existence of this experiment flag.
  if (IsAutofillUpstreamUpdatePromptExplanationExperimentEnabled()) {
    upload_request_.active_experiments.push_back(
        kAutofillUpstreamUpdatePromptExplanation.name);
  }

  // Don't send pan_first_six, as potentially migrating multiple local cards at
  // once will negate its usefulness.
  payments_client_->GetUploadDetails(
      upload_request_.profiles, GetDetectedValues(),
      /*pan_first_six=*/std::string(), upload_request_.active_experiments,
      app_locale_);
}

bool LocalCardMigrationManager::IsCreditCardMigrationEnabled() {
  // Confirm that the user is signed in, syncing, and the proper experiment
  // flags are enabled.
  bool migration_experiment_enabled =
      IsAutofillCreditCardLocalCardMigrationExperimentEnabled();
  bool credit_card_upload_enabled = ::autofill::IsCreditCardUploadEnabled(
      client_->GetPrefs(), client_->GetSyncService(),
      client_->GetIdentityManager()->GetPrimaryAccountInfo().email);
  bool has_google_payments_account =
      (static_cast<int64_t>(payments_client_->GetPrefService()->GetDouble(
           prefs::kAutofillBillingCustomerNumber)) != 0);
  return migration_experiment_enabled && credit_card_upload_enabled &&
         has_google_payments_account;
}

void LocalCardMigrationManager::OnDidGetUploadDetails(
    AutofillClient::PaymentsRpcResult result,
    const base::string16& context_token,
    std::unique_ptr<base::DictionaryValue> legal_message) {
  if (result == AutofillClient::SUCCESS) {
    upload_request_.context_token = context_token;
    legal_message_ = std::move(legal_message);
    // If we successfully received the legal docs, trigger the offer-to-migrate
    // dialog.
    // TODO(crbug.com/852904): Show intermediate migration prompt here! Relies
    // on CL/1117929 first.
  }
}

// TODO(crbug.com/852904): Starts the upload of the next local card if one
// exists.
void LocalCardMigrationManager::OnDidUploadCard(
    AutofillClient::PaymentsRpcResult result,
    const std::string& server_id) {}

int LocalCardMigrationManager::GetDetectedValues() const {
  int detected_values = 0;

  // If all cards to be migrated have a cardholder name, include it in the
  // detected values.
  bool all_cards_have_cardholder_name = true;
  for (CreditCard card : migratable_credit_cards_) {
    all_cards_have_cardholder_name &=
        !card.GetInfo(AutofillType(CREDIT_CARD_NAME_FULL), app_locale_).empty();
  }
  if (all_cards_have_cardholder_name)
    detected_values |= CreditCardSaveManager::DetectedValue::CARDHOLDER_NAME;

  // Local card migration should ONLY be offered when the user already has a
  // Google Payments account.
  DCHECK(static_cast<int64_t>(payments_client_->GetPrefService()->GetDouble(
             prefs::kAutofillBillingCustomerNumber)) != 0);
  detected_values |=
      CreditCardSaveManager::DetectedValue::HAS_GOOGLE_PAYMENTS_ACCOUNT;

  return detected_values;
}

bool LocalCardMigrationManager::IsServerCard(CreditCard* local_card) const {
  std::vector<CreditCard*> server_credit_cards =
      personal_data_manager_->GetServerCreditCards();
  // Check whether the current card is already uploaded.
  for (const CreditCard* server_card : server_credit_cards) {
    if (local_card->HasSameNumberAs(*server_card))
      return true;
  }
  return false;
}

}  // namespace autofill
