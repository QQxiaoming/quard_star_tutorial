// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/compositing/compositing_requirements_updater.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/core/paint/paint_layer_scrollable_area.h"
#include "third_party/blink/renderer/core/testing/core_unit_test_helper.h"
#include "third_party/blink/renderer/platform/graphics/graphics_layer.h"

namespace blink {

class CompositingRequirementsUpdaterTest : public RenderingTest {
 public:
  CompositingRequirementsUpdaterTest()
      : RenderingTest(SingleChildLocalFrameClient::Create()) {}

  void SetUp() final;
};

void CompositingRequirementsUpdaterTest::SetUp() {
  RenderingTest::SetUp();
  EnableCompositing();
}

TEST_F(CompositingRequirementsUpdaterTest, FixedPosOverlap) {
  SetBodyInnerHTML(R"HTML(
    <div style="position: relative; width: 500px; height: 300px;
        will-change: transform"></div>
    <div id=fixed style="position: fixed; width: 500px; height: 300px;
        top: 300px"></div>
    <div style="width: 200px; height: 3000px"></div>
  )HTML");

  LayoutBoxModelObject* fixed =
      ToLayoutBoxModelObject(GetLayoutObjectByElementId("fixed"));

  EXPECT_EQ(
      CompositingReason::kOverlap | CompositingReason::kSquashingDisallowed,
      fixed->Layer()->GetCompositingReasons());

  GetDocument().View()->LayoutViewport()->ScrollBy(ScrollOffset(0, 100),
                                                   kUserScroll);
  GetDocument().View()->UpdateAllLifecyclePhases();

  // No longer overlaps the first div.
  EXPECT_EQ(CompositingReason::kNone, fixed->Layer()->GetCompositingReasons());
}

TEST_F(CompositingRequirementsUpdaterTest,
       NoOverlapReasonForNonSelfPaintingLayer) {
  SetBodyInnerHTML(R"HTML(
    <style>
      #target {
       overflow: auto;
       width: 100px;
       height: 100px;
       margin-top: -50px;
     }
    </style>
    <div style="position: relative; width: 500px; height: 300px;
        will-change: transform"></div>
    <div id=target></div>
  )HTML");

  PaintLayer* target =
      ToLayoutBoxModelObject(GetLayoutObjectByElementId("target"))->Layer();
  EXPECT_FALSE(target->GetCompositingReasons());

  // Now make |target| self-painting.
  GetDocument().getElementById("target")->setAttribute(HTMLNames::styleAttr,
                                                       "position: relative");
  GetDocument().View()->UpdateAllLifecyclePhases();

  EXPECT_EQ(CompositingReason::kOverlap, target->GetCompositingReasons());
}

TEST_F(CompositingRequirementsUpdaterTest,
       NoAssumedOverlapReasonForNonSelfPaintingLayer) {
  SetBodyInnerHTML(R"HTML(
    <style>
      #target {
       overflow: auto;
       width: 100px;
       height: 100px;
     }
    </style>
    <div style="position: relative; width: 500px; height: 300px;
        transform: translateZ(0)"></div>
    <div id=target></div>
  )HTML");

  PaintLayer* target =
      ToLayoutBoxModelObject(GetLayoutObjectByElementId("target"))->Layer();
  EXPECT_FALSE(target->GetCompositingReasons());

  // Now make |target| self-painting.
  GetDocument().getElementById("target")->setAttribute(HTMLNames::styleAttr,
                                                       "position: relative");
  GetDocument().View()->UpdateAllLifecyclePhases();
  EXPECT_EQ(CompositingReason::kAssumedOverlap,
            target->GetCompositingReasons());
}

TEST_F(CompositingRequirementsUpdaterTest,
       NoDescendantReasonForNonSelfPaintingLayer) {
  SetBodyInnerHTML(R"HTML(
    <style>
      #target {
        overflow: auto;
        width: 100px;
        height: 100px;
      }
    </style>
    <div id=target>
      <div style="backface-visibility: hidden"></div>
    </div>
  )HTML");

  PaintLayer* target =
      ToLayoutBoxModelObject(GetLayoutObjectByElementId("target"))->Layer();
  EXPECT_FALSE(target->GetCompositingReasons());

  // Now make |target| self-painting.
  GetDocument().getElementById("target")->setAttribute(HTMLNames::styleAttr,
                                                       "position: relative");
  GetDocument().View()->UpdateAllLifecyclePhases();
  EXPECT_EQ(CompositingReason::kClipsCompositingDescendants,
            target->GetCompositingReasons());
}

// This test sets up a situation where a squashed PaintLayer loses its
// backing, but does not change visual rect. Therefore the compositing system
// must invalidate it because of change of backing.
TEST_F(CompositingRequirementsUpdaterTest,
       NeedsLayerAssignmentAfterSquashingRemoval) {
  SetBodyInnerHTML(R"HTML(
    <style>
      * {
        margin: 0
      }
      #target {
        width: 100px; height: 100px; backface-visibility: hidden
      }
      div {
        width: 100px; height: 100px;
        position: absolute;
        background: lightblue;
        top: 0px;
      }
    </style>
    <div id=target></div>
    <div id=squashed></div>
  )HTML");

  PaintLayer* squashed =
      ToLayoutBoxModelObject(GetLayoutObjectByElementId("squashed"))->Layer();
  EXPECT_EQ(kPaintsIntoGroupedBacking, squashed->GetCompositingState());

  GetDocument().View()->SetTracksPaintInvalidations(true);

  GetDocument().getElementById("target")->setAttribute(HTMLNames::styleAttr,
                                                       "display: none");
  GetDocument().View()->UpdateAllLifecyclePhases();

  EXPECT_EQ(kNotComposited, squashed->GetCompositingState());
  auto* tracking = GetDocument()
                       .View()
                       ->GetLayoutView()
                       ->Layer()
                       ->GraphicsLayerBacking()
                       ->GetRasterInvalidationTracking();
  EXPECT_TRUE(tracking->HasInvalidations());

  EXPECT_EQ(IntRect(0, 0, 100, 100), tracking->Invalidations()[0].rect);
}

}  // namespace blink
