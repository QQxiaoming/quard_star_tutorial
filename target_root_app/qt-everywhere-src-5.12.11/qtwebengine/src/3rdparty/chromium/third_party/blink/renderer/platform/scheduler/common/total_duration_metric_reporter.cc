// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/scheduler/common/total_duration_metric_reporter.h"

namespace blink {
namespace scheduler {

namespace {

constexpr base::TimeDelta kMinimalValue = base::TimeDelta::FromSeconds(1);
constexpr base::TimeDelta kMaximalValue = base::TimeDelta::FromHours(1);
constexpr int kBucketCount = 50;

}  // namespace

TotalDurationMetricReporter::TotalDurationMetricReporter(
    const char* positive_histogram_name,
    const char* negative_histogram_name)
    : positive_histogram_(base::Histogram::FactoryGet(
          positive_histogram_name,
          kMinimalValue.InSeconds(),
          kMaximalValue.InSeconds(),
          kBucketCount,
          base::Histogram::kUmaTargetedHistogramFlag)),
      negative_histogram_(base::Histogram::FactoryGet(
          negative_histogram_name,
          kMinimalValue.InSeconds(),
          kMaximalValue.InSeconds(),
          kBucketCount,
          base::Histogram::kUmaTargetedHistogramFlag)) {}

void TotalDurationMetricReporter::RecordAdditionalDuration(
    base::TimeDelta duration) {
  if (reported_value_)
    negative_histogram_->Add(reported_value_->InSeconds());
  reported_value_ = reported_value_.value_or(base::TimeDelta()) + duration;
  positive_histogram_->Add(reported_value_->InSeconds());
}

void TotalDurationMetricReporter::Reset() {
  reported_value_ = base::nullopt;
}

}  // namespace scheduler
}  // namespace blink
