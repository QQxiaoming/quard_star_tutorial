// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DISCOVER_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DISCOVER_SCREEN_HANDLER_H_

#include "base/macros.h"
#include "chrome/browser/chromeos/login/screens/discover_screen_view.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/login/discover/discover_ui.h"

namespace chromeos {

class DiscoverScreen;

// The sole implementation of the DiscoverScreenView, using WebUI.
class DiscoverScreenHandler : public BaseScreenHandler,
                              public DiscoverScreenView {
 public:
  DiscoverScreenHandler();
  ~DiscoverScreenHandler() override;

  // BaseScreenHandler:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void GetAdditionalParameters(base::DictionaryValue* dict) override;
  void RegisterMessages() override;

  // DiscoverScreenView:
  void Bind(DiscoverScreen* screen) override;
  void Hide() override;
  void Initialize() override;
  void Show() override;

 private:
  DiscoverScreen* screen_ = nullptr;

  DiscoverUI discover_ui_;

  DISALLOW_COPY_AND_ASSIGN(DiscoverScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DISCOVER_SCREEN_HANDLER_H_
