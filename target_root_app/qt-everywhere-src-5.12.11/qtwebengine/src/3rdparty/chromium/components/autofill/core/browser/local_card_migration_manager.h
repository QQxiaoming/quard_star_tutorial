// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_LOCAL_CARD_MIGRATION_MANAGER_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_LOCAL_CARD_MIGRATION_MANAGER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "components/autofill/core/browser/autofill_client.h"
#include "components/autofill/core/browser/payments/payments_client.h"

namespace autofill {

class CreditCard;
class PersonalDataManager;

// Manages logic for determining whether migration of locally saved credit cards
// to Google Payments is available as well as multiple local card uploading.
// Owned by FormDataImporter.
class LocalCardMigrationManager : public payments::PaymentsClientSaveDelegate {
 public:
  // The parameters should outlive the LocalCardMigrationManager.
  LocalCardMigrationManager(AutofillClient* client,
                            payments::PaymentsClient* payments_client,
                            const std::string& app_locale,
                            PersonalDataManager* personal_data_manager);
  virtual ~LocalCardMigrationManager();

  // Returns true if all of the conditions for allowing local credit card
  // migration are satisfied. Initializes the local card list for upload.
  bool ShouldOfferLocalCardMigration(int imported_credit_card_record_type);

  // Called from FormDataImporter when all migration requirements are met.
  // Fetches legal documents and triggers the OnDidGetUploadDetails callback.
  void AttemptToOfferLocalCardMigration();

  // Check that the user is signed in, syncing, and the proper experiment
  // flags are enabled. Override in the test class.
  virtual bool IsCreditCardMigrationEnabled();

  // Determines what detected_values metadata to send (generally, cardholder
  // name if it exists on all cards, and existence of Payments customer).
  int GetDetectedValues() const;

 protected:
  // payments::PaymentsClientSaveDelegate:
  // Callback after successfully getting the legal documents. On success,
  // displays the offer-to-migrate dialog, which the user can accept or not.
  void OnDidGetUploadDetails(
      AutofillClient::PaymentsRpcResult result,
      const base::string16& context_token,
      std::unique_ptr<base::DictionaryValue> legal_message) override;

  // payments::PaymentsClientSaveDelegate:
  // Callback after a local card was uploaded. Starts the upload of the next
  // local card if one exists.
  // Exposed for testing.
  void OnDidUploadCard(AutofillClient::PaymentsRpcResult result,
                       const std::string& server_id) override;

  // Check whether a local card is already a server card.
  bool IsServerCard(CreditCard* local_card) const;

  AutofillClient* const client_;

  // Handles Payments service requests.
  // Owned by AutofillManager.
  payments::PaymentsClient* payments_client_;

 private:
  std::unique_ptr<base::DictionaryValue> legal_message_;

  std::string app_locale_;

  // The personal data manager, used to save and load personal data to/from the
  // web database.  This is overridden by the AutofillManagerTest.
  // Weak reference.
  // May be NULL.  NULL indicates OTR.
  PersonalDataManager* personal_data_manager_;

  // Collected information about a pending upload request.
  payments::PaymentsClient::UploadRequestDetails upload_request_;

  // The local credit cards to be uploaded.
  std::vector<CreditCard> migratable_credit_cards_;

  DISALLOW_COPY_AND_ASSIGN(LocalCardMigrationManager);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_LOCAL_CARD_MIGRATION_MANAGER_H_
