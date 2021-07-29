// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_SHAPE_DETECTION_BARCODE_DETECTION_IMPL_MAC_VISION_H_
#define SERVICES_SHAPE_DETECTION_BARCODE_DETECTION_IMPL_MAC_VISION_H_

#include "base/mac/availability.h"
#include "base/macros.h"
#include "services/shape_detection/public/mojom/barcodedetection.mojom.h"
#include "services/shape_detection/public/mojom/barcodedetection_provider.mojom.h"

class SkBitmap;

namespace shape_detection {

// This class is the implementation of Barcode Detection based on Mac OS Vision
// framework(https://developer.apple.com/documentation/vision).
class API_AVAILABLE(macos(10.13)) BarcodeDetectionImplMacVision
    : public mojom::BarcodeDetection {
 public:
  explicit BarcodeDetectionImplMacVision(
      mojom::BarcodeDetectorOptionsPtr options);
  ~BarcodeDetectionImplMacVision() override;

  void Detect(const SkBitmap& bitmap,
              mojom::BarcodeDetection::DetectCallback callback) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(BarcodeDetectionImplMacVision);
};

}  // namespace shape_detection

#endif  // SERVICES_SHAPE_DETECTION_BARCODE_DETECTION_IMPL_MAC_VISION_H_
