// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_ASSISTANT_OPTIN_READY_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_ASSISTANT_OPTIN_READY_SCREEN_HANDLER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "chrome/browser/ui/webui/chromeos/assistant_optin/assistant_optin_screen_exit_code.h"
#include "chrome/browser/ui/webui/chromeos/login/base_webui_handler.h"

namespace chromeos {

class ReadyScreenHandler : public BaseWebUIHandler {
 public:
  ReadyScreenHandler();
  ~ReadyScreenHandler() override;

  // BaseWebUIHandler:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void Initialize() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ReadyScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_ASSISTANT_OPTIN_READY_SCREEN_HANDLER_H_
