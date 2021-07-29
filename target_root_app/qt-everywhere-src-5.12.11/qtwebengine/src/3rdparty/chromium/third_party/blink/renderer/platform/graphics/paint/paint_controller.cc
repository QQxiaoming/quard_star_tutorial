// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/graphics/paint/paint_controller.h"

#include <memory>
#include "base/auto_reset.h"
#include "third_party/blink/renderer/platform/graphics/logging_canvas.h"
#include "third_party/blink/renderer/platform/graphics/paint/drawing_display_item.h"
#include "third_party/blink/renderer/platform/instrumentation/tracing/trace_event.h"

namespace blink {

PaintController::PaintController(Usage usage)
    : usage_(usage),
      current_paint_artifact_(PaintArtifact::Empty()),
      new_display_item_list_(0) {
  // frame_first_paints_ should have one null frame since the beginning, so
  // that PaintController is robust even if it paints outside of BeginFrame
  // and EndFrame cycles. It will also enable us to combine the first paint
  // data in this PaintController into another PaintController on which we
  // replay the recorded results in the future.
  frame_first_paints_.push_back(FrameFirstPaint(nullptr));
}

PaintController::~PaintController() {
  // New display items should be committed before PaintController is destroyed.
  DCHECK(new_display_item_list_.IsEmpty());
}

bool PaintController::UseCachedDrawingIfPossible(
    const DisplayItemClient& client,
    DisplayItem::Type type) {
  DCHECK(DisplayItem::IsDrawingType(type));

  if (usage_ == kTransient)
    return false;

  if (DisplayItemConstructionIsDisabled())
    return false;

  if (!ClientCacheIsValid(client))
    return false;

  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled() &&
      IsCheckingUnderInvalidation()) {
    // We are checking under-invalidation of a subsequence enclosing this
    // display item. Let the client continue to actually paint the display item.
    return false;
  }

  size_t cached_item =
      FindCachedItem(DisplayItem::Id(client, type, current_fragment_));
  if (cached_item == kNotFound) {
    // See FindOutOfOrderCachedItemForward() for explanation of the situation.
    return false;
  }

  ++num_cached_new_items_;
  EnsureNewDisplayItemListInitialCapacity();
  // Visual rect can change without needing invalidation of the client, e.g.
  // when ancestor clip changes. Update the visual rect to the current value.
  current_paint_artifact_->GetDisplayItemList()[cached_item].UpdateVisualRect();
  if (!RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled())
    ProcessNewItem(MoveItemFromCurrentListToNewList(cached_item));

  next_item_to_match_ = cached_item + 1;
  // Items before |next_item_to_match_| have been copied so we don't need to
  // index them.
  if (next_item_to_match_ > next_item_to_index_)
    next_item_to_index_ = next_item_to_match_;

  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled()) {
    if (!IsCheckingUnderInvalidation()) {
      under_invalidation_checking_begin_ = cached_item;
      under_invalidation_checking_end_ = cached_item + 1;
      under_invalidation_message_prefix_ = "";
    }
    // Return false to let the painter actually paint. We will check if the new
    // painting is the same as the cached one.
    return false;
  }

  return true;
}

bool PaintController::UseCachedSubsequenceIfPossible(
    const DisplayItemClient& client) {
  if (usage_ == kTransient)
    return false;

  if (DisplayItemConstructionIsDisabled() || SubsequenceCachingIsDisabled())
    return false;

  if (!ClientCacheIsValid(client))
    return false;

  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled() &&
      IsCheckingUnderInvalidation()) {
    // We are checking under-invalidation of an ancestor subsequence enclosing
    // this one. The ancestor subsequence is supposed to have already "copied",
    // so we should let the client continue to actually paint the descendant
    // subsequences without "copying".
    return false;
  }

  SubsequenceMarkers* markers = GetSubsequenceMarkers(client);
  if (!markers) {
    return false;
  }

  if (current_paint_artifact_->GetDisplayItemList()[markers->start]
          .IsTombstone()) {
    // The subsequence has already been copied, indicating that the same client
    // created multiple subsequences. If DCHECK_IS_ON(), then we should have
    // encountered the DCHECK at the end of EndSubsequence() during the previous
    // paint.
    NOTREACHED();
    return false;
  }

  EnsureNewDisplayItemListInitialCapacity();

  if (next_item_to_match_ == markers->start) {
    // We are matching new and cached display items sequentially. Skip the
    // subsequence for later sequential matching of individual display items.
    next_item_to_match_ = markers->end;
    // Items before |next_item_to_match_| have been copied so we don't need to
    // index them.
    if (next_item_to_match_ > next_item_to_index_)
      next_item_to_index_ = next_item_to_match_;
  }

  num_cached_new_items_ += markers->end - markers->start;

  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled()) {
    DCHECK(!IsCheckingUnderInvalidation());
    under_invalidation_checking_begin_ = markers->start;
    under_invalidation_checking_end_ = markers->end;
    under_invalidation_message_prefix_ =
        "(In cached subsequence for " + client.DebugName() + ")";
    // Return false to let the painter actually paint. We will check if the new
    // painting is the same as the cached one.
    return false;
  }

  size_t start = BeginSubsequence();
  CopyCachedSubsequence(markers->start, markers->end);
  EndSubsequence(client, start);
  return true;
}

PaintController::SubsequenceMarkers* PaintController::GetSubsequenceMarkers(
    const DisplayItemClient& client) {
  auto result = current_cached_subsequences_.find(&client);
  if (result == current_cached_subsequences_.end())
    return nullptr;
  return &result->value;
}

size_t PaintController::BeginSubsequence() {
  // Force new paint chunk which is required for subsequence caching.
  new_paint_chunks_.ForceNewChunk();
  return new_display_item_list_.size();
}

void PaintController::EndSubsequence(const DisplayItemClient& client,
                                     size_t start) {
  size_t end = new_display_item_list_.size();

  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled() &&
      IsCheckingUnderInvalidation()) {
    SubsequenceMarkers* markers = GetSubsequenceMarkers(client);
    if (!markers && start != end) {
      ShowSequenceUnderInvalidationError(
          "under-invalidation : unexpected subsequence", client, start, end);
      CHECK(false);
    }
    if (markers && markers->end - markers->start != end - start) {
      ShowSequenceUnderInvalidationError(
          "under-invalidation: new subsequence wrong length", client, start,
          end);
      CHECK(false);
    }
  }

  if (start == end) {
    // Omit the empty subsequence. The forcing-new-chunk flag set by
    // BeginSubsequence() still applies, but this not a big deal because empty
    // subsequences are not common. Also we should not clear the flag because
    // there might be unhandled flag that was set before this empty subsequence.
    return;
  }

  // Force new paint chunk which is required for subsequence caching.
  new_paint_chunks_.ForceNewChunk();

  DCHECK(!new_cached_subsequences_.Contains(&client))
      << "Multiple subsequences for client: " << client.DebugName();

  new_cached_subsequences_.insert(&client, SubsequenceMarkers(start, end));
}

const DisplayItem* PaintController::LastDisplayItem(unsigned offset) {
  if (offset < new_display_item_list_.size())
    return &new_display_item_list_[new_display_item_list_.size() - offset - 1];
  return nullptr;
}

void PaintController::ProcessNewItem(DisplayItem& display_item) {
  DCHECK(!construction_disabled_);

  if (IsSkippingCache()) {
    DCHECK_EQ(usage_, kMultiplePaints);
    display_item.Client().Invalidate(PaintInvalidationReason::kUncacheable);
  }

#if DCHECK_IS_ON()
  bool chunk_added =
#endif
      new_paint_chunks_.IncrementDisplayItemIndex(display_item);
  auto& last_chunk = new_paint_chunks_.LastChunk();

#if DCHECK_IS_ON()
  if (chunk_added && last_chunk.is_cacheable) {
    AddToIndicesByClientMap(last_chunk.id.client,
                            new_paint_chunks_.LastChunkIndex(),
                            new_paint_chunk_indices_by_client_);
  }
#endif

  last_chunk.outset_for_raster_effects =
      std::max(last_chunk.outset_for_raster_effects,
               display_item.OutsetForRasterEffects());

#if DCHECK_IS_ON()
  if (usage_ == kMultiplePaints && !IsSkippingCache()) {
    size_t index = FindMatchingItemFromIndex(
        display_item.GetId(), new_display_item_indices_by_client_,
        new_display_item_list_);
    if (index != kNotFound) {
      ShowDebugData();
      NOTREACHED()
          << "DisplayItem " << display_item.AsDebugString().Utf8().data()
          << " has duplicated id with previous "
          << new_display_item_list_[index].AsDebugString().Utf8().data()
          << " (index=" << index << ")";
    }
    AddToIndicesByClientMap(display_item.Client(),
                            new_display_item_list_.size() - 1,
                            new_display_item_indices_by_client_);
  }
#endif  // DCHECK_IS_ON()

  if (usage_ == kMultiplePaints &&
      RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled())
    CheckUnderInvalidation();

  if (!frame_first_paints_.back().first_painted && display_item.IsDrawing() &&
      // Here we ignore all document-background paintings because we don't
      // know if the background is default. ViewPainter should have called
      // setFirstPainted() if this display item is for non-default
      // background.
      display_item.GetType() != DisplayItem::kDocumentBackground &&
      display_item.DrawsContent()) {
    SetFirstPainted();
  }
}

DisplayItem& PaintController::MoveItemFromCurrentListToNewList(size_t index) {
  return new_display_item_list_.AppendByMoving(
      current_paint_artifact_->GetDisplayItemList()[index]);
}

void PaintController::InvalidateAll() {
  DCHECK(!RuntimeEnabledFeatures::SlimmingPaintV2Enabled());
  InvalidateAllInternal();
}

void PaintController::InvalidateAllInternal() {
  // TODO(wangxianzhu): Rename this to InvalidateAllForTesting() for SPv2.
  // Can only be called during layout/paintInvalidation, not during painting.
  DCHECK(new_display_item_list_.IsEmpty());
  current_paint_artifact_ = PaintArtifact::Empty();
  current_cached_subsequences_.clear();
  cache_is_all_invalid_ = true;
}

bool PaintController::CacheIsAllInvalid() const {
  DCHECK(!RuntimeEnabledFeatures::SlimmingPaintV2Enabled());
  DCHECK(!cache_is_all_invalid_ || current_paint_artifact_->IsEmpty());
  return cache_is_all_invalid_;
}

bool PaintController::ClientCacheIsValid(
    const DisplayItemClient& client) const {
#if DCHECK_IS_ON()
  DCHECK(client.IsAlive());
#endif
  if (IsSkippingCache() || cache_is_all_invalid_)
    return false;
  return client.IsValid();
}

size_t PaintController::FindMatchingItemFromIndex(
    const DisplayItem::Id& id,
    const IndicesByClientMap& display_item_indices_by_client,
    const DisplayItemList& list) {
  IndicesByClientMap::const_iterator it =
      display_item_indices_by_client.find(&id.client);
  if (it == display_item_indices_by_client.end())
    return kNotFound;

  const Vector<size_t>& indices = it->value;
  for (size_t index : indices) {
    const DisplayItem& existing_item = list[index];
    if (existing_item.IsTombstone())
      continue;
    DCHECK(existing_item.Client() == id.client);
    if (id == existing_item.GetId())
      return index;
  }

  return kNotFound;
}

void PaintController::AddToIndicesByClientMap(const DisplayItemClient& client,
                                              size_t index,
                                              IndicesByClientMap& map) {
  auto it = map.find(&client);
  auto& indices =
      it == map.end()
          ? map.insert(&client, Vector<size_t>()).stored_value->value
          : it->value;
  indices.push_back(index);
}

size_t PaintController::FindCachedItem(const DisplayItem::Id& id) {
  DCHECK(ClientCacheIsValid(id.client));

  if (next_item_to_match_ <
      current_paint_artifact_->GetDisplayItemList().size()) {
    // If current_list[next_item_to_match_] matches the new item, we don't need
    // to update and lookup the index, which is fast. This is the common case
    // that the current list and the new list are in the same order around the
    // new item.
    const DisplayItem& item =
        current_paint_artifact_->GetDisplayItemList()[next_item_to_match_];
    // We encounter an item that has already been copied which indicates we
    // can't do sequential matching.
    if (!item.IsTombstone() && id == item.GetId()) {
#if DCHECK_IS_ON()
      ++num_sequential_matches_;
#endif
      return next_item_to_match_;
    }
  }

  size_t found_index =
      FindMatchingItemFromIndex(id, out_of_order_item_indices_,
                                current_paint_artifact_->GetDisplayItemList());
  if (found_index != kNotFound) {
#if DCHECK_IS_ON()
    ++num_out_of_order_matches_;
#endif
    return found_index;
  }

  return FindOutOfOrderCachedItemForward(id);
}

// Find forward for the item and index all skipped indexable items.
size_t PaintController::FindOutOfOrderCachedItemForward(
    const DisplayItem::Id& id) {
  for (size_t i = next_item_to_index_;
       i < current_paint_artifact_->GetDisplayItemList().size(); ++i) {
    const DisplayItem& item = current_paint_artifact_->GetDisplayItemList()[i];
    if (item.IsTombstone())
      continue;
    if (id == item.GetId()) {
#if DCHECK_IS_ON()
      ++num_sequential_matches_;
#endif
      return i;
    }
    if (item.IsCacheable()) {
#if DCHECK_IS_ON()
      ++num_indexed_items_;
#endif
      AddToIndicesByClientMap(item.Client(), i, out_of_order_item_indices_);
      next_item_to_index_ = i + 1;
    }
  }

  // The display item newly appears while the client is not invalidated. The
  // situation alone (without other kinds of under-invalidations) won't corrupt
  // rendering, but causes AddItemToIndexIfNeeded() for all remaining display
  // item, which is not the best for performance. In this case, the caller
  // should fall back to repaint the display item.
  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled()) {
#if DCHECK_IS_ON()
    ShowDebugData();
#endif
    // Ensure our paint invalidation tests don't trigger the less performant
    // situation which should be rare.
    LOG(WARNING) << "Can't find cached display item: " << id.client.DebugName()
                 << " " << id.ToString();
  }
  return kNotFound;
}

// Copies a cached subsequence from current list to the new list.
// When paintUnderInvaldiationCheckingEnabled() we'll not actually
// copy the subsequence, but mark the begin and end of the subsequence for
// under-invalidation checking.
void PaintController::CopyCachedSubsequence(size_t begin_index,
                                            size_t end_index) {
  DCHECK(!RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled());

  const DisplayItem* cached_item =
      &current_paint_artifact_->GetDisplayItemList()[begin_index];

  auto* cached_chunk =
      current_paint_artifact_->FindChunkByDisplayItemIndex(begin_index);
  DCHECK(cached_chunk != current_paint_artifact_->PaintChunks().end());
  auto properties_before_subsequence =
      new_paint_chunks_.CurrentPaintChunkProperties();
  UpdateCurrentPaintChunkPropertiesUsingIdWithFragment(
      cached_chunk->id, cached_chunk->properties.GetPropertyTreeState());

  for (size_t current_index = begin_index; current_index < end_index;
       ++current_index) {
    cached_item = &current_paint_artifact_->GetDisplayItemList()[current_index];
    SECURITY_CHECK(!cached_item->IsTombstone());
#if DCHECK_IS_ON()
    DCHECK(cached_item->Client().IsAlive());
#endif

    if (current_index == cached_chunk->end_index) {
      ++cached_chunk;
      DCHECK(cached_chunk != current_paint_artifact_->PaintChunks().end());
      new_paint_chunks_.ForceNewChunk();
      UpdateCurrentPaintChunkPropertiesUsingIdWithFragment(
          cached_chunk->id, cached_chunk->properties.GetPropertyTreeState());
    }

#if DCHECK_IS_ON()
    // Visual rect change should not happen in a cached subsequence.
    // However, because of different method of pixel snapping in different
    // paths, there are false positives. Just log an error.
    if (cached_item->VisualRect() !=
        FloatRect(cached_item->Client().VisualRect())) {
      LOG(ERROR) << "Visual rect changed in a cached subsequence: "
                 << cached_item->Client().DebugName()
                 << " old=" << cached_item->VisualRect().ToString()
                 << " new=" << cached_item->Client().VisualRect().ToString();
    }
#endif

    auto& item = MoveItemFromCurrentListToNewList(current_index);
    item.SetCopiedFromCachedSubsequence(true);
    ProcessNewItem(item);
    DCHECK((!new_paint_chunks_.LastChunk().is_cacheable &&
            !cached_chunk->is_cacheable) ||
           new_paint_chunks_.LastChunk().Matches(*cached_chunk));
  }

  if (RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled()) {
    under_invalidation_checking_end_ = end_index;
    DCHECK(IsCheckingUnderInvalidation());
  } else {
    // Restore properties and force new chunk for any trailing display items
    // after the cached subsequence without new properties.
    new_paint_chunks_.ForceNewChunk();
    UpdateCurrentPaintChunkProperties(base::nullopt,
                                      properties_before_subsequence);
  }
}

void PaintController::ResetCurrentListIndices() {
  next_item_to_match_ = 0;
  next_item_to_index_ = 0;
  under_invalidation_checking_begin_ = 0;
  under_invalidation_checking_end_ = 0;
}

DISABLE_CFI_PERF
void PaintController::CommitNewDisplayItems() {
  TRACE_EVENT2("blink,benchmark", "PaintController::commitNewDisplayItems",
               "current_display_list_size",
               (int)current_paint_artifact_->GetDisplayItemList().size(),
               "num_non_cached_new_items",
               (int)new_display_item_list_.size() - num_cached_new_items_);

  num_cached_new_items_ = 0;
#if DCHECK_IS_ON()
  new_display_item_indices_by_client_.clear();
  new_paint_chunk_indices_by_client_.clear();
#endif

  cache_is_all_invalid_ = false;
  committed_ = true;

  new_cached_subsequences_.swap(current_cached_subsequences_);
  new_cached_subsequences_.clear();

  // The new list will not be appended to again so we can release unused memory.
  new_display_item_list_.ShrinkToFit();

  current_paint_artifact_ =
      PaintArtifact::Create(std::move(new_display_item_list_),
                            new_paint_chunks_.ReleasePaintChunks());

  ResetCurrentListIndices();
  out_of_order_item_indices_.clear();

  // We'll allocate the initial buffer when we start the next paint.
  new_display_item_list_ = DisplayItemList(0);

#if DCHECK_IS_ON()
  num_sequential_matches_ = 0;
  num_out_of_order_matches_ = 0;
  num_indexed_items_ = 0;
#endif
}

void PaintController::FinishCycle() {
  if (usage_ == kTransient)
    return;

  DCHECK(new_display_item_list_.IsEmpty());
  DCHECK(new_paint_chunks_.IsInInitialState());

  if (committed_) {
    committed_ = false;

    // Validate display item clients that have validly cached subsequence or
    // display items in this PaintController.
    for (auto& item : current_cached_subsequences_) {
      if (item.key->IsCacheable())
        item.key->Validate();
    }
    for (const auto& item : current_paint_artifact_->GetDisplayItemList()) {
      const auto& client = item.Client();

      if (item.IsCopiedFromCachedSubsequence()) {
        // We don't need to validate the clients of a display item that is
        // copied from a cached subsequence, because it should be already
        // valid. See http://crbug.com/1050090 for more details.
#if DCHECK_IS_ON()
        DCHECK(client.IsAlive());
        DCHECK(client.IsValid() || !client.IsCacheable());
#endif
        continue;
      }
      client.ClearPartialInvalidationVisualRect();
      if (client.IsCacheable())
        client.Validate();
    }
    for (const auto& chunk : current_paint_artifact_->PaintChunks()) {
        const auto& client = chunk.id.client;
        if (client.IsCacheable())
          client.Validate();
    }
  }

  current_paint_artifact_->FinishCycle();

#if DCHECK_IS_ON()
  if (VLOG_IS_ON(2)) {
    LOG(ERROR) << "PaintController::FinishCycle() done";
    if (VLOG_IS_ON(3))
      ShowDebugDataWithRecords();
    else
      ShowDebugData();
  }
#endif
}

size_t PaintController::ApproximateUnsharedMemoryUsage() const {
  size_t memory_usage = sizeof(*this);

  // Memory outside this class due to current_paint_artifact_.
  memory_usage += current_paint_artifact_->ApproximateUnsharedMemoryUsage();

  // External objects, shared with the embedder, such as PaintRecord, should be
  // excluded to avoid double counting. It is the embedder's responsibility to
  // count such objects.

  // Memory outside this class due to new_display_item_list_.
  DCHECK(new_display_item_list_.IsEmpty());
  memory_usage += new_display_item_list_.MemoryUsageInBytes();

  // Memory outside this class due to current_cached_subsequences_ and
  // new_cached_subsequences_.
  memory_usage += current_cached_subsequences_.Capacity() *
                  sizeof(*current_cached_subsequences_.begin());
  DCHECK(new_cached_subsequences_.IsEmpty());
  memory_usage += new_cached_subsequences_.Capacity() *
                  sizeof(*new_cached_subsequences_.begin());

  return memory_usage;
}

namespace {

class DebugDrawingClient final : public DisplayItemClient {
 public:
  DebugDrawingClient() { Invalidate(PaintInvalidationReason::kUncacheable); }
  String DebugName() const final { return "DebugDrawing"; }
  LayoutRect VisualRect() const final {
    return LayoutRect(LayoutRect::InfiniteIntRect());
  }
};

}  // anonymous namespace

void PaintController::AppendDebugDrawingAfterCommit(
    sk_sp<const PaintRecord> record,
    const PropertyTreeState& property_tree_state) {
  DEFINE_STATIC_LOCAL(DebugDrawingClient, debug_drawing_client, ());

  DCHECK(!RuntimeEnabledFeatures::SlimmingPaintV2Enabled());
  DCHECK(new_display_item_list_.IsEmpty());
  auto& display_item_list = current_paint_artifact_->GetDisplayItemList();
  auto& display_item =
      display_item_list.AllocateAndConstruct<DrawingDisplayItem>(
          debug_drawing_client, DisplayItem::kDebugDrawing, std::move(record));

  // Create a PaintChunk for the debug drawing.
  current_paint_artifact_->PaintChunks().emplace_back(
      display_item_list.size() - 1, display_item_list.size(),
      display_item.GetId(), property_tree_state);
}

void PaintController::ShowUnderInvalidationError(
    const char* reason,
    const DisplayItem& new_item,
    const DisplayItem* old_item) const {
  LOG(ERROR) << under_invalidation_message_prefix_ << " " << reason;
#if DCHECK_IS_ON()
  LOG(ERROR) << "New display item: " << new_item.AsDebugString();
  LOG(ERROR) << "Old display item: "
             << (old_item ? old_item->AsDebugString() : "None");
  LOG(ERROR) << "See http://crbug.com/619103.";

  const PaintRecord* new_record = nullptr;
  if (new_item.IsDrawing()) {
    new_record =
        static_cast<const DrawingDisplayItem&>(new_item).GetPaintRecord().get();
  }
  const PaintRecord* old_record = nullptr;
  if (old_item->IsDrawing()) {
    old_record = static_cast<const DrawingDisplayItem*>(old_item)
                     ->GetPaintRecord()
                     .get();
  }
  LOG(INFO) << "new record:\n"
            << (new_record ? RecordAsDebugString(*new_record).Utf8().data()
                           : "None");
  LOG(INFO) << "old record:\n"
            << (old_record ? RecordAsDebugString(*old_record).Utf8().data()
                           : "None");

  ShowDebugData();
#else
  LOG(ERROR) << "Run a build with DCHECK on to get more details.";
  LOG(ERROR) << "See http://crbug.com/619103.";
#endif
}

void PaintController::ShowSequenceUnderInvalidationError(
    const char* reason,
    const DisplayItemClient& client,
    int start,
    int end) {
  LOG(ERROR) << under_invalidation_message_prefix_ << " " << reason;
  LOG(ERROR) << "Subsequence client: " << client.DebugName();
#if DCHECK_IS_ON()
  ShowDebugData();
#else
  LOG(ERROR) << "Run a build with DCHECK on to get more details.";
#endif
  LOG(ERROR) << "See http://crbug.com/619103.";
}

void PaintController::CheckUnderInvalidation() {
  DCHECK_EQ(usage_, kMultiplePaints);
  DCHECK(RuntimeEnabledFeatures::PaintUnderInvalidationCheckingEnabled());

  if (!IsCheckingUnderInvalidation())
    return;

  if (IsSkippingCache()) {
    // We allow cache skipping and temporary under-invalidation in cached
    // subsequences. See the usage of DisplayItemCacheSkipper in BoxPainter.
    under_invalidation_checking_end_ = 0;
    // Match the remaining display items in the subsequence normally.
    next_item_to_match_ = next_item_to_index_ =
        under_invalidation_checking_begin_;
    return;
  }

  const DisplayItem& new_item = new_display_item_list_.Last();
  size_t old_item_index = under_invalidation_checking_begin_;
  DisplayItem* old_item =
      old_item_index < current_paint_artifact_->GetDisplayItemList().size()
          ? &current_paint_artifact_->GetDisplayItemList()[old_item_index]
          : nullptr;

  if (!old_item || !new_item.Equals(*old_item)) {
    // If we ever skipped reporting any under-invalidations, report the earliest
    // one.
    ShowUnderInvalidationError(
        "under-invalidation: display item changed",
        new_display_item_list_.Last(),
        &current_paint_artifact_
             ->GetDisplayItemList()[under_invalidation_checking_begin_]);
    CHECK(false);
  }

  // Discard the forced repainted display item and move the cached item into
  // new_display_item_list_. This is to align with the
  // non-under-invalidation-checking path to empty the original cached slot,
  // leaving only disappeared or invalidated display items in the old list after
  // painting.
  new_display_item_list_.RemoveLast();
  MoveItemFromCurrentListToNewList(old_item_index);

  ++under_invalidation_checking_begin_;
}

void PaintController::SetFirstPainted() {
  frame_first_paints_.back().first_painted = true;
}

void PaintController::SetTextPainted() {
  frame_first_paints_.back().text_painted = true;
}

void PaintController::SetImagePainted() {
  frame_first_paints_.back().image_painted = true;
}

void PaintController::BeginFrame(const void* frame) {
  frame_first_paints_.push_back(FrameFirstPaint(frame));
}

FrameFirstPaint PaintController::EndFrame(const void* frame) {
  FrameFirstPaint result = frame_first_paints_.back();
  DCHECK(result.frame == frame);
  frame_first_paints_.pop_back();
  return result;
}

#if DCHECK_IS_ON()
void PaintController::CheckDuplicatePaintChunkId(const PaintChunk::Id& id) {
  if (IsSkippingCache())
    return;

  auto it = new_paint_chunk_indices_by_client_.find(&id.client);
  if (it != new_paint_chunk_indices_by_client_.end()) {
    const auto& indices = it->value;
    for (auto index : indices) {
      const auto& chunk = new_paint_chunks_.PaintChunkAt(index);
      if (chunk.id == id) {
        ShowDebugData();
        DLOG(ERROR)  << "New paint chunk id " << id.ToString().Utf8().data()
                     << " has duplicated id with previous chuck "
                     << chunk.ToString().Utf8().data();
      }
    }
  }
}
#endif

}  // namespace blink
