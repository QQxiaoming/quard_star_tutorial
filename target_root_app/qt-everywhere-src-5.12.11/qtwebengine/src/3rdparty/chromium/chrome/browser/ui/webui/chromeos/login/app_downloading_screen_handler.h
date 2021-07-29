// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_APP_DOWNLOADING_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_APP_DOWNLOADING_SCREEN_HANDLER_H_

#include "base/macros.h"
#include "chrome/browser/chromeos/login/screens/app_downloading_screen_view.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"

namespace chromeos {

class AppDownloadingScreen;

// The sole implementation of the AppDownloadingScreenView, using WebUI.
class AppDownloadingScreenHandler : public BaseScreenHandler,
                                    public AppDownloadingScreenView {
 public:
  AppDownloadingScreenHandler();
  ~AppDownloadingScreenHandler() override;

  // BaseScreenHandler:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void RegisterMessages() override;

  // AppDownloadingScreenView:
  void Bind(AppDownloadingScreen* screen) override;
  void Show() override;
  void Hide() override;

 private:
  // BaseScreenHandler:
  void Initialize() override;

  AppDownloadingScreen* screen_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(AppDownloadingScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_APP_DOWNLOADING_SCREEN_HANDLER_H_
