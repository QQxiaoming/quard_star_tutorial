// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_ASSISTANT_OPTIN_ASSISTANT_OPTIN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_ASSISTANT_OPTIN_ASSISTANT_OPTIN_HANDLER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "chrome/browser/chromeos/arc/voice_interaction/voice_interaction_controller_client.h"
#include "chrome/browser/ui/webui/chromeos/login/base_webui_handler.h"
#include "chromeos/services/assistant/public/mojom/settings.mojom.h"

namespace chromeos {

class AssistantOptInHandler
    : public BaseWebUIHandler,
      public arc::VoiceInteractionControllerClient::Observer {
 public:
  explicit AssistantOptInHandler(JSCallsContainer* js_calls_container);
  ~AssistantOptInHandler() override;

  // BaseWebUIHandler:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void RegisterMessages() override;
  void Initialize() override;

  // Send messages to the page.
  void ShowNextScreen();

  // Handle user opt-in result.
  void OnActivityControlOptInResult(bool opted_in);
  void OnEmailOptInResult(bool opted_in);

 private:
  // arc::VoiceInteractionControllerClient::Observer overrides
  void OnStateChanged(ash::mojom::VoiceInteractionState state) override;

  // Connect to assistant settings manager.
  void BindAssistantSettingsManager();

  // Send GetSettings request for the opt-in UI.
  void SendGetSettingsRequest();

  // Send message and consent data to the page.
  void ReloadContent(const base::DictionaryValue& dict);
  void AddSettingZippy(const std::string& type, const base::ListValue& data);

  // Handle response from the settings manager.
  void OnGetSettingsResponse(const std::string& settings);
  void OnUpdateSettingsResponse(const std::string& settings);

  // Handler for JS WebUI message.
  void HandleInitialized();

  // Consent token used to complete the opt-in.
  std::string consent_token_;

  // Whether activity control is needed for user.
  bool activity_control_needed_ = true;

  // Whether email optin is needed for user.
  bool email_optin_needed_ = false;

  assistant::mojom::AssistantSettingsManagerPtr settings_manager_;
  base::WeakPtrFactory<AssistantOptInHandler> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AssistantOptInHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_ASSISTANT_OPTIN_ASSISTANT_OPTIN_HANDLER_H_
