// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_CORS_CORS_URL_LOADER_FACTORY_H_
#define SERVICES_NETWORK_CORS_CORS_URL_LOADER_FACTORY_H_

#include <memory>

#include "base/callback_forward.h"
#include "base/containers/unique_ptr_adapters.h"
#include "base/macros.h"
#include "mojo/public/cpp/bindings/strong_binding_set.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace network {

class NetworkContext;
class ResourceSchedulerClient;
class URLLoader;
struct ResourceRequest;

namespace cors {

// A factory class to create a URLLoader that supports CORS.
// This class takes a network::mojom::URLLoaderFactory instance in the
// constructor and owns it to make network requests for CORS preflight, and
// actual network request.
class COMPONENT_EXPORT(NETWORK_SERVICE) CORSURLLoaderFactory final
    : public mojom::URLLoaderFactory {
 public:
  // Used by network::NetworkContext.
  CORSURLLoaderFactory(
      NetworkContext* context,
      mojom::URLLoaderFactoryParamsPtr params,
      scoped_refptr<ResourceSchedulerClient> resource_scheduler_client,
      mojom::URLLoaderFactoryRequest request);
  // Used by content::ResourceMessageFilter.
  // TODO(yhirano): Remove this once when the network service is fully enabled.
  CORSURLLoaderFactory(
      bool disable_web_security,
      std::unique_ptr<mojom::URLLoaderFactory> network_loader_factory,
      const base::RepeatingCallback<void(int)>& preflight_finalizer);
  ~CORSURLLoaderFactory() override;

  void OnLoaderCreated(std::unique_ptr<mojom::URLLoader> loader);
  void DestroyURLLoader(mojom::URLLoader* loader);

 private:
  // Implements mojom::URLLoaderFactory.
  void CreateLoaderAndStart(mojom::URLLoaderRequest request,
                            int32_t routing_id,
                            int32_t request_id,
                            uint32_t options,
                            const ResourceRequest& resource_request,
                            mojom::URLLoaderClientPtr client,
                            const net::MutableNetworkTrafficAnnotationTag&
                                traffic_annotation) override;
  void Clone(mojom::URLLoaderFactoryRequest request) override;

  void DeleteIfNeeded();

  static bool IsSane(const ResourceRequest& request);

  mojo::BindingSet<mojom::URLLoaderFactory> bindings_;

  // Used when constructed by NetworkContext.
  // The NetworkContext owns |this|.
  NetworkContext* const context_ = nullptr;
  scoped_refptr<ResourceSchedulerClient> resource_scheduler_client_;
  std::set<std::unique_ptr<mojom::URLLoader>, base::UniquePtrComparator>
      loaders_;

  const bool disable_web_security_;
  std::unique_ptr<mojom::URLLoaderFactory> network_loader_factory_;

  // Used when constructed by ResourceMessageFilter.
  base::RepeatingCallback<void(int)> preflight_finalizer_;

  DISALLOW_COPY_AND_ASSIGN(CORSURLLoaderFactory);
};

}  // namespace cors

}  // namespace network

#endif  // SERVICES_NETWORK_CORS_CORS_URL_LOADER_FACTORY_H_
