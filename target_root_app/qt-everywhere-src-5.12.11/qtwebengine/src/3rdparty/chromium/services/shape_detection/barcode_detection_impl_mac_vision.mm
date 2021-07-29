// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/shape_detection/barcode_detection_impl_mac_vision.h"

#include <Foundation/Foundation.h>
#include <utility>

#include "base/logging.h"

namespace shape_detection {

BarcodeDetectionImplMacVision::BarcodeDetectionImplMacVision(
    mojom::BarcodeDetectorOptionsPtr options) {
  Class request_class = NSClassFromString(@"VNDetectBarcodesRequest");
  if (!request_class) {
    DLOG(ERROR) << "Failed to load VNDetectBarcodesRequest class";
    return;
  }
}

BarcodeDetectionImplMacVision::~BarcodeDetectionImplMacVision() = default;

void BarcodeDetectionImplMacVision::Detect(const SkBitmap& bitmap,
                                           DetectCallback callback) {
  std::move(callback).Run({});
}

}  // namespace shape_detection
