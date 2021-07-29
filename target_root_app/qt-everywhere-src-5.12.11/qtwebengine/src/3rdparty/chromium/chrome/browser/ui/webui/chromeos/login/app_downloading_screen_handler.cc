// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/app_downloading_screen_handler.h"

#include "chrome/browser/chromeos/login/screens/app_downloading_screen.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"

namespace {

const char kJsScreenPath[] = "login.AppDownloadingScreen";

}  // namespace

namespace chromeos {

AppDownloadingScreenHandler::AppDownloadingScreenHandler()
    : BaseScreenHandler(kScreenId) {
  set_call_js_prefix(kJsScreenPath);
}

AppDownloadingScreenHandler::~AppDownloadingScreenHandler() {}

void AppDownloadingScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("appDownloadingScreenTitle",
               IDS_LOGIN_APP_DOWNLOADING_SCREEN_TITLE);
  builder->Add("appDownloadingScreenDescription",
               IDS_LOGIN_APP_DOWNLOADING_SCREEN_DESCRIPTION);
  builder->Add("appDownloadingContinueSetup",
               IDS_LOGIN_APP_DOWNLOADING_CONTINUE_SETUP);
}

void AppDownloadingScreenHandler::RegisterMessages() {
  BaseScreenHandler::RegisterMessages();
}

void AppDownloadingScreenHandler::Bind(AppDownloadingScreen* screen) {
  screen_ = screen;
  BaseScreenHandler::SetBaseScreen(screen);
}

void AppDownloadingScreenHandler::Show() {
  ShowScreen(kScreenId);
}

void AppDownloadingScreenHandler::Hide() {}

void AppDownloadingScreenHandler::Initialize() {}

}  // namespace chromeos
