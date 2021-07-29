// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/scheduler/public/background_scheduler.h"

#include "base/location.h"
#include "base/task_scheduler/post_task.h"

namespace blink {

void BackgroundScheduler::PostOnBackgroundThread(const base::Location& location,
                                                 CrossThreadClosure closure) {
  base::PostTaskWithTraits(location,
                           {base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
                           ConvertToBaseCallback(std::move(closure)));
}

}  // namespace blink
