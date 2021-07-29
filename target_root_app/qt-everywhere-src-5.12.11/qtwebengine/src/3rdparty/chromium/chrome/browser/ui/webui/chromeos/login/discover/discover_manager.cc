// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/discover/discover_manager.h"

#include <algorithm>

#include "base/logging.h"
#include "chrome/browser/ui/webui/chromeos/login/discover/discover_handler.h"
#include "chrome/browser/ui/webui/chromeos/login/discover/modules/discover_module_welcome.h"

namespace chromeos {

DiscoverManager::DiscoverManager() {
  CreateModules();
}

DiscoverManager::~DiscoverManager() = default;

bool DiscoverManager::IsCompleted() const {
  // Returns true if all of the modules are completed.
  return std::all_of(modules_.begin(), modules_.end(),
                     [](const auto& module_pair) {
                       return module_pair.second->IsCompleted();
                     });
}

void DiscoverManager::CreateModules() {
  modules_[DiscoverModuleWelcome::kModuleName] =
      std::make_unique<DiscoverModuleWelcome>();
}

std::vector<std::unique_ptr<DiscoverHandler>>
DiscoverManager::CreateWebUIHandlers() const {
  std::vector<std::unique_ptr<DiscoverHandler>> handlers;
  for (const auto& module_pair : modules_) {
    handlers.emplace_back(module_pair.second->CreateWebUIHandler());
  }
  return handlers;
}

}  // namespace chromeos
