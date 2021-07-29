// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/embedded_content_painter.h"

#include "base/optional.h"
#include "third_party/blink/renderer/core/frame/embedded_content_view.h"
#include "third_party/blink/renderer/core/layout/layout_embedded_content.h"
#include "third_party/blink/renderer/core/paint/adjust_paint_offset_scope.h"
#include "third_party/blink/renderer/core/paint/box_painter.h"
#include "third_party/blink/renderer/core/paint/object_painter.h"
#include "third_party/blink/renderer/core/paint/paint_info.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/core/paint/replaced_painter.h"
#include "third_party/blink/renderer/core/paint/scrollable_area_painter.h"
#include "third_party/blink/renderer/core/paint/selection_painting_utils.h"
#include "third_party/blink/renderer/platform/graphics/paint/display_item_cache_skipper.h"
#include "third_party/blink/renderer/platform/graphics/paint/drawing_recorder.h"

namespace blink {

bool EmbeddedContentPainter::IsSelected() const {
  SelectionState s = layout_embedded_content_.GetSelectionState();
  if (s == SelectionState::kNone)
    return false;

  return true;
}

void EmbeddedContentPainter::Paint(const PaintInfo& paint_info) {
  // TODO(crbug.com/797779): For now embedded contents don't know whether
  // they are painted in a fragmented context and may do something bad in a
  // fragmented context, e.g. creating subsequences. Skip cache to avoid that.
  // This will be unnecessary when the contents are fragment aware.
  base::Optional<DisplayItemCacheSkipper> cache_skipper;
  DCHECK(layout_embedded_content_.HasLayer());
  if (layout_embedded_content_.Layer()->EnclosingPaginationLayer())
    cache_skipper.emplace(paint_info.context);

  AdjustPaintOffsetScope adjustment(layout_embedded_content_, paint_info);
  const auto& local_paint_info = adjustment.GetPaintInfo();
  auto paint_offset = adjustment.PaintOffset();
  if (!ReplacedPainter(layout_embedded_content_)
           .ShouldPaint(local_paint_info, paint_offset))
    return;

  LayoutRect border_rect(paint_offset, layout_embedded_content_.Size());

  if (layout_embedded_content_.HasBoxDecorationBackground() &&
      (local_paint_info.phase == PaintPhase::kForeground ||
       local_paint_info.phase == PaintPhase::kSelection)) {
    BoxPainter(layout_embedded_content_)
        .PaintBoxDecorationBackground(local_paint_info, paint_offset);
  }

  if (local_paint_info.phase == PaintPhase::kMask) {
    BoxPainter(layout_embedded_content_)
        .PaintMask(local_paint_info, paint_offset);
    return;
  }

  if (ShouldPaintSelfOutline(local_paint_info.phase)) {
    ObjectPainter(layout_embedded_content_)
        .PaintOutline(local_paint_info, paint_offset);
  }

  if (local_paint_info.phase != PaintPhase::kForeground)
    return;

  if (layout_embedded_content_.GetEmbeddedContentView()) {
    base::Optional<ScopedPaintChunkProperties> scoped_paint_chunk_properties;
    if (layout_embedded_content_.Style()->HasBorderRadius()) {
      if (border_rect.IsEmpty())
        return;

      const auto* fragment =
          local_paint_info.FragmentToPaint(layout_embedded_content_);
      if (!fragment)
        return;
      const auto* properties = fragment->PaintProperties();
      DCHECK(properties && properties->InnerBorderRadiusClip());
      scoped_paint_chunk_properties.emplace(
          local_paint_info.context.GetPaintController(),
          properties->InnerBorderRadiusClip(), layout_embedded_content_,
          DisplayItem::PaintPhaseToDrawingType(local_paint_info.phase));
    }

    layout_embedded_content_.PaintContents(local_paint_info, paint_offset);
  }

  // Paint a partially transparent wash over selected EmbeddedContentViews.
  if (IsSelected() && !local_paint_info.IsPrinting() &&
      !DrawingRecorder::UseCachedDrawingIfPossible(local_paint_info.context,
                                                   layout_embedded_content_,
                                                   local_paint_info.phase)) {
    LayoutRect rect = layout_embedded_content_.LocalSelectionRect();
    rect.MoveBy(paint_offset);
    IntRect selection_rect = PixelSnappedIntRect(rect);
    DrawingRecorder recorder(local_paint_info.context, layout_embedded_content_,
                             local_paint_info.phase);
    Color selection_bg = SelectionPaintingUtils::SelectionBackgroundColor(
        layout_embedded_content_.GetDocument(),
        layout_embedded_content_.StyleRef(),
        layout_embedded_content_.GetNode());
    local_paint_info.context.FillRect(selection_rect, selection_bg);
  }

  if (layout_embedded_content_.CanResize()) {
    ScrollableAreaPainter(
        *layout_embedded_content_.Layer()->GetScrollableArea())
        .PaintResizer(local_paint_info.context, RoundedIntPoint(paint_offset),
                      local_paint_info.GetCullRect());
  }
}

void EmbeddedContentPainter::PaintContents(const PaintInfo& paint_info,
                                           const LayoutPoint& paint_offset) {
  EmbeddedContentView* embedded_content_view =
      layout_embedded_content_.GetEmbeddedContentView();
  CHECK(embedded_content_view);

  IntPoint paint_location(RoundedIntPoint(
      paint_offset +
      layout_embedded_content_.ReplacedContentRect().Location()));

  // Views don't support painting with a paint offset, but instead
  // offset themselves using the frame rect location. To paint Views at
  // our desired location, we need to apply paint offset as a transform, with
  // the frame rect neutralized.
  IntSize view_paint_offset =
      paint_location - embedded_content_view->FrameRect().Location();
  CullRect adjusted_cull_rect(paint_info.GetCullRect(), -view_paint_offset);
  embedded_content_view->Paint(paint_info.context,
                               paint_info.GetGlobalPaintFlags(),
                               adjusted_cull_rect, view_paint_offset);
}

}  // namespace blink
