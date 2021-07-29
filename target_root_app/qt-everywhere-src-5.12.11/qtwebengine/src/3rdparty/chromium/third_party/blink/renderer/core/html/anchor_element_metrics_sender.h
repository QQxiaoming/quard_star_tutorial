// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ANCHOR_ELEMENT_METRICS_SENDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ANCHOR_ELEMENT_METRICS_SENDER_H_

#include "base/macros.h"
#include "third_party/blink/public/mojom/loader/navigation_predictor.mojom-blink.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/supplementable.h"

namespace blink {

class Document;

// AnchorElementMetricsSender is responsible to send anchor element metrics to
// the browser process for a given document.
class AnchorElementMetricsSender final
    : public GarbageCollectedFinalized<AnchorElementMetricsSender>,
      public Supplement<Document> {
  USING_GARBAGE_COLLECTED_MIXIN(AnchorElementMetricsSender);

 public:
  static const char kSupplementName[];

  virtual ~AnchorElementMetricsSender();

  // Returns the anchor element metrics sender of the root document of
  // |Document|. Constructs new one if it does not exist.
  static AnchorElementMetricsSender* From(Document&);

  // Send metrics of anchor element clicked by the user to the browser.
  void SendClickedAnchorMetricsToBrowser(
      mojom::blink::AnchorElementMetricsPtr metric) const;

  void Trace(blink::Visitor*) override;

 private:
  explicit AnchorElementMetricsSender(Document&);

  // Browser host to which the anchor element metrics are sent.
  mojom::blink::AnchorElementMetricsHostPtr metrics_host_;

  DISALLOW_COPY_AND_ASSIGN(AnchorElementMetricsSender);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ANCHOR_ELEMENT_METRICS_SENDER_H_
