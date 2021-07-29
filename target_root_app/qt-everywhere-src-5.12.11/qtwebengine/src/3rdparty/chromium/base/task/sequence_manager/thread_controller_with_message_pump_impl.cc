// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/sequence_manager/thread_controller_with_message_pump_impl.h"

#include "base/auto_reset.h"
#include "base/message_loop/message_pump_default.h"
#include "base/time/tick_clock.h"
#include "base/trace_event/trace_event.h"

namespace base {
namespace sequence_manager {
namespace internal {

ThreadControllerWithMessagePumpImpl::ThreadControllerWithMessagePumpImpl(
    TickClock* time_source)
    : main_thread_id_(PlatformThread::CurrentId()),
      pump_(new MessagePumpDefault()),
      time_source_(time_source) {
  RunLoop::RegisterDelegateForCurrentThread(this);
}

ThreadControllerWithMessagePumpImpl::~ThreadControllerWithMessagePumpImpl() {
  // Destructors of RunLoop::Delegate and ThreadTaskRunnerHandle
  // will do all the clean-up.
}

ThreadControllerWithMessagePumpImpl::MainThreadOnly::MainThreadOnly() = default;

ThreadControllerWithMessagePumpImpl::MainThreadOnly::~MainThreadOnly() =
    default;

void ThreadControllerWithMessagePumpImpl::SetSequencedTaskSource(
    SequencedTaskSource* task_source) {
  DCHECK(task_source);
  DCHECK(!main_thread_only().task_source);
  main_thread_only().task_source = task_source;
}

void ThreadControllerWithMessagePumpImpl::SetWorkBatchSize(
    int work_batch_size) {
  DCHECK_GE(work_batch_size, 1);
  main_thread_only().batch_size = work_batch_size;
}

void ThreadControllerWithMessagePumpImpl::WillQueueTask(
    PendingTask* pending_task) {
  task_annotator_.WillQueueTask("ThreadController::Task", pending_task);
}

void ThreadControllerWithMessagePumpImpl::ScheduleWork() {
  // Continuation will be posted if necessary.
  if (RunsTasksInCurrentSequence() && is_doing_work())
    return;

  pump_->ScheduleWork();
}

void ThreadControllerWithMessagePumpImpl::SetNextDelayedDoWork(
    LazyNow* lazy_now,
    TimeTicks run_time) {
  if (main_thread_only().next_delayed_work == run_time)
    return;
  main_thread_only().next_delayed_work = run_time;

  if (run_time == TimeTicks::Max())
    return;

  // Continuation will be posted if necessary.
  if (is_doing_work())
    return;

  // |lazy_now| will be removed in this method soon.
  DCHECK_LT(time_source_->NowTicks(), run_time);
  pump_->ScheduleDelayedWork(run_time);
}

const TickClock* ThreadControllerWithMessagePumpImpl::GetClock() {
  return time_source_;
}

bool ThreadControllerWithMessagePumpImpl::RunsTasksInCurrentSequence() {
  return main_thread_id_ == PlatformThread::CurrentId();
}

void ThreadControllerWithMessagePumpImpl::SetDefaultTaskRunner(
    scoped_refptr<SingleThreadTaskRunner> task_runner) {
  main_thread_only().thread_task_runner_handle =
      std::make_unique<ThreadTaskRunnerHandle>(task_runner);
}

void ThreadControllerWithMessagePumpImpl::RestoreDefaultTaskRunner() {
  // There's no default task runner unlike with the MessageLoop.
  main_thread_only().thread_task_runner_handle.reset();
}

void ThreadControllerWithMessagePumpImpl::AddNestingObserver(
    RunLoop::NestingObserver* observer) {
  DCHECK_LE(main_thread_only().run_depth, 1);
  DCHECK(!main_thread_only().nesting_observer);
  DCHECK(observer);
  main_thread_only().nesting_observer = observer;
}

void ThreadControllerWithMessagePumpImpl::RemoveNestingObserver(
    RunLoop::NestingObserver* observer) {
  DCHECK_EQ(main_thread_only().nesting_observer, observer);
  main_thread_only().nesting_observer = nullptr;
}

bool ThreadControllerWithMessagePumpImpl::DoWork() {
  DCHECK(main_thread_only().task_source);
  bool task_ran = false;

  {
    AutoReset<int> do_work_scope(&main_thread_only().do_work_depth,
                                 main_thread_only().do_work_depth + 1);

    for (int i = 0; i < main_thread_only().batch_size; i++) {
      Optional<PendingTask> task = main_thread_only().task_source->TakeTask();
      if (!task)
        break;

      TRACE_TASK_EXECUTION("ThreadController::Task", *task);
      task_annotator_.RunTask("ThreadController::Task", &*task);
      task_ran = true;

      main_thread_only().task_source->DidRunTask();

      if (main_thread_only().quit_do_work) {
        // When Quit() is called we must stop running the batch because
        // caller expects per-task granularity.
        main_thread_only().quit_do_work = false;
        return true;
      }
    }
  }  // DoWorkScope.

  LazyNow lazy_now(time_source_);
  TimeDelta do_work_delay =
      main_thread_only().task_source->DelayTillNextTask(&lazy_now);
  DCHECK_GE(do_work_delay, TimeDelta());
  // Schedule a continuation.
  if (do_work_delay.is_zero()) {
    // Need to run new work immediately.
    pump_->ScheduleWork();
  } else if (do_work_delay != TimeDelta::Max()) {
    SetNextDelayedDoWork(&lazy_now, lazy_now.Now() + do_work_delay);
  } else {
    SetNextDelayedDoWork(&lazy_now, TimeTicks::Max());
  }

  return task_ran;
}

bool ThreadControllerWithMessagePumpImpl::DoDelayedWork(
    TimeTicks* next_run_time) {
  // Delayed work is getting processed in DoWork().
  return false;
}

bool ThreadControllerWithMessagePumpImpl::DoIdleWork() {
  // RunLoop::Delegate knows whether we called Run() or RunUntilIdle().
  if (ShouldQuitWhenIdle())
    Quit();
  return false;
}

void ThreadControllerWithMessagePumpImpl::Run(bool application_tasks_allowed) {
  // No system messages are being processed by this class.
  DCHECK(application_tasks_allowed);

  // We already have a MessagePump::Run() running, so we're in a nested RunLoop.
  if (main_thread_only().run_depth > 0 && main_thread_only().nesting_observer)
    main_thread_only().nesting_observer->OnBeginNestedRunLoop();

  {
    AutoReset<int> run_scope(&main_thread_only().run_depth,
                             main_thread_only().run_depth + 1);
    // MessagePump::Run() blocks until Quit() called, but previously started
    // Run() calls continue to block.
    pump_->Run(this);
  }

  // We'll soon continue to run an outer MessagePump::Run() loop.
  if (main_thread_only().run_depth > 0 && main_thread_only().nesting_observer)
    main_thread_only().nesting_observer->OnExitNestedRunLoop();
}

void ThreadControllerWithMessagePumpImpl::Quit() {
  // Interrupt a batch of work.
  if (is_doing_work())
    main_thread_only().quit_do_work = true;
  // If we're in a nested RunLoop, continuation will be posted if necessary.
  pump_->Quit();
}

void ThreadControllerWithMessagePumpImpl::EnsureWorkScheduled() {
  ScheduleWork();
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base
