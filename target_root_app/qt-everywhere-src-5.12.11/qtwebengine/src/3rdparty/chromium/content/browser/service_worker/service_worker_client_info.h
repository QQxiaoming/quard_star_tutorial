// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_CLIENT_INFO_H_
#define CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_CLIENT_INFO_H_

#include "base/callback.h"
#include "content/common/content_export.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/mojom/service_worker/service_worker_provider_type.mojom.h"

namespace content {

// Holds information about a single service worker client:
// https://w3c.github.io/ServiceWorker/#client
struct CONTENT_EXPORT ServiceWorkerClientInfo {
  ServiceWorkerClientInfo();
  ServiceWorkerClientInfo(
      int process_id,
      int route_id,
      const base::RepeatingCallback<WebContents*(void)>& web_contents_getter,
      blink::mojom::ServiceWorkerProviderType type);
  ServiceWorkerClientInfo(const ServiceWorkerClientInfo& other);
  ~ServiceWorkerClientInfo();

  // The renderer process this client lives in.
  int process_id;
  // If this client is a document |route_id| is its frame id; otherwise it is
  // MSG_ROUTING_NONE.
  int route_id;
  // Non-null if this client is a document and its corresponding
  // ServiceWorkerProviderHost was pre-created for a navigation. Returns
  // information indicating the tab where the navigation is occurring or
  // occurred in.
  base::RepeatingCallback<WebContents*(void)> web_contents_getter;
  // The client type.
  blink::mojom::ServiceWorkerProviderType type;
};

}  // namespace content

#endif  // CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_CLIENT_INFO_H_
