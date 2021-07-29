// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/input/mouse_wheel_event_manager.h"

#include "build/build_config.h"
#include "third_party/blink/public/platform/web_mouse_wheel_event.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/events/wheel_event.h"
#include "third_party/blink/renderer/core/frame/local_frame_view.h"
#include "third_party/blink/renderer/core/input/event_handler.h"
#include "third_party/blink/renderer/core/input/event_handling_util.h"
#include "third_party/blink/renderer/core/layout/hit_test_request.h"
#include "third_party/blink/renderer/core/layout/hit_test_result.h"
#include "third_party/blink/renderer/core/layout/layout_view.h"

namespace blink {
MouseWheelEventManager::MouseWheelEventManager(LocalFrame& frame)
    : frame_(frame), wheel_target_(nullptr) {}

void MouseWheelEventManager::Trace(blink::Visitor* visitor) {
  visitor->Trace(frame_);
  visitor->Trace(wheel_target_);
}

void MouseWheelEventManager::Clear() {
  wheel_target_ = nullptr;
}

WebInputEventResult MouseWheelEventManager::HandleWheelEvent(
    const WebMouseWheelEvent& event) {
  Document* doc = frame_->GetDocument();
  if (!doc || !doc->GetLayoutView())
    return WebInputEventResult::kNotHandled;

  LocalFrameView* view = frame_->View();
  if (!view)
    return WebInputEventResult::kNotHandled;

  const int kWheelEventPhaseEndedEventMask =
      WebMouseWheelEvent::kPhaseEnded | WebMouseWheelEvent::kPhaseCancelled;
  const int kWheelEventPhaseNoEventMask =
      kWheelEventPhaseEndedEventMask | WebMouseWheelEvent::kPhaseMayBegin;

  if ((event.phase & kWheelEventPhaseEndedEventMask) ||
      (event.momentum_phase & kWheelEventPhaseEndedEventMask)) {
    wheel_target_ = nullptr;
  }

  if ((event.phase & kWheelEventPhaseNoEventMask) ||
      (event.momentum_phase & kWheelEventPhaseNoEventMask)) {
    // Filter wheel events with zero deltas and reset the wheel_target_ node.
    DCHECK(!event.delta_x && !event.delta_y);
    return WebInputEventResult::kNotHandled;
  }

  bool has_phase_info = event.phase != WebMouseWheelEvent::kPhaseNone ||
                        event.momentum_phase != WebMouseWheelEvent::kPhaseNone;
  if (!has_phase_info) {
    // Synthetic wheel events generated from GesturePinchUpdate don't have
    // phase info. Send these events to the target under the cursor.
    wheel_target_ = FindTargetNode(event, doc, view);
  } else if (event.phase == WebMouseWheelEvent::kPhaseBegan || !wheel_target_) {
    // Find and save the wheel_target_, this target will be used for the rest
    // of the current scrolling sequence.
    wheel_target_ = FindTargetNode(event, doc, view);
  }

  LocalFrame* subframe =
      EventHandlingUtil::SubframeForTargetNode(wheel_target_.Get());
  if (subframe) {
    WebInputEventResult result =
        subframe->GetEventHandler().HandleWheelEvent(event);
    return result;
  }

  if (wheel_target_) {
    WheelEvent* dom_event =
        WheelEvent::Create(event, wheel_target_->GetDocument().domWindow());
    // The event handler might remove |wheel_target_| from DOM so we should get
    // this value now (see https://crbug.com/857013).
    bool should_enforce_vertical_scroll =
        wheel_target_->GetDocument().IsVerticalScrollEnforced();
    DispatchEventResult dom_event_result =
        wheel_target_->DispatchEvent(dom_event);
    if (dom_event_result != DispatchEventResult::kNotCanceled) {
      // Reset the target if the dom event is cancelled to make sure that new
      // targeting happens for the next wheel event.
      wheel_target_ = nullptr;
      // TODO(ekaramad): This does not seem correct. The behavior of shift +
      // scrolling seems different on Mac vs Linux/Windows. We need this done
      // properly and perhaps even tag WebMouseWheelEvent with a scrolling
      // direction (https://crbug.com/853292).
      // When using shift + mouse scroll (to horizontally scroll), the expected
      // value of |delta_x| is exactly zero.
      bool is_vertical =
          (std::abs(dom_event->deltaX()) < std::abs(dom_event->deltaY())) &&
          (!dom_event->shiftKey() || dom_event->deltaX() != 0);
      // TODO(ekaramad): If the only wheel handlers on the page are from such
      // disabled frames we should simply start scrolling on CC and the events
      // must get here as passive (https://crbug.com/853059).
      // Overwriting the dispatch results ensures that vertical scroll cannot be
      // blocked by disabled frames.
      return (should_enforce_vertical_scroll && is_vertical)
                 ? WebInputEventResult::kNotHandled
                 : EventHandlingUtil::ToWebInputEventResult(dom_event_result);
    }
  }

  return WebInputEventResult::kNotHandled;
}

void MouseWheelEventManager::ElementRemoved(Node* target) {
  if (wheel_target_ == target)
    wheel_target_ = nullptr;
}

Node* MouseWheelEventManager::FindTargetNode(const WebMouseWheelEvent& event,
                                             const Document* doc,
                                             const LocalFrameView* view) {
  DCHECK(doc && doc->GetLayoutView() && view);
  LayoutPoint v_point =
      view->ConvertFromRootFrame(FlooredIntPoint(event.PositionInRootFrame()));

  HitTestRequest request(HitTestRequest::kReadOnly);
  HitTestLocation location(v_point);
  HitTestResult result(request, location);
  doc->GetLayoutView()->HitTest(location, result);

  Node* node = result.InnerNode();
  // Wheel events should not dispatch to text nodes.
  if (node && node->IsTextNode())
    node = FlatTreeTraversal::Parent(*node);

  // If we're over the frame scrollbar, scroll the document.
  if (!node && result.GetScrollbar())
    node = doc->documentElement();

  return node;
}

}  // namespace blink
