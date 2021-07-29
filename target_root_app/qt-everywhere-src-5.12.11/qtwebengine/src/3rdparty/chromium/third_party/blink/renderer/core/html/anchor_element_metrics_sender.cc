// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/html/anchor_element_metrics_sender.h"

#include "services/service_manager/public/cpp/interface_provider.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"

namespace blink {

// static
const char AnchorElementMetricsSender::kSupplementName[] =
    "DocumentAnchorElementMetricsSender";

AnchorElementMetricsSender::~AnchorElementMetricsSender() = default;

AnchorElementMetricsSender* AnchorElementMetricsSender::From(
    Document& document) {
  // Only root document owns the AnchorElementMetricsSender.
  DCHECK(!document.ParentDocument());

  AnchorElementMetricsSender* sender =
      Supplement<Document>::From<AnchorElementMetricsSender>(document);
  if (!sender) {
    sender = new AnchorElementMetricsSender(document);
    ProvideTo(document, sender);
  }
  return sender;
}

void AnchorElementMetricsSender::SendClickedAnchorMetricsToBrowser(
    mojom::blink::AnchorElementMetricsPtr metric) const {
  metrics_host_->UpdateAnchorElementMetrics(std::move(metric));
}

void AnchorElementMetricsSender::Trace(blink::Visitor* visitor) {
  Supplement<Document>::Trace(visitor);
}

AnchorElementMetricsSender::AnchorElementMetricsSender(Document& document)
    : Supplement<Document>(document) {
  DCHECK(!document.ParentDocument());

  document.GetFrame()->LocalFrameRoot().GetInterfaceProvider().GetInterface(
      mojo::MakeRequest(&metrics_host_));
}

}  // namespace blink
