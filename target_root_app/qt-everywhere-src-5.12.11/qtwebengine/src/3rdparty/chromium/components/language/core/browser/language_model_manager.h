// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_LANGUAGE_CORE_BROWSER_LANGUAGE_MODEL_MANAGER_H_
#define COMPONENTS_LANGUAGE_CORE_BROWSER_LANGUAGE_MODEL_MANAGER_H_

#include <string>
#include <vector>

#include "components/keyed_service/core/keyed_service.h"
#include "components/language/core/browser/language_model.h"
#include "components/prefs/pref_service.h"

namespace language {

// Manages a set of LanguageModel objects.
class LanguageModelManager : public KeyedService {
 public:
  LanguageModelManager(PrefService* prefs, const std::string& ui_lang);

  ~LanguageModelManager() override;

  void SetDefaultModel(std::unique_ptr<LanguageModel> model);
  LanguageModel* GetDefaultModel();

 private:
  std::unique_ptr<LanguageModel> default_model_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(LanguageModelManager);
};

}  // namespace language

#endif  // COMPONENTS_LANGUAGE_CORE_BROWSER_LANGUAGE_MODEL_MANAGER_H_
