// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/scheduler/main_thread/frame_task_queue_controller.h"

#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/memory/scoped_refptr.h"
#include "base/run_loop.h"
#include "base/task/sequence_manager/task_queue.h"
#include "base/task/sequence_manager/test/sequence_manager_for_test.h"
#include "base/test/scoped_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/platform/task_type.h"
#include "third_party/blink/renderer/platform/scheduler/main_thread/frame_scheduler_impl.h"
#include "third_party/blink/renderer/platform/scheduler/main_thread/main_thread_scheduler_impl.h"
#include "third_party/blink/renderer/platform/scheduler/main_thread/main_thread_task_queue.h"
#include "third_party/blink/renderer/platform/scheduler/main_thread/page_scheduler_impl.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"

using base::sequence_manager::TaskQueue;
using QueueType = blink::scheduler::MainThreadTaskQueue::QueueType;
using QueueTraits = blink::scheduler::MainThreadTaskQueue::QueueTraits;

namespace blink {
namespace scheduler {

class FrameTaskQueueControllerTest : public testing::Test,
                                     FrameTaskQueueController::Delegate {
 public:
  FrameTaskQueueControllerTest()
      : task_environment_(
            base::test::ScopedTaskEnvironment::MainThreadType::MOCK_TIME,
            base::test::ScopedTaskEnvironment::ExecutionMode::QUEUED),
        task_queue_created_count_(0) {}

  ~FrameTaskQueueControllerTest() override = default;

  void SetUp() override {
    scheduler_.reset(new MainThreadSchedulerImpl(
        base::sequence_manager::SequenceManagerForTest::Create(
            nullptr, task_environment_.GetMainThreadTaskRunner(),
            task_environment_.GetMockTickClock()),
        base::nullopt));
    page_scheduler_.reset(new PageSchedulerImpl(nullptr, scheduler_.get()));
    frame_scheduler_ = FrameSchedulerImpl::Create(
        page_scheduler_.get(), nullptr, FrameScheduler::FrameType::kSubframe);
    frame_task_queue_controller_.reset(new FrameTaskQueueController(
        scheduler_.get(), frame_scheduler_.get(), this));
  }

  void TearDown() override {
    frame_task_queue_controller_.reset();
    frame_scheduler_.reset();
    page_scheduler_.reset();
    scheduler_->Shutdown();
    scheduler_.reset();
  }

  // FrameTaskQueueController::Delegate implementation.
  void OnTaskQueueCreated(MainThreadTaskQueue* task_queue,
                          TaskQueue::QueueEnabledVoter* voter) override {
    ++task_queue_created_count_;
  }

 protected:
  scoped_refptr<MainThreadTaskQueue> NewLoadingTaskQueue() const {
    return frame_task_queue_controller_->NewLoadingTaskQueue();
  }

  scoped_refptr<MainThreadTaskQueue> NewLoadingControlTaskQueue() const {
    return frame_task_queue_controller_->NewLoadingControlTaskQueue();
  }

  scoped_refptr<MainThreadTaskQueue> NewNonLoadingTaskQueue(
      QueueTraits queue_traits) const {
    return frame_task_queue_controller_->NewNonLoadingTaskQueue(queue_traits);
  }

  scoped_refptr<MainThreadTaskQueue> NewResourceLoadingTaskQueue() const {
    return frame_task_queue_controller_->NewResourceLoadingTaskQueue();
  }

  size_t task_queue_created_count() const { return task_queue_created_count_; }

 protected:
  base::test::ScopedTaskEnvironment task_environment_;
  std::unique_ptr<MainThreadSchedulerImpl> scheduler_;
  std::unique_ptr<PageSchedulerImpl> page_scheduler_;
  std::unique_ptr<FrameSchedulerImpl> frame_scheduler_;
  std::unique_ptr<FrameTaskQueueController> frame_task_queue_controller_;

 private:
  size_t task_queue_created_count_;

  DISALLOW_COPY_AND_ASSIGN(FrameTaskQueueControllerTest);
};

TEST_F(FrameTaskQueueControllerTest, CreateAllTaskQueues) {
  enum class QueueCheckResult { kDidNotSeeQueue, kDidSeeQueue };

  WTF::HashMap<scoped_refptr<MainThreadTaskQueue>, QueueCheckResult>
      all_task_queues;

  scoped_refptr<MainThreadTaskQueue> task_queue = NewLoadingTaskQueue();
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  task_queue = NewLoadingControlTaskQueue();
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  // Create the 4 default non-loading task queues used by FrameSchedulerImpl.
  task_queue = NewNonLoadingTaskQueue(QueueTraits()
                                          .SetCanBeThrottled(true)
                                          .SetCanBeDeferred(true)
                                          .SetCanBeFrozen(true)
                                          .SetCanBePaused(true));
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  task_queue = NewNonLoadingTaskQueue(
      QueueTraits().SetCanBeDeferred(true).SetCanBePaused(true));
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  task_queue = NewNonLoadingTaskQueue(QueueTraits().SetCanBePaused(true));
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  task_queue = NewNonLoadingTaskQueue(QueueTraits());
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  // Add a couple resource loading task queues.
  task_queue = NewResourceLoadingTaskQueue();
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  task_queue = NewResourceLoadingTaskQueue();
  EXPECT_FALSE(all_task_queues.Contains(task_queue));
  all_task_queues.insert(task_queue.get(), QueueCheckResult::kDidNotSeeQueue);
  EXPECT_EQ(all_task_queues.size(), task_queue_created_count());

  // Verify that we get all of the queues that we added, and only those queues.
  EXPECT_EQ(all_task_queues.size(),
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
  for (const auto& task_queue_and_voter :
       frame_task_queue_controller_->GetAllTaskQueuesAndVoters()) {
    MainThreadTaskQueue* task_queue;
    TaskQueue::QueueEnabledVoter* voter;
    std::tie(task_queue, voter) = task_queue_and_voter;

    EXPECT_NE(task_queue, nullptr);
    EXPECT_TRUE(all_task_queues.find(task_queue) != all_task_queues.end());
    // Make sure we don't get the same queue twice.
    auto it = all_task_queues.find(task_queue);
    EXPECT_FALSE(it == all_task_queues.end());
    EXPECT_EQ(it->value, QueueCheckResult::kDidNotSeeQueue);
    all_task_queues.Set(task_queue, QueueCheckResult::kDidSeeQueue);
    if (task_queue->queue_type() ==
            MainThreadTaskQueue::QueueType::kFrameLoading ||
        task_queue->queue_type() ==
            MainThreadTaskQueue::QueueType::kFrameLoadingControl) {
      EXPECT_NE(voter, nullptr);
    } else if (task_queue->GetQueueTraits().can_be_paused) {
      EXPECT_NE(voter, nullptr);
    } else {
      EXPECT_EQ(voter, nullptr);
    }
  }
}

TEST_F(FrameTaskQueueControllerTest, RemoveResourceLoadingTaskQueues) {
  scoped_refptr<MainThreadTaskQueue> resource_loading_queue1 =
      NewResourceLoadingTaskQueue();
  EXPECT_EQ(1u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
  scoped_refptr<MainThreadTaskQueue> resource_loading_queue2 =
      NewResourceLoadingTaskQueue();
  EXPECT_EQ(2u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());

  // Check that we can remove the resource loading queues.
  bool was_removed =
      frame_task_queue_controller_->RemoveResourceLoadingTaskQueue(
          resource_loading_queue1);
  EXPECT_TRUE(was_removed);
  EXPECT_EQ(1u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
  // Can't delete twice.
  was_removed = frame_task_queue_controller_->RemoveResourceLoadingTaskQueue(
      resource_loading_queue1);
  EXPECT_FALSE(was_removed);
  EXPECT_EQ(1u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());

  was_removed = frame_task_queue_controller_->RemoveResourceLoadingTaskQueue(
      resource_loading_queue2);
  EXPECT_TRUE(was_removed);
  EXPECT_EQ(0u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
  // Can't delete twice.
  was_removed = frame_task_queue_controller_->RemoveResourceLoadingTaskQueue(
      resource_loading_queue2);
  EXPECT_FALSE(was_removed);
  EXPECT_EQ(0u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
}

TEST_F(FrameTaskQueueControllerTest, CannotRemoveNonResourceLoadingTaskQueues) {
  scoped_refptr<MainThreadTaskQueue> task_queue = NewLoadingTaskQueue();
  EXPECT_EQ(1u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
  bool was_removed =
      frame_task_queue_controller_->RemoveResourceLoadingTaskQueue(task_queue);
  EXPECT_FALSE(was_removed);
  EXPECT_EQ(1u,
            frame_task_queue_controller_->GetAllTaskQueuesAndVoters().size());
}

}  // namespace scheduler
}  // namespace blink
