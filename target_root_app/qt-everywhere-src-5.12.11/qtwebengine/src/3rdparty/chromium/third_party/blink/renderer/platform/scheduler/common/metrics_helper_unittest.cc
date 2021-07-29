// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/scheduler/common/metrics_helper.h"

#include "base/task/sequence_manager/test/fake_task.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/time/time_override.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::sequence_manager::TaskQueue;

namespace blink {
namespace scheduler {

namespace {

using base::sequence_manager::FakeTask;
using base::sequence_manager::FakeTaskTiming;

class MetricsHelperForTest : public MetricsHelper {
 public:
  MetricsHelperForTest(WebThreadType thread_type,
                       bool has_cpu_timing_for_each_task)
      : MetricsHelper(thread_type, has_cpu_timing_for_each_task) {}
  ~MetricsHelperForTest() = default;

  using MetricsHelper::RecordCommonTaskMetrics;
};

base::TimeTicks Seconds(int seconds) {
  return base::TimeTicks() + base::TimeDelta::FromSeconds(seconds);
}

base::ThreadTicks ThreadSeconds(int seconds) {
  return base::ThreadTicks() + base::TimeDelta::FromSeconds(seconds);
}

}  // namespace

TEST(MetricsHelperTest, TaskDurationPerThreadType) {
  base::HistogramTester histogram_tester;

  MetricsHelperForTest main_thread_metrics(
      WebThreadType::kMainThread, false /* has_cpu_timing_for_each_task */);
  MetricsHelperForTest compositor_metrics(
      WebThreadType::kCompositorThread,
      false /* has_cpu_timing_for_each_task */);
  MetricsHelperForTest worker_metrics(WebThreadType::kUnspecifiedWorkerThread,
                                      false /* has_cpu_timing_for_each_task */);

  main_thread_metrics.RecordCommonTaskMetrics(
      nullptr, FakeTask(),
      FakeTaskTiming(Seconds(10), Seconds(50), ThreadSeconds(0),
                     ThreadSeconds(15)));
  compositor_metrics.RecordCommonTaskMetrics(
      nullptr, FakeTask(),
      FakeTaskTiming(Seconds(10), Seconds(80), ThreadSeconds(0),
                     ThreadSeconds(5)));
  compositor_metrics.RecordCommonTaskMetrics(
      nullptr, FakeTask(), FakeTaskTiming(Seconds(100), Seconds(200)));
  worker_metrics.RecordCommonTaskMetrics(
      nullptr, FakeTask(),
      FakeTaskTiming(Seconds(10), Seconds(125), ThreadSeconds(0),
                     ThreadSeconds(25)));

  EXPECT_THAT(
      histogram_tester.GetAllSamples(
          "RendererScheduler.TaskDurationPerThreadType2"),
      testing::UnorderedElementsAre(
          base::Bucket(static_cast<int>(WebThreadType::kMainThread), 40),
          base::Bucket(static_cast<int>(WebThreadType::kCompositorThread), 170),
          base::Bucket(
              static_cast<int>(WebThreadType::kUnspecifiedWorkerThread), 115)));

  EXPECT_THAT(
      histogram_tester.GetAllSamples(
          "RendererScheduler.TaskCPUDurationPerThreadType2"),
      testing::UnorderedElementsAre(
          base::Bucket(static_cast<int>(WebThreadType::kMainThread), 15),
          base::Bucket(static_cast<int>(WebThreadType::kCompositorThread), 5),
          base::Bucket(
              static_cast<int>(WebThreadType::kUnspecifiedWorkerThread), 25)));
}

TEST(MetricsHelperTest, TrackedCPUTimeMetrics) {
  base::HistogramTester histogram_tester;
  base::subtle::ScopedTimeClockOverrides time_override(
      []() { return base::Time(); }, []() { return Seconds(1); },
      []() { return ThreadSeconds(1); });

  MetricsHelperForTest main_thread_metrics(
      WebThreadType::kMainThread, true /* has_cpu_timing_for_each_task */);

  main_thread_metrics.RecordCommonTaskMetrics(
      nullptr, FakeTask(),
      FakeTaskTiming(Seconds(10), Seconds(50), ThreadSeconds(5),
                     ThreadSeconds(15)));
  main_thread_metrics.RecordCommonTaskMetrics(
      nullptr, FakeTask(),
      FakeTaskTiming(Seconds(10), Seconds(50), ThreadSeconds(20),
                     ThreadSeconds(25)));

  EXPECT_THAT(histogram_tester.GetAllSamples(
                  "Scheduler.Experimental.Renderer.CPUTimePerThread.Tracked"),
              testing::UnorderedElementsAre(base::Bucket(
                  static_cast<int>(WebThreadType::kMainThread), 15)));
  // 9 = 4 seconds before task 1 and 5 seconds between tasks 1 and 2.
  EXPECT_THAT(histogram_tester.GetAllSamples(
                  "Scheduler.Experimental.Renderer.CPUTimePerThread.Untracked"),
              testing::UnorderedElementsAre(base::Bucket(
                  static_cast<int>(WebThreadType::kMainThread), 9)));
}

}  // namespace scheduler
}  // namespace blink
