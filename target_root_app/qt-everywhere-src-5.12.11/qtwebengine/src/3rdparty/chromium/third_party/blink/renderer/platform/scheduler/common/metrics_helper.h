// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_SCHEDULER_COMMON_METRICS_HELPER_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_SCHEDULER_COMMON_METRICS_HELPER_H_

#include "base/optional.h"
#include "base/task/sequence_manager/task_queue.h"
#include "base/time/time.h"
#include "third_party/blink/public/platform/web_thread_type.h"
#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/scheduler/common/total_duration_metric_reporter.h"
#include "third_party/blink/renderer/platform/scheduler/util/task_duration_metric_reporter.h"

namespace base {
namespace sequence_manager {
class TaskQueue;
}
}  // namespace base

namespace blink {
namespace scheduler {

// Helper class to take care of task metrics shared between main thread
// and worker threads of the renderer process, including per-thread
// task metrics.
//
// Each thread-specific scheduler should have its own subclass of MetricsHelper
// (MainThreadMetricsHelper, WorkerMetricsHelper, etc) and should call
// RecordCommonTaskMetrics manually.
// Note that this is code reuse, not data reuse -- each thread should have its
// own instantiation of this class.
class PLATFORM_EXPORT MetricsHelper {
 public:
  MetricsHelper(WebThreadType thread_type, bool has_cpu_timing_for_each_task);
  ~MetricsHelper();

 protected:
  bool ShouldDiscardTask(
      base::sequence_manager::TaskQueue* queue,
      const base::sequence_manager::TaskQueue::Task& task,
      const base::sequence_manager::TaskQueue::TaskTiming& task_timing);

  // Record task metrics which are shared between threads.
  void RecordCommonTaskMetrics(
      base::sequence_manager::TaskQueue* queue,
      const base::sequence_manager::TaskQueue::Task& task,
      const base::sequence_manager::TaskQueue::TaskTiming& task_timing);

 protected:
  const WebThreadType thread_type_;
  const bool has_cpu_timing_for_each_task_;

 private:
  base::ThreadTicks last_known_time_;

  TaskDurationMetricReporter<WebThreadType> thread_task_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType> thread_task_cpu_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType>
      foreground_thread_task_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType>
      foreground_thread_task_cpu_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType>
      background_thread_task_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType>
      background_thread_task_cpu_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType> tracked_cpu_duration_reporter_;
  TaskDurationMetricReporter<WebThreadType> non_tracked_cpu_duration_reporter_;

  DISALLOW_COPY_AND_ASSIGN(MetricsHelper);
};

}  // namespace scheduler
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_SCHEDULER_COMMON_METRICS_HELPER_H_
