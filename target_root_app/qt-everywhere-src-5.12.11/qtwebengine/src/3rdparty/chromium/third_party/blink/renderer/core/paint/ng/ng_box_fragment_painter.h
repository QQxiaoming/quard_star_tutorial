// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_PAINT_NG_NG_BOX_FRAGMENT_PAINTER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_PAINT_NG_NG_BOX_FRAGMENT_PAINTER_H_

#include "third_party/blink/renderer/core/layout/api/hit_test_action.h"
#include "third_party/blink/renderer/core/layout/background_bleed_avoidance.h"
#include "third_party/blink/renderer/core/layout/ng/geometry/ng_border_edges.h"
#include "third_party/blink/renderer/core/layout/ng/ng_physical_box_fragment.h"
#include "third_party/blink/renderer/core/paint/box_painter_base.h"
#include "third_party/blink/renderer/platform/geometry/layout_point.h"
#include "third_party/blink/renderer/platform/geometry/layout_size.h"
#include "third_party/blink/renderer/platform/wtf/allocator.h"

namespace blink {

class FillLayer;
class HitTestLocation;
class HitTestRequest;
class HitTestResult;
class LayoutRect;
class NGPaintFragment;
class NGPhysicalFragment;
struct PaintInfo;

// Painter for LayoutNG box fragments, paints borders and background. Delegates
// to NGTextFragmentPainter to paint line box fragments.
class NGBoxFragmentPainter : public BoxPainterBase {
  STACK_ALLOCATED();

 public:
  NGBoxFragmentPainter(const NGPaintFragment&);

  void Paint(const PaintInfo&);
  void PaintInlineBox(const PaintInfo&, const LayoutPoint& paint_offset);

  // Hit tests this box fragment.
  // @param physical_offset Physical offset of this box fragment in paint layer.
  // TODO(eae): Change to take a HitTestResult pointer instead as it mutates.
  bool NodeAtPoint(HitTestResult&,
                   const HitTestLocation& location_in_container,
                   const LayoutPoint& physical_offset,
                   HitTestAction);

 protected:
  BoxPainterBase::FillLayerInfo GetFillLayerInfo(
      const Color&,
      const FillLayer&,
      BackgroundBleedAvoidance) const override;

  void PaintTextClipMask(GraphicsContext&,
                         const IntRect& mask_rect,
                         const LayoutPoint& paint_offset,
                         bool object_has_multiple_boxes) override;
  LayoutRect AdjustRectForScrolledContent(const PaintInfo&,
                                          const BoxPainterBase::FillLayerInfo&,
                                          const LayoutRect&) override;

 private:
  bool IsPaintingBackgroundOfPaintContainerIntoScrollingContentsLayer(
      const NGPaintFragment&,
      const PaintInfo&);
  bool IntersectsPaintRect(const PaintInfo&,
                           const LayoutPoint& paint_offset) const;

  void PaintBoxDecorationBackground(const PaintInfo&,
                                    const LayoutPoint& paint_offset);
  void PaintAllPhasesAtomically(const PaintInfo&,
                                bool is_self_painting);
  void PaintBlockChildren(const PaintInfo&);
  void PaintLineBoxChildren(const Vector<std::unique_ptr<NGPaintFragment>>&,
                            const PaintInfo&,
                            const LayoutPoint& paint_offset);
  void PaintInlineChildren(const Vector<std::unique_ptr<NGPaintFragment>>&,
                           const PaintInfo&,
                           const LayoutPoint& paint_offset);
  void PaintInlineChildrenOutlines(
      const Vector<std::unique_ptr<NGPaintFragment>>&,
      const PaintInfo&,
      const LayoutPoint& paint_offset);
  void PaintInlineChildBoxUsingLegacyFallback(const NGPhysicalFragment&,
                                              const PaintInfo&);
  void PaintObject(const PaintInfo&,
                   const LayoutPoint&,
                   bool suppress_box_decoration_background = false);
  void PaintBlockFlowContents(const PaintInfo&,
                              const LayoutPoint& paint_offset);
  void PaintInlineChild(const NGPaintFragment&,
                        const PaintInfo&,
                        const LayoutPoint& paint_offset);
  void PaintAtomicInlineChild(const NGPaintFragment&, const PaintInfo&);
  void PaintTextChild(const NGPaintFragment&,
                      const PaintInfo&,
                      const LayoutPoint& paint_offset);
  void PaintFloatingChildren(const Vector<std::unique_ptr<NGPaintFragment>>&,
                             const PaintInfo&);
  void PaintFloats(const PaintInfo&);
  void PaintMask(const PaintInfo&, const LayoutPoint& paint_offset);
  void PaintOverflowControlsIfNeeded(const PaintInfo&,
                                     const LayoutPoint& paint_offset);
  void PaintAtomicInline(const PaintInfo&);
  void PaintBackground(const PaintInfo&,
                       const LayoutRect&,
                       const Color& background_color,
                       BackgroundBleedAvoidance = kBackgroundBleedNone);
  void PaintSymbol(const NGPaintFragment&,
                   const PaintInfo&,
                   const LayoutPoint& paint_offset);

  bool IsInSelfHitTestingPhase(HitTestAction) const;
  bool VisibleToHitTestRequest(const HitTestRequest&) const;

  // Hit tests the children of a container fragment, which is either
  // |box_fragment_|, or one of its child line box fragments.
  // @param physical_offset Physical offset of the container fragment's content
  // box in paint layer. Note that this includes scrolling offset when the
  // container has 'overflow: scroll'.
  bool HitTestChildren(HitTestResult&,
                       const Vector<std::unique_ptr<NGPaintFragment>>&,
                       const HitTestLocation& location_in_container,
                       const LayoutPoint& physical_offset,
                       HitTestAction);

  // Hit tests the given text fragment.
  // @param physical_offset Physical offset of the text fragment in paint layer.
  bool HitTestTextFragment(HitTestResult&,
                           const NGPaintFragment&,
                           const HitTestLocation& location_in_container,
                           const LayoutPoint& physical_offset);

  // Hit tests the given line box fragment.
  // @param physical_offset Physical offset of the line box fragment in paint
  // layer.
  bool HitTestLineBoxFragment(HitTestResult&,
                              const NGPaintFragment&,
                              const HitTestLocation& location_in_container,
                              const LayoutPoint& physical_offset,
                              HitTestAction);

  // Returns whether the hit test location is completely outside the border box,
  // which possibly has rounded corners.
  bool HitTestClippedOutByBorder(const HitTestLocation&,
                                 const LayoutPoint& border_box_location) const;

  LayoutPoint FlipForWritingModeForChild(
      const NGPhysicalFragment& child_fragment,
      const LayoutPoint& offset);

  const NGPhysicalBoxFragment& PhysicalFragment() const;

  const NGPaintFragment& box_fragment_;

  NGBorderEdges border_edges_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_PAINT_NG_NG_BOX_FRAGMENT_PAINTER_H_
