// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/cookie_store/service_worker_global_scope_cookie_store.h"

#include <utility>

#include "services/network/public/mojom/restricted_cookie_manager.mojom-blink.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "third_party/blink/renderer/core/workers/worker_thread.h"
#include "third_party/blink/renderer/modules/cookie_store/cookie_store.h"
#include "third_party/blink/renderer/modules/cookie_store/global_cookie_store_impl.h"
#include "third_party/blink/renderer/modules/service_worker/service_worker_global_scope.h"

namespace blink {

template <>
CookieStore* GlobalCookieStoreImpl<WorkerGlobalScope>::BuildCookieStore(
    ExecutionContext* execution_context,
    service_manager::InterfaceProvider* interface_provider) {
  network::mojom::blink::RestrictedCookieManagerPtr cookie_manager_ptr;
  interface_provider->GetInterface(mojo::MakeRequest(&cookie_manager_ptr));

  blink::mojom::blink::CookieStorePtr cookie_store_ptr;
  interface_provider->GetInterface(mojo::MakeRequest(&cookie_store_ptr));

  return CookieStore::Create(execution_context, std::move(cookie_manager_ptr),
                             std::move(cookie_store_ptr));
}

CookieStore* ServiceWorkerGlobalScopeCookieStore::cookieStore(
    ServiceWorkerGlobalScope& worker) {
  // ServiceWorkerGlobalScope is Supplementable<WorkerGlobalScope>, not
  // Supplementable<ServiceWorkerGlobalScope>.
  return GlobalCookieStoreImpl<WorkerGlobalScope>::From(worker).GetCookieStore(
      worker);
}

}  // namespace blink
