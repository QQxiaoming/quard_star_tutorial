// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/replaced_painter.h"

#include "base/optional.h"
#include "third_party/blink/renderer/core/layout/api/selection_state.h"
#include "third_party/blink/renderer/core/layout/layout_replaced.h"
#include "third_party/blink/renderer/core/layout/svg/layout_svg_root.h"
#include "third_party/blink/renderer/core/paint/adjust_paint_offset_scope.h"
#include "third_party/blink/renderer/core/paint/box_painter.h"
#include "third_party/blink/renderer/core/paint/compositing/composited_layer_mapping.h"
#include "third_party/blink/renderer/core/paint/object_painter.h"
#include "third_party/blink/renderer/core/paint/paint_info.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/core/paint/rounded_inner_rect_clipper.h"
#include "third_party/blink/renderer/core/paint/selection_painting_utils.h"
#include "third_party/blink/renderer/platform/graphics/paint/drawing_recorder.h"
#include "third_party/blink/renderer/platform/graphics/paint/scoped_paint_chunk_properties.h"

namespace blink {

void ReplacedPainter::Paint(const PaintInfo& paint_info) {
  AdjustPaintOffsetScope adjustment(layout_replaced_, paint_info);
  const auto& local_paint_info = adjustment.GetPaintInfo();
  auto paint_offset = adjustment.PaintOffset();

  if (!ShouldPaint(local_paint_info, paint_offset))
    return;

  LayoutRect border_rect(paint_offset, layout_replaced_.Size());

  if (ShouldPaintSelfBlockBackground(local_paint_info.phase)) {
    if (layout_replaced_.Style()->Visibility() == EVisibility::kVisible &&
        layout_replaced_.HasBoxDecorationBackground()) {
      if (layout_replaced_.HasLayer() &&
          layout_replaced_.Layer()->GetCompositingState() ==
              kPaintsIntoOwnBacking &&
          layout_replaced_.Layer()
              ->GetCompositedLayerMapping()
              ->DrawsBackgroundOntoContentLayer())
        return;

      layout_replaced_.PaintBoxDecorationBackground(local_paint_info,
                                                    paint_offset);
    }
    // We're done. We don't bother painting any children.
    if (local_paint_info.phase == PaintPhase::kSelfBlockBackgroundOnly)
      return;
  }

  if (local_paint_info.phase == PaintPhase::kMask) {
    layout_replaced_.PaintMask(local_paint_info, paint_offset);
    return;
  }

  if (ShouldPaintSelfOutline(local_paint_info.phase)) {
    ObjectPainter(layout_replaced_)
        .PaintOutline(local_paint_info, paint_offset);
    return;
  }

  if (local_paint_info.phase != PaintPhase::kForeground &&
      local_paint_info.phase != PaintPhase::kSelection &&
      !layout_replaced_.CanHaveChildren())
    return;

  if (local_paint_info.phase == PaintPhase::kSelection &&
      layout_replaced_.GetSelectionState() == SelectionState::kNone)
    return;

  {
    base::Optional<ScopedPaintChunkProperties> chunk_properties;
    bool completely_clipped_out = false;

    if (layout_replaced_.Style()->HasBorderRadius() && border_rect.IsEmpty())
      completely_clipped_out = true;

    if (!layout_replaced_.IsSVGRoot()) {
      if (const auto* fragment = paint_info.FragmentToPaint(layout_replaced_)) {
        if (const auto* paint_properties = fragment->PaintProperties()) {
          // Check filter for optimized image policy violation highlights, which
          // may be applied locally.
          if (paint_properties->Filter() &&
              (!layout_replaced_.HasLayer() ||
               !layout_replaced_.Layer()->IsSelfPaintingLayer())) {
            chunk_properties.emplace(
                local_paint_info.context.GetPaintController(),
                fragment->ContentsProperties(), layout_replaced_,
                DisplayItem::PaintPhaseToDrawingType(local_paint_info.phase));
          } else if (layout_replaced_.Style()->HasBorderRadius()) {
            DCHECK(paint_properties->InnerBorderRadiusClip());
            chunk_properties.emplace(
                local_paint_info.context.GetPaintController(),
                paint_properties->InnerBorderRadiusClip(), layout_replaced_,
                DisplayItem::PaintPhaseToDrawingType(local_paint_info.phase));
          }
        }
      }
    }

    if (!completely_clipped_out)
      layout_replaced_.PaintReplaced(local_paint_info, paint_offset);
  }

  // The selection tint never gets clipped by border-radius rounding, since we
  // want it to run right up to the edges of surrounding content.
  bool draw_selection_tint =
      local_paint_info.phase == PaintPhase::kForeground &&
      layout_replaced_.GetSelectionState() != SelectionState::kNone &&
      !local_paint_info.IsPrinting();
  if (draw_selection_tint && !DrawingRecorder::UseCachedDrawingIfPossible(
                                 local_paint_info.context, layout_replaced_,
                                 DisplayItem::kSelectionTint)) {
    LayoutRect selection_painting_rect = layout_replaced_.LocalSelectionRect();
    selection_painting_rect.MoveBy(paint_offset);
    IntRect selection_painting_int_rect =
        PixelSnappedIntRect(selection_painting_rect);

    DrawingRecorder recorder(local_paint_info.context, layout_replaced_,
                             DisplayItem::kSelectionTint);
    Color selection_bg = SelectionPaintingUtils::SelectionBackgroundColor(
        layout_replaced_.GetDocument(), layout_replaced_.StyleRef(),
        layout_replaced_.GetNode());
    local_paint_info.context.FillRect(selection_painting_int_rect,
                                      selection_bg);
  }
}

bool ReplacedPainter::ShouldPaint(const PaintInfo& paint_info,
                                  const LayoutPoint& paint_offset) const {
  if (paint_info.phase != PaintPhase::kForeground &&
      !ShouldPaintSelfOutline(paint_info.phase) &&
      paint_info.phase != PaintPhase::kSelection &&
      paint_info.phase != PaintPhase::kMask &&
      !ShouldPaintSelfBlockBackground(paint_info.phase))
    return false;

  if (layout_replaced_.IsTruncated())
    return false;

  // If we're invisible or haven't received a layout yet, just bail.
  // But if it's an SVG root, there can be children, so we'll check visibility
  // later.
  if (!layout_replaced_.IsSVGRoot() &&
      layout_replaced_.Style()->Visibility() != EVisibility::kVisible)
    return false;

  LayoutRect paint_rect(layout_replaced_.VisualOverflowRect());
  paint_rect.Unite(layout_replaced_.LocalSelectionRect());
  paint_rect.MoveBy(paint_offset);

  if (!paint_info.GetCullRect().IntersectsCullRect(paint_rect))
    return false;

  return true;
}

}  // namespace blink
