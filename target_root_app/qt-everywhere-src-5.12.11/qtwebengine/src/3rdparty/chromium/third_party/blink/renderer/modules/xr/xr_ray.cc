// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_ray.h"

#include "third_party/blink/renderer/modules/xr/xr_utils.h"
#include "third_party/blink/renderer/platform/transforms/transformation_matrix.h"

namespace blink {
XRRay::XRRay(std::unique_ptr<TransformationMatrix> transform_matrix)
    : transform_matrix_(std::move(transform_matrix)) {
  FloatPoint3D origin = transform_matrix_->MapPoint(FloatPoint3D(0, 0, 0));
  FloatPoint3D dir = transform_matrix_->MapPoint(FloatPoint3D(0, 0, -1));
  dir.Move(-origin.X(), -origin.Y(), -origin.Z());
  dir.Normalize();

  origin_ = DOMPointReadOnly::Create(origin.X(), origin.Y(), origin.Z(), 1.0);
  direction_ = DOMPointReadOnly::Create(dir.X(), dir.Y(), dir.Z(), 0.0);
}

XRRay::~XRRay() {}

DOMFloat32Array* XRRay::transformMatrix() const {
  if (!transform_matrix_)
    return nullptr;
  // TODO(https://crbug.com/845296): rename
  // transformationMatrixToFloat32Array() to
  // TransformationMatrixToDOMFloat32Array().
  return transformationMatrixToFloat32Array(*transform_matrix_);
}

void XRRay::Trace(blink::Visitor* visitor) {
  visitor->Trace(origin_);
  visitor->Trace(direction_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
