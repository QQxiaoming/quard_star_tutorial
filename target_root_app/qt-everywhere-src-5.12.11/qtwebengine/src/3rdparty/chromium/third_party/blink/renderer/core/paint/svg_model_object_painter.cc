// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/svg_model_object_painter.h"

#include "third_party/blink/renderer/core/layout/svg/layout_svg_model_object.h"
#include "third_party/blink/renderer/core/paint/object_painter.h"
#include "third_party/blink/renderer/core/paint/paint_info.h"

namespace blink {

bool SVGModelObjectPainter::CullRectSkipsPainting(const PaintInfo& paint_info) {
  // We do not apply cull rect optimizations across transforms for two reasons:
  //   1) Performance: We can optimize transform changes by not repainting.
  //   2) Complexity: Difficulty updating clips when ancestor transforms change.
  // For these reasons, we do not cull painting if there is a transform.
  if (layout_svg_model_object_.StyleRef().HasTransform())
    return false;

  // LayoutSVGHiddenContainer's visual rect is always empty but we need to
  // paint its descendants so we cannot skip painting.
  if (layout_svg_model_object_.IsSVGHiddenContainer())
    return false;

  return !paint_info.GetCullRect().IntersectsCullRect(
      layout_svg_model_object_.LocalToSVGParentTransform(),
      layout_svg_model_object_.VisualRectInLocalSVGCoordinates());
}

void SVGModelObjectPainter::PaintOutline(const PaintInfo& paint_info) {
  if (paint_info.phase != PaintPhase::kForeground)
    return;
  if (layout_svg_model_object_.Style()->Visibility() != EVisibility::kVisible)
    return;
  if (!layout_svg_model_object_.Style()->OutlineWidth())
    return;

  PaintInfo outline_paint_info(paint_info);
  outline_paint_info.phase = PaintPhase::kSelfOutlineOnly;
  auto visual_rect = layout_svg_model_object_.VisualRectInLocalSVGCoordinates();
  ObjectPainter(layout_svg_model_object_)
      .PaintOutline(outline_paint_info, LayoutPoint(visual_rect.Location()));
}

}  // namespace blink
