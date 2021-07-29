// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/input/mouse_wheel_event_queue.h"

#include "base/metrics/histogram_macros.h"
#include "base/trace_event/trace_event.h"
#include "content/common/input/input_event_dispatch_type.h"
#include "content/public/common/content_features.h"
#include "ui/events/base_event_utils.h"
#include "ui/events/blink/web_input_event_traits.h"

using blink::WebGestureEvent;
using blink::WebInputEvent;
using blink::WebMouseWheelEvent;
using ui::LatencyInfo;

namespace content {

// This class represents a single queued mouse wheel event. Its main use
// is that it is reported via trace events.
class QueuedWebMouseWheelEvent : public MouseWheelEventWithLatencyInfo {
 public:
  QueuedWebMouseWheelEvent(const MouseWheelEventWithLatencyInfo& original_event)
      : MouseWheelEventWithLatencyInfo(original_event) {
    TRACE_EVENT_ASYNC_BEGIN0("input", "MouseWheelEventQueue::QueueEvent", this);
  }

  ~QueuedWebMouseWheelEvent() {
    TRACE_EVENT_ASYNC_END0("input", "MouseWheelEventQueue::QueueEvent", this);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(QueuedWebMouseWheelEvent);
};

MouseWheelEventQueue::MouseWheelEventQueue(MouseWheelEventQueueClient* client)
    : client_(client),
      send_wheel_events_async_(false),
      scrolling_device_(blink::kWebGestureDeviceUninitialized) {
  DCHECK(client);
}

MouseWheelEventQueue::~MouseWheelEventQueue() {
}

void MouseWheelEventQueue::QueueEvent(
    const MouseWheelEventWithLatencyInfo& event) {
  TRACE_EVENT0("input", "MouseWheelEventQueue::QueueEvent");

  if (event_sent_for_gesture_ack_ && !wheel_queue_.empty()) {
    QueuedWebMouseWheelEvent* last_event = wheel_queue_.back().get();
    if (last_event->CanCoalesceWith(event)) {
      // Terminate the LatencyInfo of the event before it gets coalesced away.
      event.latency.Terminate();

      last_event->CoalesceWith(event);
      TRACE_EVENT_INSTANT2("input", "MouseWheelEventQueue::CoalescedWheelEvent",
                           TRACE_EVENT_SCOPE_THREAD, "total_dx",
                           last_event->event.delta_x, "total_dy",
                           last_event->event.delta_y);
      return;
    }
  }

  wheel_queue_.push_back(std::make_unique<QueuedWebMouseWheelEvent>(event));
  TryForwardNextEventToRenderer();
  LOCAL_HISTOGRAM_COUNTS_100("Renderer.WheelQueueSize", wheel_queue_.size());
}

bool MouseWheelEventQueue::CanGenerateGestureScroll(
    InputEventAckState ack_result) const {
  if (ack_result == INPUT_EVENT_ACK_STATE_CONSUMED) {
    TRACE_EVENT_INSTANT0("input", "Wheel Event Consumed",
                         TRACE_EVENT_SCOPE_THREAD);
    return false;
  }

  if (!ui::WebInputEventTraits::CanCauseScroll(
          event_sent_for_gesture_ack_->event)) {
    TRACE_EVENT_INSTANT0("input", "Wheel Event Cannot Cause Scroll",
                         TRACE_EVENT_SCOPE_THREAD);
    return false;
  }

  if (event_sent_for_gesture_ack_->event.resending_plugin_id != -1) {
    TRACE_EVENT_INSTANT0("input", "Wheel Event Resending Plugin Id Is Not -1",
                         TRACE_EVENT_SCOPE_THREAD);
    return false;
  }

  if (scrolling_device_ != blink::kWebGestureDeviceUninitialized &&
      scrolling_device_ != blink::kWebGestureDeviceTouchpad) {
    TRACE_EVENT_INSTANT0("input",
                         "Autoscroll or Touchscreen Scroll In Progress",
                         TRACE_EVENT_SCOPE_THREAD);
    return false;
  }

  return true;
}

void MouseWheelEventQueue::ProcessMouseWheelAck(
    InputEventAckSource ack_source,
    InputEventAckState ack_result,
    const LatencyInfo& latency_info) {
  TRACE_EVENT0("input", "MouseWheelEventQueue::ProcessMouseWheelAck");
  if (!event_sent_for_gesture_ack_)
    return;

  event_sent_for_gesture_ack_->latency.AddNewLatencyFrom(latency_info);
  client_->OnMouseWheelEventAck(*event_sent_for_gesture_ack_, ack_source,
                                ack_result);

  // If event wasn't consumed then generate a gesture scroll for it.
  if (CanGenerateGestureScroll(ack_result)) {
    WebGestureEvent scroll_update(
        WebInputEvent::kGestureScrollUpdate, WebInputEvent::kNoModifiers,
        event_sent_for_gesture_ack_->event.TimeStamp(),
        blink::kWebGestureDeviceTouchpad);

    scroll_update.SetPositionInWidget(
        event_sent_for_gesture_ack_->event.PositionInWidget());
    scroll_update.SetPositionInScreen(
        event_sent_for_gesture_ack_->event.PositionInScreen());
    scroll_update.resending_plugin_id = -1;

    // Swap X & Y if Shift is down and when there is no horizontal movement.
    if ((event_sent_for_gesture_ack_->event.GetModifiers() &
         WebInputEvent::kShiftKey) != 0 &&
        event_sent_for_gesture_ack_->event.delta_x == 0) {
      scroll_update.data.scroll_update.delta_x =
          event_sent_for_gesture_ack_->event.delta_y;
      scroll_update.data.scroll_update.delta_y =
          event_sent_for_gesture_ack_->event.delta_x;
    } else {
      scroll_update.data.scroll_update.delta_x =
          event_sent_for_gesture_ack_->event.delta_x;
      scroll_update.data.scroll_update.delta_y =
          event_sent_for_gesture_ack_->event.delta_y;
    }

    if (event_sent_for_gesture_ack_->event.momentum_phase !=
        blink::WebMouseWheelEvent::kPhaseNone) {
      scroll_update.data.scroll_update.inertial_phase =
          WebGestureEvent::kMomentumPhase;
    } else if (event_sent_for_gesture_ack_->event.phase !=
               blink::WebMouseWheelEvent::kPhaseNone) {
      scroll_update.data.scroll_update.inertial_phase =
          WebGestureEvent::kNonMomentumPhase;
    }
    if (event_sent_for_gesture_ack_->event.scroll_by_page) {
      scroll_update.data.scroll_update.delta_units = WebGestureEvent::kPage;

      // Turn page scrolls into a *single* page scroll because
      // the magnitude the number of ticks is lost when coalescing.
      if (scroll_update.data.scroll_update.delta_x)
        scroll_update.data.scroll_update.delta_x =
            scroll_update.data.scroll_update.delta_x > 0 ? 1 : -1;
      if (scroll_update.data.scroll_update.delta_y)
        scroll_update.data.scroll_update.delta_y =
            scroll_update.data.scroll_update.delta_y > 0 ? 1 : -1;
    } else {
      scroll_update.data.scroll_update.delta_units =
          event_sent_for_gesture_ack_->event.has_precise_scrolling_deltas
              ? WebGestureEvent::kPrecisePixels
              : WebGestureEvent::kPixels;

      if (event_sent_for_gesture_ack_->event.rails_mode ==
          WebInputEvent::kRailsModeVertical)
        scroll_update.data.scroll_update.delta_x = 0;
      if (event_sent_for_gesture_ack_->event.rails_mode ==
          WebInputEvent::kRailsModeHorizontal)
        scroll_update.data.scroll_update.delta_y = 0;
    }

    bool current_phase_ended = false;
    bool scroll_phase_ended = false;
    bool momentum_phase_ended = false;
    bool has_phase_info = false;

    if (event_sent_for_gesture_ack_->event.phase !=
            blink::WebMouseWheelEvent::kPhaseNone ||
        event_sent_for_gesture_ack_->event.momentum_phase !=
            blink::WebMouseWheelEvent::kPhaseNone) {
      has_phase_info = true;
      scroll_phase_ended = event_sent_for_gesture_ack_->event.phase ==
                               blink::WebMouseWheelEvent::kPhaseEnded ||
                           event_sent_for_gesture_ack_->event.phase ==
                               blink::WebMouseWheelEvent::kPhaseCancelled;
      momentum_phase_ended =
          event_sent_for_gesture_ack_->event.momentum_phase ==
              blink::WebMouseWheelEvent::kPhaseEnded ||
          event_sent_for_gesture_ack_->event.momentum_phase ==
              blink::WebMouseWheelEvent::kPhaseCancelled;
      current_phase_ended = scroll_phase_ended || momentum_phase_ended;
    }

    bool needs_update = scroll_update.data.scroll_update.delta_x != 0 ||
                        scroll_update.data.scroll_update.delta_y != 0;

    // For every GSU event record whether it is latched or not.
    if (needs_update)
      RecordLatchingUmaMetric(client_->IsWheelScrollInProgress());

    bool synthetic = event_sent_for_gesture_ack_->event.has_synthetic_phase;
    if (event_sent_for_gesture_ack_->event.phase ==
        blink::WebMouseWheelEvent::kPhaseBegan) {
      // Wheel event with phaseBegan must have non-zero deltas.
      DCHECK(needs_update);
      send_wheel_events_async_ = true;

      if (!client_->IsWheelScrollInProgress())
        SendScrollBegin(scroll_update, synthetic);
    }

    if (needs_update) {
      // It is possible that the wheel event with phaseBegan is consumed and
      // no GSB is sent.
      if (!client_->IsWheelScrollInProgress())
        SendScrollBegin(scroll_update, synthetic);
      client_->ForwardGestureEventWithLatencyInfo(
          scroll_update, ui::LatencyInfo(ui::SourceEventType::WHEEL));
    }

    if (current_phase_ended && client_->IsWheelScrollInProgress()) {
      // Send GSE when GSB is sent and no fling is going to happen next.
      SendScrollEnd(scroll_update, synthetic);
    }
  }

  event_sent_for_gesture_ack_.reset();
  TryForwardNextEventToRenderer();
}

void MouseWheelEventQueue::OnGestureScrollEvent(
    const GestureEventWithLatencyInfo& gesture_event) {
  if (gesture_event.event.GetType() ==
      blink::WebInputEvent::kGestureScrollBegin) {
    scrolling_device_ = gesture_event.event.SourceDevice();
  } else if (scrolling_device_ == gesture_event.event.SourceDevice() &&
             gesture_event.event.GetType() ==
                 blink::WebInputEvent::kGestureScrollEnd) {
    scrolling_device_ = blink::kWebGestureDeviceUninitialized;
  } else if (gesture_event.event.GetType() ==
             blink::WebInputEvent::kGestureFlingStart) {
    // With browser side fling we shouldn't reset scrolling_device_ on GFS since
    // the fling_controller processes the GFS to generate and send GSU events.
  }
}

void MouseWheelEventQueue::TryForwardNextEventToRenderer() {
  TRACE_EVENT0("input", "MouseWheelEventQueue::TryForwardNextEventToRenderer");

  if (wheel_queue_.empty() || event_sent_for_gesture_ack_)
    return;

  event_sent_for_gesture_ack_ = std::move(wheel_queue_.front());
  wheel_queue_.pop_front();

  DCHECK(event_sent_for_gesture_ack_->event.phase !=
             blink::WebMouseWheelEvent::kPhaseNone ||
         event_sent_for_gesture_ack_->event.momentum_phase !=
             blink::WebMouseWheelEvent::kPhaseNone);
  if (event_sent_for_gesture_ack_->event.phase ==
      blink::WebMouseWheelEvent::kPhaseBegan) {
    send_wheel_events_async_ = false;
  } else if (send_wheel_events_async_) {
    event_sent_for_gesture_ack_->event.dispatch_type =
        WebInputEvent::kEventNonBlocking;
  }

  client_->SendMouseWheelEventImmediately(*event_sent_for_gesture_ack_);
}

void MouseWheelEventQueue::SendScrollEnd(WebGestureEvent update_event,
                                         bool synthetic) {
  DCHECK(client_->IsWheelScrollInProgress());

  WebGestureEvent scroll_end(update_event);
  scroll_end.SetTimeStamp(ui::EventTimeForNow());
  scroll_end.SetType(WebInputEvent::kGestureScrollEnd);
  scroll_end.resending_plugin_id = -1;
  scroll_end.data.scroll_end.synthetic = synthetic;
  scroll_end.data.scroll_end.inertial_phase =
      update_event.data.scroll_update.inertial_phase;
  scroll_end.data.scroll_end.delta_units =
      update_event.data.scroll_update.delta_units;

  client_->ForwardGestureEventWithLatencyInfo(
      scroll_end, ui::LatencyInfo(ui::SourceEventType::WHEEL));
}

void MouseWheelEventQueue::SendScrollBegin(
    const WebGestureEvent& gesture_update,
    bool synthetic) {
  DCHECK(!client_->IsWheelScrollInProgress());

  WebGestureEvent scroll_begin(gesture_update);
  scroll_begin.SetType(WebInputEvent::kGestureScrollBegin);
  scroll_begin.data.scroll_begin.synthetic = synthetic;
  scroll_begin.data.scroll_begin.inertial_phase =
      gesture_update.data.scroll_update.inertial_phase;
  scroll_begin.data.scroll_begin.delta_x_hint =
      gesture_update.data.scroll_update.delta_x;
  scroll_begin.data.scroll_begin.delta_y_hint =
      gesture_update.data.scroll_update.delta_y;
  scroll_begin.data.scroll_begin.target_viewport = false;
  scroll_begin.data.scroll_begin.delta_hint_units =
      gesture_update.data.scroll_update.delta_units;

  client_->ForwardGestureEventWithLatencyInfo(
      scroll_begin, ui::LatencyInfo(ui::SourceEventType::WHEEL));
}

void MouseWheelEventQueue::RecordLatchingUmaMetric(bool latched) {
  UMA_HISTOGRAM_BOOLEAN("WheelScrolling.WasLatched", latched);
}

}  // namespace content
