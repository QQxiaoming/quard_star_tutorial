// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_METRICS_CALL_STACK_PROFILE_BUILDER_H_
#define COMPONENTS_METRICS_CALL_STACK_PROFILE_BUILDER_H_

#include "base/profiler/stack_sampling_profiler.h"

#include <map>

#include "base/callback.h"

namespace metrics {

// CallStackProfileBuilder builds a CallStackProfile from the collected sampling
// data.
//
// The results of the profile building -- a CallStackProfile, is passed to the
// completed callback. A CallStackProfile contains a set of Samples and
// Modules, and other sampling information. One Sample corresponds to a single
// recorded stack, and the Modules record those modules associated with the
// recorded stack frames.
class CallStackProfileBuilder
    : public base::StackSamplingProfiler::ProfileBuilder {
 public:
  // The callback type used to collect a completed profile. The passed
  // CallStackProfile is move-only. Other threads, including the UI thread, may
  // block on callback completion so this should run as quickly as possible.
  //
  // IMPORTANT NOTE: The callback is invoked on a thread the profiler
  // constructs, rather than on the thread used to construct the profiler, and
  // thus the callback must be callable on any thread. For threads with message
  // loops that create CallStackProfileBuilders, posting a task to the message
  // loop with the moved (i.e. std::move) profile is the thread-safe callback
  // implementation.
  using CompletedCallback =
      base::Callback<void(base::StackSamplingProfiler::CallStackProfile)>;

  CallStackProfileBuilder(const CompletedCallback& callback);

  ~CallStackProfileBuilder() override;

  // base::StackSamplingProfiler::ProfileBuilder:
  void RecordAnnotations() override;
  void OnSampleCompleted(std::vector<base::StackSamplingProfiler::InternalFrame>
                             internal_frames) override;
  void OnProfileCompleted(base::TimeDelta profile_duration,
                          base::TimeDelta sampling_period) override;

  // Sets the current system state that is recorded with each captured stack
  // frame. This is thread-safe so can be called from anywhere. The parameter
  // value should be from an enumeration of the appropriate type with values
  // ranging from 0 to 31, inclusive. This sets bits within Sample field of
  // |process_milestones|. The actual meanings of these bits are defined
  // (globally) by the caller(s).
  static void SetProcessMilestone(int milestone);

 private:
  // The collected stack samples.
  base::StackSamplingProfiler::CallStackProfile profile_;

  // The current sample being recorded.
  base::StackSamplingProfiler::Sample sample_;

  // The indexes of internal modules, indexed by module's base_address.
  std::map<uintptr_t, size_t> module_index_;

  // Callback made when sampling a profile completes.
  const CompletedCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(CallStackProfileBuilder);
};

}  // namespace metrics

#endif  // COMPONENTS_METRICS_CALL_STACK_PROFILE_BUILDER_H_
