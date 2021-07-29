// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/background_fetch/background_fetch_type_converters.h"

#include <utility>
#include "third_party/blink/renderer/modules/background_fetch/background_fetch_registration.h"
#include "third_party/blink/renderer/modules/manifest/image_resource_type_converters.h"
#include "third_party/blink/renderer/platform/heap/heap_allocator.h"

namespace mojo {

blink::BackgroundFetchRegistration*
TypeConverter<blink::BackgroundFetchRegistration*,
              blink::mojom::blink::BackgroundFetchRegistrationPtr>::
    Convert(const blink::mojom::blink::BackgroundFetchRegistrationPtr&
                mojo_registration) {
  if (!mojo_registration)
    return nullptr;

  return new blink::BackgroundFetchRegistration(
      mojo_registration->developer_id, mojo_registration->unique_id,
      mojo_registration->upload_total, mojo_registration->uploaded,
      mojo_registration->download_total, mojo_registration->downloaded);
}

blink::mojom::blink::BackgroundFetchOptionsPtr TypeConverter<
    blink::mojom::blink::BackgroundFetchOptionsPtr,
    blink::BackgroundFetchOptions>::Convert(const blink::BackgroundFetchOptions&
                                                options) {
  blink::mojom::blink::BackgroundFetchOptionsPtr mojo_options =
      blink::mojom::blink::BackgroundFetchOptions::New();

  WTF::Vector<blink::mojom::blink::ManifestImageResourcePtr> mojo_icons;
  mojo_icons.ReserveInitialCapacity(options.icons().size());

  for (const auto& icon : options.icons()) {
    mojo_icons.push_back(
        blink::mojom::blink::ManifestImageResource::From(icon));
  }

  mojo_options->icons = std::move(mojo_icons);
  mojo_options->download_total = options.downloadTotal();
  mojo_options->title = options.title();

  return mojo_options;
}

}  // namespace mojo
