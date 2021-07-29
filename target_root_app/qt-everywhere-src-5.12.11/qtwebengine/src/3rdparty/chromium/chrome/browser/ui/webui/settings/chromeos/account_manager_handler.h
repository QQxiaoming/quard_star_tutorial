// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_ACCOUNT_MANAGER_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_ACCOUNT_MANAGER_HANDLER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "chromeos/account_manager/account_manager.h"
#include "components/signin/core/browser/account_tracker_service.h"

namespace chromeos {
namespace settings {

class AccountManagerUIHandler : public ::settings::SettingsPageUIHandler,
                                public AccountManager::Observer,
                                public AccountTrackerService::Observer {
 public:
  // Accepts non-owning pointers to |AccountManager| and
  // |AccountTrackerService|. Both of these must outlive |this| instance.
  AccountManagerUIHandler(AccountManager* account_manager,
                          AccountTrackerService* account_tracker_service);
  ~AccountManagerUIHandler() override;

  // WebUIMessageHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // |AccountManager::Observer| overrides.
  // |AccountManager| is considered to be the source of truth for account
  // information.
  void OnTokenUpserted(const AccountManager::AccountKey& account_key) override;
  void OnAccountRemoved(const AccountManager::AccountKey& account_key) override;

  // |AccountTrackerService::Observer| overrides.
  void OnAccountUpdated(const AccountInfo& info) override;
  void OnAccountImageUpdated(const std::string& account_id,
                             const gfx::Image& image) override;
  void OnAccountRemoved(const AccountInfo& account_key) override;

 private:
  // WebUI "getAccounts" message callback.
  void HandleGetAccounts(const base::ListValue* args);

  // WebUI "addAccount" message callback.
  void HandleAddAccount(const base::ListValue* args);

  // |AccountManager::GetAccounts| callback.
  void GetAccountsCallbackHandler(
      base::Value callback_id,
      std::vector<AccountManager::AccountKey> account_keys);

  // Refreshes the UI.
  void RefreshUI();

  // A non-owning pointer to |AccountManager|.
  AccountManager* const account_manager_;

  // A non-owning pointer to |AccountTrackerService|.
  AccountTrackerService* const account_tracker_service_;

  base::WeakPtrFactory<AccountManagerUIHandler> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(AccountManagerUIHandler);
};

}  // namespace settings
}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_ACCOUNT_MANAGER_HANDLER_H_
