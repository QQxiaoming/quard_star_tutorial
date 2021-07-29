// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/animation/compositor_animation_host.h"

#include <memory>

#include "base/memory/scoped_refptr.h"
#include "cc/animation/animation_host.h"
#include "third_party/blink/renderer/platform/animation/compositor_animation_timeline.h"
#include "third_party/blink/renderer/platform/testing/compositor_test.h"

namespace blink {

class CompositorAnimationHostTest : public CompositorTest {};

TEST_F(CompositorAnimationHostTest, AnimationHostNullWhenTimelineDetached) {
  std::unique_ptr<CompositorAnimationTimeline> timeline =
      CompositorAnimationTimeline::Create();

  scoped_refptr<cc::AnimationTimeline> cc_timeline =
      timeline->GetAnimationTimeline();
  EXPECT_FALSE(cc_timeline->animation_host());

  std::unique_ptr<cc::AnimationHost> animation_host =
      cc::AnimationHost::CreateMainInstance();
  CompositorAnimationHost compositor_animation_host(animation_host.get());

  compositor_animation_host.AddTimeline(*timeline);
  EXPECT_TRUE(cc_timeline->animation_host());

  compositor_animation_host.RemoveTimeline(*timeline);
  EXPECT_FALSE(cc_timeline->animation_host());
}

}  // namespace blink
