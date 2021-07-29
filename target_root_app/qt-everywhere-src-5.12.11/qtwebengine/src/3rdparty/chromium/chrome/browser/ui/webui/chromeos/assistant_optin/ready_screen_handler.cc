// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/assistant_optin/ready_screen_handler.h"

#include "chrome/browser/browser_process.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"

namespace chromeos {

ReadyScreenHandler::ReadyScreenHandler() : BaseWebUIHandler() {}

ReadyScreenHandler::~ReadyScreenHandler() = default;

void ReadyScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("assistantReadyTitle", IDS_ASSISTANT_READY_SCREEN_TITLE);
  builder->Add("assistantReadyMessage", IDS_ASSISTANT_READY_SCREEN_MESSAGE);
  builder->Add("assistantReadyButton", IDS_ASSISTANT_DONE_BUTTON);
}

void ReadyScreenHandler::Initialize() {}

}  // namespace chromeos
