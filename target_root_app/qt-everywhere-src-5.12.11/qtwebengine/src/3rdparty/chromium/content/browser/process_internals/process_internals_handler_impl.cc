// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/process_internals/process_internals_handler_impl.h"

#include "content/browser/process_internals/process_internals.mojom.h"
#include "content/public/browser/site_isolation_policy.h"
#include "content/public/browser/web_contents.h"

namespace content {

ProcessInternalsHandlerImpl::ProcessInternalsHandlerImpl(
    mojo::InterfaceRequest<::mojom::ProcessInternalsHandler> request)
    : binding_(this, std::move(request)) {}

ProcessInternalsHandlerImpl::~ProcessInternalsHandlerImpl() = default;

void ProcessInternalsHandlerImpl::GetIsolationMode(
    GetIsolationModeCallback callback) {
  std::vector<base::StringPiece> modes;
  if (SiteIsolationPolicy::UseDedicatedProcessesForAllSites())
    modes.push_back("Site Per Process");
  if (SiteIsolationPolicy::IsTopDocumentIsolationEnabled())
    modes.push_back("Top Document Isolation");
  if (SiteIsolationPolicy::AreIsolatedOriginsEnabled())
    modes.push_back("Isolate Origins");

  std::move(callback).Run(modes.empty() ? "Disabled"
                                        : base::JoinString(modes, ", "));
}

void ProcessInternalsHandlerImpl::GetIsolatedOriginsSize(
    GetIsolatedOriginsSizeCallback callback) {
  int size = SiteIsolationPolicy::GetIsolatedOrigins().size();
  std::move(callback).Run(size);
}

}  // namespace content
