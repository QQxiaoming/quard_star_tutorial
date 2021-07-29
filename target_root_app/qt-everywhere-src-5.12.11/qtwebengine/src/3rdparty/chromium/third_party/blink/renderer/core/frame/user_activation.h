// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_USER_ACTIVATION_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_USER_ACTIVATION_H_

#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"

namespace blink {

class LocalDOMWindow;

class UserActivation final : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit UserActivation(LocalDOMWindow* window);

  ~UserActivation() override;

  void Trace(blink::Visitor*) override;

  bool hasBeenActive() const;
  bool isActive() const;

 private:
  Member<LocalDOMWindow> window_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_USER_ACTIVATION_H_
