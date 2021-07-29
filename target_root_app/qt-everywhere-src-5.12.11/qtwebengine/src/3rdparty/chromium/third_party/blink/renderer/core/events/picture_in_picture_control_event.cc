// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/events/picture_in_picture_control_event.h"

namespace blink {

PictureInPictureControlEvent* PictureInPictureControlEvent::Create(
    const AtomicString& type,
    String id) {
  return new PictureInPictureControlEvent(type, id);
}

PictureInPictureControlEvent* PictureInPictureControlEvent::Create(
    const AtomicString& type,
    const PictureInPictureControlEventInit& initializer) {
  return new PictureInPictureControlEvent(type, initializer);
}

String PictureInPictureControlEvent::id() const {
  return id_;
}
void PictureInPictureControlEvent::setId(String id) {
  id_ = id;
}

PictureInPictureControlEvent::PictureInPictureControlEvent(
    AtomicString const& type,
    String id)
    : Event(type, Bubbles::kYes, Cancelable::kNo), id_(id) {}

PictureInPictureControlEvent::PictureInPictureControlEvent(
    AtomicString const& type,
    const PictureInPictureControlEventInit& initializer)
    : Event(type, initializer), id_(initializer.id()) {}

}  // namespace blink
