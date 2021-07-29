// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DISCOVER_DISCOVER_UI_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DISCOVER_DISCOVER_UI_H_

#include <vector>

#include "base/macros.h"

namespace base {
class DictionaryValue;
}

namespace content {
class WebUI;
}

namespace chromeos {

class DiscoverHandler;

class DiscoverUI {
 public:
  DiscoverUI();

  ~DiscoverUI();

  // Register WebUI handlers
  void RegisterMessages(content::WebUI* web_ui);

  // Returns localized strings and data.
  void GetAdditionalParameters(base::DictionaryValue* dict);

  void Show();

 private:
  bool initialized_ = false;

  // Non-owninng.
  // Handler are owned by WebUI, but we need to keep this list to be able to
  // to refresh string resources.
  std::vector<DiscoverHandler*> handlers_;

  DISALLOW_COPY_AND_ASSIGN(DiscoverUI);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DISCOVER_DISCOVER_UI_H_
