// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/animation/css/css_animations.h"
#include "third_party/blink/renderer/core/animation/animation.h"
#include "third_party/blink/renderer/core/animation/element_animations.h"
#include "third_party/blink/renderer/core/dom/node_computed_style.h"
#include "third_party/blink/renderer/core/layout/layout_object.h"
#include "third_party/blink/renderer/core/testing/core_unit_test_helper.h"
#include "third_party/blink/renderer/platform/animation/compositor_animation_delegate.h"
#include "third_party/blink/renderer/platform/testing/testing_platform_support_with_mock_scheduler.h"

namespace blink {

namespace {

class TestingPlatformSupportWithMockSchedulerAndThreadedAnimations
    : public TestingPlatformSupportWithMockScheduler {
 public:
  bool IsThreadedAnimationEnabled() override { return true; }
};

}  // namespace

class CSSAnimationsTest : public RenderingTest {
 public:
  void SetUp() override {
    platform_->SetAutoAdvanceNowToPendingTasks(false);
    // Advance timer manually as RenderingTest expects the time to be non-zero.
    platform_->AdvanceClockSeconds(1.);
    RenderingTest::SetUp();
    EnableCompositing();
  }

  void TearDown() override {
    platform_->SetAutoAdvanceNowToPendingTasks(true);
    platform_->RunUntilIdle();
  }

  void StartAnimationOnCompositor(Animation* animation) {
    static_cast<CompositorAnimationDelegate*>(animation)
        ->NotifyAnimationStarted(
            (CurrentTimeTicks() - base::TimeTicks()).InSecondsF(),
            animation->CompositorGroup());
  }

  void AdvanceClockSeconds(double seconds) {
    platform_->AdvanceClockSeconds(seconds);
    platform_->RunUntilIdle();
  }

  double GetContrastFilterAmount(Element* element) {
    EXPECT_EQ(1u, element->GetComputedStyle()->Filter().size());
    const FilterOperation* filter =
        element->GetComputedStyle()->Filter().Operations()[0];
    EXPECT_EQ(FilterOperation::OperationType::CONTRAST, filter->GetType());
    return static_cast<const BasicComponentTransferFilterOperation*>(filter)
        ->Amount();
  }

 private:
  ScopedTestingPlatformSupport<
      TestingPlatformSupportWithMockSchedulerAndThreadedAnimations>
      platform_;
};

// Verify that a composited animation is retargeted according to its composited
// time.
TEST_F(CSSAnimationsTest, RetargetedTransition) {
  SetBodyInnerHTML(R"HTML(
    <style>
      #test { transition: filter linear 1s; }
      .contrast1 { filter: contrast(50%); }
      .contrast2 { filter: contrast(0%); }
    </style>
    <div id='test'></div>
  )HTML");
  Element* element = GetDocument().getElementById("test");
  element->setAttribute(HTMLNames::classAttr, "contrast1");
  GetDocument().View()->UpdateAllLifecyclePhases();
  ElementAnimations* animations = element->GetElementAnimations();
  EXPECT_EQ(1u, animations->Animations().size());
  Animation* animation = (*animations->Animations().begin()).key;
  // Start animation on compositor and advance .8 seconds.
  StartAnimationOnCompositor(animation);
  EXPECT_TRUE(animation->HasActiveAnimationsOnCompositor());
  AdvanceClockSeconds(0.8);

  // Starting the second transition should retarget the active transition.
  element->setAttribute(HTMLNames::classAttr, "contrast2");
  GetPage().Animator().ServiceScriptedAnimations(CurrentTimeTicks());
  GetDocument().View()->UpdateAllLifecyclePhases();
  EXPECT_DOUBLE_EQ(0.6, GetContrastFilterAmount(element));

  // As it has been retargeted, advancing halfway should go to 0.3.
  AdvanceClockSeconds(0.5);
  GetPage().Animator().ServiceScriptedAnimations(CurrentTimeTicks());
  GetDocument().View()->UpdateAllLifecyclePhases();
  EXPECT_DOUBLE_EQ(0.3, GetContrastFilterAmount(element));
}

// Test that when an incompatible in progress compositor transition
// would be retargeted it does not incorrectly combine with a new
// transition target.
TEST_F(CSSAnimationsTest, IncompatibleRetargetedTransition) {
  SetBodyInnerHTML(R"HTML(
    <style>
      #test { transition: filter 1s; }
      .saturate { filter: saturate(20%); }
      .contrast { filter: contrast(20%); }
    </style>
    <div id='test'></div>
  )HTML");
  Element* element = GetDocument().getElementById("test");
  element->setAttribute(HTMLNames::classAttr, "saturate");
  GetDocument().View()->UpdateAllLifecyclePhases();
  ElementAnimations* animations = element->GetElementAnimations();
  EXPECT_EQ(1u, animations->Animations().size());
  Animation* animation = (*animations->Animations().begin()).key;

  // Start animation on compositor and advance partially.
  StartAnimationOnCompositor(animation);
  EXPECT_TRUE(animation->HasActiveAnimationsOnCompositor());
  AdvanceClockSeconds(0.003);

  // The computed style still contains no filter until the next frame.
  EXPECT_TRUE(element->GetComputedStyle()->Filter().IsEmpty());

  // Now we start a contrast filter. Since it will try to combine with
  // the in progress saturate filter, and be incompatible, there should
  // be no transition and it should immediately apply on the next frame.
  element->setAttribute(HTMLNames::classAttr, "contrast");
  EXPECT_TRUE(element->GetComputedStyle()->Filter().IsEmpty());
  GetDocument().View()->UpdateAllLifecyclePhases();
  EXPECT_EQ(0.2, GetContrastFilterAmount(element));
}

}  // namespace blink
