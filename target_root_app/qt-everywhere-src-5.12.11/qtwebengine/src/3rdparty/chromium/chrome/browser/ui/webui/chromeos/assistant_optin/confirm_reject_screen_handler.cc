// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/assistant_optin/confirm_reject_screen_handler.h"

#include "chrome/browser/browser_process.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"

namespace {

constexpr char kJsScreenPath[] = "AssistantConfirmRejectScreen";

}  // namespace

namespace chromeos {

ConfirmRejectScreenHandler::ConfirmRejectScreenHandler(
    OnAssistantOptInScreenExitCallback callback)
    : BaseWebUIHandler(), exit_callback_(std::move(callback)) {
  set_call_js_prefix(kJsScreenPath);
}

ConfirmRejectScreenHandler::~ConfirmRejectScreenHandler() = default;

void ConfirmRejectScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {}

void ConfirmRejectScreenHandler::RegisterMessages() {
  AddPrefixedCallback("userActed",
                      &ConfirmRejectScreenHandler::HandleUserAction);
}

void ConfirmRejectScreenHandler::Initialize() {}

void ConfirmRejectScreenHandler::HandleUserAction(bool confirm_result) {
  DCHECK(exit_callback_);
  if (confirm_result) {
    std::move(exit_callback_)
        .Run(AssistantOptInScreenExitCode::CONFIRM_ACCEPTED);
  } else {
    std::move(exit_callback_)
        .Run(AssistantOptInScreenExitCode::CONFIRM_REJECTED);
  }
}

}  // namespace chromeos
