// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/frame/user_activation.h"

#include "third_party/blink/renderer/core/frame/local_dom_window.h"

namespace blink {

UserActivation::UserActivation(LocalDOMWindow* window) : window_(window) {}

UserActivation::~UserActivation() = default;

void UserActivation::Trace(blink::Visitor* visitor) {
  visitor->Trace(window_);
  ScriptWrappable::Trace(visitor);
}

bool UserActivation::hasBeenActive() const {
  LocalFrame* frame = window_ ? window_->GetFrame() : nullptr;
  if (!frame)
    return false;
  return frame->HasBeenActivated();
}

bool UserActivation::isActive() const {
  LocalFrame* frame = window_ ? window_->GetFrame() : nullptr;
  if (!frame)
    return false;
  return Frame::HasTransientUserActivation(frame);
}

}  // namespace blink
