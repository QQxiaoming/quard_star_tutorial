// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/adjust_paint_offset_scope.h"

namespace blink {

void AdjustPaintOffsetScope::AdjustForPaintOffsetTranslation(
    const LayoutObject& object,
    const TransformPaintPropertyNode* paint_offset_translation) {
  if (input_paint_info_.context.InDrawingRecorder()) {
    // If we are recording drawings, we should issue the translation as a raw
    // paint operation instead of paint chunk properties. One case is that we
    // are painting table row background behind a cell having paint offset
    // translation.
    input_paint_info_.context.Save();
    FloatSize translation =
        paint_offset_translation->Matrix().To2DTranslation();
    input_paint_info_.context.Translate(translation.Width(),
                                        translation.Height());
    paint_offset_translation_as_drawing_ = true;
  } else {
    chunk_properties_.emplace(
        input_paint_info_.context.GetPaintController(),
        paint_offset_translation, object,
        DisplayItem::PaintPhaseToDrawingType(input_paint_info_.phase));
  }

  adjusted_paint_info_.emplace(input_paint_info_);
  adjusted_paint_info_->UpdateCullRect(
      paint_offset_translation->Matrix().ToAffineTransform());
}

void AdjustPaintOffsetScope::FinishPaintOffsetTranslationAsDrawing() {
  // This scope should not interlace with scopes of DrawingRecorders.
  DCHECK(paint_offset_translation_as_drawing_);
  DCHECK(input_paint_info_.context.InDrawingRecorder());
  input_paint_info_.context.Restore();
}

}  // namespace blink
