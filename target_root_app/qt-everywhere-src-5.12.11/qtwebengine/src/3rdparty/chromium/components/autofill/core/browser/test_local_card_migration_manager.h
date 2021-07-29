// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_TEST_LOCAL_CARD_MIGRATION_MANAGER_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_TEST_LOCAL_CARD_MIGRATION_MANAGER_H_

#include <string>

#include "components/autofill/core/browser/local_card_migration_manager.h"

namespace autofill {

namespace payments {
class TestPaymentsClient;
}  // namespace payments

class AutofillClient;
class AutofillDriver;
class PersonalDataManager;

class TestLocalCardMigrationManager : public LocalCardMigrationManager {
 public:
  TestLocalCardMigrationManager(AutofillDriver* driver,
                                AutofillClient* client,
                                payments::TestPaymentsClient* payments_client,
                                PersonalDataManager* personal_data_manager);
  ~TestLocalCardMigrationManager() override;

  // Override the base function. Checks the existnece of billing customer number
  // and the experiment flag, but unlike the real class, does not check if the
  // user is signed in/syncing.
  bool IsCreditCardMigrationEnabled() override;

  // Returns whether the first round migration pop-up window was triggered.
  bool LocalCardMigrationWasTriggered();

 private:
  void OnDidGetUploadDetails(
      AutofillClient::PaymentsRpcResult result,
      const base::string16& context_token,
      std::unique_ptr<base::DictionaryValue> legal_message) override;

  payments::TestPaymentsClient* test_payments_client_;  // Weak reference.
  bool local_card_migration_was_triggered_ = false;

  DISALLOW_COPY_AND_ASSIGN(TestLocalCardMigrationManager);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_TEST_LOCAL_CARD_MIGRATION_MANAGER_H_
