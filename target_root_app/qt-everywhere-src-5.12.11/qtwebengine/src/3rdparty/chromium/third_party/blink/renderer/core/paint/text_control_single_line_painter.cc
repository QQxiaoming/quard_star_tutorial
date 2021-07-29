// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/text_control_single_line_painter.h"

#include "third_party/blink/renderer/core/layout/layout_text_control_single_line.h"
#include "third_party/blink/renderer/core/layout/layout_theme.h"
#include "third_party/blink/renderer/core/paint/adjust_paint_offset_scope.h"
#include "third_party/blink/renderer/core/paint/block_painter.h"
#include "third_party/blink/renderer/core/paint/theme_painter.h"
#include "third_party/blink/renderer/platform/graphics/paint/drawing_recorder.h"

namespace blink {

void TextControlSingleLinePainter::Paint(const PaintInfo& paint_info) {
  BlockPainter(text_control_).Paint(paint_info);

  if (!ShouldPaintSelfBlockBackground(paint_info.phase) ||
      !text_control_.ShouldDrawCapsLockIndicator())
    return;

  if (DrawingRecorder::UseCachedDrawingIfPossible(
          paint_info.context, text_control_, DisplayItem::kCapsLockIndicator))
    return;
  DrawingRecorder recorder(paint_info.context, text_control_,
                           DisplayItem::kCapsLockIndicator);

  LayoutRect contents_rect = text_control_.ContentBoxRect();

  // Center in the block progression direction.
  if (text_control_.IsHorizontalWritingMode()) {
    contents_rect.SetY(
        (text_control_.Size().Height() - contents_rect.Height()) / 2);
  } else {
    contents_rect.SetX((text_control_.Size().Width() - contents_rect.Width()) /
                       2);
  }

  // Convert the rect into the coords used for painting the content.
  AdjustPaintOffsetScope adjustment(text_control_, paint_info);
  const auto& local_paint_info = adjustment.GetPaintInfo();
  contents_rect.MoveBy(adjustment.PaintOffset());
  IntRect snapped_rect = PixelSnappedIntRect(contents_rect);
  LayoutTheme::GetTheme().Painter().PaintCapsLockIndicator(
      text_control_, local_paint_info, snapped_rect);
}

}  // namespace blink
