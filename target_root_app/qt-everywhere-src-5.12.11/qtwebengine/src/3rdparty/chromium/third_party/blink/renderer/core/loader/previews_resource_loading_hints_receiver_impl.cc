// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/loader/previews_resource_loading_hints_receiver_impl.h"

#include "base/metrics/histogram_macros.h"

namespace blink {

PreviewsResourceLoadingHintsReceiverImpl::
    PreviewsResourceLoadingHintsReceiverImpl(
        mojom::blink::PreviewsResourceLoadingHintsReceiverRequest request)
    : binding_(this, std::move(request)) {}

PreviewsResourceLoadingHintsReceiverImpl::
    ~PreviewsResourceLoadingHintsReceiverImpl() {}

void PreviewsResourceLoadingHintsReceiverImpl::SetResourceLoadingHints(
    mojom::blink::PreviewsResourceLoadingHintsPtr resource_loading_hints) {
  // TODO(tbansal): https://crbug.com/856247. Block loading of resources based
  // on |resource_loading_hints|.
  UMA_HISTOGRAM_COUNTS_100(
      "ResourceLoadingHints.CountBlockedSubresourcePatterns",
      resource_loading_hints->subresources_to_block.size());
}

}  // namespace blink
