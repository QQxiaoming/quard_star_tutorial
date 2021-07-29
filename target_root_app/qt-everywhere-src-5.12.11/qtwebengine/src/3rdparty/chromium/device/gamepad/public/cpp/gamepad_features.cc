// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/gamepad/public/cpp/gamepad_features.h"

#include <algorithm>

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "device/gamepad/public/cpp/gamepad_switches.h"

namespace features {

namespace {

const size_t kPollingIntervalMillisecondsMin = 4;   // ~250 Hz
const size_t kPollingIntervalMillisecondsMax = 16;  // ~62.5 Hz

size_t OverrideIntervalIfValid(base::StringPiece param_value,
                               size_t default_interval) {
  size_t interval;
  if (param_value.empty() || !base::StringToSizeT(param_value, &interval))
    return default_interval;
  // Clamp interval duration to valid range.
  interval = std::max(interval, kPollingIntervalMillisecondsMin);
  interval = std::min(interval, kPollingIntervalMillisecondsMax);
  return interval;
}

}  // namespace

const base::Feature kGamepadPollingInterval{"GamepadPollingInterval",
                                            base::FEATURE_DISABLED_BY_DEFAULT};

const char kGamepadPollingIntervalParamKey[] = "interval-ms";

size_t GetGamepadPollingInterval() {
  size_t polling_interval = kPollingIntervalMillisecondsMax;

  // Check if the polling interval is overridden by a field trial.
  if (base::FeatureList::IsEnabled(kGamepadPollingInterval)) {
    std::string param_value = base::GetFieldTrialParamValueByFeature(
        kGamepadPollingInterval, kGamepadPollingIntervalParamKey);
    polling_interval = OverrideIntervalIfValid(param_value, polling_interval);
  }

  // Check if the polling interval is overridden by a command-line flag.
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line &&
      command_line->HasSwitch(switches::kGamepadPollingInterval)) {
    std::string switch_value =
        command_line->GetSwitchValueASCII(switches::kGamepadPollingInterval);
    polling_interval = OverrideIntervalIfValid(switch_value, polling_interval);
  }

  return polling_interval;
}

}  // namespace features
