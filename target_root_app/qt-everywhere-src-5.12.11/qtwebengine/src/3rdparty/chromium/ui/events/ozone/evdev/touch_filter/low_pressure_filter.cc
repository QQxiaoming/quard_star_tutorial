// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/events/ozone/evdev/touch_filter/low_pressure_filter.h"

#include <stddef.h>

#include <cmath>

#include "base/macros.h"

namespace ui {

namespace {

const float kMinPressure = 0.19;

}  // namespace

LowPressureFilter::LowPressureFilter() {}
LowPressureFilter::~LowPressureFilter() {}

void LowPressureFilter::Filter(
    const std::vector<InProgressTouchEvdev>& touches,
    base::TimeTicks time,
    std::bitset<kNumTouchEvdevSlots>* slots_should_delay) {
  for (const InProgressTouchEvdev& touch : touches) {
    size_t slot = touch.slot;

    if (!touch.touching && !touch.was_touching)
      continue;  // Only look at slots with active touches.

    if (!touch.was_touching)
      slots_filtered_.set(slot, true);  // Start tracking a new touch.

    if (touch.pressure >= kMinPressure)
      slots_filtered_.set(slot, false);  // Release touches above threshold.
  }

  (*slots_should_delay) |= slots_filtered_;
}

}  // namespace ui
