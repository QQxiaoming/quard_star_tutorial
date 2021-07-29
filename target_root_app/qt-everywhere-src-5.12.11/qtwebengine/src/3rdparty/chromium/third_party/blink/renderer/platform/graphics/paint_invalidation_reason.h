// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_GRAPHICS_PAINT_INVALIDATION_REASON_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_GRAPHICS_PAINT_INVALIDATION_REASON_H_

#include <iosfwd>
#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

namespace blink {

enum class PaintInvalidationReason : uint8_t {
  kNone,
  kIncremental,
  kRectangle,
  kSelection,
  // Hit test changes do not require raster invalidation.
  kHitTest,
  // The following reasons will all cause full paint invalidation.
  // Any unspecified reason of full invalidation.
  kFull,
  kStyle,
  // Layout or visual geometry change.
  kGeometry,
  kCompositing,
  kAppeared,
  kDisappeared,
  kScroll,
  // Scroll bars, scroll corner, etc.
  kScrollControl,
  kOutline,
  kSubtree,
  kSVGResource,
  kBackground,
  kBackgroundOnScrollingContentsLayer,
  kCaret,
  kDocumentMarker,
  kImage,
  kUncacheable,
  // The initial PaintInvalidationReason of a DisplayItemClient.
  kJustCreated,
  kReordered,
  kChunkAppeared,
  kChunkDisappeared,
  kChunkUncacheable,
  kChunkReordered,
  kPaintProperty,
  // For tracking of direct raster invalidation of full composited layers. The
  // invalidation may be implicit, e.g. when a layer is created.
  kFullLayer,
  kForTesting,
  // kDelayedFull means that kFull is needed in order to fully paint the
  // content, but that painting of the object can be delayed until a future
  // frame. This can be the case for an object whose content is not visible to
  // the user.
  kDelayedFull,

  kMax = kDelayedFull
};

PLATFORM_EXPORT const char* PaintInvalidationReasonToString(
    PaintInvalidationReason);

inline bool IsFullPaintInvalidationReason(PaintInvalidationReason reason) {
  // LayoutNG fully invalidates selections on NGPaintFragments.
  // TODO(wangxianzhu): Move kSelection after kFull for LayoutNG.
  if (RuntimeEnabledFeatures::LayoutNGEnabled())
    return reason >= PaintInvalidationReason::kSelection;
  return reason >= PaintInvalidationReason::kFull;
}

inline bool IsImmediateFullPaintInvalidationReason(
    PaintInvalidationReason reason) {
  return IsFullPaintInvalidationReason(reason) &&
         reason != PaintInvalidationReason::kDelayedFull;
}

PLATFORM_EXPORT std::ostream& operator<<(std::ostream&,
                                         PaintInvalidationReason);

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_GRAPHICS_PAINT_INVALIDATION_REASON_H_
