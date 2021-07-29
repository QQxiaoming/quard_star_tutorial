// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/dns/host_resolver_wrapper.h"

#include "net/dns/host_resolver.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

namespace extensions {

HostResolverWrapper::HostResolverWrapper() : resolver_(NULL) {}

// static
HostResolverWrapper* HostResolverWrapper::GetInstance() {
  return base::Singleton<extensions::HostResolverWrapper>::get();
}

net::HostResolver* HostResolverWrapper::GetHostResolver(
    net::URLRequestContextGetter* url_request_context_getter) {
  return resolver_ ? resolver_
                   : url_request_context_getter->GetURLRequestContext()
                         ->host_resolver();
}

void HostResolverWrapper::SetHostResolverForTesting(
    net::HostResolver* mock_resolver) {
  resolver_ = mock_resolver;
}

}  // namespace extensions
