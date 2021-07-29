// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/service_worker/service_worker_metrics.h"

#include "base/test/metrics/histogram_tester.h"
#include "content/browser/service_worker/embedded_worker_status.h"
#include "content/test/test_content_browser_client.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/common/service_worker/service_worker_utils.h"

namespace content {

namespace service_worker_metrics_unittest {
const std::string kNavigationPreloadSuffix = "_NavigationPreloadEnabled";
const std::string kStartWorkerDuringStartupSuffix = "_StartWorkerDuringStartup";
const std::string kWorkerStartOccurred = "_WorkerStartOccurred";
const std::string kStartWorkerExistingReadyProcessSuffix =
    "_StartWorkerExistingReadyProcess";
const std::string kPreparationTime =
    "ServiceWorker.ActivatedWorkerPreparationForMainFrame.Time";
const std::string kPreparationType =
    "ServiceWorker.ActivatedWorkerPreparationForMainFrame.Type";
const std::string kPreparationTypeSearch =
    "ServiceWorker.ActivatedWorkerPreparationForMainFrame.Type.search";

void ExpectNoNavPreloadMainFrameUMA(
    const base::HistogramTester& histogram_tester) {
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerPreparationType_MainFrame", 0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame", 0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame", 0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame", 0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame", 0);

  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame_WorkerStartOccurred", 0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame_WorkerStartOccurred",
      0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame_WorkerStartOccurred",
      0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame_WorkerStartOccurred",
      0);
}

void ExpectNoNavPreloadWorkerStartOccurredUMA(
    const base::HistogramTester& histogram_tester) {
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame_WorkerStartOccurred", 0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame_WorkerStartOccurred",
      0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame_WorkerStartOccurred",
      0);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame_WorkerStartOccurred",
      0);
}

base::TimeTicks AdvanceTime(base::TimeTicks* time, int milliseconds) {
  *time += base::TimeDelta::FromMilliseconds(milliseconds);
  return *time;
}

using CrossProcessTimeDelta = ServiceWorkerMetrics::CrossProcessTimeDelta;
using StartSituation = ServiceWorkerMetrics::StartSituation;
using WorkerPreparationType = ServiceWorkerMetrics::WorkerPreparationType;

class MetricTestContentBrowserClient : public TestContentBrowserClient {
 public:
  std::string GetMetricSuffixForURL(const GURL& url) override {
    if (url.host() == "search.example.com")
      return "search";
    return std::string();
  }
};

TEST(ServiceWorkerMetricsTest, ActivatedWorkerPreparation) {
  base::TimeDelta time = base::TimeDelta::FromMilliseconds(123);
  {
    // Test preparation when the worker was STARTING.
    base::HistogramTester histogram_tester;
    ServiceWorkerMetrics::RecordActivatedWorkerPreparationForMainFrame(
        time, EmbeddedWorkerStatus::STARTING,
        ServiceWorkerMetrics::StartSituation::UNKNOWN,
        false /* did_navigation_preload */, GURL("https://example.com"));
    histogram_tester.ExpectUniqueSample(
        kPreparationType, static_cast<int>(WorkerPreparationType::STARTING), 1);
    histogram_tester.ExpectTotalCount(
        kPreparationType + kNavigationPreloadSuffix, 0);

    // We don't record .Time histograms when S13nServiceWorker is enabled.
    if (!blink::ServiceWorkerUtils::IsServicificationEnabled()) {
      histogram_tester.ExpectTimeBucketCount(kPreparationTime, time, 1);
      histogram_tester.ExpectTimeBucketCount(
          kPreparationTime + "_StartingWorker", time, 1);
      histogram_tester.ExpectTotalCount(
          kPreparationTime + kNavigationPreloadSuffix, 0);
    }
  }

  {
    // Test preparation when the worker started up during startup.
    base::HistogramTester histogram_tester;
    ServiceWorkerMetrics::RecordActivatedWorkerPreparationForMainFrame(
        time, EmbeddedWorkerStatus::STOPPED, StartSituation::DURING_STARTUP,
        true /* did_navigation_preload */, GURL("https://example.com"));
    histogram_tester.ExpectUniqueSample(
        kPreparationType,
        static_cast<int>(WorkerPreparationType::START_DURING_STARTUP), 1);
    histogram_tester.ExpectUniqueSample(
        kPreparationType + kNavigationPreloadSuffix,
        static_cast<int>(WorkerPreparationType::START_DURING_STARTUP), 1);

    // We don't record .Time histograms when S13nServiceWorker is enabled.
    if (!blink::ServiceWorkerUtils::IsServicificationEnabled()) {
      histogram_tester.ExpectTimeBucketCount(kPreparationTime, time, 1);
      histogram_tester.ExpectTimeBucketCount(
          kPreparationTime + kNavigationPreloadSuffix, time, 1);
      histogram_tester.ExpectTotalCount(
          kPreparationTime + kWorkerStartOccurred + kNavigationPreloadSuffix,
          1);
    }
  }

  {
    // Test preparation when the worker started up in an existing process.
    base::HistogramTester histogram_tester;
    ServiceWorkerMetrics::RecordActivatedWorkerPreparationForMainFrame(
        time, EmbeddedWorkerStatus::STOPPED,
        StartSituation::EXISTING_READY_PROCESS,
        true /* did_navigation_preload */, GURL("https://example.com"));
    histogram_tester.ExpectUniqueSample(
        kPreparationType,
        static_cast<int>(
            WorkerPreparationType::START_IN_EXISTING_READY_PROCESS),
        1);
    histogram_tester.ExpectUniqueSample(
        kPreparationType + kNavigationPreloadSuffix,
        static_cast<int>(
            WorkerPreparationType::START_IN_EXISTING_READY_PROCESS),
        1);

    // We don't record .Time histograms when S13nServiceWorker is enabled.
    if (!blink::ServiceWorkerUtils::IsServicificationEnabled()) {
      histogram_tester.ExpectTimeBucketCount(kPreparationTime, time, 1);
      histogram_tester.ExpectTimeBucketCount(
          kPreparationTime + kNavigationPreloadSuffix, time, 1);
      histogram_tester.ExpectTimeBucketCount(
          kPreparationTime + kWorkerStartOccurred + kNavigationPreloadSuffix,
          time, 1);
    }
  }

  // Suffixed metric test.
  MetricTestContentBrowserClient test_browser_client;
  ContentBrowserClient* old_browser_client =
      SetBrowserClientForTesting(&test_browser_client);
  {
    base::HistogramTester histogram_tester;
    ServiceWorkerMetrics::RecordActivatedWorkerPreparationForMainFrame(
        time, EmbeddedWorkerStatus::STOPPED,
        StartSituation::EXISTING_READY_PROCESS,
        true /* did_navigation_preload */, GURL("https://search.example.com"));
    histogram_tester.ExpectUniqueSample(
        kPreparationType,
        static_cast<int>(
            WorkerPreparationType::START_IN_EXISTING_READY_PROCESS),
        1);
    histogram_tester.ExpectUniqueSample(
        kPreparationTypeSearch,
        static_cast<int>(
            WorkerPreparationType::START_IN_EXISTING_READY_PROCESS),
        1);
  }
  {
    base::HistogramTester histogram_tester;
    ServiceWorkerMetrics::RecordActivatedWorkerPreparationForMainFrame(
        time, EmbeddedWorkerStatus::STOPPED,
        StartSituation::EXISTING_READY_PROCESS,
        true /* did_navigation_preload */,
        GURL("https://notsearch.example.com"));
    histogram_tester.ExpectUniqueSample(
        kPreparationType,
        static_cast<int>(
            WorkerPreparationType::START_IN_EXISTING_READY_PROCESS),
        1);
    histogram_tester.ExpectTotalCount(kPreparationTypeSearch, 0);
  }
  SetBrowserClientForTesting(old_browser_client);
}

// ===========================================================================
// Navigation preload tests
// ===========================================================================
// The worker was already running.
TEST(ServiceWorkerMetricsTest,
     NavigationPreloadResponse_WorkerAlreadyRunning_MainFrame) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(123);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(789);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::RUNNING;
  StartSituation start_situation = StartSituation::EXISTING_READY_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_MAIN_FRAME);

  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.WorkerPreparationType_MainFrame",
      static_cast<int>(WorkerPreparationType::RUNNING), 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame", response_start, 1);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame", false, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame", worker_start, 1);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame", 0);

  ExpectNoNavPreloadWorkerStartOccurredUMA(histogram_tester);
}

// The worker was already running (subframe).
TEST(ServiceWorkerMetricsTest,
     NavigationPreloadResponse_WorkerAlreadyRunning_SubFrame) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(123);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(789);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::RUNNING;
  StartSituation start_situation = StartSituation::EXISTING_READY_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_SUB_FRAME);

  ExpectNoNavPreloadMainFrameUMA(histogram_tester);
}

// The worker started up.
TEST(ServiceWorkerMetricsTest,
     NavigationPreloadResponse_WorkerStart_MainFrame) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(234);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(789);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::STOPPED;
  StartSituation start_situation = StartSituation::NEW_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_MAIN_FRAME);

  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.WorkerPreparationType_MainFrame",
      static_cast<int>(WorkerPreparationType::START_IN_NEW_PROCESS), 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame", response_start, 1);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame", false, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame", worker_start, 1);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame", 0);

  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame_WorkerStartOccurred",
      response_start, 1);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame_WorkerStartOccurred",
      false, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame_WorkerStartOccurred",
      worker_start, 1);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame_WorkerStartOccurred",
      0);
}

// The worker started up (subframe).
TEST(ServiceWorkerMetricsTest, NavigationPreloadResponse_WorkerStart_SubFrame) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(234);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(789);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::STOPPED;
  StartSituation start_situation = StartSituation::EXISTING_READY_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_SUB_FRAME);

  ExpectNoNavPreloadMainFrameUMA(histogram_tester);
}

// The worker was already starting up (STARTING). This gets logged to the
// _WorkerStartOccurred suffix as well.
TEST(ServiceWorkerMetricsTest,
     NavigationPreloadResponse_WorkerStart_WorkerStarting) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(234);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(789);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::STARTING;
  StartSituation start_situation = StartSituation::NEW_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_MAIN_FRAME);

  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.WorkerPreparationType_MainFrame",
      static_cast<int>(WorkerPreparationType::STARTING), 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame", response_start, 1);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame", false, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame", worker_start, 1);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame", 0);

  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame_"
      "WorkerStartOccurred",
      response_start, 1);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame_"
      "WorkerStartOccurred",
      false, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame_"
      "WorkerStartOccurred",
      worker_start, 1);
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame_"
      "WorkerStartOccurred",
      0);
}

// The worker started up, but navigation preload arrived first.
TEST(ServiceWorkerMetricsTest,
     NavigationPreloadResponse_NavPreloadFinishedFirst) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(2345);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(456);
  base::TimeDelta wait_time = worker_start - response_start;
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::STOPPING;
  StartSituation start_situation = StartSituation::NEW_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_MAIN_FRAME);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.WorkerPreparationType_MainFrame",
      static_cast<int>(WorkerPreparationType::STOPPING), 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ResponseTime_MainFrame", response_start, 1);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame", true, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame", response_start, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame", wait_time, 1);

  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.FinishedFirst_MainFrame_"
      "WorkerStartOccurred",
      true, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.ConcurrentTime_MainFrame_"
      "WorkerStartOccurred",
      response_start, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.NavPreload.WorkerWaitTime_MainFrame_"
      "WorkerStartOccurred",
      wait_time, 1);
}

// The worker started up, but navigation preload arrived first (subframe).
TEST(ServiceWorkerMetricsTest,
     NavigationPreloadResponse_NavPreloadFinishedFirst_SubFrame) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(2345);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(456);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::STOPPED;
  StartSituation start_situation = StartSituation::NEW_PROCESS;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_SUB_FRAME);

  ExpectNoNavPreloadMainFrameUMA(histogram_tester);
}

// The worker started up during browser startup.
TEST(ServiceWorkerMetricsTest, NavigationPreloadResponse_BrowserStartup) {
  base::TimeDelta worker_start = base::TimeDelta::FromMilliseconds(2345);
  base::TimeDelta response_start = base::TimeDelta::FromMilliseconds(456);
  EmbeddedWorkerStatus initial_worker_status = EmbeddedWorkerStatus::STOPPED;
  StartSituation start_situation = StartSituation::DURING_STARTUP;
  base::HistogramTester histogram_tester;
  ServiceWorkerMetrics::RecordNavigationPreloadResponse(
      worker_start, response_start, initial_worker_status, start_situation,
      RESOURCE_TYPE_MAIN_FRAME);
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.NavPreload.WorkerPreparationType_MainFrame",
      static_cast<int>(WorkerPreparationType::START_DURING_STARTUP), 1);
}

TEST(ServiceWorkerMetricsTest, EmbeddedWorkerStartTiming) {
  ServiceWorkerMetrics::StartTimes times;
  auto current = base::TimeTicks::Now();
  times.local_start = current;
  times.local_start_worker_sent = AdvanceTime(&current, 11);
  times.remote_start_worker_received = AdvanceTime(&current, 33);
  times.remote_script_evaluation_start = AdvanceTime(&current, 55);
  times.remote_script_evaluation_end = AdvanceTime(&current, 77);
  times.local_end = AdvanceTime(&current, 22);

  StartSituation start_situation = StartSituation::EXISTING_READY_PROCESS;
  base::HistogramTester histogram_tester;

  ServiceWorkerMetrics::RecordStartWorkerTiming(times, start_situation);

  // Total duration.
  histogram_tester.ExpectTimeBucketCount("ServiceWorker.StartTiming.Duration",
                                         times.local_end - times.local_start,
                                         1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.Duration.ExistingReadyProcess",
      times.local_end - times.local_start, 1);

  // SentStartWorker milestone.
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.StartToSentStartWorker",
      times.local_start_worker_sent - times.local_start, 1);

  // ReceivedStartWorker milestone.
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.StartToReceivedStartWorker",
      times.remote_start_worker_received - times.local_start, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.SentStartWorkerToReceivedStartWorker",
      times.remote_start_worker_received - times.local_start_worker_sent, 1);

  // ScriptEvaluationStart milestone.
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.StartToScriptEvaluationStart",
      times.remote_script_evaluation_start - times.local_start, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.ReceivedStartWorkerToScriptEvaluationStart",
      times.remote_script_evaluation_start - times.remote_start_worker_received,
      1);

  // ScriptEvaluationEnd milestone.
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.StartToScriptEvaluationEnd",
      times.remote_script_evaluation_end - times.local_start, 1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.ScriptEvaluationStartToScriptEvaluationEnd",
      times.remote_script_evaluation_end - times.remote_script_evaluation_start,
      1);

  // End milestone.
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.ScriptEvaluationEndToEnd",
      times.local_end - times.remote_script_evaluation_end, 1);

  // Clock consistency.
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.StartTiming.ClockConsistency",
      CrossProcessTimeDelta::NORMAL, 1);
}

TEST(ServiceWorkerMetricsTest, EmbeddedWorkerStartTiming_BrowserStartup) {
  ServiceWorkerMetrics::StartTimes times;
  auto current = base::TimeTicks::Now();
  times.local_start = current;
  times.local_start_worker_sent = AdvanceTime(&current, 11);
  times.remote_start_worker_received = AdvanceTime(&current, 66);
  times.remote_script_evaluation_start = AdvanceTime(&current, 55);
  times.remote_script_evaluation_end = AdvanceTime(&current, 77);
  times.local_end = AdvanceTime(&current, 22);

  StartSituation start_situation = StartSituation::DURING_STARTUP;
  base::HistogramTester histogram_tester;

  ServiceWorkerMetrics::RecordStartWorkerTiming(times, start_situation);

  // Total duration.
  histogram_tester.ExpectTimeBucketCount("ServiceWorker.StartTiming.Duration",
                                         times.local_end - times.local_start,
                                         1);
  histogram_tester.ExpectTimeBucketCount(
      "ServiceWorker.StartTiming.Duration.DuringStartup",
      times.local_end - times.local_start, 1);
}

TEST(ServiceWorkerMetricsTest,
     EmbeddedWorkerStartTiming_NegativeLatencyForStartIPC) {
  ServiceWorkerMetrics::StartTimes times;
  auto current = base::TimeTicks::Now();
  times.local_start = current;
  times.local_start_worker_sent = AdvanceTime(&current, 11);
  // Go back in time.
  times.remote_start_worker_received = AdvanceTime(&current, -777);
  times.remote_script_evaluation_start = AdvanceTime(&current, 55);
  times.remote_script_evaluation_end = AdvanceTime(&current, 77);
  times.local_end = AdvanceTime(&current, 22);

  StartSituation start_situation = StartSituation::EXISTING_READY_PROCESS;
  base::HistogramTester histogram_tester;

  ServiceWorkerMetrics::RecordStartWorkerTiming(times, start_situation);

  // Duration and breakdowns should not be logged.
  histogram_tester.ExpectTotalCount("ServiceWorker.StartTiming.Duration", 0);
  // Just test one arbitrarily chosen breakdown metric.
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.StartTiming.StartToScriptEvaluationStart", 0);

  // Clock consistency.
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.StartTiming.ClockConsistency",
      CrossProcessTimeDelta::NEGATIVE, 1);
}

TEST(ServiceWorkerMetricsTest,
     EmbeddedWorkerStartTiming_NegativeLatencyForStartedIPC) {
  ServiceWorkerMetrics::StartTimes times;
  auto current = base::TimeTicks::Now();
  times.local_start = current;
  times.local_start_worker_sent = AdvanceTime(&current, 11);
  times.remote_start_worker_received = AdvanceTime(&current, 777);
  times.remote_script_evaluation_start = AdvanceTime(&current, 55);
  times.remote_script_evaluation_end = AdvanceTime(&current, 77);
  // Go back in time.
  times.local_end = AdvanceTime(&current, -123);

  StartSituation start_situation = StartSituation::EXISTING_READY_PROCESS;
  base::HistogramTester histogram_tester;

  ServiceWorkerMetrics::RecordStartWorkerTiming(times, start_situation);

  // Duration and breakdowns should not be logged.
  histogram_tester.ExpectTotalCount("ServiceWorker.StartTiming.Duration", 0);
  // Just test one arbitrarily chosen breakdown metric.
  histogram_tester.ExpectTotalCount(
      "ServiceWorker.StartTiming.ScriptEvaluationStartToScriptEvaluationEnd",
      0);

  // Clock consistency.
  histogram_tester.ExpectUniqueSample(
      "ServiceWorker.StartTiming.ClockConsistency",
      CrossProcessTimeDelta::NEGATIVE, 1);
}

}  // namespace service_worker_metrics_unittest
}  // namespace content
