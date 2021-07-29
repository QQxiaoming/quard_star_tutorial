// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/box_clipper.h"

#include "third_party/blink/renderer/core/layout/layout_box.h"
#include "third_party/blink/renderer/core/layout/svg/layout_svg_root.h"
#include "third_party/blink/renderer/core/paint/object_paint_properties.h"
#include "third_party/blink/renderer/core/paint/paint_info.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/platform/graphics/graphics_layer.h"
#include "third_party/blink/renderer/platform/graphics/paint/paint_controller.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

namespace blink {

DISABLE_CFI_PERF
BoxClipper::BoxClipper(const LayoutBox& box, const PaintInfo& paint_info)
    : box_(box), paint_info_(paint_info) {
  DCHECK(paint_info_.phase != PaintPhase::kSelfBlockBackgroundOnly &&
         paint_info_.phase != PaintPhase::kSelfOutlineOnly);

  if (paint_info_.phase == PaintPhase::kMask)
    return;

  InitializeScopedClipProperty(paint_info.FragmentToPaint(box_), box,
                               paint_info);
}

}  // namespace blink
