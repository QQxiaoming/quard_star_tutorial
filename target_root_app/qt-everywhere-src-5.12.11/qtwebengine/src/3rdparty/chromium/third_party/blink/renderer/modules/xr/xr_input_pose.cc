// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_input_pose.h"

#include "third_party/blink/renderer/modules/xr/xr_utils.h"

namespace blink {

XRInputPose::XRInputPose(std::unique_ptr<TransformationMatrix> pointer_matrix,
                         std::unique_ptr<TransformationMatrix> grip_matrix,
                         bool emulated_position)
    : target_ray_(new XRRay(std::move(pointer_matrix))),
      grip_matrix_(std::move(grip_matrix)),
      emulated_position_(emulated_position) {}

XRInputPose::~XRInputPose() {}

DOMFloat32Array* XRInputPose::gripMatrix() const {
  if (!grip_matrix_)
    return nullptr;
  return transformationMatrixToFloat32Array(*grip_matrix_);
}

void XRInputPose::Trace(blink::Visitor* visitor) {
  visitor->Trace(target_ray_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
