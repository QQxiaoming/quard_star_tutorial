// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/assistant_optin/assistant_optin_ui.h"

#include <memory>

#include "base/bind.h"
#include "base/macros.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/webui/chromeos/assistant_optin/confirm_reject_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/assistant_optin/get_more_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/assistant_optin/ready_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/assistant_optin/third_party_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/assistant_optin/value_prop_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "components/arc/arc_prefs.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

namespace chromeos {

namespace {

bool is_active = false;

constexpr int kAssistantOptInDialogWidth = 576;
constexpr int kAssistantOptInDialogHeight = 480;

}  // namespace

AssistantOptInUI::AssistantOptInUI(content::WebUI* web_ui)
    : ui::WebDialogUI(web_ui), weak_factory_(this) {
  // Set up the chrome://assistant-optin source.
  content::WebUIDataSource* source =
      content::WebUIDataSource::Create(chrome::kChromeUIAssistantOptInHost);

  js_calls_container_ = std::make_unique<JSCallsContainer>();

  auto assistant_handler =
      std::make_unique<AssistantOptInHandler>(js_calls_container_.get());
  assistant_handler_ = assistant_handler.get();
  AddScreenHandler(std::move(assistant_handler));
  assistant_handler_->Initialize();

  AddScreenHandler(std::make_unique<ValuePropScreenHandler>(
      base::BindOnce(&AssistantOptInUI::OnExit, weak_factory_.GetWeakPtr())));
  AddScreenHandler(std::make_unique<ConfirmRejectScreenHandler>(
      base::BindOnce(&AssistantOptInUI::OnExit, weak_factory_.GetWeakPtr())));
  AddScreenHandler(std::make_unique<ThirdPartyScreenHandler>(
      base::BindOnce(&AssistantOptInUI::OnExit, weak_factory_.GetWeakPtr())));
  AddScreenHandler(std::make_unique<GetMoreScreenHandler>(
      base::BindOnce(&AssistantOptInUI::OnExit, weak_factory_.GetWeakPtr())));
  AddScreenHandler(std::make_unique<ReadyScreenHandler>());

  base::DictionaryValue localized_strings;
  for (auto* handler : screen_handlers_)
    handler->GetLocalizedStrings(&localized_strings);
  source->AddLocalizedStrings(localized_strings);

  source->SetJsonPath("strings.js");
  source->AddResourcePath("assistant_optin.js", IDR_ASSISTANT_OPTIN_JS);
  source->AddResourcePath("assistant_logo.png", IDR_ASSISTANT_LOGO_PNG);
  source->SetDefaultResource(IDR_ASSISTANT_OPTIN_HTML);
  content::WebUIDataSource::Add(Profile::FromWebUI(web_ui), source);
}

AssistantOptInUI::~AssistantOptInUI() = default;

void AssistantOptInUI::AddScreenHandler(
    std::unique_ptr<BaseWebUIHandler> handler) {
  screen_handlers_.push_back(handler.get());
  web_ui()->AddMessageHandler(std::move(handler));
}

void AssistantOptInUI::OnExit(AssistantOptInScreenExitCode exit_code) {
  switch (exit_code) {
    case AssistantOptInScreenExitCode::VALUE_PROP_SKIPPED:
      assistant_handler_->ShowNextScreen();
      break;
    case AssistantOptInScreenExitCode::VALUE_PROP_ACCEPTED:
      assistant_handler_->OnActivityControlOptInResult(true);
      break;
    case AssistantOptInScreenExitCode::CONFIRM_ACCEPTED:
      assistant_handler_->OnActivityControlOptInResult(true);
      break;
    case AssistantOptInScreenExitCode::CONFIRM_REJECTED:
      assistant_handler_->OnActivityControlOptInResult(false);
      break;
    case AssistantOptInScreenExitCode::THIRD_PARTY_CONTINUED:
      assistant_handler_->ShowNextScreen();
      break;
    case AssistantOptInScreenExitCode::EMAIL_OPTED_IN:
      assistant_handler_->OnEmailOptInResult(true);
      break;
    case AssistantOptInScreenExitCode::EMAIL_OPTED_OUT:
      assistant_handler_->OnEmailOptInResult(false);
      break;
    default:
      NOTREACHED();
  }
}

// AssistantOptInDialog

// static
void AssistantOptInDialog::Show(
    ash::mojom::AssistantSetup::StartAssistantOptInFlowCallback callback) {
  DCHECK(!is_active);
  AssistantOptInDialog* dialog = new AssistantOptInDialog(std::move(callback));
  dialog->ShowSystemDialog(true);
}

// static
bool AssistantOptInDialog::IsActive() {
  return is_active;
}

AssistantOptInDialog::AssistantOptInDialog(
    ash::mojom::AssistantSetup::StartAssistantOptInFlowCallback callback)
    : SystemWebDialogDelegate(GURL(chrome::kChromeUIAssistantOptInURL),
                              base::string16()),
      callback_(std::move(callback)) {
  DCHECK(!is_active);
  is_active = true;
}

AssistantOptInDialog::~AssistantOptInDialog() {
  is_active = false;
}

void AssistantOptInDialog::GetDialogSize(gfx::Size* size) const {
  size->SetSize(kAssistantOptInDialogWidth, kAssistantOptInDialogHeight);
}

std::string AssistantOptInDialog::GetDialogArgs() const {
  return std::string();
}

bool AssistantOptInDialog::ShouldShowDialogTitle() const {
  return false;
}

void AssistantOptInDialog::OnDialogClosed(const std::string& json_retval) {
  PrefService* prefs = ProfileManager::GetActiveUserProfile()->GetPrefs();
  const bool completed =
      prefs->GetBoolean(arc::prefs::kVoiceInteractionEnabled) &&
      prefs->GetBoolean(arc::prefs::kArcVoiceInteractionValuePropAccepted);
  std::move(callback_).Run(completed);
  SystemWebDialogDelegate::OnDialogClosed(json_retval);
}

}  // namespace chromeos
