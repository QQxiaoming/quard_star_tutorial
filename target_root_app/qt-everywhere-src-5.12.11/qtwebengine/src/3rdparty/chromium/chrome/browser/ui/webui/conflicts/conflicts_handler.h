// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CONFLICTS_CONFLICTS_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CONFLICTS_CONFLICTS_HANDLER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/conflicts/module_database_observer_win.h"
#include "content/public/browser/web_ui_message_handler.h"

#if defined(GOOGLE_CHROME_BUILD)
#include "chrome/browser/conflicts/third_party_conflicts_manager_win.h"
#endif

namespace base {
class Listvalue;
}

// This class takes care of sending the list of all loaded modules to the
// chrome://conflicts WebUI page when it is requested.
class ConflictsHandler : public content::WebUIMessageHandler,
                         public ModuleDatabaseObserver {
 public:
  ConflictsHandler();
  ~ConflictsHandler() override;

 private:
  enum ThirdPartyFeaturesStatus {
    // The third-party features are not available in non-Google Chrome builds.
    kNonGoogleChromeBuild,
    // The ThirdPartyBlockingEnabled group policy is disabled.
    kPolicyDisabled,
    // Both the IncompatibleApplicationsWarning and the
    // ThirdPartyModulesBlocking features are disabled.
    kFeatureDisabled,
    // The Module List version received is invalid.
    kModuleListInvalid,
    // There is no Module List version available.
    kNoModuleListAvailable,
    // Only the IncompatibleApplicationsWarning feature is initialized.
    kWarningInitialized,
    // Only the ThirdPartyModulesBlocking feature is initialized.
    kBlockingInitialized,
    // Both the IncompatibleApplicationsWarning and the
    // ThirdPartyModulesBlocking feature are initialized.
    kWarningAndBlockingInitialized,
  };

  // content::WebUIMessageHandler:
  void RegisterMessages() override;

  // ModuleDatabaseObserver:
  void OnNewModuleFound(const ModuleInfoKey& module_key,
                        const ModuleInfoData& module_data) override;
  void OnModuleDatabaseIdle() override;

  // Callback for the "requestModuleList" message.
  void HandleRequestModuleList(const base::ListValue* args);

#if defined(GOOGLE_CHROME_BUILD)
  // Invoked when the ThirdPartyConflictsManager initialization state is
  // available.
  void OnManagerInitializationComplete(ThirdPartyConflictsManager::State state);
#endif

  // Registers this instance to the ModuleDatabase to retrieve the list of
  // modules via the ModuleDatabaseObserver API.
  void GetListOfModules();

  // Returns true if one of the third-party features is enabled and active.
  static bool IsThirdPartyFeatureEnabled(ThirdPartyFeaturesStatus status);

  // Returns the status string of the third-party features.
  static std::string GetThirdPartyFeaturesStatusString(
      ThirdPartyFeaturesStatus status);

  // The ID of the callback that will get invoked with the module list.
  std::string module_list_callback_id_;

  // Temporarily holds the module list while the modules are being
  // enumerated.
  std::unique_ptr<base::ListValue> module_list_;

  base::Optional<ThirdPartyFeaturesStatus> third_party_features_status_;

  base::WeakPtrFactory<ConflictsHandler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ConflictsHandler);
};

#endif  // CHROME_BROWSER_UI_WEBUI_CONFLICTS_CONFLICTS_HANDLER_H_
