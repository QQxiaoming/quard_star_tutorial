// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_SYNC_CONSENT_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_SYNC_CONSENT_SCREEN_HANDLER_H_

#include <unordered_set>

#include "base/macros.h"
#include "chrome/browser/chromeos/login/screens/sync_consent_screen_view.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"

namespace chromeos {

class SyncConsentScreen;

// The sole implementation of the SyncConsentScreenView, using WebUI.
class SyncConsentScreenHandler : public BaseScreenHandler,
                                 public SyncConsentScreenView {
 public:
  SyncConsentScreenHandler();
  ~SyncConsentScreenHandler() override;

  // BaseScreenHandler:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;

  // SyncConsentScreenView:
  void Bind(SyncConsentScreen* screen) override;
  void Show() override;
  void Hide() override;
  void SetThrobberVisible(bool visible) override;

 private:
  // BaseScreenHandler:
  void Initialize() override;
  void RegisterMessages() override;
  void GetAdditionalParameters(base::DictionaryValue* parameters) override;

  // WebUI message handlers
  void HandleContinueAndReview(const ::login::StringList& consent_description,
                               const std::string& consent_confirmation);
  void HandleContinueWithDefaults(
      const ::login::StringList& consent_description,
      const std::string& consent_confirmation);

  // Adds resource |resource_id| both to |builder| and to |known_string_ids_|.
  void RememberLocalizedValue(const std::string& name,
                              const int resource_id,
                              ::login::LocalizedValuesBuilder* builder);

  // Resource IDs of the displayed strings.
  std::unordered_set<int> known_string_ids_;

  SyncConsentScreen* screen_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(SyncConsentScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_SYNC_CONSENT_SCREEN_HANDLER_H_
