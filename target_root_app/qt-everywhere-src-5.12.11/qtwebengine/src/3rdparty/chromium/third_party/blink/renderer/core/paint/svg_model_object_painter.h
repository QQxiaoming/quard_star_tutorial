// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_PAINT_SVG_MODEL_OBJECT_PAINTER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_PAINT_SVG_MODEL_OBJECT_PAINTER_H_

#include "third_party/blink/renderer/platform/wtf/allocator.h"

namespace blink {

struct PaintInfo;
class LayoutSVGModelObject;

class SVGModelObjectPainter {
  STACK_ALLOCATED();

 public:
  SVGModelObjectPainter(const LayoutSVGModelObject& layout_svg_model_object)
      : layout_svg_model_object_(layout_svg_model_object) {}

  // If the object is outside the cull rect, painting can be skipped in most
  // cases. An important exception is when there is a transform style: see the
  // comment in the implementation.
  bool CullRectSkipsPainting(const PaintInfo&);

  void PaintOutline(const PaintInfo&);

 private:
  const LayoutSVGModelObject& layout_svg_model_object_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_PAINT_SVG_MODEL_OBJECT_PAINTER_H_
