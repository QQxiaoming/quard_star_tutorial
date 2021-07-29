// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/animation/animation_host.h"

#include "base/memory/ptr_util.h"
#include "cc/animation/animation_id_provider.h"
#include "cc/animation/animation_timeline.h"
#include "cc/animation/scroll_timeline.h"
#include "cc/animation/worklet_animation.h"
#include "cc/test/animation_test_common.h"
#include "cc/test/animation_timelines_test_common.h"
#include "cc/test/mock_layer_tree_mutator.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace cc {
namespace {

class AnimationHostTest : public AnimationTimelinesTest {
 public:
  AnimationHostTest() = default;
  ~AnimationHostTest() override = default;

  void AttachWorkletAnimation() {
    client_.RegisterElement(element_id_, ElementListType::ACTIVE);
    client_impl_.RegisterElement(element_id_, ElementListType::PENDING);
    client_impl_.RegisterElement(element_id_, ElementListType::ACTIVE);

    worklet_animation_ = WorkletAnimation::Create(
        worklet_animation_id_, "test_name", nullptr, nullptr);
    int cc_id = worklet_animation_->id();
    worklet_animation_->AttachElement(element_id_);
    host_->AddAnimationTimeline(timeline_);
    timeline_->AttachAnimation(worklet_animation_);

    host_->PushPropertiesTo(host_impl_);
    timeline_impl_ = host_impl_->GetTimelineById(timeline_id_);
    worklet_animation_impl_ =
        ToWorkletAnimation(timeline_impl_->GetAnimationById(cc_id));
  }

  void SetOutputState(base::TimeDelta local_time) {
    worklet_animation_impl_->SetOutputState(
        {worklet_animation_id_, local_time});
  }

  scoped_refptr<WorkletAnimation> worklet_animation_;
  scoped_refptr<WorkletAnimation> worklet_animation_impl_;
  WorkletAnimationId worklet_animation_id_{11, 12};
};

// See Animation tests on layer registration/unregistration in
// animation_unittest.cc.

TEST_F(AnimationHostTest, SyncTimelinesAddRemove) {
  std::unique_ptr<AnimationHost> host(
      AnimationHost::CreateForTesting(ThreadInstance::MAIN));
  std::unique_ptr<AnimationHost> host_impl(
      AnimationHost::CreateForTesting(ThreadInstance::IMPL));

  const int timeline_id = AnimationIdProvider::NextTimelineId();
  scoped_refptr<AnimationTimeline> timeline(
      AnimationTimeline::Create(timeline_id));
  host->AddAnimationTimeline(timeline.get());
  EXPECT_TRUE(timeline->animation_host());

  EXPECT_FALSE(host_impl->GetTimelineById(timeline_id));

  host->PushPropertiesTo(host_impl.get());

  scoped_refptr<AnimationTimeline> timeline_impl =
      host_impl->GetTimelineById(timeline_id);
  EXPECT_TRUE(timeline_impl);
  EXPECT_EQ(timeline_impl->id(), timeline_id);

  host->PushPropertiesTo(host_impl.get());
  EXPECT_EQ(timeline_impl, host_impl->GetTimelineById(timeline_id));

  host->RemoveAnimationTimeline(timeline.get());
  EXPECT_FALSE(timeline->animation_host());

  host->PushPropertiesTo(host_impl.get());
  EXPECT_FALSE(host_impl->GetTimelineById(timeline_id));

  EXPECT_FALSE(timeline_impl->animation_host());
}

TEST_F(AnimationHostTest, ImplOnlyTimeline) {
  std::unique_ptr<AnimationHost> host(
      AnimationHost::CreateForTesting(ThreadInstance::MAIN));
  std::unique_ptr<AnimationHost> host_impl(
      AnimationHost::CreateForTesting(ThreadInstance::IMPL));

  const int timeline_id1 = AnimationIdProvider::NextTimelineId();
  const int timeline_id2 = AnimationIdProvider::NextTimelineId();

  scoped_refptr<AnimationTimeline> timeline(
      AnimationTimeline::Create(timeline_id1));
  scoped_refptr<AnimationTimeline> timeline_impl(
      AnimationTimeline::Create(timeline_id2));
  timeline_impl->set_is_impl_only(true);

  host->AddAnimationTimeline(timeline.get());
  host_impl->AddAnimationTimeline(timeline_impl.get());

  host->PushPropertiesTo(host_impl.get());

  EXPECT_TRUE(host->GetTimelineById(timeline_id1));
  EXPECT_TRUE(host_impl->GetTimelineById(timeline_id2));
}

TEST_F(AnimationHostTest, ImplOnlyScrollAnimationUpdateTargetIfDetached) {
  client_.RegisterElement(element_id_, ElementListType::ACTIVE);
  client_impl_.RegisterElement(element_id_, ElementListType::PENDING);

  gfx::ScrollOffset target_offset(0., 2.);
  gfx::ScrollOffset current_offset(0., 1.);
  host_impl_->ImplOnlyScrollAnimationCreate(element_id_, target_offset,
                                            current_offset, base::TimeDelta(),
                                            base::TimeDelta());

  gfx::Vector2dF scroll_delta(0, 0.5);
  gfx::ScrollOffset max_scroll_offset(0., 3.);

  base::TimeTicks time;

  time += base::TimeDelta::FromSecondsD(0.1);
  EXPECT_TRUE(host_impl_->ImplOnlyScrollAnimationUpdateTarget(
      element_id_, scroll_delta, max_scroll_offset, time, base::TimeDelta()));

  // Detach all animations from layers and timelines.
  host_impl_->ClearMutators();

  time += base::TimeDelta::FromSecondsD(0.1);
  EXPECT_FALSE(host_impl_->ImplOnlyScrollAnimationUpdateTarget(
      element_id_, scroll_delta, max_scroll_offset, time, base::TimeDelta()));
}

// Tests that verify interaction of AnimationHost with LayerTreeMutator.

TEST_F(AnimationHostTest, LayerTreeMutatorUpdateTakesEffectInSameFrame) {
  AttachWorkletAnimation();

  const float start_opacity = .7f;
  const float end_opacity = .3f;
  const double duration = 1.;

  const float expected_opacity =
      start_opacity + (end_opacity - start_opacity) / 2;
  AddOpacityTransitionToAnimation(worklet_animation_.get(), duration,
                                  start_opacity, end_opacity, true);

  base::TimeDelta local_time = base::TimeDelta::FromSecondsD(duration / 2);

  MockLayerTreeMutator* mock_mutator = new NiceMock<MockLayerTreeMutator>();
  host_impl_->SetLayerTreeMutator(
      base::WrapUnique<LayerTreeMutator>(mock_mutator));
  ON_CALL(*mock_mutator, HasAnimators()).WillByDefault(Return(true));
  ON_CALL(*mock_mutator, MutateRef(_))
      .WillByDefault(InvokeWithoutArgs(
          [this, local_time]() { this->SetOutputState(local_time); }));

  // Push the opacity animation to the impl thread.
  host_->PushPropertiesTo(host_impl_);
  host_impl_->ActivateAnimations();

  // Ticking host should cause layer tree mutator to update output state which
  // should take effect in the same animation frame.
  TickAnimationsTransferEvents(base::TimeTicks(), 0u);

  TestLayer* layer =
      client_.FindTestLayer(element_id_, ElementListType::ACTIVE);
  EXPECT_FALSE(layer->is_property_mutated(TargetProperty::OPACITY));
  client_impl_.ExpectOpacityPropertyMutated(
      element_id_, ElementListType::ACTIVE, expected_opacity);
}

}  // namespace
}  // namespace cc
