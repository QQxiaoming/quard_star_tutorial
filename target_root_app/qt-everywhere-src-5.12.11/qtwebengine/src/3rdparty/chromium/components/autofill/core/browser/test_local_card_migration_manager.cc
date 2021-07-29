// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/test_local_card_migration_manager.h"

#include "components/autofill/core/browser/autofill_experiments.h"
#include "components/autofill/core/browser/autofill_metrics.h"
#include "components/autofill/core/browser/payments/test_payments_client.h"
#include "services/identity/public/cpp/identity_manager.h"

namespace autofill {

TestLocalCardMigrationManager::TestLocalCardMigrationManager(
    AutofillDriver* driver,
    AutofillClient* client,
    payments::TestPaymentsClient* payments_client,
    PersonalDataManager* personal_data_manager)
    : LocalCardMigrationManager(client,
                                payments_client,
                                "en-US",
                                personal_data_manager),
      test_payments_client_(payments_client) {
  if (test_payments_client_)
    test_payments_client_->SetSaveDelegate(this);
}

TestLocalCardMigrationManager::~TestLocalCardMigrationManager() {}

bool TestLocalCardMigrationManager::IsCreditCardMigrationEnabled() {
  bool migration_experiment_enabled =
      IsAutofillCreditCardLocalCardMigrationExperimentEnabled();
  bool has_google_payments_account =
      (static_cast<int64_t>(payments_client_->GetPrefService()->GetDouble(
           prefs::kAutofillBillingCustomerNumber)) != 0);
  return migration_experiment_enabled && has_google_payments_account;
}

bool TestLocalCardMigrationManager::LocalCardMigrationWasTriggered() {
  return local_card_migration_was_triggered_;
}

void TestLocalCardMigrationManager::OnDidGetUploadDetails(
    AutofillClient::PaymentsRpcResult result,
    const base::string16& context_token,
    std::unique_ptr<base::DictionaryValue> legal_message) {
  if (result == AutofillClient::SUCCESS) {
    local_card_migration_was_triggered_ = true;
    LocalCardMigrationManager::OnDidGetUploadDetails(result, context_token,
                                                     std::move(legal_message));
  }
}

}  // namespace autofill
