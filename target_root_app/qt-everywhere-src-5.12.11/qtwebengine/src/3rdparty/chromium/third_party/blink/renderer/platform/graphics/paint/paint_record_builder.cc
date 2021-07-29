// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/graphics/paint/paint_record_builder.h"

#include "third_party/blink/renderer/platform/geometry/float_rect.h"
#include "third_party/blink/renderer/platform/graphics/graphics_context.h"
#include "third_party/blink/renderer/platform/graphics/paint/paint_controller.h"

namespace blink {

PaintRecordBuilder::PaintRecordBuilder(SkMetaData* meta_data,
                                       GraphicsContext* containing_context,
                                       PaintController* paint_controller)
    : paint_controller_(nullptr) {
  GraphicsContext::DisabledMode disabled_mode =
      GraphicsContext::kNothingDisabled;
  if (containing_context && containing_context->ContextDisabled())
    disabled_mode = GraphicsContext::kFullyDisabled;

  if (paint_controller) {
    paint_controller_ = paint_controller;
  } else {
    own_paint_controller_ =
        PaintController::Create(PaintController::kTransient);
    paint_controller_ = own_paint_controller_.get();
  }

  paint_controller_->UpdateCurrentPaintChunkProperties(
      base::nullopt, PropertyTreeState::Root());

  const HighContrastSettings* high_contrast_settings =
      containing_context ? &containing_context->high_contrast_settings()
                         : nullptr;
  context_ = std::make_unique<GraphicsContext>(*paint_controller_,
                                               disabled_mode, meta_data);
  if (high_contrast_settings)
    context_->SetHighContrast(*high_contrast_settings);

  if (containing_context) {
    context_->SetDeviceScaleFactor(containing_context->DeviceScaleFactor());
    context_->SetPrinting(containing_context->Printing());
  }
}

PaintRecordBuilder::~PaintRecordBuilder() = default;

sk_sp<PaintRecord> PaintRecordBuilder::EndRecording(
    const PropertyTreeState& replay_state) {
  context_->BeginRecording(FloatRect());
  paint_controller_->CommitNewDisplayItems();
  paint_controller_->FinishCycle();
  paint_controller_->GetPaintArtifact().Replay(*context_, replay_state);
  return context_->EndRecording();
}

void PaintRecordBuilder::EndRecording(cc::PaintCanvas& canvas,
                                      const PropertyTreeState& replay_state) {
  paint_controller_->CommitNewDisplayItems();
  paint_controller_->FinishCycle();
  paint_controller_->GetPaintArtifact().Replay(canvas, replay_state);
}

}  // namespace blink
