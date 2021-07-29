// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webgl/webgl2_compute_rendering_context_base.h"

#include "third_party/blink/public/platform/web_graphics_context_3d_provider.h"

namespace blink {

WebGL2ComputeRenderingContextBase::WebGL2ComputeRenderingContextBase(
    CanvasRenderingContextHost* host,
    std::unique_ptr<WebGraphicsContext3DProvider> context_provider,
    bool using_gpu_compositing,
    const CanvasContextCreationAttributesCore& requested_attributes)
    : WebGL2RenderingContextBase(host,
                                 std::move(context_provider),
                                 using_gpu_compositing,
                                 requested_attributes,
                                 Platform::kWebGL2ComputeContextType) {}

void WebGL2ComputeRenderingContextBase::DestroyContext() {
  WebGL2RenderingContextBase::DestroyContext();
}

void WebGL2ComputeRenderingContextBase::InitializeNewContext() {
  DCHECK(!isContextLost());

  WebGL2RenderingContextBase::InitializeNewContext();
}

void WebGL2ComputeRenderingContextBase::dispatchCompute(GLuint numGroupsX,
                                                        GLuint numGroupsY,
                                                        GLuint numGroupsZ) {
  SynthesizeGLError(GL_INVALID_OPERATION, "dispatchCompute", "UNIMPLEMENTED");
  return;
}

void WebGL2ComputeRenderingContextBase::bindImageTexture(GLuint unit,
                                                         WebGLTexture* texture,
                                                         GLint level,
                                                         GLboolean layered,
                                                         GLint layer,
                                                         GLenum access,
                                                         GLenum format) {
  SynthesizeGLError(GL_INVALID_OPERATION, "bindImageTexture", "UNIMPLEMENTED");
}

void WebGL2ComputeRenderingContextBase::memoryBarrier(GLbitfield barriers) {
  SynthesizeGLError(GL_INVALID_OPERATION, "memoryBarrier", "UNIMPLEMENTED");
}

void WebGL2ComputeRenderingContextBase::memoryBarrierByRegion(
    GLbitfield barriers) {
  SynthesizeGLError(GL_INVALID_OPERATION, "memoryBarrierByRegion",
                    "UNIMPLEMENTED");
}

void WebGL2ComputeRenderingContextBase::Trace(blink::Visitor* visitor) {
  WebGL2RenderingContextBase::Trace(visitor);
}

}  // namespace blink
