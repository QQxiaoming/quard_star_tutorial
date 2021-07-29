// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/webui_util.h"

#include "base/strings/utf_string_conversions.h"
#include "base/values.h"

namespace webui {
namespace webui_util {

void AddPair(base::ListValue* list,
             const base::string16& key,
             const base::string16& value) {
  std::unique_ptr<base::DictionaryValue> results(new base::DictionaryValue());
  results->SetString("key", key);
  results->SetString("value", value);
  list->Append(std::move(results));
}

void AddPair(base::ListValue* list,
             const base::string16& key,
             const std::string& value) {
  AddPair(list, key, base::UTF8ToUTF16(value));
}

}  // namespace webui_util
}  // namespace webui
