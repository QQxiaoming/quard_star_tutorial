// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/call_stack_profile_builder.h"

#include "base/files/file_path.h"
#include "base/test/bind_test_util.h"
#include "base/time/time.h"
#include "testing/gtest/include/gtest/gtest.h"

using StackSamplingProfiler = base::StackSamplingProfiler;
using InternalFrame = StackSamplingProfiler::InternalFrame;
using InternalModule = StackSamplingProfiler::InternalModule;
using CallStackProfile = StackSamplingProfiler::CallStackProfile;

namespace metrics {

namespace {

// Called on the profiler thread when complete, to collect profile.
void SaveProfile(CallStackProfile* profile, CallStackProfile pending_profile) {
  *profile = std::move(pending_profile);
}

}  // namespace

TEST(CallStackProfileBuilderTest, SetProcessMilestone) {
  CallStackProfile profile;

  // Set up a callback to record the CallStackProfile to local variable
  // |profile|.
  auto profile_builder = std::make_unique<CallStackProfileBuilder>(
      Bind(&SaveProfile, Unretained(&profile)));

  profile_builder->RecordAnnotations();
  profile_builder->OnSampleCompleted(std::vector<InternalFrame>());

  CallStackProfileBuilder::SetProcessMilestone(1);
  profile_builder->RecordAnnotations();
  profile_builder->OnSampleCompleted(std::vector<InternalFrame>());

  profile_builder->OnProfileCompleted(base::TimeDelta(), base::TimeDelta());

  ASSERT_EQ(2u, profile.samples.size());
  EXPECT_EQ(0u, profile.samples[0].process_milestones);
  EXPECT_EQ(1u << 1, profile.samples[1].process_milestones);
}

TEST(CallStackProfileBuilderTest, OnSampleCompleted) {
  CallStackProfile profile;

  // Set up a callback to record the CallStackProfile to local variable
  // |profile|.
  auto profile_builder = std::make_unique<CallStackProfileBuilder>(
      Bind(&SaveProfile, Unretained(&profile)));

  InternalModule module1 = {0xccccdddd, "1",
                            base::FilePath(FILE_PATH_LITERAL("file_path_1"))};
  InternalModule module2 = {0xccddccdd, "2",
                            base::FilePath(FILE_PATH_LITERAL("file_path_2"))};
  InternalFrame frame1 = {0xaaaabbbb, module1};
  InternalFrame frame2 = {0xaabbaabb, module2};

  std::vector<InternalFrame> frames = {frame1, frame2};

  profile_builder->OnSampleCompleted(frames);
  profile_builder->OnProfileCompleted(base::TimeDelta(), base::TimeDelta());

  ASSERT_EQ(1u, profile.samples.size());
  ASSERT_EQ(2u, profile.samples[0].frames.size());
  EXPECT_EQ(0u, profile.samples[0].frames[0].module_index);
  EXPECT_EQ(1u, profile.samples[0].frames[1].module_index);
}

TEST(CallStackProfileBuilderTest, OnProfileCompleted) {
  CallStackProfile profile;

  // Set up a callback to record the CallStackProfile to local variable
  // |profile|.
  auto profile_builder = std::make_unique<CallStackProfileBuilder>(
      Bind(&SaveProfile, Unretained(&profile)));

  InternalModule module1 = {0xccccdddd, "1",
                            base::FilePath(FILE_PATH_LITERAL("file_path_1"))};
  InternalModule module2 = {0xccddccdd, "2",
                            base::FilePath(FILE_PATH_LITERAL("file_path_2"))};
  InternalFrame frame1 = {0xaaaabbbb, module1};
  InternalFrame frame2 = {0xaabbaabb, module2};

  std::vector<InternalFrame> frames = {frame1, frame2};

  profile_builder->OnSampleCompleted(frames);
  profile_builder->OnProfileCompleted(base::TimeDelta::FromMilliseconds(500),
                                      base::TimeDelta::FromMilliseconds(100));

  ASSERT_EQ(1u, profile.samples.size());
  ASSERT_EQ(2u, profile.samples[0].frames.size());
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(500), profile.profile_duration);
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(100), profile.sampling_period);
}

TEST(CallStackProfileBuilderTest, InvalidModule) {
  CallStackProfile profile;

  // Set up a callback to record the CallStackProfile to local variable
  // |profile|.
  auto profile_builder = std::make_unique<CallStackProfileBuilder>(
      Bind(&SaveProfile, Unretained(&profile)));

  InternalModule module1;
  InternalModule module2 = {0xccddccdd, "2",
                            base::FilePath(FILE_PATH_LITERAL("file_path_2"))};
  InternalFrame frame1 = {0xaaaabbbb, module1};
  InternalFrame frame2 = {0xaabbaabb, module2};

  std::vector<InternalFrame> frames = {frame1, frame2};

  profile_builder->OnSampleCompleted(frames);
  profile_builder->OnProfileCompleted(base::TimeDelta(), base::TimeDelta());

  ASSERT_EQ(1u, profile.samples.size());
  ASSERT_EQ(2u, profile.samples[0].frames.size());

  // module1 has no information hence invalid. The module index of the frame is
  // therefore base::kUnknownModuleIndex.
  EXPECT_EQ(base::kUnknownModuleIndex,
            profile.samples[0].frames[0].module_index);

  EXPECT_EQ(0u, profile.samples[0].frames[1].module_index);
}

TEST(CallStackProfileBuilderTest, DedupModules) {
  CallStackProfile profile;
  auto profile_builder = std::make_unique<CallStackProfileBuilder>(
      Bind(&SaveProfile, Unretained(&profile)));

  InternalModule module1 = {0xccccdddd, "1",
                            base::FilePath(FILE_PATH_LITERAL("file_path_1"))};
  InternalModule module2 = {0xccccdddd, "2",
                            base::FilePath(FILE_PATH_LITERAL("file_path_2"))};
  InternalFrame frame1 = {0xaaaabbbb, module1};
  InternalFrame frame2 = {0xaabbaabb, module2};

  std::vector<InternalFrame> frames = {frame1, frame2};

  profile_builder->OnSampleCompleted(frames);
  profile_builder->OnProfileCompleted(base::TimeDelta(), base::TimeDelta());

  ASSERT_EQ(1u, profile.samples.size());
  ASSERT_EQ(2u, profile.samples[0].frames.size());

  // Since module1 and module2 have the same base address 0xccccdddd, they are
  // considered the same module and therefore deduped.
  EXPECT_EQ(0u, profile.samples[0].frames[0].module_index);
  EXPECT_EQ(0u, profile.samples[0].frames[1].module_index);
}

}  // namespace metrics
