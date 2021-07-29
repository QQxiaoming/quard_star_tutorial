// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/cookie_store/window_cookie_store.h"

#include <utility>

#include "services/network/public/mojom/restricted_cookie_manager.mojom-blink.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/modules/cookie_store/cookie_store.h"
#include "third_party/blink/renderer/modules/cookie_store/global_cookie_store_impl.h"

namespace blink {

template <>
CookieStore* GlobalCookieStoreImpl<LocalDOMWindow>::BuildCookieStore(
    ExecutionContext* execution_context,
    service_manager::InterfaceProvider* interface_provider) {
  network::mojom::blink::RestrictedCookieManagerPtr cookie_manager_ptr;
  interface_provider->GetInterface(mojo::MakeRequest(&cookie_manager_ptr));

  return CookieStore::Create(execution_context, std::move(cookie_manager_ptr),
                             blink::mojom::blink::CookieStorePtr());
}

CookieStore* WindowCookieStore::cookieStore(LocalDOMWindow& window) {
  return GlobalCookieStoreImpl<LocalDOMWindow>::From(window).GetCookieStore(
      window);
}

}  // namespace blink
