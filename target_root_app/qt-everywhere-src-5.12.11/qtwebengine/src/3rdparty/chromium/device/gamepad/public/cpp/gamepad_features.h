// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_GAMEPAD_PUBLIC_CPP_GAMEPAD_FEATURES_H_
#define DEVICE_GAMEPAD_PUBLIC_CPP_GAMEPAD_FEATURES_H_

#include "base/feature_list.h"

namespace features {

extern const base::Feature kGamepadPollingInterval;
extern const char kGamepadPollingIntervalParamKey[];

size_t GetGamepadPollingInterval();

}  // namespace features

#endif  // DEVICE_GAMEPAD_PUBLIC_CPP_GAMEPAD_FEATURES_H_
