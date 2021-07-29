// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DEMO_PREFERENCES_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DEMO_PREFERENCES_SCREEN_HANDLER_H_

#include "chrome/browser/chromeos/login/screens/demo_preferences_screen_view.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"

namespace chromeos {

class DemoPreferencesScreen;

// WebUI implementation of DemoPreferencesScreenView.
class DemoPreferencesScreenHandler : public BaseScreenHandler,
                                     public DemoPreferencesScreenView {
 public:
  DemoPreferencesScreenHandler();
  ~DemoPreferencesScreenHandler() override;

  // DemoPreferencesScreenView:
  void Show() override;
  void Hide() override;
  void Bind(DemoPreferencesScreen* screen) override;

  // BaseScreenHandler:
  void Initialize() override;
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;

 private:
  DemoPreferencesScreen* screen_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(DemoPreferencesScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DEMO_PREFERENCES_SCREEN_HANDLER_H_
