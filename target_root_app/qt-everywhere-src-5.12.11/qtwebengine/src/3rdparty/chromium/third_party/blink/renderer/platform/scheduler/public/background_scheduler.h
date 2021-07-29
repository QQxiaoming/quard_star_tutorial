// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_SCHEDULER_PUBLIC_BACKGROUND_SCHEDULER_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_SCHEDULER_PUBLIC_BACKGROUND_SCHEDULER_H_

#include "base/location.h"
#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/wtf/functional.h"

namespace blink {

namespace BackgroundScheduler {

// This is a thin wrapper around base::TaskScheduler to accomodate
// Blink's CrossThreadClosure, which only allows background tasks.
//
// Non-background tasks should be posted using another scheduler, e.g.
// FrameShceduler.
PLATFORM_EXPORT void PostOnBackgroundThread(const base::Location&,
                                            CrossThreadClosure);

}  // namespace BackgroundScheduler

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_SCHEDULER_PUBLIC_BACKGROUND_SCHEDULER_H_
