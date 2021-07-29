// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/scheduler/base/sequence_manager_fuzzer_processor.h"

#include <memory>

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/platform/scheduler/base/proto/sequence_manager_test_description.pb.h"
#include "third_party/protobuf/src/google/protobuf/text_format.h"
#include "third_party/protobuf/src/google/protobuf/util/message_differencer.h"

namespace base {
namespace sequence_manager {

using testing::ContainerEq;

class SequenceManagerFuzzerProcessorForTest
    : public SequenceManagerFuzzerProcessor {
 public:
  SequenceManagerFuzzerProcessorForTest()
      : SequenceManagerFuzzerProcessor(true) {}

  static void ParseAndRun(std::string test_description,
                          std::vector<TaskForTest>* executed_tasks,
                          std::vector<ActionForTest>* executed_actions) {
    SequenceManagerTestDescription proto_description;
    google::protobuf::TextFormat::ParseFromString(test_description,
                                                  &proto_description);

    SequenceManagerFuzzerProcessorForTest processor;
    processor.RunTest(proto_description);

    if (executed_tasks)
      *executed_tasks = processor.ordered_tasks();
    if (executed_actions)
      *executed_actions = processor.ordered_actions();
  }

  using SequenceManagerFuzzerProcessor::ordered_actions;
  using SequenceManagerFuzzerProcessor::ordered_tasks;

  using SequenceManagerFuzzerProcessor::ActionForTest;
  using SequenceManagerFuzzerProcessor::TaskForTest;
};

using ActionForTest = SequenceManagerFuzzerProcessorForTest::ActionForTest;
using TaskForTest = SequenceManagerFuzzerProcessorForTest::TaskForTest;

TEST(SequenceManagerFuzzerProcessorTest, CreateTaskQueue) {
  std::vector<ActionForTest> executed_actions;
  SequenceManagerFuzzerProcessorForTest::ParseAndRun(R"(
       initial_actions {
          action_id : 1
          create_task_queue {
          }
       })",
                                                     nullptr,
                                                     &executed_actions);

  std::vector<ActionForTest> expected_actions;
  expected_actions.emplace_back(1, ActionForTest::ActionType::kCreateTaskQueue,
                                0);

  EXPECT_THAT(executed_actions, ContainerEq(expected_actions));
}

TEST(SequenceManagerFuzzerProcessorTest, PostDelayedTaskWithDuration) {
  std::vector<TaskForTest> executed_tasks;
  std::vector<ActionForTest> executed_actions;

  // Posts an 10 ms delayed task of duration 20 ms.
  SequenceManagerFuzzerProcessorForTest::ParseAndRun(R"(
      initial_actions {
        action_id : 1
        post_delayed_task {
          task_queue_id : 1
          delay_ms : 10
          task {
            task_id : 1
            duration_ms : 20
          }
        }
      })",
                                                     &executed_tasks,
                                                     &executed_actions);

  std::vector<TaskForTest> expected_tasks;
  expected_tasks.emplace_back(1, 10, 30);
  EXPECT_THAT(executed_tasks, ContainerEq(expected_tasks));

  std::vector<ActionForTest> expected_actions;
  expected_actions.emplace_back(1, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  EXPECT_THAT(executed_actions, ContainerEq(expected_actions));
}

TEST(SequenceManagerFuzzerProcessorTest,
     TaskDurationBlocksOtherPendingTasksPostedFromOutsideOfTask) {
  std::vector<TaskForTest> executed_tasks;
  std::vector<ActionForTest> executed_actions;

  // Posts a task of duration 40 ms and a 10 ms delayed task of duration 20 ms.
  SequenceManagerFuzzerProcessorForTest::ParseAndRun(R"(
        initial_actions {
          action_id : 1
          post_delayed_task {
            delay_ms : 10
            task {
              task_id : 1
              duration_ms : 20
            }
          }
        }
        initial_actions {
          action_id :2
          post_delayed_task {
            delay_ms : 0
            task {
              task_id : 2
              duration_ms : 40
            }
          }
        })",
                                                     &executed_tasks,
                                                     &executed_actions);

  // Task with id 2 is expected to run first and block the other task until it
  // done.
  std::vector<TaskForTest> expected_tasks;
  expected_tasks.emplace_back(2, 0, 40);
  expected_tasks.emplace_back(1, 40, 60);
  EXPECT_THAT(executed_tasks, ContainerEq(expected_tasks));

  std::vector<ActionForTest> expected_actions;
  expected_actions.emplace_back(1, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  expected_actions.emplace_back(2, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  EXPECT_THAT(executed_actions, ContainerEq(expected_actions));
}

TEST(SequenceManagerFuzzerProcessorTest,
     TaskDurationBlocksOtherNonNestableTaskWhenPostedFromTheWithinTask) {
  // Posts an instant task of duration 40 ms that posts another non-nested
  // instant task.
  std::vector<TaskForTest> executed_tasks;
  SequenceManagerFuzzerProcessorForTest::ParseAndRun(R"(
        initial_actions {
          post_delayed_task {
            task {
              task_id : 1
              duration_ms : 40
              action {
                post_delayed_task {
                  task {
                    task_id : 2
                  }
                }
              }
            }
          }
        })",
                                                     &executed_tasks, nullptr);

  // Task with task id 1 is expected to run for 40 ms, and block the other
  // posted task from running until its done. Note that the task with id 2 is
  // blocked since it is non-nested, so it is not supposed to run from within
  // the posting task.
  std::vector<TaskForTest> expected_tasks;
  expected_tasks.emplace_back(1, 0, 40);
  expected_tasks.emplace_back(2, 40, 40);

  EXPECT_THAT(executed_tasks, ContainerEq(expected_tasks));
}

TEST(SequenceManagerFuzzerProcessorTest, PostNonEmptyTask) {
  // Posts a 5 ms delayed task of duration 40 ms that creates a task queue,
  // posts a 4 ms delayed task, posts an instant task, creates a task queue,
  // and then posts a 40 ms delayed task.
  std::vector<TaskForTest> executed_tasks;
  std::vector<ActionForTest> executed_actions;

  SequenceManagerFuzzerProcessorForTest::ParseAndRun(R"(
      initial_actions {
        action_id : 1
        post_delayed_task {
          delay_ms: 5
          task {
            task_id : 1
            duration_ms : 40
            action {
              action_id : 2
              create_task_queue {
              }
            }
            action {
              action_id : 3
              post_delayed_task {
                delay_ms : 4
                task {
                  task_id : 2
                }
              }
            }
            action {
              action_id: 4
              post_delayed_task {
                task {
                  task_id : 3
                }
              }
            }
            action {
              action_id : 5
              create_task_queue {
              }
            }
            action {
              action_id : 6
              post_delayed_task {
               delay_ms : 40
               task {
                 task_id : 4
               }
              }
            }
          }
        }
      })",
                                                     &executed_tasks,
                                                     &executed_actions);

  // Task with task id 1 is expected to run first, and block all other pending
  // tasks until its done. The remaining tasks will be executed in
  // non-decreasing order of the delay parameter with ties broken by
  // the post order.
  std::vector<TaskForTest> expected_tasks;
  expected_tasks.emplace_back(1, 5, 45);
  expected_tasks.emplace_back(3, 45, 45);
  expected_tasks.emplace_back(2, 45, 45);
  expected_tasks.emplace_back(4, 45, 45);
  EXPECT_THAT(executed_tasks, ContainerEq(expected_tasks));

  std::vector<ActionForTest> expected_actions;
  expected_actions.emplace_back(1, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  expected_actions.emplace_back(2, ActionForTest::ActionType::kCreateTaskQueue,
                                5);
  expected_actions.emplace_back(3, ActionForTest::ActionType::kPostDelayedTask,
                                5);
  expected_actions.emplace_back(4, ActionForTest::ActionType::kPostDelayedTask,
                                5);
  expected_actions.emplace_back(5, ActionForTest::ActionType::kCreateTaskQueue,
                                5);
  expected_actions.emplace_back(6, ActionForTest::ActionType::kPostDelayedTask,
                                5);
  EXPECT_THAT(executed_actions, ContainerEq(expected_actions));
}

TEST(SequenceManagerFuzzerProcessorTest, OrderOfSimpleUnnestedExecutedActions) {
  // Creates a task queue, creates a task queue after 10 ms of delay, posts a
  // task after 20 ms delay, posts a 10 ms duration task after 15 ms of delay,
  // and posts a task after 100 ms of delay.
  std::vector<TaskForTest> executed_tasks;
  std::vector<ActionForTest> executed_actions;
  SequenceManagerFuzzerProcessorForTest::ParseAndRun(R"(
      initial_actions {
        action_id : 1
        create_task_queue {
        }
      }
      initial_actions {
        action_id : 2
        post_delayed_task {
          delay_ms : 10
          task {
            task_id : 1
            action {
              action_id : 3
              create_task_queue {
              }
            }
          }
        }
      }
      initial_actions {
        action_id : 4
        post_delayed_task {
          delay_ms : 20
          task {
            task_id : 2
          }
        }
      }
      initial_actions {
        action_id : 5
        post_delayed_task {
          delay_ms : 15
          task {
            task_id : 3
            duration_ms : 10
          }
        }
      }
      initial_actions {
        action_id : 6
        post_delayed_task {
          delay_ms : 100
          task {
            task_id : 4
          }
        }
      })",
                                                     &executed_tasks,
                                                     &executed_actions);

  // Tasks are expected to run in order of non-decreasing delay with ties broken
  // by order of posting. Note that the task with id 3 will block the task with
  // id 2 from running at its scheduled time.
  std::vector<TaskForTest> expected_tasks;
  expected_tasks.emplace_back(1, 10, 10);
  expected_tasks.emplace_back(3, 15, 25);
  expected_tasks.emplace_back(2, 25, 25);
  expected_tasks.emplace_back(4, 100, 100);
  EXPECT_THAT(executed_tasks, ContainerEq(expected_tasks));

  std::vector<ActionForTest> expected_actions;
  expected_actions.emplace_back(1, ActionForTest::ActionType::kCreateTaskQueue,
                                0);
  expected_actions.emplace_back(2, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  expected_actions.emplace_back(4, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  expected_actions.emplace_back(5, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  expected_actions.emplace_back(6, ActionForTest::ActionType::kPostDelayedTask,
                                0);
  expected_actions.emplace_back(3, ActionForTest::ActionType::kCreateTaskQueue,
                                10);
  EXPECT_THAT(executed_actions, ContainerEq(expected_actions));
}

}  // namespace sequence_manager
}  // namespace base
