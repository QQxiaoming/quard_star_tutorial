// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/frame_sinks/external_begin_frame_source_mojo.h"

namespace viz {

ExternalBeginFrameSourceMojo::ExternalBeginFrameSourceMojo(
    mojom::ExternalBeginFrameControllerAssociatedRequest controller_request,
    mojom::ExternalBeginFrameControllerClientPtr client)
    : ExternalBeginFrameSource(this),
      binding_(this, std::move(controller_request)),
      client_(std::move(client)) {}

ExternalBeginFrameSourceMojo::~ExternalBeginFrameSourceMojo() {
  DCHECK(!display_);
}

void ExternalBeginFrameSourceMojo::IssueExternalBeginFrame(
    const BeginFrameArgs& args) {
  OnBeginFrame(args);

  // Ensure that Display will receive the BeginFrame (as a missed one), even
  // if it doesn't currently need it. This way, we ensure that
  // OnDisplayDidFinishFrame will be called for this BeginFrame.
  DCHECK(display_);
  display_->SetNeedsOneBeginFrame();
}

void ExternalBeginFrameSourceMojo::OnNeedsBeginFrames(bool needs_begin_frames) {
  needs_begin_frames_ = needs_begin_frames;
  client_->OnNeedsBeginFrames(needs_begin_frames_);
}

void ExternalBeginFrameSourceMojo::OnDisplayDidFinishFrame(
    const BeginFrameAck& ack) {
  client_->OnDisplayDidFinishFrame(ack);
}

void ExternalBeginFrameSourceMojo::OnDisplayDestroyed() {
  // As part of destruction, we are automatically removed as a display
  // observer. No need to call RemoveObserver.
  display_ = nullptr;
}

void ExternalBeginFrameSourceMojo::SetDisplay(Display* display) {
  if (display_)
    display_->RemoveObserver(this);
  display_ = display;
  if (display_)
    display_->AddObserver(this);
}

}  // namespace viz
