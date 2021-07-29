// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_TRACING_AGENT_REGISTRY_H_
#define SERVICES_TRACING_AGENT_REGISTRY_H_

#include <map>
#include <memory>
#include <string>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/service_manager/public/cpp/identity.h"
#include "services/tracing/public/mojom/tracing.mojom.h"

namespace service_manager {
struct BindSourceInfo;
}  // namespace service_manager

namespace tracing {

class AgentRegistry : public mojom::AgentRegistry {
 public:
  class AgentEntry : public base::SupportsWeakPtr<AgentEntry> {
   public:
    AgentEntry(size_t id,
               AgentRegistry* agent_registry,
               mojom::AgentPtr agent,
               const std::string& label,
               mojom::TraceDataType type,
               bool supports_explicit_clock_sync,
               base::ProcessId pid);
    ~AgentEntry();

    void AddDisconnectClosure(const void* closure_name,
                              base::OnceClosure closure);
    bool RemoveDisconnectClosure(const void* closure_name);
    bool HasDisconnectClosure(const void* closure_name);
    size_t num_disconnect_closures_for_testing() const {
      return closures_.size();
    }

    mojom::Agent* agent() const { return agent_.get(); }
    const std::string& label() const { return label_; }
    mojom::TraceDataType type() const { return type_; }
    bool supports_explicit_clock_sync() const {
      return supports_explicit_clock_sync_;
    }
    bool is_tracing() const { return is_tracing_; }
    void set_is_tracing(bool is_tracing) { is_tracing_ = is_tracing; }
    base::ProcessId pid() const { return pid_; }

   private:
    void OnConnectionError();

    const size_t id_;
    AgentRegistry* agent_registry_;
    mojom::AgentPtr agent_;
    const std::string label_;
    const mojom::TraceDataType type_;
    const bool supports_explicit_clock_sync_;
    const base::ProcessId pid_;
    std::map<const void*, base::OnceClosure> closures_;
    bool is_tracing_;

    DISALLOW_COPY_AND_ASSIGN(AgentEntry);
  };

  // A function to be run for every agent that registers itself.
  using AgentInitializationCallback =
      base::RepeatingCallback<void(AgentEntry*)>;

  AgentRegistry();
  ~AgentRegistry() override;

  void BindAgentRegistryRequest(
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      mojom::AgentRegistryRequest request,
      const service_manager::BindSourceInfo& source_info);
  void SetAgentInitializationCallback(
      const AgentInitializationCallback& callback);
  void RemoveAgentInitializationCallback();
  bool HasDisconnectClosure(const void* closure_name);

  template <typename FunctionType>
  void ForAllAgents(FunctionType function) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    for (const auto& key_value : agents_) {
      function(key_value.second.get());
    }
  }

 private:
  friend class AgentRegistryTest;  // For testing.
  friend class CoordinatorTest;    // For testing.

  void BindAgentRegistryRequestOnSequence(
      mojom::AgentRegistryRequest request,
      const service_manager::BindSourceInfo& source_info);

  // mojom::AgentRegistry
  void RegisterAgent(mojom::AgentPtr agent,
                     const std::string& label,
                     mojom::TraceDataType type,
                     bool supports_explicit_clock_sync,
                     base::ProcessId pid) override;

  void UnregisterAgent(size_t agent_id);

  mojo::BindingSet<mojom::AgentRegistry, service_manager::Identity> bindings_;
  size_t next_agent_id_ = 0;
  std::map<size_t, std::unique_ptr<AgentEntry>> agents_;
  AgentInitializationCallback agent_initialization_callback_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AgentRegistry);
};

}  // namespace tracing

#endif  // SERVICES_TRACING_AGENT_REGISTRY_H_
