// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/graphics/paint/paint_artifact.h"

#include "cc/paint/display_item_list.h"
#include "third_party/blink/renderer/platform/geometry/int_rect.h"
#include "third_party/blink/renderer/platform/graphics/compositing/paint_chunks_to_cc_layer.h"
#include "third_party/blink/renderer/platform/graphics/graphics_layer.h"
#include "third_party/blink/renderer/platform/graphics/paint/drawing_display_item.h"
#include "third_party/blink/renderer/platform/graphics/paint/geometry_mapper.h"
#include "third_party/blink/renderer/platform/instrumentation/tracing/trace_event.h"
#include "third_party/skia/include/core/SkRegion.h"

namespace blink {

namespace {

void ComputeChunkBoundsAndOpaqueness(const DisplayItemList& display_items,
                                     Vector<PaintChunk>& paint_chunks) {
  for (PaintChunk& chunk : paint_chunks) {
    // This happens in tests testing paint chunks without display items.
    if (!chunk.size())
      continue;

    FloatRect bounds;
    SkRegion known_to_be_opaque_region;
    for (const DisplayItem& item : display_items.ItemsInPaintChunk(chunk)) {
      bounds.Unite(item.VisualRect());
      if (!RuntimeEnabledFeatures::SlimmingPaintV2Enabled() ||
          !item.IsDrawing())
        continue;
      const auto& drawing = static_cast<const DrawingDisplayItem&>(item);
      if (drawing.GetPaintRecord() && drawing.KnownToBeOpaque()) {
        known_to_be_opaque_region.op(
            SkIRect(EnclosedIntRect(drawing.VisualRect())),
            SkRegion::kUnion_Op);
      }
    }
    chunk.bounds = bounds;
    if (known_to_be_opaque_region.contains(EnclosingIntRect(bounds)))
      chunk.known_to_be_opaque = true;
  }
}

}  // namespace

PaintArtifact::PaintArtifact() : display_item_list_(0) {}

PaintArtifact::PaintArtifact(DisplayItemList display_items,
                             Vector<PaintChunk> chunks)
    : display_item_list_(std::move(display_items)), chunks_(std::move(chunks)) {
  ComputeChunkBoundsAndOpaqueness(display_item_list_, chunks_);
}

PaintArtifact::~PaintArtifact() = default;

scoped_refptr<PaintArtifact> PaintArtifact::Create(
    DisplayItemList display_items,
    Vector<PaintChunk> chunks) {
  return base::AdoptRef(
      new PaintArtifact(std::move(display_items), std::move(chunks)));
}

scoped_refptr<PaintArtifact> PaintArtifact::Empty() {
  DEFINE_STATIC_REF(PaintArtifact, empty, base::AdoptRef(new PaintArtifact()));
  return empty;
}

size_t PaintArtifact::ApproximateUnsharedMemoryUsage() const {
  size_t total_size = sizeof(*this) + display_item_list_.MemoryUsageInBytes() +
                      chunks_.capacity() * sizeof(chunks_[0]);
  for (const auto& chunk : chunks_)
    total_size += chunk.MemoryUsageInBytes();
  return total_size;
}

void PaintArtifact::Replay(GraphicsContext& graphics_context,
                           const PropertyTreeState& replay_state,
                           const IntPoint& offset) const {
  Replay(*graphics_context.Canvas(), replay_state, offset);
}

void PaintArtifact::Replay(cc::PaintCanvas& canvas,
                           const PropertyTreeState& replay_state,
                           const IntPoint& offset) const {
  TRACE_EVENT0("blink,benchmark", "PaintArtifact::replay");
  scoped_refptr<cc::DisplayItemList> display_item_list =
      PaintChunksToCcLayer::Convert(
          PaintChunks(), replay_state, gfx::Vector2dF(offset.X(), offset.Y()),
          GetDisplayItemList(),
          cc::DisplayItemList::kToBeReleasedAsPaintOpBuffer);
  canvas.drawPicture(display_item_list->ReleaseAsRecord());
}

DISABLE_CFI_PERF
void PaintArtifact::AppendToDisplayItemList(const FloatSize& visual_rect_offset,
                                            cc::DisplayItemList& list) const {
  TRACE_EVENT0("blink,benchmark", "PaintArtifact::AppendToDisplayItemList");
  for (const DisplayItem& item : display_item_list_)
    item.AppendToDisplayItemList(visual_rect_offset, list);
}

void PaintArtifact::FinishCycle() {
  for (auto& chunk : chunks_) {
    chunk.client_is_just_created = false;
    chunk.properties.ClearChangedToRoot();
  }
}

}  // namespace blink
