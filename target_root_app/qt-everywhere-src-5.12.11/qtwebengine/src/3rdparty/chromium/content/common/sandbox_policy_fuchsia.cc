// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/sandbox_policy_fuchsia.h"

#include <fuchsia/fonts/cpp/fidl.h>
#include <lib/fdio/spawn.h>
#include <zircon/processargs.h>

#include "base/base_paths_fuchsia.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/fuchsia/component_context.h"
#include "base/fuchsia/filtered_service_directory.h"
#include "base/process/launch.h"
#include "base/process/process.h"
#include "content/public/common/content_switches.h"
#include "services/service_manager/sandbox/switches.h"

namespace content {

constexpr const char* const kRendererServices[] = {
    fuchsia::fonts::FontProvider::Name_};

SandboxPolicyFuchsia::SandboxPolicyFuchsia() = default;
SandboxPolicyFuchsia::~SandboxPolicyFuchsia() = default;

void SandboxPolicyFuchsia::Initialize(service_manager::SandboxType type) {
  DCHECK_NE(type, service_manager::SANDBOX_TYPE_INVALID);
  DCHECK_EQ(type_, service_manager::SANDBOX_TYPE_INVALID);

  type_ = type;

  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          service_manager::switches::kNoSandbox)) {
    type_ = service_manager::SANDBOX_TYPE_NO_SANDBOX;
  }

  if (type_ == service_manager::SANDBOX_TYPE_RENDERER) {
    // Create FilteredServicesDirectory for the renderer process and export all
    // services in kRendererServices. ServiceDirectoryProxy must be initialized
    // on a thread that has async_dispatcher.
    service_directory_ =
        std::make_unique<base::fuchsia::FilteredServiceDirectory>(
            base::fuchsia::ComponentContext::GetDefault());
    for (const char* service_name : kRendererServices)
      service_directory_->AddService(service_name);
  }
}

void SandboxPolicyFuchsia::UpdateLaunchOptionsForSandbox(
    base::LaunchOptions* options) {
  DCHECK_NE(type_, service_manager::SANDBOX_TYPE_INVALID);

  if (type_ == service_manager::SANDBOX_TYPE_NO_SANDBOX) {
    options->spawn_flags = FDIO_SPAWN_CLONE_NAMESPACE | FDIO_SPAWN_CLONE_JOB |
                           FDIO_SPAWN_CLONE_STDIO;
    options->clear_environ = false;
    return;
  }

  // Map /pkg (read-only files deployed from the package) and /tmp into the
  // child's namespace.
  options->paths_to_clone.push_back(base::GetPackageRoot());
  base::FilePath temp_dir;
  base::GetTempDir(&temp_dir);
  options->paths_to_clone.push_back(temp_dir);

  // Clear environmental variables to better isolate the child from
  // this process.
  options->clear_environ = true;

  // Propagate stdout/stderr/stdin to the child.
  options->spawn_flags = FDIO_SPAWN_CLONE_STDIO;

  if (service_directory_) {
    // Provide the child process with a restricted set of services.
    options->paths_to_transfer.push_back(base::PathToTransfer{
        base::FilePath("/svc"), service_directory_->ConnectClient().release()});
  }
}

}  // namespace content
