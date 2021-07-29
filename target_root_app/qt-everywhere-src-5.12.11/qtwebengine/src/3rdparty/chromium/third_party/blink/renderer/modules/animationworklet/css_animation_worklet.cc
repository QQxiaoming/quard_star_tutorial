// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/animationworklet/css_animation_worklet.h"

#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"

namespace blink {

// static
AnimationWorklet* CSSAnimationWorklet::animationWorklet(
    ScriptState* script_state) {
  LocalDOMWindow* window = ToLocalDOMWindow(script_state->GetContext());

  if (!window->GetFrame())
    return nullptr;
  return From(*window).animation_worklet_.Get();
}

// Break the following cycle when the context gets detached.
// Otherwise, the worklet object will leak.
//
// window => CSS.animationWorklet
// => CSSAnimationWorklet
// => AnimationWorklet  <--- break this reference
// => ThreadedWorkletMessagingProxy
// => Document
// => ... => window
void CSSAnimationWorklet::ContextDestroyed(ExecutionContext*) {
  animation_worklet_ = nullptr;
}

void CSSAnimationWorklet::Trace(blink::Visitor* visitor) {
  visitor->Trace(animation_worklet_);
  Supplement<LocalDOMWindow>::Trace(visitor);
  ContextLifecycleObserver::Trace(visitor);
}

// static
CSSAnimationWorklet& CSSAnimationWorklet::From(LocalDOMWindow& window) {
  CSSAnimationWorklet* supplement =
      Supplement<LocalDOMWindow>::From<CSSAnimationWorklet>(window);
  if (!supplement) {
    supplement = new CSSAnimationWorklet(window.GetFrame()->GetDocument());
    ProvideTo(window, supplement);
  }
  return *supplement;
}

CSSAnimationWorklet::CSSAnimationWorklet(Document* document)
    : ContextLifecycleObserver(document),
      animation_worklet_(new AnimationWorklet(document)) {
  DCHECK(GetExecutionContext());
}

const char CSSAnimationWorklet::kSupplementName[] = "CSSAnimationWorklet";

}  // namespace blink
