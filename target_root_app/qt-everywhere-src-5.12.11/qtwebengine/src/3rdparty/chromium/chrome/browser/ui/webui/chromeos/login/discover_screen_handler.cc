// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/discover_screen_handler.h"

#include "chrome/browser/chromeos/login/screens/discover_screen.h"

namespace {

const char kJsScreenPath[] = "login.DiscoverScreen";

}  // namespace

namespace chromeos {

DiscoverScreenHandler::DiscoverScreenHandler() : BaseScreenHandler(kScreenId) {
  set_call_js_prefix(kJsScreenPath);
}

DiscoverScreenHandler::~DiscoverScreenHandler() {}

void DiscoverScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {}

void DiscoverScreenHandler::RegisterMessages() {
  BaseWebUIHandler::RegisterMessages();
  discover_ui_.RegisterMessages(web_ui());
}

void DiscoverScreenHandler::GetAdditionalParameters(
    base::DictionaryValue* dict) {
  discover_ui_.GetAdditionalParameters(dict);
}

void DiscoverScreenHandler::Bind(DiscoverScreen* screen) {
  screen_ = screen;
  BaseScreenHandler::SetBaseScreen(screen);
}

void DiscoverScreenHandler::Hide() {}

void DiscoverScreenHandler::Initialize() {}

void DiscoverScreenHandler::Show() {
  ShowScreen(kScreenId);
  discover_ui_.Show();
}

}  // namespace chromeos
