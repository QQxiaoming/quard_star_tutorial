// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/layout/ng/inline/ng_abstract_inline_text_box.h"

#include "third_party/blink/renderer/core/accessibility/ax_object_cache.h"
#include "third_party/blink/renderer/core/layout/ng/inline/ng_physical_line_box_fragment.h"
#include "third_party/blink/renderer/core/layout/ng/inline/ng_physical_text_fragment.h"
#include "third_party/blink/renderer/core/paint/ng/ng_paint_fragment.h"
#include "third_party/blink/renderer/core/paint/ng/ng_paint_fragment_traversal.h"
#include "third_party/blink/renderer/platform/fonts/character_range.h"
#include "third_party/blink/renderer/platform/fonts/shaping/shape_result_buffer.h"

namespace blink {

NGAbstractInlineTextBox::FragmentToNGAbstractInlineTextBoxHashMap*
    NGAbstractInlineTextBox::g_abstract_inline_text_box_map_ = nullptr;

scoped_refptr<AbstractInlineTextBox> NGAbstractInlineTextBox::GetOrCreate(
    LineLayoutText line_layout_item,
    const NGPaintFragment& fragment) {
  DCHECK(fragment.GetLayoutObject()->IsText()) << fragment.GetLayoutObject();
  if (!g_abstract_inline_text_box_map_) {
    g_abstract_inline_text_box_map_ =
        new FragmentToNGAbstractInlineTextBoxHashMap();
  }
  const auto it = g_abstract_inline_text_box_map_->find(&fragment);
  if (it != g_abstract_inline_text_box_map_->end())
    return it->value;
  scoped_refptr<AbstractInlineTextBox> obj =
      base::AdoptRef(new NGAbstractInlineTextBox(line_layout_item, fragment));
  g_abstract_inline_text_box_map_->Set(&fragment, obj);
  return obj;
}

void NGAbstractInlineTextBox::WillDestroy(NGPaintFragment* fragment) {
  if (!g_abstract_inline_text_box_map_)
    return;
  const auto it = g_abstract_inline_text_box_map_->find(fragment);
  if (it != g_abstract_inline_text_box_map_->end()) {
    it->value->Detach();
    g_abstract_inline_text_box_map_->erase(fragment);
  }
}

NGAbstractInlineTextBox::NGAbstractInlineTextBox(
    LineLayoutText line_layout_item,
    const NGPaintFragment& fragment)
    : AbstractInlineTextBox(line_layout_item), fragment_(&fragment) {
  DCHECK(fragment_->PhysicalFragment().IsText()) << fragment_;
}

NGAbstractInlineTextBox::~NGAbstractInlineTextBox() {
  DCHECK(!fragment_);
}

void NGAbstractInlineTextBox::Detach() {
  if (Node* const node = GetNode()) {
    if (AXObjectCache* cache = node->GetDocument().ExistingAXObjectCache())
      cache->InlineTextBoxesUpdated(GetLineLayoutItem());
  }
  AbstractInlineTextBox::Detach();
  fragment_ = nullptr;
}

bool NGAbstractInlineTextBox::HasSoftWrapToNextLine() const {
  return ToNGPhysicalLineBoxFragment(
             fragment_->ContainerLineBox()->PhysicalFragment())
      .HasSoftWrapToNextLine();
}

const NGPhysicalTextFragment& NGAbstractInlineTextBox::PhysicalTextFragment()
    const {
  return ToNGPhysicalTextFragment(fragment_->PhysicalFragment());
}

bool NGAbstractInlineTextBox::NeedsLayout() const {
  return fragment_->GetLayoutObject()->NeedsLayout();
}

bool NGAbstractInlineTextBox::NeedsTrailingSpace() const {
  if (!HasSoftWrapToNextLine())
    return false;
  const NGPaintFragment* next_fragment = NextTextFragmentForSameLayoutObject();
  if (!next_fragment)
    return false;
  return ToNGPhysicalTextFragment(next_fragment->PhysicalFragment())
             .StartOffset() != PhysicalTextFragment().EndOffset();
}

const NGPaintFragment*
NGAbstractInlineTextBox::NextTextFragmentForSameLayoutObject() const {
  const auto fragments =
      NGPaintFragment::InlineFragmentsFor(fragment_->GetLayoutObject());
  const auto it =
      std::find_if(fragments.begin(), fragments.end(),
                   [&](const auto& sibling) { return fragment_ == sibling; });
  DCHECK(it != fragments.end());
  const auto next_it = std::next(it);
  return next_it == fragments.end() ? nullptr : *next_it;
}

scoped_refptr<AbstractInlineTextBox>
NGAbstractInlineTextBox::NextInlineTextBox() const {
  if (!fragment_)
    return nullptr;
  DCHECK(!NeedsLayout());
  const NGPaintFragment* next_fragment = NextTextFragmentForSameLayoutObject();
  if (!next_fragment)
    return nullptr;
  return GetOrCreate(GetLineLayoutItem(), *next_fragment);
}

LayoutRect NGAbstractInlineTextBox::LocalBounds() const {
  if (!fragment_ || !GetLineLayoutItem())
    return LayoutRect();
  return LayoutRect(fragment_->InlineOffsetToContainerBox().ToLayoutPoint(),
                    fragment_->Size().ToLayoutSize());
}

unsigned NGAbstractInlineTextBox::Len() const {
  if (!fragment_)
    return 0;
  if (NeedsTrailingSpace())
    return PhysicalTextFragment().Length() + 1;
  return PhysicalTextFragment().Length();
}

AbstractInlineTextBox::Direction NGAbstractInlineTextBox::GetDirection() const {
  if (!fragment_ || !GetLineLayoutItem())
    return kLeftToRight;
  const TextDirection text_direction =
      PhysicalTextFragment().ResolvedDirection();
  if (GetLineLayoutItem().Style()->IsHorizontalWritingMode())
    return IsLtr(text_direction) ? kLeftToRight : kRightToLeft;
  return IsLtr(text_direction) ? kTopToBottom : kBottomToTop;
}

void NGAbstractInlineTextBox::CharacterWidths(Vector<float>& widths) const {
  if (!fragment_)
    return;
  if (!PhysicalTextFragment().TextShapeResult()) {
    // When |fragment_| for BR, we don't have shape result.
    // "aom-computed-boolean-properties.html" reaches here.
    widths.resize(Len());
    return;
  }
  const ShapeResult& shape_result = *PhysicalTextFragment().TextShapeResult();
  ShapeResultBuffer buffer;
  buffer.AppendResult(&shape_result);
  const Vector<CharacterRange> ranges = buffer.IndividualCharacterRanges(
      shape_result.Direction(), shape_result.Width());
  widths.ReserveCapacity(ranges.size());
  widths.resize(0);
  for (const auto& range : ranges)
    widths.push_back(range.Width());
  // The shaper can fail to return glyph metrics for all characters (see
  // crbug.com/613915 and crbug.com/615661) so add empty ranges to ensure all
  // characters have an associated range.
  widths.resize(Len());
}

String NGAbstractInlineTextBox::GetText() const {
  if (!fragment_ || !GetLineLayoutItem())
    return String();
  // For compatibility with |InlineTextBox|, we should have a space character
  // for soft line break.
  // Following tests require this:
  //  - accessibility/inline-text-change-style.html
  //  - accessibility/inline-text-changes.html
  //  - accessibility/inline-text-word-boundaries.html
  if (NeedsTrailingSpace())
    return PhysicalTextFragment().Text().ToString() + " ";
  return PhysicalTextFragment().Text().ToString();
}

bool NGAbstractInlineTextBox::IsFirst() const {
  if (!fragment_)
    return true;
  DCHECK(!NeedsLayout());
  const auto fragments =
      NGPaintFragment::InlineFragmentsFor(fragment_->GetLayoutObject());
  return fragment_ == &fragments.front();
}

bool NGAbstractInlineTextBox::IsLast() const {
  if (!fragment_)
    return true;
  DCHECK(!NeedsLayout());
  const auto fragments =
      NGPaintFragment::InlineFragmentsFor(fragment_->GetLayoutObject());
  return fragment_ == &fragments.back();
}

scoped_refptr<AbstractInlineTextBox> NGAbstractInlineTextBox::NextOnLine()
    const {
  if (!fragment_)
    return nullptr;
  DCHECK(!NeedsLayout());
  DCHECK(fragment_->ContainerLineBox());
  NGPaintFragmentTraversal cursor(*fragment_->ContainerLineBox(), *fragment_);
  for (cursor.MoveToNext(); !cursor.IsAtEnd(); cursor.MoveToNext()) {
    if (cursor->GetLayoutObject()->IsText())
      return GetOrCreate(GetLineLayoutItem(), *cursor);
  }
  return nullptr;
}

scoped_refptr<AbstractInlineTextBox> NGAbstractInlineTextBox::PreviousOnLine()
    const {
  if (!fragment_)
    return nullptr;
  DCHECK(!NeedsLayout());
  DCHECK(fragment_->ContainerLineBox());
  NGPaintFragmentTraversal cursor(*fragment_->ContainerLineBox(), *fragment_);
  for (cursor.MoveToPrevious(); !cursor.IsAtEnd(); cursor.MoveToPrevious()) {
    if (cursor->GetLayoutObject()->IsText())
      return GetOrCreate(GetLineLayoutItem(), *cursor);
  }
  return nullptr;
}

}  // namespace blink
