// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_NEW_PASSWORD_FORM_MANAGER_H_
#define COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_NEW_PASSWORD_FORM_MANAGER_H_

#include <map>
#include <vector>

#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/autofill/core/common/form_data.h"
#include "components/password_manager/core/browser/form_fetcher.h"
#include "components/password_manager/core/browser/form_parsing/password_field_prediction.h"
#include "components/password_manager/core/browser/password_form_manager_for_ui.h"

namespace autofill {
class FormStructure;
}

namespace password_manager {

class PasswordFormMetricsRecorder;
class PasswordManagerClient;
class PasswordManagerDriver;

// This class helps with filling the observed form and with saving/updating the
// stored information about it. It is aimed to replace PasswordFormManager and
// to be renamed in new Password Manager design. Details
// go/new-cpm-design-refactoring.
class NewPasswordFormManager : public PasswordFormManagerForUI,
                               public FormFetcher::Consumer {
 public:
  // TODO(crbug.com/621355): So far, |form_fetcher| can be null. In that case
  // |this| creates an instance of it itself (meant for production code). Once
  // the fetcher is shared between PasswordFormManager instances, it will be
  // required that |form_fetcher| is not null.
  NewPasswordFormManager(PasswordManagerClient* client,
                         const base::WeakPtr<PasswordManagerDriver>& driver,
                         const autofill::FormData& observed_form,
                         FormFetcher* form_fetcher);

  ~NewPasswordFormManager() override;

  // The upper limit on how many times Chrome will try to autofill the same
  // form.
  static constexpr int kMaxTimesAutofill = 5;

  // Compares |observed_form_| with |form| and returns true if they are the
  // same and if |driver| is the same as |driver_|.
  bool DoesManage(const autofill::FormData& form,
                  const PasswordManagerDriver* driver) const;

  // If |submitted_form| is managed by *this (i.e. DoesManage returns true for
  // |submitted_form| and |driver|) then saves |submitted_form| to
  // |submitted_form_| field, sets |is_submitted| = true and returns true.
  // Otherwise returns false.
  bool SetSubmittedFormIfIsManaged(const autofill::FormData& submitted_form,
                                   const PasswordManagerDriver* driver);
  bool is_submitted() { return is_submitted_; }
  void set_not_submitted() { is_submitted_ = false; }

  void set_old_parsing_result(const autofill::PasswordForm& form) {
    old_parsing_result_ = form;
  }

  // Selects from |predictions| predictions that corresponds to
  // |observed_form_|, initiates filling and stores predictions in
  // |predictions_|.
  void ProcessServerPredictions(
      const std::vector<autofill::FormStructure*>& predictions);

  // Sends fill data to the renderer.
  void Fill();

  // PasswordFormManagerForUI:
  FormFetcher* GetFormFetcher() override;
  const GURL& GetOrigin() const override;
  const std::map<base::string16, const autofill::PasswordForm*>&
  GetBestMatches() const override;
  const autofill::PasswordForm& GetPendingCredentials() const override;
  metrics_util::CredentialSourceType GetCredentialSource() override;
  PasswordFormMetricsRecorder* GetMetricsRecorder() override;
  const std::vector<const autofill::PasswordForm*>& GetBlacklistedMatches()
      const override;
  bool IsBlacklisted() const override;
  bool IsPasswordOverridden() const override;
  const autofill::PasswordForm* GetPreferredMatch() const override;

  void Save() override;
  void Update(const autofill::PasswordForm& credentials_to_update) override;
  void UpdateUsername(const base::string16& new_username) override;
  void UpdatePasswordValue(const base::string16& new_password) override;

  void OnNopeUpdateClicked() override;
  void OnNeverClicked() override;
  void OnNoInteraction(bool is_update) override;
  void PermanentlyBlacklist() override;
  void OnPasswordsRevealed() override;

 protected:
  // FormFetcher::Consumer:
  void ProcessMatches(
      const std::vector<const autofill::PasswordForm*>& non_federated,
      size_t filtered_count) override;

 private:
  // Compares |parsed_form| with |old_parsing_result_| and records UKM metric.
  // TODO(https://crbug.com/831123): Remove it when the old form parsing is
  // removed.
  void RecordMetricOnCompareParsingResult(
      const autofill::PasswordForm& parsed_form);

  // Report the time between receiving credentials from the password store and
  // the autofill server responding to the lookup request.
  void ReportTimeBetweenStoreAndServerUMA();

  // The client which implements embedder-specific PasswordManager operations.
  PasswordManagerClient* client_;

  base::WeakPtr<PasswordManagerDriver> driver_;

  const autofill::FormData observed_form_;

  // Set of nonblacklisted PasswordForms from the DB that best match the form
  // being managed by |this|, indexed by username. The PasswordForms are owned
  // by |form_fetcher_|.
  std::map<base::string16, const autofill::PasswordForm*> best_matches_;

  // Set of blacklisted forms from the PasswordStore that best match the current
  // form. They are owned by |form_fetcher_|.
  std::vector<const autofill::PasswordForm*> blacklisted_matches_;

  // Convenience pointer to entry in best_matches_ that is marked
  // as preferred. This is only allowed to be null if there are no best matches
  // at all, since there will always be one preferred login when there are
  // multiple matches (when first saved, a login is marked preferred).
  const autofill::PasswordForm* preferred_match_;

  // Takes care of recording metrics and events for |*this|.
  scoped_refptr<PasswordFormMetricsRecorder> metrics_recorder_;

  // When not null, then this is the object which |form_fetcher_| points to.
  std::unique_ptr<FormFetcher> owned_form_fetcher_;

  // FormFetcher instance which owns the login data from PasswordStore.
  FormFetcher* form_fetcher_;

  // |is_submitted_| = true means that a submission of the managed form was seen
  // and then |submitted_form_| contains the submitted form.
  bool is_submitted_ = false;
  autofill::FormData submitted_form_;

  base::Optional<FormPredictions> predictions_;

  // If Chrome has already autofilled a few times, it is probable that autofill
  // is triggered by programmatic changes in the page. We set a maximum number
  // of times that Chrome will autofill to avoid being stuck in an infinite
  // loop.
  int autofills_left_ = kMaxTimesAutofill;

  // Used for comparison metrics.
  // TODO(https://crbug.com/831123): Remove it when the old form parsing is
  // removed.
  autofill::PasswordForm old_parsing_result_;

  // Time when stored credentials are received from the store. Used for metrics.
  base::TimeTicks received_stored_credentials_time_;

  base::WeakPtrFactory<NewPasswordFormManager> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(NewPasswordFormManager);
};

}  // namespace  password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_NEW_PASSWORD_FORM_MANAGER_H_
