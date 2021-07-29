// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_SERVICE_FRAME_SINKS_EXTERNAL_BEGIN_FRAME_SOURCE_MOJO_H_
#define COMPONENTS_VIZ_SERVICE_FRAME_SINKS_EXTERNAL_BEGIN_FRAME_SOURCE_MOJO_H_

#include <memory>

#include "components/viz/common/frame_sinks/begin_frame_args.h"
#include "components/viz/common/frame_sinks/begin_frame_source.h"
#include "components/viz/service/display/display.h"
#include "mojo/public/cpp/bindings/associated_binding.h"
#include "services/viz/privileged/interfaces/compositing/external_begin_frame_controller.mojom.h"

namespace viz {

// Implementation of ExternalBeginFrameSource that's controlled by IPCs over
// the mojom::ExternalBeginFrameController interface. Replaces the Display's
// default BeginFrameSource. Observes the Display to be notified of BeginFrame
// completion.
class VIZ_SERVICE_EXPORT ExternalBeginFrameSourceMojo
    : public mojom::ExternalBeginFrameController,
      public ExternalBeginFrameSourceClient,
      public DisplayObserver,
      public ExternalBeginFrameSource {
 public:
  ExternalBeginFrameSourceMojo(
      mojom::ExternalBeginFrameControllerAssociatedRequest controller_request,
      mojom::ExternalBeginFrameControllerClientPtr client);
  ~ExternalBeginFrameSourceMojo() override;

  // mojom::ExternalBeginFrameController implementation.
  void IssueExternalBeginFrame(const BeginFrameArgs& args) override;

  void SetDisplay(Display* display);

 private:
  // ExternalBeginFrameSourceClient implementation.
  void OnNeedsBeginFrames(bool needs_begin_frames) override;

  // DisplayObserver implementation.
  void OnDisplayDidFinishFrame(const BeginFrameAck& ack) override;
  void OnDisplayDestroyed() override;

  mojo::AssociatedBinding<mojom::ExternalBeginFrameController> binding_;
  mojom::ExternalBeginFrameControllerClientPtr client_;

  bool needs_begin_frames_ = false;
  Display* display_ = nullptr;
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_SERVICE_FRAME_SINKS_EXTERNAL_BEGIN_FRAME_SOURCE_MOJO_H_
