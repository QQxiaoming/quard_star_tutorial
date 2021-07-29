// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/settings/chromeos/account_manager_handler.h"

#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/values.h"
#include "chrome/browser/chromeos/profiles/profile_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "chrome/browser/ui/webui/signin/inline_login_handler_dialog_chromeos.h"
#include "chromeos/account_manager/account_manager.h"
#include "chromeos/account_manager/account_manager_factory.h"
#include "components/signin/core/browser/account_tracker_service.h"
#include "components/user_manager/user.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/webui/web_ui_util.h"

namespace chromeos {
namespace settings {

AccountManagerUIHandler::AccountManagerUIHandler(
    AccountManager* account_manager,
    AccountTrackerService* account_tracker_service)
    : account_manager_(account_manager),
      account_tracker_service_(account_tracker_service),
      weak_factory_(this) {
  DCHECK(account_manager_);
  DCHECK(account_tracker_service_);

  account_manager_->AddObserver(this);
  account_tracker_service_->AddObserver(this);
}

AccountManagerUIHandler::~AccountManagerUIHandler() {
  account_manager_->RemoveObserver(this);
  account_tracker_service_->RemoveObserver(this);
}

void AccountManagerUIHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getAccounts",
      base::BindRepeating(&AccountManagerUIHandler::HandleGetAccounts,
                          weak_factory_.GetWeakPtr()));
  web_ui()->RegisterMessageCallback(
      "addAccount",
      base::BindRepeating(&AccountManagerUIHandler::HandleAddAccount,
                          weak_factory_.GetWeakPtr()));
}

void AccountManagerUIHandler::HandleGetAccounts(const base::ListValue* args) {
  AllowJavascript();
  CHECK(!args->GetList().empty());
  base::Value callback_id = args->GetList()[0].Clone();

  account_manager_->GetAccounts(
      base::BindOnce(&AccountManagerUIHandler::GetAccountsCallbackHandler,
                     weak_factory_.GetWeakPtr(), std::move(callback_id)));
}

void AccountManagerUIHandler::GetAccountsCallbackHandler(
    base::Value callback_id,
    std::vector<AccountManager::AccountKey> account_keys) {
  base::ListValue accounts;

  const AccountId device_account_id =
      ProfileHelper::Get()
          ->GetUserByProfile(Profile::FromWebUI(web_ui()))
          ->GetAccountId();

  base::DictionaryValue device_account;
  for (const auto& account_key : account_keys) {
    // We are only interested in listing GAIA accounts.
    if (account_key.account_type !=
        account_manager::AccountType::ACCOUNT_TYPE_GAIA) {
      continue;
    }
    AccountInfo account_info =
        account_tracker_service_->FindAccountInfoByGaiaId(account_key.id);
    DCHECK(!account_info.IsEmpty());

    if (account_info.full_name.empty()) {
      // Account info has not been fully fetched yet from GAIA. Ignore this
      // account.
      continue;
    }

    base::DictionaryValue account;
    account.SetString("fullName", account_info.full_name);
    account.SetString("email", account_info.email);
    gfx::Image icon =
        account_tracker_service_->GetAccountImage(account_info.account_id);
    account.SetString("pic", webui::GetBitmapDataUrl(icon.AsBitmap()));

    // |account_key| is a GAIA account and hence |id| is the obfuscated GAIA id
    // (see |AccountManager::AccountKey|)
    if (account_key.id != device_account_id.GetGaiaId()) {
      accounts.GetList().push_back(std::move(account));
    } else {
      device_account = std::move(account);
    }
  }

  // Device account must show up at the top.
  if (!device_account.empty()) {
    accounts.GetList().insert(accounts.GetList().begin(),
                              std::move(device_account));
  }

  ResolveJavascriptCallback(callback_id, accounts);
}

void AccountManagerUIHandler::HandleAddAccount(const base::ListValue* args) {
  AllowJavascript();
  InlineLoginHandlerDialogChromeOS::Show();
}

void AccountManagerUIHandler::OnJavascriptAllowed() {}

void AccountManagerUIHandler::OnJavascriptDisallowed() {}

// |AccountManager::Observer| overrides.
// Note: We need to listen on |AccountManager| in addition to
// |AccountTrackerService| because there is no guarantee that |AccountManager|
// (our source of truth) will have a newly added account by the time
// |AccountTrackerService| has it.
void AccountManagerUIHandler::OnTokenUpserted(
    const AccountManager::AccountKey& account_key) {
  RefreshUI();
}

void AccountManagerUIHandler::OnAccountRemoved(
    const AccountManager::AccountKey& account_key) {
  RefreshUI();
}

// |AccountTrackerService::Observer| overrides.
// For newly added accounts, |AccountTrackerService| may take some time to
// fetch user's full name and account image. Whenever that is completed, we
// may need to update the UI with this new set of information.
// Note that we may be listening to |AccountTrackerService| but we still
// consider |AccountManager| to be the source of truth for account list.
void AccountManagerUIHandler::OnAccountUpdated(const AccountInfo& info) {
  RefreshUI();
}

void AccountManagerUIHandler::OnAccountImageUpdated(
    const std::string& account_id,
    const gfx::Image& image) {
  RefreshUI();
}

void AccountManagerUIHandler::OnAccountRemoved(const AccountInfo& account_key) {
}

void AccountManagerUIHandler::RefreshUI() {
  if (!IsJavascriptAllowed()) {
    return;
  }

  FireWebUIListener("accounts-changed");
}

}  // namespace settings
}  // namespace chromeos
