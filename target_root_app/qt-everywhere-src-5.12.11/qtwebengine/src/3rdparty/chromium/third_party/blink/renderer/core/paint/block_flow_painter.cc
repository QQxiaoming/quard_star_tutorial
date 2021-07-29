// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/block_flow_painter.h"

#include "third_party/blink/renderer/core/layout/floating_objects.h"
#include "third_party/blink/renderer/core/layout/layout_block_flow.h"
#include "third_party/blink/renderer/core/paint/adjust_paint_offset_scope.h"
#include "third_party/blink/renderer/core/paint/block_painter.h"
#include "third_party/blink/renderer/core/paint/line_box_list_painter.h"
#include "third_party/blink/renderer/core/paint/object_painter.h"
#include "third_party/blink/renderer/core/paint/paint_info.h"

namespace blink {

void BlockFlowPainter::PaintContents(const PaintInfo& paint_info,
                                     const LayoutPoint& paint_offset) {
  if (paint_info.SuppressPaintingDescendants() &&
      !layout_block_flow_.IsLayoutView()) {
    return;
  }

  if (!layout_block_flow_.ChildrenInline()) {
    BlockPainter(layout_block_flow_).PaintContents(paint_info, paint_offset);
    return;
  }
  if (ShouldPaintDescendantOutlines(paint_info.phase)) {
    ObjectPainter(layout_block_flow_).PaintInlineChildrenOutlines(paint_info);
  } else {
    LineBoxListPainter(layout_block_flow_.LineBoxes())
        .Paint(layout_block_flow_, paint_info, paint_offset);
  }
}

void BlockFlowPainter::PaintFloats(const PaintInfo& paint_info) {
  if (!layout_block_flow_.GetFloatingObjects())
    return;

  DCHECK(paint_info.phase == PaintPhase::kFloat ||
         paint_info.phase == PaintPhase::kSelection ||
         paint_info.phase == PaintPhase::kTextClip);
  PaintInfo float_paint_info(paint_info);
  if (paint_info.phase == PaintPhase::kFloat)
    float_paint_info.phase = PaintPhase::kForeground;

  for (const auto& floating_object :
       layout_block_flow_.GetFloatingObjects()->Set()) {
    if (!floating_object->ShouldPaint())
      continue;

    const LayoutBox* floating_layout_object =
        floating_object->GetLayoutObject();
    // TODO(wangxianzhu): Should this be a DCHECK?
    if (floating_layout_object->HasSelfPaintingLayer())
      continue;

    ObjectPainter(*floating_layout_object)
        .PaintAllPhasesAtomically(float_paint_info);
  }
}

}  // namespace blink
