// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/core/browser/password_autofill_manager.h"

#include <stddef.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/i18n/case_conversion.h"
#include "base/logging.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "components/autofill/core/browser/autofill_client.h"
#include "components/autofill/core/browser/autofill_experiments.h"
#include "components/autofill/core/browser/popup_item_ids.h"
#include "components/autofill/core/browser/suggestion.h"
#include "components/autofill/core/common/autofill_constants.h"
#include "components/autofill/core/common/autofill_data_validation.h"
#include "components/autofill/core/common/autofill_util.h"
#include "components/favicon/core/favicon_util.h"
#include "components/password_manager/core/browser/android_affiliation/affiliation_utils.h"
#include "components/password_manager/core/browser/password_manager_client.h"
#include "components/password_manager/core/browser/password_manager_driver.h"
#include "components/password_manager/core/browser/password_manager_metrics_recorder.h"
#include "components/password_manager/core/browser/password_manager_metrics_util.h"
#include "components/password_manager/core/browser/password_manager_util.h"
#include "components/password_manager/core/common/password_manager_features.h"
#include "components/security_state/core/security_state.h"
#include "components/strings/grit/components_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

#if defined(OS_ANDROID)
#include "base/android/build_info.h"
#endif

namespace password_manager {

namespace {

constexpr base::char16 kPasswordReplacementChar = 0x2022;

// Returns |username| unless it is empty. For an empty |username| returns a
// localised string saying this username is empty. Use this for displaying the
// usernames to the user. |replaced| is set to true iff |username| is empty.
base::string16 ReplaceEmptyUsername(const base::string16& username,
                                    bool* replaced) {
  *replaced = username.empty();
  if (username.empty())
    return l10n_util::GetStringUTF16(IDS_PASSWORD_MANAGER_EMPTY_LOGIN);
  return username;
}

// Returns the prettified version of |signon_realm| to be displayed on the UI.
base::string16 GetHumanReadableRealm(const std::string& signon_realm) {
  // For Android application realms, remove the hash component. Otherwise, make
  // no changes.
  FacetURI maybe_facet_uri(FacetURI::FromPotentiallyInvalidSpec(signon_realm));
  if (maybe_facet_uri.IsValidAndroidFacetURI())
    return base::UTF8ToUTF16("android://" +
                             maybe_facet_uri.android_package_name() + "/");
  GURL realm(signon_realm);
  if (realm.is_valid())
    return base::UTF8ToUTF16(realm.host());
  return base::UTF8ToUTF16(signon_realm);
}

// If |suggestion| was made for an empty username, then return the empty
// string, otherwise return |suggestion|.
base::string16 GetUsernameFromSuggestion(const base::string16& suggestion) {
  return suggestion ==
                 l10n_util::GetStringUTF16(IDS_PASSWORD_MANAGER_EMPTY_LOGIN)
             ? base::string16()
             : suggestion;
}

// If |field_suggestion| matches |field_content|, creates a Suggestion out of it
// and appends to |suggestions|.
void AppendSuggestionIfMatching(
    const base::string16& field_suggestion,
    const base::string16& field_contents,
    const gfx::Image& custom_icon,
    const std::string& signon_realm,
    bool show_all,
    bool is_password_field,
    size_t password_length,
    std::vector<autofill::Suggestion>* suggestions) {
  base::string16 lower_suggestion = base::i18n::ToLower(field_suggestion);
  base::string16 lower_contents = base::i18n::ToLower(field_contents);
  if (show_all || autofill::FieldIsSuggestionSubstringStartingOnTokenBoundary(
                      lower_suggestion, lower_contents, true)) {
    bool replaced_username;
    autofill::Suggestion suggestion(
        ReplaceEmptyUsername(field_suggestion, &replaced_username));
    suggestion.is_value_secondary = replaced_username;
    suggestion.label = GetHumanReadableRealm(signon_realm);
    suggestion.additional_label =
        base::string16(password_length, kPasswordReplacementChar);
    suggestion.frontend_id = is_password_field
                                 ? autofill::POPUP_ITEM_ID_PASSWORD_ENTRY
                                 : autofill::POPUP_ITEM_ID_USERNAME_ENTRY;
    suggestion.match =
        show_all || base::StartsWith(lower_suggestion, lower_contents,
                                     base::CompareCase::SENSITIVE)
            ? autofill::Suggestion::PREFIX_MATCH
            : autofill::Suggestion::SUBSTRING_MATCH;
    suggestion.custom_icon = custom_icon;
    // The UI code will pick up an icon from the resources based on the string.
    suggestion.icon = base::ASCIIToUTF16("globeIcon");
    suggestions->push_back(suggestion);
  }
}

// This function attempts to fill |suggestions| from |fill_data| based on
// |current_username| that is the current value of the field. Unless |show_all|
// is true, it only picks suggestions allowed by
// FieldIsSuggestionSubstringStartingOnTokenBoundary. It can pick either a
// substring or a prefix based on the flag.
void GetSuggestions(const autofill::PasswordFormFillData& fill_data,
                    const base::string16& current_username,
                    const gfx::Image& custom_icon,
                    bool show_all,
                    bool is_password_field,
                    std::vector<autofill::Suggestion>* suggestions) {
  AppendSuggestionIfMatching(
      fill_data.username_field.value, current_username, custom_icon,
      fill_data.preferred_realm, show_all, is_password_field,
      fill_data.password_field.value.size(), suggestions);

  for (const auto& login : fill_data.additional_logins) {
    AppendSuggestionIfMatching(login.first, current_username, custom_icon,
                               login.second.realm, show_all, is_password_field,
                               login.second.password.size(), suggestions);
  }

  // Prefix matches should precede other token matches.
  if (!show_all && autofill::IsFeatureSubstringMatchEnabled()) {
    std::sort(suggestions->begin(), suggestions->end(),
              [](const autofill::Suggestion& a, const autofill::Suggestion& b) {
                return a.match < b.match;
              });
  }
}

bool ShouldShowManualFallbackForPreLollipop(syncer::SyncService* sync_service) {
#if defined(OS_ANDROID)
  return ((base::android::BuildInfo::GetInstance()->sdk_int() >=
           base::android::SDK_VERSION_LOLLIPOP) ||
          (password_manager_util::GetPasswordSyncState(sync_service) ==
           SYNCING_NORMAL_ENCRYPTION));
#else
  return true;
#endif
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// PasswordAutofillManager, public:

PasswordAutofillManager::PasswordAutofillManager(
    PasswordManagerDriver* password_manager_driver,
    autofill::AutofillClient* autofill_client,
    PasswordManagerClient* password_client)
    : form_data_key_(-1),
      password_manager_driver_(password_manager_driver),
      autofill_client_(autofill_client),
      password_client_(password_client),
      weak_ptr_factory_(this) {}

PasswordAutofillManager::~PasswordAutofillManager() {
  if (deletion_callback_)
    std::move(deletion_callback_).Run();
}

bool PasswordAutofillManager::FillSuggestion(int key,
                                             const base::string16& username) {
  autofill::PasswordFormFillData fill_data;
  autofill::PasswordAndRealm password_and_realm;
  if (FindLoginInfo(key, &fill_data) &&
      GetPasswordAndRealmForUsername(
          username, fill_data, &password_and_realm)) {
    bool is_android_credential = FacetURI::FromPotentiallyInvalidSpec(
        password_and_realm.realm).IsValidAndroidFacetURI();
    metrics_util::LogFilledCredentialIsFromAndroidApp(is_android_credential);
    password_manager_driver_->FillSuggestion(
        username, password_and_realm.password);
    return true;
  }
  return false;
}

bool PasswordAutofillManager::PreviewSuggestion(
    int key,
    const base::string16& username) {
  autofill::PasswordFormFillData fill_data;
  autofill::PasswordAndRealm password_and_realm;
  if (FindLoginInfo(key, &fill_data) &&
      GetPasswordAndRealmForUsername(
          username, fill_data, &password_and_realm)) {
    password_manager_driver_->PreviewSuggestion(
        username, password_and_realm.password);
    return true;
  }
  return false;
}

void PasswordAutofillManager::OnAddPasswordFormMapping(
    int key,
    const autofill::PasswordFormFillData& fill_data) {
  if (!autofill::IsValidPasswordFormFillData(fill_data))
    return;

  login_to_password_info_[key] = fill_data;
  RequestFavicon(fill_data.origin);
}

void PasswordAutofillManager::OnShowPasswordSuggestions(
    int key,
    base::i18n::TextDirection text_direction,
    const base::string16& typed_username,
    int options,
    const gfx::RectF& bounds) {
  std::vector<autofill::Suggestion> suggestions;
  LoginToPasswordInfoMap::const_iterator fill_data_it =
      login_to_password_info_.find(key);
  if (fill_data_it == login_to_password_info_.end()) {
    // Probably a compromised renderer.
    NOTREACHED();
    return;
  }
  GetSuggestions(fill_data_it->second, typed_username, page_favicon_,
                 (options & autofill::SHOW_ALL) != 0,
                 (options & autofill::IS_PASSWORD_FIELD) != 0, &suggestions);

  form_data_key_ = key;

  if (suggestions.empty()) {
    autofill_client_->HideAutofillPopup();
    return;
  }

  GURL origin = fill_data_it->second.origin;

  if (ShouldShowManualFallbackForPreLollipop(
          autofill_client_->GetSyncService())) {
    if (password_client_ &&
        password_client_->IsFillingFallbackEnabledForCurrentPage()) {
      autofill::Suggestion suggestion(
          l10n_util::GetStringUTF8(IDS_PASSWORD_MANAGER_MANAGE_PASSWORDS),
          std::string(), std::string(),
          autofill::POPUP_ITEM_ID_ALL_SAVED_PASSWORDS_ENTRY);
      suggestions.push_back(suggestion);

      metrics_util::LogContextOfShowAllSavedPasswordsShown(
          metrics_util::SHOW_ALL_SAVED_PASSWORDS_CONTEXT_PASSWORD);
    }
  }

  autofill_client_->ShowAutofillPopup(bounds, text_direction, suggestions,
                                      false, weak_ptr_factory_.GetWeakPtr());
}

bool PasswordAutofillManager::MaybeShowPasswordSuggestions(
    const gfx::RectF& bounds,
    base::i18n::TextDirection text_direction) {
  if (login_to_password_info_.empty())
    return false;
  OnShowPasswordSuggestions(
      login_to_password_info_.begin()->first, text_direction, base::string16(),
      autofill::SHOW_ALL | autofill::IS_PASSWORD_FIELD, bounds);
  return true;
}

bool PasswordAutofillManager::MaybeShowPasswordSuggestionsWithGeneration(
    const gfx::RectF& bounds,
    base::i18n::TextDirection text_direction) {
  if (login_to_password_info_.empty())
    return false;
  std::vector<autofill::Suggestion> suggestions;
  GetSuggestions(login_to_password_info_.begin()->second, base::string16(),
                 page_favicon_, true /* show_all */,
                 true /* is_password_field */, &suggestions);
  form_data_key_ = login_to_password_info_.begin()->first;

  // Add 'Generation' option.
  // The UI code will pick up an icon from the resources based on the string.
  autofill::Suggestion suggestion(
      l10n_util::GetStringUTF8(IDS_PASSWORD_MANAGER_GENERATE_PASSWORD),
      std::string(), std::string("keyIcon"),
      autofill::POPUP_ITEM_ID_GENERATE_PASSWORD_ENTRY);
  suggestions.push_back(suggestion);

  // Add "Manage passwords".
  if (ShouldShowManualFallbackForPreLollipop(
          autofill_client_->GetSyncService())) {
    autofill::Suggestion suggestion(
        l10n_util::GetStringUTF8(IDS_PASSWORD_MANAGER_MANAGE_PASSWORDS),
        std::string(), std::string(),
        autofill::POPUP_ITEM_ID_ALL_SAVED_PASSWORDS_ENTRY);
    suggestions.push_back(suggestion);

    metrics_util::LogContextOfShowAllSavedPasswordsShown(
        metrics_util::SHOW_ALL_SAVED_PASSWORDS_CONTEXT_PASSWORD);
  }

  autofill_client_->ShowAutofillPopup(bounds, text_direction, suggestions,
                                      false, weak_ptr_factory_.GetWeakPtr());
  return true;
}

void PasswordAutofillManager::DidNavigateMainFrame() {
  login_to_password_info_.clear();
  favicon_tracker_.TryCancelAll();
  page_favicon_ = gfx::Image();
}

bool PasswordAutofillManager::FillSuggestionForTest(
    int key,
    const base::string16& username) {
  return FillSuggestion(key, username);
}

bool PasswordAutofillManager::PreviewSuggestionForTest(
    int key,
    const base::string16& username) {
  return PreviewSuggestion(key, username);
}

void PasswordAutofillManager::OnPopupShown() {
}

void PasswordAutofillManager::OnPopupHidden() {
}

void PasswordAutofillManager::DidSelectSuggestion(const base::string16& value,
                                                  int identifier) {
  ClearPreviewedForm();
  if (identifier == autofill::POPUP_ITEM_ID_ALL_SAVED_PASSWORDS_ENTRY ||
      identifier == autofill::POPUP_ITEM_ID_GENERATE_PASSWORD_ENTRY)
    return;
  bool success =
      PreviewSuggestion(form_data_key_, GetUsernameFromSuggestion(value));
  DCHECK(success);
}

void PasswordAutofillManager::DidAcceptSuggestion(const base::string16& value,
                                                  int identifier,
                                                  int position) {
  autofill_client_->ExecuteCommand(identifier);
  if (identifier == autofill::POPUP_ITEM_ID_GENERATE_PASSWORD_ENTRY) {
    password_client_->GeneratePassword();
  } else if (identifier == autofill::POPUP_ITEM_ID_ALL_SAVED_PASSWORDS_ENTRY) {
    metrics_util::LogContextOfShowAllSavedPasswordsAccepted(
        metrics_util::SHOW_ALL_SAVED_PASSWORDS_CONTEXT_PASSWORD);

    if (password_client_) {
      using UserAction =
          password_manager::PasswordManagerMetricsRecorder::PageLevelUserAction;

      password_client_->GetMetricsRecorder().RecordPageLevelUserAction(
          UserAction::kShowAllPasswordsWhileSomeAreSuggested);
      }
  } else {
    bool success =
        FillSuggestion(form_data_key_, GetUsernameFromSuggestion(value));
    DCHECK(success);
  }

  autofill_client_->HideAutofillPopup();
}

bool PasswordAutofillManager::GetDeletionConfirmationText(
    const base::string16& value,
    int identifier,
    base::string16* title,
    base::string16* body) {
  return false;
}

bool PasswordAutofillManager::RemoveSuggestion(const base::string16& value,
                                               int identifier) {
  // Password suggestions cannot be deleted this way.
  // See http://crbug.com/329038#c15
  return false;
}

void PasswordAutofillManager::ClearPreviewedForm() {
  password_manager_driver_->ClearPreviewedForm();
}

autofill::PopupType PasswordAutofillManager::GetPopupType() const {
  return autofill::PopupType::kPasswords;
}

autofill::AutofillDriver* PasswordAutofillManager::GetAutofillDriver() {
  return password_manager_driver_->GetAutofillDriver();
}

void PasswordAutofillManager::RegisterDeletionCallback(
    base::OnceClosure deletion_callback) {
  deletion_callback_ = std::move(deletion_callback);
}

////////////////////////////////////////////////////////////////////////////////
// PasswordAutofillManager, private:

bool PasswordAutofillManager::GetPasswordAndRealmForUsername(
    const base::string16& current_username,
    const autofill::PasswordFormFillData& fill_data,
    autofill::PasswordAndRealm* password_and_realm) {
  // TODO(dubroy): When password access requires some kind of authentication
  // (e.g. Keychain access on Mac OS), use |password_manager_client_| here to
  // fetch the actual password. See crbug.com/178358 for more context.

  // Look for any suitable matches to current field text.
  if (fill_data.username_field.value == current_username) {
    password_and_realm->password = fill_data.password_field.value;
    password_and_realm->realm = fill_data.preferred_realm;
    return true;
  }

  // Scan additional logins for a match.
  for (autofill::PasswordFormFillData::LoginCollection::const_iterator iter =
           fill_data.additional_logins.begin();
       iter != fill_data.additional_logins.end(); ++iter) {
    if (iter->first == current_username) {
      *password_and_realm = iter->second;
      return true;
    }
  }

  return false;
}

bool PasswordAutofillManager::FindLoginInfo(
    int key,
    autofill::PasswordFormFillData* found_password) {
  LoginToPasswordInfoMap::iterator iter = login_to_password_info_.find(key);
  if (iter == login_to_password_info_.end())
    return false;

  *found_password = iter->second;
  return true;
}

void PasswordAutofillManager::RequestFavicon(const GURL& url) {
  if (!password_client_)
    return;
  favicon::GetFaviconImageForPageURL(
      password_client_->GetFaviconService(), url,
      favicon_base::IconType::kFavicon,
      base::BindRepeating(&PasswordAutofillManager::OnFaviconReady,
                          weak_ptr_factory_.GetWeakPtr()),
      &favicon_tracker_);
}

void PasswordAutofillManager::OnFaviconReady(
    const favicon_base::FaviconImageResult& result) {
  if (!result.image.IsEmpty())
    page_favicon_ = result.image;
}

}  //  namespace password_manager
