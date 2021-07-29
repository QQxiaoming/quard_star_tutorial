// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_LOADER_PREVIEWS_RESOURCE_LOADING_HINTS_RECEIVER_IMPL_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_LOADER_PREVIEWS_RESOURCE_LOADING_HINTS_RECEIVER_IMPL_H_

#include "mojo/public/cpp/bindings/binding.h"
#include "third_party/blink/public/mojom/loader/previews_resource_loading_hints.mojom-blink.h"

namespace blink {

// Created and attached to a LocalFrame when
// PreviewsResourceLoadingHintsReceiverRequest is received by the frame from the
// browser process.
class PreviewsResourceLoadingHintsReceiverImpl
    : public mojom::blink::PreviewsResourceLoadingHintsReceiver {
 public:
  explicit PreviewsResourceLoadingHintsReceiverImpl(
      mojom::blink::PreviewsResourceLoadingHintsReceiverRequest request);
  ~PreviewsResourceLoadingHintsReceiverImpl() override;

 private:
  void SetResourceLoadingHints(mojom::blink::PreviewsResourceLoadingHintsPtr
                                   resource_loading_hints) override;

  // TODO(tbansal): https://crbug.com/800641. Consider using a RevocableBinding.
  mojo::Binding<mojom::blink::PreviewsResourceLoadingHintsReceiver> binding_;

  DISALLOW_COPY_AND_ASSIGN(PreviewsResourceLoadingHintsReceiverImpl);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_LOADER_PREVIEWS_RESOURCE_LOADING_HINTS_RECEIVER_IMPL_H_
