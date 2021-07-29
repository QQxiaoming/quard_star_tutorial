// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/language/core/browser/language_model_manager.h"

namespace language {

LanguageModelManager::LanguageModelManager(PrefService* prefs,
                                           const std::string& ui_lang) {
  // TODO(crbug.com/855192): put code to add UI language to the blacklist here.
}

LanguageModelManager::~LanguageModelManager() {}

void LanguageModelManager::SetDefaultModel(
    std::unique_ptr<LanguageModel> model) {
  default_model_ = std::move(model);
}

LanguageModel* LanguageModelManager::GetDefaultModel() {
  return default_model_.get();
}

}  // namespace language
