// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_SANDBOX_POLICY_FUCHSIA_H_
#define CONTENT_COMMON_SANDBOX_POLICY_FUCHSIA_H_

#include "services/service_manager/sandbox/sandbox_type.h"

namespace base {
struct LaunchOptions;

namespace fuchsia {
class FilteredServiceDirectory;
}  // namespace fuchsia

}  // namespace base

namespace content {

class SandboxPolicyFuchsia {
 public:
  SandboxPolicyFuchsia();
  ~SandboxPolicyFuchsia();

  // Initializes the policy of the given sandbox |type|. Must be called on the
  // IO thread.
  void Initialize(service_manager::SandboxType type);

  // Modifies the process launch |options| to achieve  the level of
  // isolation appropriate for current the sandbox type. The caller may then add
  // any descriptors or handles afterward to grant additional capabilities
  // to the new process.
  void UpdateLaunchOptionsForSandbox(base::LaunchOptions* options);

 private:
  service_manager::SandboxType type_ = service_manager::SANDBOX_TYPE_INVALID;

  // Services directory used for the /svc namespace of the child process.
  std::unique_ptr<base::fuchsia::FilteredServiceDirectory> service_directory_;
};

}  // namespace content

#endif  // CONTENT_COMMON_SANDBOX_POLICY_FUCHSIA_H_
