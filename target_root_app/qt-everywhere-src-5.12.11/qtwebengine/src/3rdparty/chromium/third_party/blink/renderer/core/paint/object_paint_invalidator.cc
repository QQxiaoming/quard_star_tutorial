// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/object_paint_invalidator.h"

#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/frame/local_frame_view.h"
#include "third_party/blink/renderer/core/layout/layout_embedded_content.h"
#include "third_party/blink/renderer/core/layout/layout_view.h"
#include "third_party/blink/renderer/core/page/chrome_client.h"
#include "third_party/blink/renderer/core/paint/compositing/composited_layer_mapping.h"
#include "third_party/blink/renderer/core/paint/find_paint_offset_and_visual_rect_needing_update.h"
#include "third_party/blink/renderer/core/paint/ng/ng_paint_fragment.h"
#include "third_party/blink/renderer/core/paint/paint_invalidator.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/platform/graphics/graphics_layer.h"

namespace blink {

using LayoutObjectTraversalFunctor = std::function<void(const LayoutObject&)>;

static void TraverseNonCompositingDescendantsInPaintOrder(
    const LayoutObject&,
    const LayoutObjectTraversalFunctor&);

static void
TraverseNonCompositingDescendantsBelongingToAncestorPaintInvalidationContainer(
    const LayoutObject& object,
    const LayoutObjectTraversalFunctor& functor) {
  // |object| is a paint invalidation container, but is not a stacking context
  // or is a non-block, so the paint invalidation container of stacked
  // descendants may not belong to |object| but belong to an ancestor. This
  // function traverses all such descendants. See Case 1a and Case 2 below for
  // details.
  DCHECK(object.IsPaintInvalidationContainer() &&
         (!object.StyleRef().IsStackingContext() || !object.IsLayoutBlock()));

  LayoutObject* descendant = object.NextInPreOrder(&object);
  while (descendant) {
    if (!descendant->HasLayer() || !descendant->StyleRef().IsStacked()) {
      // Case 1: The descendant is not stacked (or is stacked but has not been
      // allocated a layer yet during style change), so either it's a paint
      // invalidation container in the same situation as |object|, or its paint
      // invalidation container is in such situation. Keep searching until a
      // stacked layer is found.
      if (!object.IsLayoutBlock() && descendant->IsFloating()) {
        // Case 1a (rare): However, if the descendant is a floating object below
        // a composited non-block object, the subtree may belong to an ancestor
        // in paint order, thus recur into the subtree. Note that for
        // performance, we don't check whether the floating object's container
        // is above or under |object|, so we may traverse more than expected.
        // Example:
        // <span id="object" class="position: relative; will-change: transform">
        //   <div id="descendant" class="float: left"></div>"
        // </span>
        TraverseNonCompositingDescendantsInPaintOrder(*descendant, functor);
        descendant = descendant->NextInPreOrderAfterChildren(&object);
      } else {
        descendant = descendant->NextInPreOrder(&object);
      }
    } else if (!descendant->IsPaintInvalidationContainer()) {
      // Case 2: The descendant is stacked and is not composited.
      // The invalidation container of its subtree is our ancestor,
      // thus recur into the subtree.
      TraverseNonCompositingDescendantsInPaintOrder(*descendant, functor);
      descendant = descendant->NextInPreOrderAfterChildren(&object);
    } else if (descendant->StyleRef().IsStackingContext() &&
               descendant->IsLayoutBlock()) {
      // Case 3: The descendant is an invalidation container and is a stacking
      // context.  No objects in the subtree can have invalidation container
      // outside of it, thus skip the whole subtree.
      // This excludes non-block because there might be floating objects under
      // the descendant belonging to some ancestor in paint order (Case 1a).
      descendant = descendant->NextInPreOrderAfterChildren(&object);
    } else {
      // Case 4: The descendant is an invalidation container but not a stacking
      // context, or the descendant is a non-block stacking context.
      // This is the same situation as |object|, thus keep searching.
      descendant = descendant->NextInPreOrder(&object);
    }
  }
}

static void TraverseNonCompositingDescendantsInPaintOrder(
    const LayoutObject& object,
    const LayoutObjectTraversalFunctor& functor) {
  functor(object);
  LayoutObject* descendant = object.NextInPreOrder(&object);
  while (descendant) {
    if (!descendant->IsPaintInvalidationContainer()) {
      functor(*descendant);
      descendant = descendant->NextInPreOrder(&object);
    } else if (descendant->StyleRef().IsStackingContext() &&
               descendant->IsLayoutBlock()) {
      // The descendant is an invalidation container and is a stacking context.
      // No objects in the subtree can have invalidation container outside of
      // it, thus skip the whole subtree.
      // This excludes non-blocks because there might be floating objects under
      // the descendant belonging to some ancestor in paint order (Case 1a).
      descendant = descendant->NextInPreOrderAfterChildren(&object);
    } else {
      // If a paint invalidation container is not a stacking context, or the
      // descendant is a non-block stacking context, some of its descendants may
      // belong to the parent container.
      TraverseNonCompositingDescendantsBelongingToAncestorPaintInvalidationContainer(
          *descendant, functor);
      descendant = descendant->NextInPreOrderAfterChildren(&object);
    }
  }
}

static void SetPaintingLayerNeedsRepaintDuringTraverse(
    const LayoutObject& object) {
  if (object.HasLayer() &&
      ToLayoutBoxModelObject(object).HasSelfPaintingLayer()) {
    ToLayoutBoxModelObject(object).Layer()->SetNeedsRepaint();
  } else if (object.IsFloating() && object.Parent() &&
             !object.Parent()->IsLayoutBlock()) {
    object.PaintingLayer()->SetNeedsRepaint();
  }
}

void ObjectPaintInvalidator::
    InvalidateDisplayItemClientsIncludingNonCompositingDescendants(
        PaintInvalidationReason reason) {
  // This is valid because we want to invalidate the client in the display item
  // list of the current backing.
  DisableCompositingQueryAsserts disabler;

  SlowSetPaintingLayerNeedsRepaint();
  TraverseNonCompositingDescendantsInPaintOrder(
      object_, [reason](const LayoutObject& object) {
        SetPaintingLayerNeedsRepaintDuringTraverse(object);
        object.InvalidateDisplayItemClients(reason);
      });
}

void ObjectPaintInvalidator::
    InvalidatePaintIncludingNonCompositingDescendants() {
  DCHECK(!RuntimeEnabledFeatures::SlimmingPaintV2Enabled());
  SlowSetPaintingLayerNeedsRepaint();
  // This method may be used to invalidate paint of objects changing paint
  // invalidation container. Clear previous visual rects on the original paint
  // invalidation container to avoid under-invalidation if the visual rect on
  // the new paint invalidation container happens to be the same as the old one.
  TraverseNonCompositingDescendantsInPaintOrder(
      object_, [](const LayoutObject& object) {
        SetPaintingLayerNeedsRepaintDuringTraverse(object);
        object.GetMutableForPainting().ClearPreviousVisualRects();
      });
}

void ObjectPaintInvalidator::
    InvalidatePaintIncludingNonSelfPaintingLayerDescendants() {
  SlowSetPaintingLayerNeedsRepaint();
  // This method may be used to invalidate paint of objects changing paint
  // invalidation container. Clear previous visual rect on the original paint
  // invalidation container to avoid under-invalidation if the visual rect on
  // the new paint invalidation container happens to be the same as the old one.
  std::function<void(const LayoutObject&)> traverse =
      [&traverse](const LayoutObject& object) {
        object.GetMutableForPainting().ClearPreviousVisualRects();
        for (LayoutObject* child = object.SlowFirstChild(); child;
             child = child->NextSibling()) {
          if (!child->HasLayer() ||
              !ToLayoutBoxModelObject(child)->Layer()->IsSelfPaintingLayer())
            traverse(*child);
        }
      };
  traverse(object_);
}

void ObjectPaintInvalidator::InvalidateDisplayItemClient(
    const DisplayItemClient& client,
    PaintInvalidationReason reason) {
  // It's caller's responsibility to ensure PaintingLayer's NeedsRepaint is set.
  // Don't set the flag here because getting PaintLayer has cost and the caller
  // can use various ways (e.g. PaintInvalidatinContext::painting_layer) to
  // reduce the cost.
  DCHECK(!object_.PaintingLayer() || object_.PaintingLayer()->NeedsRepaint());

  if (&client == &object_) {
    TRACE_EVENT_INSTANT1(
        TRACE_DISABLED_BY_DEFAULT("devtools.timeline.invalidationTracking"),
        "PaintInvalidationTracking", TRACE_EVENT_SCOPE_THREAD, "data",
        InspectorPaintInvalidationTrackingEvent::Data(object_));
  }

  client.Invalidate(reason);

  if (LocalFrameView* frame_view = object_.GetFrameView())
    frame_view->TrackObjectPaintInvalidation(client, reason);
}

void ObjectPaintInvalidator::SlowSetPaintingLayerNeedsRepaint() {
  if (PaintLayer* painting_layer = object_.PaintingLayer())
    painting_layer->SetNeedsRepaint();
}

DISABLE_CFI_PERF
PaintInvalidationReason
ObjectPaintInvalidatorWithContext::ComputePaintInvalidationReason() {
  // This is before any early return to ensure the background obscuration status
  // is saved.
  bool background_obscuration_changed = false;
  bool background_obscured = object_.BackgroundIsKnownToBeObscured();
  if (background_obscured != object_.PreviousBackgroundObscured()) {
    object_.GetMutableForPainting().SetPreviousBackgroundObscured(
        background_obscured);
    background_obscuration_changed = true;
  }

  if (!object_.ShouldCheckForPaintInvalidation() &&
      (!context_.subtree_flags ||
       context_.subtree_flags ==
           PaintInvalidatorContext::kSubtreeVisualRectUpdate)) {
    // No paint invalidation flag, or just kSubtreeVisualRectUpdate (which has
    // been handled in PaintInvalidator). No paint invalidation is needed.
    DCHECK(!background_obscuration_changed);
    return PaintInvalidationReason::kNone;
  }

  if (context_.subtree_flags &
      PaintInvalidatorContext::kSubtreeFullInvalidation)
    return PaintInvalidationReason::kSubtree;

  if (object_.ShouldDoFullPaintInvalidation())
    return object_.FullPaintInvalidationReason();

  if (!(context_.subtree_flags &
        PaintInvalidatorContext::kInvalidateEmptyVisualRect) &&
      context_.old_visual_rect.IsEmpty() &&
      context_.fragment_data->VisualRect().IsEmpty())
    return PaintInvalidationReason::kNone;

  if (background_obscuration_changed)
    return PaintInvalidationReason::kBackground;

  if (object_.PaintedOutputOfObjectHasNoEffectRegardlessOfSize())
    return PaintInvalidationReason::kNone;

  // Force full paint invalidation if the outline may be affected by descendants
  // and this object is marked for checking paint invalidation for any reason.
  if (object_.OutlineMayBeAffectedByDescendants() ||
      object_.PreviousOutlineMayBeAffectedByDescendants()) {
    object_.GetMutableForPainting()
        .UpdatePreviousOutlineMayBeAffectedByDescendants();
    return PaintInvalidationReason::kOutline;
  }

  // If the size is zero on one of our bounds then we know we're going to have
  // to do a full invalidation of either old bounds or new bounds.
  if (context_.old_visual_rect.IsEmpty())
    return PaintInvalidationReason::kAppeared;
  if (context_.fragment_data->VisualRect().IsEmpty())
    return PaintInvalidationReason::kDisappeared;

  // If we shifted, we don't know the exact reason so we are conservative and
  // trigger a full invalidation. Shifting could be caused by some layout
  // property (left / top) or some in-flow layoutObject inserted / removed
  // before us in the tree.
  if (context_.fragment_data->VisualRect().Location() !=
      context_.old_visual_rect.Location())
    return PaintInvalidationReason::kGeometry;

  // Most paintings are pixel-snapped so subpixel change of paint offset doesn't
  // directly cause full raster invalidation.
  if (RoundedIntPoint(context_.fragment_data->PaintOffset()) !=
      RoundedIntPoint(context_.old_paint_offset))
    return PaintInvalidationReason::kGeometry;

  // Incremental invalidation is only applicable to LayoutBoxes. Return
  // PaintInvalidationIncremental no matter if oldVisualRect and newVisualRect
  // are equal because a LayoutBox may need paint invalidation if its border box
  // changes. BoxPaintInvalidator may also override this reason with a full
  // paint invalidation reason if needed.
  if (object_.IsBox())
    return PaintInvalidationReason::kIncremental;

  if (context_.old_visual_rect != context_.fragment_data->VisualRect())
    return PaintInvalidationReason::kGeometry;

  return PaintInvalidationReason::kNone;
}

DISABLE_CFI_PERF
void ObjectPaintInvalidatorWithContext::InvalidateSelection(
    PaintInvalidationReason reason) {
  // In LayoutNG, if NGPaintFragment paints the selection, we invalidate for
  // selection change in PaintInvalidator.
  if (RuntimeEnabledFeatures::LayoutNGEnabled() && object_.IsInline() &&
      // LayoutReplaced still paints selection tint by itself.
      !object_.IsLayoutReplaced() &&
      NGPaintFragment::InlineFragmentsFor(&object_)
          .IsInLayoutNGInlineFormattingContext())
    return;

  // Update selection rect when we are doing full invalidation with geometry
  // change (in case that the object is moved, composite status changed, etc.)
  // or shouldInvalidationSelection is set (in case that the selection itself
  // changed).
  bool full_invalidation = IsImmediateFullPaintInvalidationReason(reason);
  if (!full_invalidation && !object_.ShouldInvalidateSelection())
    return;

  LayoutRect old_selection_rect = object_.SelectionVisualRect();
  LayoutRect new_selection_rect;
#if DCHECK_IS_ON()
  FindVisualRectNeedingUpdateScope finder(object_, context_, old_selection_rect,
                                          new_selection_rect);
#endif
  if (context_.NeedsVisualRectUpdate(object_)) {
    new_selection_rect = object_.LocalSelectionRect();
    context_.MapLocalRectToVisualRect(object_, new_selection_rect);
  } else {
    new_selection_rect = old_selection_rect;
  }

  object_.GetMutableForPainting().SetSelectionVisualRect(new_selection_rect);

  if (full_invalidation)
    return;

  object_.GetMutableForPainting().SetPartialInvalidationVisualRect(
      UnionRect(object_.PartialInvalidationVisualRect(),
                UnionRect(new_selection_rect, old_selection_rect)));
  context_.painting_layer->SetNeedsRepaint();
  object_.InvalidateDisplayItemClients(PaintInvalidationReason::kSelection);
}

DISABLE_CFI_PERF
void ObjectPaintInvalidatorWithContext::InvalidatePartialRect(
    PaintInvalidationReason reason) {
  if (IsImmediateFullPaintInvalidationReason(reason))
    return;

  auto rect = object_.PartialInvalidationLocalRect();
  if (rect.IsEmpty())
    return;

  context_.MapLocalRectToVisualRect(object_, rect);
  if (rect.IsEmpty())
    return;

  object_.GetMutableForPainting().SetPartialInvalidationVisualRect(
      UnionRect(object_.PartialInvalidationVisualRect(), rect));

  context_.painting_layer->SetNeedsRepaint();
  object_.InvalidateDisplayItemClients(PaintInvalidationReason::kRectangle);
}

DISABLE_CFI_PERF
PaintInvalidationReason
ObjectPaintInvalidatorWithContext::InvalidatePaintWithComputedReason(
    PaintInvalidationReason reason) {
  DCHECK(!(context_.subtree_flags &
           PaintInvalidatorContext::kSubtreeNoInvalidation));

  // This is before InvalidateSelection before the latter will accumulate
  // selection visual rects to the partial rect mapped in the former.
  InvalidatePartialRect(reason);

  // We need to invalidate the selection before checking for whether we are
  // doing a full invalidation.  This is because we need to update the previous
  // selection rect regardless.
  InvalidateSelection(reason);

  switch (reason) {
    case PaintInvalidationReason::kNone:
      if (object_.IsSVG() &&
          (context_.subtree_flags &
           PaintInvalidatorContext::kSubtreeSVGResourceChange)) {
        reason = PaintInvalidationReason::kSVGResource;
        break;
      }
      return PaintInvalidationReason::kNone;
    case PaintInvalidationReason::kDelayedFull:
      return PaintInvalidationReason::kDelayedFull;
    default:
      DCHECK(IsImmediateFullPaintInvalidationReason(reason));
  }

  context_.painting_layer->SetNeedsRepaint();
  object_.InvalidateDisplayItemClients(reason);
  return reason;
}

}  // namespace blink
