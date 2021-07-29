// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/call_stack_profile_builder.h"

#include <utility>

#include "base/atomicops.h"
#include "base/logging.h"

using StackSamplingProfiler = base::StackSamplingProfiler;

namespace metrics {

namespace {

// This global variables holds the current system state and is recorded with
// every captured sample, done on a separate thread which is why updates to
// this must be atomic. A PostTask to move the the updates to that thread
// would skew the timing and a lock could result in deadlock if the thread
// making a change was also being profiled and got stopped.
static base::subtle::Atomic32 g_process_milestones = 0;

void ChangeAtomicFlags(base::subtle::Atomic32* flags,
                       base::subtle::Atomic32 set,
                       base::subtle::Atomic32 clear) {
  DCHECK(set != 0 || clear != 0);
  DCHECK_EQ(0, set & clear);

  base::subtle::Atomic32 bits = base::subtle::NoBarrier_Load(flags);
  while (true) {
    base::subtle::Atomic32 existing = base::subtle::NoBarrier_CompareAndSwap(
        flags, bits, (bits | set) & ~clear);
    if (existing == bits)
      break;
    bits = existing;
  }
}

}  // namespace

CallStackProfileBuilder::CallStackProfileBuilder(
    const CompletedCallback& callback)
    : callback_(callback) {}

CallStackProfileBuilder::~CallStackProfileBuilder() = default;

void CallStackProfileBuilder::RecordAnnotations() {
  // The code inside this method must not do anything that could acquire a
  // mutex, including allocating memory (which includes LOG messages) because
  // that mutex could be held by a stopped thread, thus resulting in deadlock.
  sample_.process_milestones =
      base::subtle::NoBarrier_Load(&g_process_milestones);
}

void CallStackProfileBuilder::OnSampleCompleted(
    std::vector<StackSamplingProfiler::InternalFrame> internal_frames) {
  DCHECK(sample_.frames.empty());

  // Dedup modules and convert InternalFrames to Frames.
  for (const auto& internal_frame : internal_frames) {
    const StackSamplingProfiler::InternalModule& module(
        internal_frame.internal_module);
    if (!module.is_valid) {
      sample_.frames.emplace_back(internal_frame.instruction_pointer,
                                  base::kUnknownModuleIndex);
      continue;
    }

    auto loc = module_index_.find(module.base_address);
    if (loc == module_index_.end()) {
      profile_.modules.emplace_back(module.base_address, module.id,
                                    module.filename);
      size_t index = profile_.modules.size() - 1;
      loc = module_index_.insert(std::make_pair(module.base_address, index))
                .first;
    }
    sample_.frames.emplace_back(internal_frame.instruction_pointer,
                                loc->second);
  }

  profile_.samples.push_back(std::move(sample_));
  sample_ = StackSamplingProfiler::Sample();
}

void CallStackProfileBuilder::OnProfileCompleted(
    base::TimeDelta profile_duration,
    base::TimeDelta sampling_period) {
  profile_.profile_duration = profile_duration;
  profile_.sampling_period = sampling_period;

  // Run the associated callback, passing the collected profile.
  callback_.Run(std::move(profile_));
}

// static
void CallStackProfileBuilder::SetProcessMilestone(int milestone) {
  DCHECK_LE(0, milestone);
  DCHECK_GT(static_cast<int>(sizeof(g_process_milestones) * 8), milestone);
  DCHECK_EQ(0, base::subtle::NoBarrier_Load(&g_process_milestones) &
                   (1 << milestone));
  ChangeAtomicFlags(&g_process_milestones, 1 << milestone, 0);
}

}  // namespace metrics
