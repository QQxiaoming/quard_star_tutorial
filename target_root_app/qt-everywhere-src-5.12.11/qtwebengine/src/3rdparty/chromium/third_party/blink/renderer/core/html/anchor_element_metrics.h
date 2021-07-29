// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ANCHOR_ELEMENT_METRICS_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ANCHOR_ELEMENT_METRICS_H_

#include "base/feature_list.h"
#include "base/optional.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/allocator.h"

namespace blink {

CORE_EXPORT extern const base::Feature kRecordAnchorMetricsClicked;

class HTMLAnchorElement;

class CORE_EXPORT AnchorElementMetrics {
  STACK_ALLOCATED();

 public:
  // Creates AnchorElementMetrics from anchor element if possible. Then records
  // the metrics, and sends them to the browser process.
  static base::Optional<AnchorElementMetrics> MaybeExtractMetricsClicked(
      const HTMLAnchorElement*);

  // Upload anchor element features.
  void RecordMetrics() const;

  // Send anchor element features to browser process.
  void SendMetricsToBrowser() const;

  // Getters of anchor element features.
  float GetRatioArea() const { return ratio_area_; }
  float GetRatioVisibleArea() const { return ratio_visible_area_; }
  float GetRatioDistanceTopToVisibleTop() const {
    return ratio_distance_top_to_visible_top_;
  }
  float GetRatioDistanceCenterToVisibleTop() const {
    return ratio_distance_center_to_visible_top_;
  }
  float GetRatioDistanceRootTop() const { return ratio_distance_root_top_; }
  float GetRatioDistanceRootBottom() const {
    return ratio_distance_root_bottom_;
  }
  bool GetIsInIframe() const { return is_in_iframe_; }
  bool GetContainsImage() const { return contains_image_; }
  bool GetIsSameHost() const { return is_same_host_; }
  bool GetIsUrlIncrementedByOne() const { return is_url_incremented_by_one_; }

 private:
  // Extract features of the anchor element.
  static base::Optional<AnchorElementMetrics> Create(const HTMLAnchorElement*);

  // The anchor element that this class is associated with.
  Member<const HTMLAnchorElement> anchor_element_;

  // The ratio of the absolute/visible clickable region area of an anchor
  // element, and the viewport area.
  const float ratio_area_;
  const float ratio_visible_area_;

  // The distance between the top/center of the clickable region of an anchor
  // element and the top edge of the visible region, divided by the viewport
  // height.
  const float ratio_distance_top_to_visible_top_;
  const float ratio_distance_center_to_visible_top_;

  // The distance between the top of the clickable region of an anchor element
  // and the top edge of the root frame, divided by the viewport height.
  const float ratio_distance_root_top_;

  // The distance between the bottom of the clickable region of an anchor
  // element and the bottom edge of the root frame, divided by the viewport
  // height.
  const float ratio_distance_root_bottom_;

  // Whether the anchor element is within an iframe.
  const bool is_in_iframe_;

  // Whether the anchor element contains an image element.
  const bool contains_image_;

  // Whether the link target has the same host as the root document.
  const bool is_same_host_;

  // Whether the target url and the host url only differ by one number,
  // and the number in target url equals the one in host url plus one.
  const bool is_url_incremented_by_one_;

  inline AnchorElementMetrics(const HTMLAnchorElement* anchor_element,
                              float ratio_area,
                              float ratio_visible_area,
                              float ratio_distance_top_to_visible_top,
                              float ratio_distance_center_to_visible_top,
                              float ratio_distance_root_top,
                              float ratio_distance_root_bottom,
                              bool is_in_iframe,
                              bool contains_image,
                              bool is_same_host,
                              bool is_url_incremented_by_one)
      : anchor_element_(anchor_element),
        ratio_area_(ratio_area),
        ratio_visible_area_(ratio_visible_area),
        ratio_distance_top_to_visible_top_(ratio_distance_top_to_visible_top),
        ratio_distance_center_to_visible_top_(
            ratio_distance_center_to_visible_top),
        ratio_distance_root_top_(ratio_distance_root_top),
        ratio_distance_root_bottom_(ratio_distance_root_bottom),
        is_in_iframe_(is_in_iframe),
        contains_image_(contains_image),
        is_same_host_(is_same_host),
        is_url_incremented_by_one_(is_url_incremented_by_one) {}
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ANCHOR_ELEMENT_METRICS_H_
