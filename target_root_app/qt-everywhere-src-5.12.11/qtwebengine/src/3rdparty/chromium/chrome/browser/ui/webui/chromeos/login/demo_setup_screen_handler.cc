// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/demo_setup_screen_handler.h"

#include "base/command_line.h"
#include "base/values.h"
#include "chrome/browser/chromeos/login/oobe_screen.h"
#include "chrome/browser/chromeos/login/screens/demo_setup_screen.h"
#include "chrome/browser/chromeos/login/wizard_controller.h"
#include "chrome/grit/generated_resources.h"
#include "chromeos/chromeos_switches.h"
#include "components/login/localized_values_builder.h"

namespace {

constexpr char kJsScreenPath[] = "login.DemoSetupScreen";

}  // namespace

namespace chromeos {

DemoSetupScreenHandler::DemoSetupScreenHandler()
    : BaseScreenHandler(kScreenId) {
  set_call_js_prefix(kJsScreenPath);
}

DemoSetupScreenHandler::~DemoSetupScreenHandler() {
  if (screen_)
    screen_->OnViewDestroyed(this);
}

void DemoSetupScreenHandler::Show() {
  ShowScreen(kScreenId);
}

void DemoSetupScreenHandler::Hide() {}

void DemoSetupScreenHandler::Bind(DemoSetupScreen* screen) {
  screen_ = screen;
  BaseScreenHandler::SetBaseScreen(screen);
}

void DemoSetupScreenHandler::OnSetupFinished(bool is_success,
                                             const std::string& message) {
  CallJS("onSetupFinished", is_success, message);
}

void DemoSetupScreenHandler::Initialize() {}

void DemoSetupScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("demoSetupProgressScreenTitle",
               IDS_OOBE_DEMO_SETUP_PROGRESS_SCREEN_TITLE);
  builder->Add("demoSetupErrorScreenTitle",
               IDS_OOBE_DEMO_SETUP_ERROR_SCREEN_TITLE);
  builder->Add("demoSetupErrorScreenSubtitle",
               IDS_OOBE_DEMO_SETUP_ERROR_SCREEN_SUBTITLE);
  builder->Add("demoSetupErrorScreenRetryButtonLabel",
               IDS_OOBE_DEMO_SETUP_ERROR_SCREEN_RETRY_BUTTON_LABEL);
}

void DemoSetupScreenHandler::GetAdditionalParameters(
    base::DictionaryValue* dict) {
  const bool is_offline_demo_mode_enabled =
      base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableOfflineDemoMode);
  dict->SetBoolean("offlineDemoModeEnabled", is_offline_demo_mode_enabled);
}

}  // namespace chromeos
