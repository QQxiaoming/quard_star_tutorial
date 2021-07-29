// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/features.h"

namespace blink {
namespace features {

// Enable eagerly setting up a CacheStorage interface pointer and
// passing it to service workers on startup as an optimization.
const base::Feature kEagerCacheStorageSetupForServiceWorkers{
    "EagerCacheStorageSetupForServiceWorkers",
    base::FEATURE_ENABLED_BY_DEFAULT};

// Enable LayoutNG.
const base::Feature kLayoutNG{"LayoutNG", base::FEATURE_DISABLED_BY_DEFAULT};

// Enable mojo Blob URL interface and better blob URL lifetime management.
// Can be enabled independently of NetworkService.
const base::Feature kMojoBlobURLs{"MojoBlobURLs",
                                  base::FEATURE_DISABLED_BY_DEFAULT};

// Nested workers. See https://crbug.com/31666
const base::Feature kNestedWorkers{"NestedWorkers",
                                   base::FEATURE_ENABLED_BY_DEFAULT};

// Enable new service worker glue for NetworkService. Can be
// enabled independently of NetworkService.
const base::Feature kServiceWorkerServicification{
    "ServiceWorkerServicification", base::FEATURE_DISABLED_BY_DEFAULT};

// The kill-switch for the fix for https://crbug.com/1051439.
// TODO(crbug.com/1053369): Remove this around M84.
const base::Feature kSuppressContentTypeForBeaconMadeWithArrayBufferView{
    "SuppressContentTypeForBeaconMadeWithArrayBufferView",
    base::FEATURE_ENABLED_BY_DEFAULT};

}  // namespace features
}  // namespace blink
