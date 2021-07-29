// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_RULES_MONITOR_SERVICE_H_
#define EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_RULES_MONITOR_SERVICE_H_

#include <memory>
#include <set>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observer.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/extension_id.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace extensions {
class InfoMap;
class ExtensionPrefs;
class ExtensionRegistry;
class WarningService;

namespace declarative_net_request {
class RulesetMatcher;

// Observes loading and unloading of extensions to load and unload their
// rulesets for the Declarative Net Request API. Lives on the UI thread. Note: A
// separate instance of RulesMonitorService is not created for incognito. Both
// the incognito and normal contexts will share the same ruleset.
class RulesMonitorService : public BrowserContextKeyedAPI,
                            public ExtensionRegistryObserver {
 public:
  // BrowserContextKeyedAPI implementation.
  static BrowserContextKeyedAPIFactory<RulesMonitorService>*
  GetFactoryInstance();

  bool HasAnyRegisteredRulesets() const;

  // Returns true if there is registered declarative ruleset corresponding to
  // the given |extension_id|.
  bool HasRegisteredRuleset(const ExtensionId& extension_id) const;

 private:
  struct LoadRulesetInfo;
  class FileSequenceState;
  class FileSequenceBridge;

  friend class BrowserContextKeyedAPIFactory<RulesMonitorService>;

  // The constructor is kept private since this should only be created by the
  // BrowserContextKeyedAPIFactory.
  explicit RulesMonitorService(content::BrowserContext* browser_context);

  ~RulesMonitorService() override;

  // BrowserContextKeyedAPI implementation.
  static const char* service_name() { return "RulesMonitorService"; }
  static const bool kServiceIsNULLWhileTesting = true;

  // ExtensionRegistryObserver implementation.
  void OnExtensionLoaded(content::BrowserContext* browser_context,
                         const Extension* extension) override;
  void OnExtensionUnloaded(content::BrowserContext* browser_context,
                           const Extension* extension,
                           UnloadedExtensionReason reason) override;

  // Callback invoked when we have loaded the ruleset for |info| on
  // |file_task_runner_|. |matcher| is null iff the ruleset loading failed.
  void OnRulesetLoaded(LoadRulesetInfo info,
                       std::unique_ptr<RulesetMatcher> matcher);

  ScopedObserver<ExtensionRegistry, ExtensionRegistryObserver>
      registry_observer_;

  std::set<ExtensionId> extensions_with_rulesets_;

  // Helper to bridge tasks to a sequence which allows file IO.
  std::unique_ptr<const FileSequenceBridge> file_sequence_bridge_;

  // Guaranteed to be valid through-out the lifetime of this instance.
  InfoMap* const info_map_;
  const ExtensionPrefs* const prefs_;
  ExtensionRegistry* const extension_registry_;
  WarningService* const warning_service_;

  // Must be the last member variable. See WeakPtrFactory documentation for
  // details.
  base::WeakPtrFactory<RulesMonitorService> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(RulesMonitorService);
};

}  // namespace declarative_net_request

template <>
void BrowserContextKeyedAPIFactory<
    declarative_net_request::RulesMonitorService>::DeclareFactoryDependencies();

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_RULES_MONITOR_SERVICE_H_
