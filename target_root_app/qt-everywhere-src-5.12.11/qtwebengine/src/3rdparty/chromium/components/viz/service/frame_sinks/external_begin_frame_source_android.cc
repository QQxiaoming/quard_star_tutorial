// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/frame_sinks/external_begin_frame_source_android.h"

#include "base/android/jni_android.h"
#include "jni/ExternalBeginFrameSourceAndroid_jni.h"

namespace viz {

ExternalBeginFrameSourceAndroid::ExternalBeginFrameSourceAndroid()
    : ExternalBeginFrameSource(this),
      j_object_(Java_ExternalBeginFrameSourceAndroid_Constructor(
          base::android::AttachCurrentThread(),
          reinterpret_cast<jlong>(this))) {}

ExternalBeginFrameSourceAndroid::~ExternalBeginFrameSourceAndroid() {
  SetEnabled(false);
}

void ExternalBeginFrameSourceAndroid::OnVSync(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    jlong time_micros,
    jlong period_micros) {
  // TODO(ericrk): This logic is ported from window_android.cc. Once OOP-D
  // conversion is complete, we can delete the logic there.

  // Warning: It is generally unsafe to manufacture TimeTicks values. The
  // following assumption is being made, AND COULD EASILY BREAK AT ANY TIME:
  // Upstream, Java code is providing "System.nanos() / 1000," and this is the
  // same timestamp that would be provided by the CLOCK_MONOTONIC POSIX clock.
  DCHECK_EQ(base::TimeTicks::GetClock(),
            base::TimeTicks::Clock::LINUX_CLOCK_MONOTONIC);
  base::TimeTicks frame_time =
      base::TimeTicks() + base::TimeDelta::FromMicroseconds(time_micros);
  base::TimeDelta vsync_period(
      base::TimeDelta::FromMicroseconds(period_micros));

  // Calculate the next frame deadline:
  base::TimeTicks deadline = frame_time + vsync_period;
  auto begin_frame_args = BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, source_id(), next_sequence_number_++, frame_time,
      deadline, vsync_period, BeginFrameArgs::NORMAL);

  OnBeginFrame(begin_frame_args);
}

void ExternalBeginFrameSourceAndroid::OnNeedsBeginFrames(
    bool needs_begin_frames) {
  SetEnabled(needs_begin_frames);
}

void ExternalBeginFrameSourceAndroid::SetEnabled(bool enabled) {
  Java_ExternalBeginFrameSourceAndroid_setEnabled(
      base::android::AttachCurrentThread(), j_object_, enabled);
}

}  // namespace viz
