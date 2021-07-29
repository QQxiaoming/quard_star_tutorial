// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_VR_DEVICE_H
#define DEVICE_VR_VR_DEVICE_H

#include "base/callback.h"
#include "base/macros.h"
#include "device/vr/public/mojom/isolated_xr_service.mojom.h"
#include "device/vr/public/mojom/vr_service.mojom.h"
#include "device/vr/vr_export.h"

namespace device {

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class VrViewerType {
  GVR_UNKNOWN = 0,
  GVR_CARDBOARD = 1,
  GVR_DAYDREAM = 2,
  ORIENTATION_SENSOR_DEVICE = 10,
  FAKE_DEVICE = 11,
  OPENVR_UNKNOWN = 20,
  OPENVR_VIVE = 21,
  OPENVR_RIFT_CV1 = 22,
  VIEWER_TYPE_COUNT,
};

// Hardcoded list of ids for each device type.
enum class VRDeviceId : unsigned int {
  GVR_DEVICE_ID = 1,
  OPENVR_DEVICE_ID = 2,
  OCULUS_DEVICE_ID = 3,
  ARCORE_DEVICE_ID = 4,
  ORIENTATION_DEVICE_ID = 5,
  FAKE_DEVICE_ID = 6,
};

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class XrRuntimeAvailable {
  NONE = 0,
  OPENVR = 1,
  COUNT,
};

}  // namespace device

#endif  // DEVICE_VR_VR_DEVICE_H
