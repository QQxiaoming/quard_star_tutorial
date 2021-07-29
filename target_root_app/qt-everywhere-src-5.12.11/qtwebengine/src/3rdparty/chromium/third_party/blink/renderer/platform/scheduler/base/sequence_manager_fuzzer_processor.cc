#include "third_party/blink/renderer/platform/scheduler/base/sequence_manager_fuzzer_processor.h"

#include <algorithm>
#include <string>

#include "base/task/sequence_manager/test/sequence_manager_for_test.h"
#include "base/test/test_mock_time_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"

namespace base {
namespace sequence_manager {

void SequenceManagerFuzzerProcessor::ParseAndRun(
    const SequenceManagerTestDescription& description) {
  SequenceManagerFuzzerProcessor processor;
  processor.RunTest(description);
}

SequenceManagerFuzzerProcessor::SequenceManagerFuzzerProcessor()
    : SequenceManagerFuzzerProcessor(false) {}

SequenceManagerFuzzerProcessor::SequenceManagerFuzzerProcessor(
    bool log_for_testing)
    : log_for_testing_(log_for_testing) {
  test_task_runner_ = WrapRefCounted(
      new TestMockTimeTaskRunner(TestMockTimeTaskRunner::Type::kBoundToThread));

  // A zero clock triggers some assertions.
  test_task_runner_->AdvanceMockTickClock(TimeDelta::FromMilliseconds(1));

  initial_time_ = test_task_runner_->GetMockTickClock()->NowTicks();

  manager_ =
      SequenceManagerForTest::Create(nullptr, ThreadTaskRunnerHandle::Get(),
                                     test_task_runner_->GetMockTickClock());

  TaskQueue::Spec spec = TaskQueue::Spec("default_task_queue");
  task_queues_.push_back(manager_->CreateTaskQueue<TestTaskQueue>(spec));
}

void SequenceManagerFuzzerProcessor::RunTest(
    const SequenceManagerTestDescription& description) {
  for (const auto& initial_action : description.initial_actions()) {
    RunAction(initial_action);
  }

  test_task_runner_->FastForwardUntilNoTasksRemain();
}

void SequenceManagerFuzzerProcessor::RunAction(
    const SequenceManagerTestDescription::Action& action) {
  if (action.has_create_task_queue()) {
    return CreateTaskQueueFromAction(action.action_id(),
                                     action.create_task_queue());
  }

  return PostDelayedTaskFromAction(action.action_id(),
                                   action.post_delayed_task());
}

void SequenceManagerFuzzerProcessor::PostDelayedTaskFromAction(
    uint64_t action_id,
    const SequenceManagerTestDescription::PostDelayedTaskAction& action) {
  DCHECK(!task_queues_.empty());

  LogActionForTesting(action_id, ActionForTest::ActionType::kPostDelayedTask,
                      test_task_runner_->GetMockTickClock()->NowTicks());

  size_t index = action.task_queue_id() % task_queues_.size();
  TestTaskQueue* chosen_task_queue = task_queues_[index].get();

  // TODO(farahcharab) After adding non-nestable/nestable tasks, fix this to
  // PostNonnestableDelayedTask for the former and PostDelayedTask for the
  // latter.
  chosen_task_queue->PostDelayedTask(
      FROM_HERE,
      BindOnce(&SequenceManagerFuzzerProcessor::ExecuteTask, Unretained(this),
               action.task()),
      TimeDelta::FromMilliseconds(action.delay_ms()));
}

void SequenceManagerFuzzerProcessor::CreateTaskQueueFromAction(
    uint64_t action_id,
    const SequenceManagerTestDescription::CreateTaskQueueAction& action) {
  LogActionForTesting(action_id, ActionForTest::ActionType::kCreateTaskQueue,
                      test_task_runner_->GetMockTickClock()->NowTicks());

  TaskQueue::Spec spec = TaskQueue::Spec("test_task_queue");
  task_queues_.push_back(manager_->CreateTaskQueue<TestTaskQueue>(spec));
}

void SequenceManagerFuzzerProcessor::ExecuteTask(
    const SequenceManagerTestDescription::Task& task) {
  TimeTicks start_time = test_task_runner_->GetMockTickClock()->NowTicks();

  // We can limit the depth of the nested post delayed action when processing
  // the proto.
  for (const auto& task_action : task.action()) {
    // TODO(farahcharab) Add run loop to deal with nested tasks later. So far,
    // we are assuming tasks are non-nestable.
    RunAction(task_action);
  }

  TimeTicks end_time = test_task_runner_->GetMockTickClock()->NowTicks();

  test_task_runner_->AdvanceMockTickClock(
      TimeDelta::FromMilliseconds(task.duration_ms()) -
      (end_time - start_time));

  LogTaskForTesting(task.task_id(), start_time,
                    test_task_runner_->GetMockTickClock()->NowTicks());
}

void SequenceManagerFuzzerProcessor::LogTaskForTesting(uint64_t task_id,
                                                       TimeTicks start_time,
                                                       TimeTicks end_time) {
  if (!log_for_testing_)
    return;

  uint64_t start_time_ms = (start_time - initial_time_).InMilliseconds();
  uint64_t end_time_ms = (end_time - initial_time_).InMilliseconds();

  ordered_tasks_.emplace_back(task_id, start_time_ms, end_time_ms);
}

void SequenceManagerFuzzerProcessor::LogActionForTesting(
    uint64_t action_id,
    ActionForTest::ActionType type,
    TimeTicks start_time) {
  if (!log_for_testing_)
    return;

  ordered_actions_.emplace_back(action_id, type,
                                (start_time - initial_time_).InMilliseconds());
}

const std::vector<SequenceManagerFuzzerProcessor::TaskForTest>&
SequenceManagerFuzzerProcessor::ordered_tasks() const {
  return ordered_tasks_;
}

const std::vector<SequenceManagerFuzzerProcessor::ActionForTest>&
SequenceManagerFuzzerProcessor::ordered_actions() const {
  return ordered_actions_;
}

SequenceManagerFuzzerProcessor::TaskForTest::TaskForTest(uint64_t id,
                                                         uint64_t start,
                                                         uint64_t end)
    : task_id(id), start_time_ms(start), end_time_ms(end) {}

bool SequenceManagerFuzzerProcessor::TaskForTest::operator==(
    const TaskForTest& rhs) const {
  return task_id == rhs.task_id && start_time_ms == rhs.start_time_ms &&
         end_time_ms == rhs.end_time_ms;
}

SequenceManagerFuzzerProcessor::ActionForTest::ActionForTest(uint64_t id,
                                                             ActionType type,
                                                             uint64_t start)
    : action_id(id), action_type(type), start_time_ms(start) {}

bool SequenceManagerFuzzerProcessor::ActionForTest::operator==(
    const ActionForTest& rhs) const {
  return action_id == rhs.action_id && action_type == rhs.action_type &&
         start_time_ms == rhs.start_time_ms;
}

}  // namespace sequence_manager
}  // namespace base
