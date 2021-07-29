// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/demo_preferences_screen_handler.h"

#include "chrome/browser/chromeos/login/oobe_screen.h"
#include "chrome/browser/chromeos/login/screens/demo_preferences_screen.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"

namespace {

constexpr char kJsScreenPath[] = "login.DemoPreferencesScreen";

}  // namespace

namespace chromeos {

DemoPreferencesScreenHandler::DemoPreferencesScreenHandler()
    : BaseScreenHandler(kScreenId) {
  set_call_js_prefix(kJsScreenPath);
}

DemoPreferencesScreenHandler::~DemoPreferencesScreenHandler() {
  if (screen_)
    screen_->OnViewDestroyed(this);
}

void DemoPreferencesScreenHandler::Show() {
  ShowScreen(kScreenId);
}

void DemoPreferencesScreenHandler::Hide() {}

void DemoPreferencesScreenHandler::Bind(DemoPreferencesScreen* screen) {
  screen_ = screen;
  BaseScreenHandler::SetBaseScreen(screen);
}

void DemoPreferencesScreenHandler::Initialize() {}

void DemoPreferencesScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("demoPreferencesScreenTitle",
               IDS_OOBE_DEMO_SETUP_PREFERENCES_SCREEN_TITLE);
  builder->Add("demoPreferencesNextButtonLabel",
               IDS_OOBE_DEMO_SETUP_PREFERENCES_SCREEN_NEXT_BUTTON_LABEL);
  builder->Add("languageDropdownTitle", IDS_LANGUAGE_DROPDOWN_TITLE);
  builder->Add("languageDropdownLabel", IDS_LANGUAGE_DROPDOWN_LABEL);
  builder->Add("keyboardDropdownTitle", IDS_KEYBOARD_DROPDOWN_TITLE);
  builder->Add("keyboardDropdownLabel", IDS_KEYBOARD_DROPDOWN_LABEL);
}

}  // namespace chromeos
