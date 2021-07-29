// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/graphics/compositor_mutator_impl.h"

#include "base/single_thread_task_runner.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/web_thread.h"
#include "third_party/blink/public/platform/web_thread_type.h"
#include "third_party/blink/renderer/platform/graphics/compositor_animator.h"
#include "third_party/blink/renderer/platform/graphics/compositor_mutator_client.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/testing/testing_platform_support.h"

#include <memory>

using ::testing::_;
using ::testing::Mock;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::Truly;

// This test uses actual threads since mutator logic requires it. This means we
// have dependency on Blink platform to create threads.

namespace blink {
namespace {

std::unique_ptr<WebThread> CreateThread(const char* name) {
  return Platform::Current()->CreateThread(
      WebThreadCreationParams(WebThreadType::kTestThread)
          .SetThreadNameForTest(name));
}

class MockCompositorAnimator
    : public GarbageCollectedFinalized<MockCompositorAnimator>,
      public CompositorAnimator {
  USING_GARBAGE_COLLECTED_MIXIN(MockCompositorAnimator);

 public:
  MockCompositorAnimator(
      scoped_refptr<base::SingleThreadTaskRunner> expected_runner)
      : expected_runner_(expected_runner) {}

  ~MockCompositorAnimator() override {}

  std::unique_ptr<AnimationWorkletOutput> Mutate(
      std::unique_ptr<AnimationWorkletInput> input) override {
    return std::unique_ptr<AnimationWorkletOutput>(MutateRef(*input));
  }

  MOCK_CONST_METHOD0(GetScopeId, int());
  MOCK_METHOD1(MutateRef,
               AnimationWorkletOutput*(const AnimationWorkletInput&));

  scoped_refptr<base::SingleThreadTaskRunner> expected_runner_;
};

class MockCompositorMutatorClient : public CompositorMutatorClient {
 public:
  MockCompositorMutatorClient(std::unique_ptr<CompositorMutatorImpl> mutator)
      : CompositorMutatorClient(std::move(mutator)) {}
  ~MockCompositorMutatorClient() override {}
  // gmock cannot mock methods with move-only args so we forward it to ourself.
  void SetMutationUpdate(
      std::unique_ptr<cc::MutatorOutputState> output_state) override {
    SetMutationUpdateRef(output_state.get());
  }

  MOCK_METHOD1(SetMutationUpdateRef,
               void(cc::MutatorOutputState* output_state));
};

class CompositorMutatorImplTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto mutator = std::make_unique<CompositorMutatorImpl>();
    mutator_ = mutator.get();
    client_ =
        std::make_unique<::testing::StrictMock<MockCompositorMutatorClient>>(
            std::move(mutator));
  }

  void TearDown() override { mutator_ = nullptr; }

  std::unique_ptr<::testing::StrictMock<MockCompositorMutatorClient>> client_;
  CompositorMutatorImpl* mutator_;
};

std::unique_ptr<CompositorMutatorInputState> CreateTestMutatorInput() {
  AnimationWorkletInput::AddAndUpdateState state1{
      {11, 1}, "test1", 5000, nullptr};

  AnimationWorkletInput::AddAndUpdateState state2{
      {22, 2}, "test2", 5000, nullptr};

  auto input = std::make_unique<CompositorMutatorInputState>();
  input->Add(std::move(state1));
  input->Add(std::move(state2));

  return input;
}

bool OnlyIncludesAnimation1(const AnimationWorkletInput& in) {
  return in.added_and_updated_animations.size() == 1 &&
         in.added_and_updated_animations[0].worklet_animation_id.animation_id ==
             1;
}

TEST_F(CompositorMutatorImplTest,
       RegisteredAnimatorShouldOnlyReceiveInputForItself) {
  std::unique_ptr<WebThread> first_thread = CreateThread("FirstThread");
  MockCompositorAnimator* first_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());

  mutator_->RegisterCompositorAnimator(first_animator,
                                       first_thread->GetTaskRunner());

  EXPECT_CALL(*first_animator, GetScopeId()).Times(1).WillOnce(Return(11));
  EXPECT_CALL(*first_animator, MutateRef(Truly(OnlyIncludesAnimation1)))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(1);

  mutator_->Mutate(CreateTestMutatorInput());
}

TEST_F(CompositorMutatorImplTest,
       RegisteredAnimatorShouldNotBeMutatedWhenNoInput) {
  std::unique_ptr<WebThread> first_thread = CreateThread("FirstThread");
  MockCompositorAnimator* first_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());

  mutator_->RegisterCompositorAnimator(first_animator,
                                       first_thread->GetTaskRunner());

  EXPECT_CALL(*first_animator, GetScopeId()).Times(1).WillOnce(Return(11));
  EXPECT_CALL(*first_animator, MutateRef(_)).Times(0);
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(0);

  AnimationWorkletInput::AddAndUpdateState state2{
      {22, 2}, "test2", 5000, nullptr};

  auto input = std::make_unique<CompositorMutatorInputState>();
  input->Add(std::move(state2));

  mutator_->Mutate(std::move(input));
}

TEST_F(CompositorMutatorImplTest,
       MutationUpdateIsNotInvokedWithNoRegisteredAnimators) {
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(0);
  std::unique_ptr<CompositorMutatorInputState> input =
      std::make_unique<CompositorMutatorInputState>();
  mutator_->Mutate(std::move(input));
}

TEST_F(CompositorMutatorImplTest, MutationUpdateIsNotInvokedWithNullOutput) {
  // Create a thread to run animator tasks.
  std::unique_ptr<WebThread> first_thread =
      CreateThread("FirstAnimationThread");
  MockCompositorAnimator* first_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());

  mutator_->RegisterCompositorAnimator(first_animator,
                                       first_thread->GetTaskRunner());
  EXPECT_CALL(*first_animator, GetScopeId()).Times(1).WillOnce(Return(11));
  EXPECT_CALL(*first_animator, MutateRef(_)).Times(1).WillOnce(Return(nullptr));
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(0);
  mutator_->Mutate(CreateTestMutatorInput());
}

TEST_F(CompositorMutatorImplTest,
       MutationUpdateIsInvokedCorrectlyWithSingleRegisteredAnimator) {
  // Create a thread to run animator tasks.
  std::unique_ptr<WebThread> first_thread =
      CreateThread("FirstAnimationThread");
  MockCompositorAnimator* first_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());

  mutator_->RegisterCompositorAnimator(first_animator,
                                       first_thread->GetTaskRunner());
  EXPECT_CALL(*first_animator, GetScopeId()).Times(1).WillOnce(Return(11));
  EXPECT_CALL(*first_animator, MutateRef(_))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(1);
  mutator_->Mutate(CreateTestMutatorInput());

  // The above call blocks on animator threads running their tasks so we can
  // safely verify here.
  Mock::VerifyAndClearExpectations(client_.get());

  // Ensure animator is not invoked after unregistration.
  EXPECT_CALL(*first_animator, MutateRef(_)).Times(0);
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(0);
  mutator_->UnregisterCompositorAnimator(first_animator);

  mutator_->Mutate(CreateTestMutatorInput());
  Mock::VerifyAndClearExpectations(client_.get());
}

TEST_F(CompositorMutatorImplTest,
       MutationUpdateInvokedCorrectlyWithTwoRegisteredAnimatorsOnSameThread) {
  std::unique_ptr<WebThread> first_thread =
      CreateThread("FirstAnimationThread");
  MockCompositorAnimator* first_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());
  MockCompositorAnimator* second_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());

  mutator_->RegisterCompositorAnimator(first_animator,
                                       first_thread->GetTaskRunner());
  mutator_->RegisterCompositorAnimator(second_animator,
                                       first_thread->GetTaskRunner());

  EXPECT_CALL(*first_animator, GetScopeId()).Times(1).WillOnce(Return(11));
  EXPECT_CALL(*first_animator, MutateRef(_))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));
  EXPECT_CALL(*second_animator, GetScopeId()).Times(1).WillOnce(Return(22));
  EXPECT_CALL(*second_animator, MutateRef(_))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));

  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(2);
  mutator_->Mutate(CreateTestMutatorInput());
}

TEST_F(
    CompositorMutatorImplTest,
    MutationUpdateInvokedCorrectlyWithTwoRegisteredAnimatorsOnDifferentThreads) {
  std::unique_ptr<WebThread> first_thread =
      CreateThread("FirstAnimationThread");
  MockCompositorAnimator* first_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          first_thread->GetTaskRunner());

  std::unique_ptr<WebThread> second_thread =
      CreateThread("SecondAnimationThread");
  MockCompositorAnimator* second_animator =
      new ::testing::StrictMock<MockCompositorAnimator>(
          second_thread->GetTaskRunner());

  mutator_->RegisterCompositorAnimator(first_animator,
                                       first_thread->GetTaskRunner());
  mutator_->RegisterCompositorAnimator(second_animator,
                                       second_thread->GetTaskRunner());

  EXPECT_CALL(*first_animator, GetScopeId()).Times(1).WillOnce(Return(11));
  EXPECT_CALL(*first_animator, MutateRef(_))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));
  EXPECT_CALL(*second_animator, GetScopeId()).Times(1).WillOnce(Return(22));
  EXPECT_CALL(*second_animator, MutateRef(_))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(2);
  mutator_->Mutate(CreateTestMutatorInput());

  // The above call blocks on animator threads running their tasks so we can
  // safely verify here.
  Mock::VerifyAndClearExpectations(client_.get());

  // Ensure animator is not invoked after unregistration.
  mutator_->UnregisterCompositorAnimator(first_animator);

  EXPECT_CALL(*first_animator, GetScopeId()).Times(0);
  EXPECT_CALL(*first_animator, MutateRef(_)).Times(0);
  EXPECT_CALL(*second_animator, GetScopeId()).Times(1).WillOnce(Return(22));
  EXPECT_CALL(*second_animator, MutateRef(_))
      .Times(1)
      .WillOnce(Return(new AnimationWorkletOutput()));
  EXPECT_CALL(*client_, SetMutationUpdateRef(_)).Times(1);
  mutator_->Mutate(CreateTestMutatorInput());
  Mock::VerifyAndClearExpectations(client_.get());
}

}  // namespace

}  // namespace blink
