// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/local_card_migration_manager.h"

#include <stddef.h>

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/guid.h"
#include "base/metrics/metrics_hashes.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "components/autofill/core/browser/autofill_experiments.h"
#include "components/autofill/core/browser/autofill_metrics.h"
#include "components/autofill/core/browser/autofill_profile.h"
#include "components/autofill/core/browser/autofill_test_utils.h"
#include "components/autofill/core/browser/credit_card.h"
#include "components/autofill/core/browser/payments/test_payments_client.h"
#include "components/autofill/core/browser/personal_data_manager.h"
#include "components/autofill/core/browser/test_autofill_client.h"
#include "components/autofill/core/browser/test_autofill_clock.h"
#include "components/autofill/core/browser/test_autofill_driver.h"
#include "components/autofill/core/browser/test_autofill_manager.h"
#include "components/autofill/core/browser/test_credit_card_save_manager.h"
#include "components/autofill/core/browser/test_local_card_migration_manager.h"
#include "components/autofill/core/browser/test_personal_data_manager.h"
#include "components/autofill/core/browser/test_sync_service.h"
#include "components/autofill/core/browser/validation.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/autofill/core/common/autofill_clock.h"
#include "components/autofill/core/common/form_data.h"
#include "components/autofill/core/common/form_field_data.h"
#include "components/prefs/pref_service.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_test_util.h"
#include "services/metrics/public/cpp/ukm_builders.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

using base::ASCIIToUTF16;
using testing::_;

namespace autofill {

class LocalCardMigrationManagerTest : public testing::Test {
 public:
  void SetUp() override {
    autofill_client_.SetPrefs(test::PrefServiceForTesting());
    personal_data_.SetPrefService(autofill_client_.GetPrefs());
    personal_data_.SetSyncServiceForTest(&sync_service_);
    autofill_driver_.reset(new TestAutofillDriver());
    request_context_ = new net::TestURLRequestContextGetter(
        base::ThreadTaskRunnerHandle::Get());
    autofill_driver_->SetURLRequestContext(request_context_.get());
    payments_client_ = new payments::TestPaymentsClient(
        autofill_driver_->GetURLLoaderFactory(), autofill_client_.GetPrefs(),
        autofill_client_.GetIdentityManager(),
        /*unmask_delegate=*/nullptr,
        /*save_delegate=*/nullptr);
    credit_card_save_manager_ =
        new TestCreditCardSaveManager(autofill_driver_.get(), &autofill_client_,
                                      payments_client_, &personal_data_);
    local_card_migration_manager_ = new TestLocalCardMigrationManager(
        autofill_driver_.get(), &autofill_client_, payments_client_,
        &personal_data_);
    autofill_manager_.reset(new TestAutofillManager(
        autofill_driver_.get(), &autofill_client_, &personal_data_,
        std::unique_ptr<CreditCardSaveManager>(credit_card_save_manager_),
        payments_client_,
        std::unique_ptr<LocalCardMigrationManager>(
            local_card_migration_manager_)));
    autofill_manager_->SetExpectedObservedSubmission(true);
  }

  void TearDown() override {
    // Order of destruction is important as AutofillManager relies on
    // PersonalDataManager to be around when it gets destroyed.
    autofill_manager_.reset();
    autofill_driver_.reset();

    personal_data_.SetPrefService(nullptr);
    personal_data_.ClearCreditCards();

    request_context_ = nullptr;
  }

  void EnableAutofillCreditCardLocalCardMigrationExperiment() {
    scoped_feature_list_.InitAndEnableFeature(
        kAutofillCreditCardLocalCardMigration);
  }

  void DisableAutofillCreditCardLocalCardMigrationExperiment() {
    scoped_feature_list_.InitAndDisableFeature(
        kAutofillCreditCardLocalCardMigration);
  }

  void FormsSeen(const std::vector<FormData>& forms) {
    autofill_manager_->OnFormsSeen(forms, base::TimeTicks());
  }

  void FormSubmitted(const FormData& form) {
    autofill_manager_->OnFormSubmitted(
        form, false, SubmissionSource::FORM_SUBMISSION, base::TimeTicks::Now());
  }

  void EditCreditCardFrom(FormData& credit_card_form,
                          const char* name_on_card,
                          const char* card_number,
                          const char* expiration_month,
                          const char* expiration_year,
                          const char* cvc) {
    DCHECK(credit_card_form.fields.size() >= 5);
    credit_card_form.fields[0].value = ASCIIToUTF16(name_on_card);
    credit_card_form.fields[1].value = ASCIIToUTF16(card_number);
    credit_card_form.fields[2].value = ASCIIToUTF16(expiration_month);
    credit_card_form.fields[3].value = ASCIIToUTF16(expiration_year);
    credit_card_form.fields[4].value = ASCIIToUTF16(cvc);
  }

  void AddLocalCrediCard(TestPersonalDataManager& personal_data,
                         const char* name_on_card,
                         const char* card_number,
                         const char* expiration_month,
                         const char* expiration_year,
                         const std::string& billing_address_id) {
    CreditCard local_card;
    test::SetCreditCardInfo(&local_card, name_on_card, card_number,
                            expiration_month, expiration_year,
                            billing_address_id);
    local_card.set_record_type(CreditCard::LOCAL_CARD);
    personal_data.AddCreditCard(local_card);
  }

 protected:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  ukm::TestAutoSetUkmRecorder test_ukm_recorder_;
  TestAutofillClient autofill_client_;
  std::unique_ptr<TestAutofillDriver> autofill_driver_;
  std::unique_ptr<TestAutofillManager> autofill_manager_;
  scoped_refptr<net::TestURLRequestContextGetter> request_context_;
  TestPersonalDataManager personal_data_;
  TestSyncService sync_service_;
  base::test::ScopedFeatureList scoped_feature_list_;
  // Ends up getting owned (and destroyed) by TestFormDataImporter:
  TestCreditCardSaveManager* credit_card_save_manager_;
  TestLocalCardMigrationManager* local_card_migration_manager_;
  // Ends up getting owned (and destroyed) by TestAutofillManager:
  payments::TestPaymentsClient* payments_client_;
};

// Having one local card on file and using it will not trigger migration.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_UseLocalCardWithOneLocal) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Having any number of local cards on file and using a new card will not
// trigger migration.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_UseNewCardWithAnyLocal) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card (but it will not match what we will enter below).
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card (but it will not match what we will enter
  // below).
  AddLocalCrediCard(personal_data_, "Flo Master", "4444333322221111", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "5555555555554444", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Use one local card with more valid local cards available, will trigger
// migration.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_UseLocalCardWithMoreLocal) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card, so it will trigger migration.
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_TRUE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Using a local card will not trigger migration even if there are other local
// cards as long as the other local cards are not eligible for migration.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_UseLocalCardWithInvalidLocal) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add other invalid local credit cards(invalid card number or expired), so it
  // will not trigger migration.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111112", "11",
                    test::NextYear().c_str(), "1");
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::LastYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Using a server card when any number of local cards are eligible for migration
// will trigger migration.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_UseServerCardWithOneValidLocal) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a masked server credit card whose |TypeAndLastFourDigits| matches what
  // we will enter below.
  CreditCard credit_card(CreditCard::MASKED_SERVER_CARD, "a123");
  test::SetCreditCardInfo(&credit_card, "Flo Master", "1111", "11",
                          test::NextYear().c_str(), "1");
  credit_card.SetNetworkForMaskedCard(kVisaCard);
  personal_data_.AddServerCreditCard(credit_card);
  // Add one valid local credit card, so it will trigger migration
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_TRUE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Using a server card will not trigger migration even if there are other local
// cards as long as the other local cards are not eligible for migration.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_UseServerCardWithNoneValidLocal) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a masked credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  CreditCard credit_card(CreditCard::MASKED_SERVER_CARD, "a123");
  test::SetCreditCardInfo(&credit_card, "Flo Master", "1111", "11",
                          test::NextYear().c_str(), "1");
  credit_card.SetNetworkForMaskedCard(kVisaCard);
  personal_data_.AddServerCreditCard(credit_card);
  // Add other invalid local credit cards(invalid card number or expired), so it
  // will not trigger migration.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111112", "11",
                    test::NextYear().c_str(), "1");
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::LastYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Use one local card with more valid local cards available but experiment flag
// is off, will not trigger migration.
TEST_F(LocalCardMigrationManagerTest, MigrateCreditCard_FeatureNotEnabled) {
  // Turn off the experiment flag.
  DisableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card.
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Use one local card with more valid local cards available but billing customer
// number is blank, will not trigger migration.
TEST_F(LocalCardMigrationManagerTest, MigrateCreditCard_NoPaymentsAccount) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card.
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Tests that local cards that match masked server cards do not count as
// migratable.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_LocalCardMatchMaskedServerCard) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a masked server card whose |TypeAndLastFourDigits| matches a local
  // card.
  CreditCard server_card(CreditCard::MASKED_SERVER_CARD, "a123");
  test::SetCreditCardInfo(&server_card, "Flo Master", "1111", "11",
                          test::NextYear().c_str(), "1");
  server_card.SetNetworkForMaskedCard(kVisaCard);
  personal_data_.AddServerCreditCard(server_card);
  // Add a local card whose |TypeAndLastFourDigits| matches a masked server
  // card.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "5555555555554444", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// Tests that local cards that match full server cards do not count as
// migratable.
TEST_F(LocalCardMigrationManagerTest,
       MigrateCreditCard_LocalCardMatchFullServerCard) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a full server card whose number matches a local card.
  CreditCard server_card(CreditCard::FULL_SERVER_CARD, "a123");
  test::SetCreditCardInfo(&server_card, "Flo Master", "4111111111111111", "11",
                          test::NextYear().c_str(), "1");
  personal_data_.AddServerCreditCard(server_card);
  // Add a local credit card whose number matches a full server card.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "5555555555554444", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_FALSE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
}

// GetDetectedValues() should includes cardholder name if all cards have it.
TEST_F(LocalCardMigrationManagerTest, GetDetectedValues_AllWithCardHolderName) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card with a different cardholder name.
  AddLocalCrediCard(personal_data_, "John Smith", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_TRUE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
  EXPECT_TRUE(local_card_migration_manager_->GetDetectedValues() &
              CreditCardSaveManager::DetectedValue::CARDHOLDER_NAME);
}

// GetDetectedValues() should not include cardholder name if not all cards have
// a cardholder name.
TEST_F(LocalCardMigrationManagerTest,
       GetDetectedValues_OneCardWithoutCardHolderName) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card without card holder name.
  AddLocalCrediCard(personal_data_, "", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");
  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_TRUE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
  EXPECT_FALSE(local_card_migration_manager_->GetDetectedValues() &
               CreditCardSaveManager::DetectedValue::CARDHOLDER_NAME);
}

// GetDetectedValues() should include the existence of a Google Payments
// account.
TEST_F(LocalCardMigrationManagerTest,
       GetDetectedValues_IncludeGooglePaymentsAccount) {
  EnableAutofillCreditCardLocalCardMigrationExperiment();
  personal_data_.ClearCreditCards();
  personal_data_.ClearProfiles();
  credit_card_save_manager_->SetCreditCardUploadEnabled(true);
  // Set the billing_customer_number Priority Preference to designate
  // existence of a Payments account.
  autofill_client_.GetPrefs()->SetDouble(prefs::kAutofillBillingCustomerNumber,
                                         12345);
  // Add a local credit card whose |TypeAndLastFourDigits| matches what we will
  // enter below.
  AddLocalCrediCard(personal_data_, "Flo Master", "4111111111111111", "11",
                    test::NextYear().c_str(), "1");
  // Add another local credit card
  AddLocalCrediCard(personal_data_, "Flo Master", "5555555555554444", "11",
                    test::NextYear().c_str(), "1");

  // Set up our credit card form data.
  FormData credit_card_form;
  test::CreateTestCreditCardFormData(&credit_card_form, true, false);
  FormsSeen(std::vector<FormData>(1, credit_card_form));

  // Edit the data, and submit.
  EditCreditCardFrom(credit_card_form, "Flo Master", "4111111111111111", "11",
                     test::NextYear().c_str(), "123");
  FormSubmitted(credit_card_form);
  EXPECT_TRUE(local_card_migration_manager_->LocalCardMigrationWasTriggered());
  EXPECT_TRUE(
      local_card_migration_manager_->GetDetectedValues() &
      CreditCardSaveManager::DetectedValue::HAS_GOOGLE_PAYMENTS_ACCOUNT);
}

}  // namespace autofill
