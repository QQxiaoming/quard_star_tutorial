// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_WEBUI_UTIL_H_
#define CHROME_BROWSER_UI_WEBUI_WEBUI_UTIL_H_

#include "base/values.h"

namespace webui {
namespace webui_util {

// Helper functions for collecting a list of key-value pairs that will
// be displayed.
void AddPair(base::ListValue* list,
             const base::string16& key,
             const base::string16& value);

void AddPair(base::ListValue* list,
             const base::string16& key,
             const std::string& value);

}  // namespace webui_util
}  // namespace webui

#endif  // CHROME_BROWSER_UI_WEBUI_WEBUI_UTIL_H_
