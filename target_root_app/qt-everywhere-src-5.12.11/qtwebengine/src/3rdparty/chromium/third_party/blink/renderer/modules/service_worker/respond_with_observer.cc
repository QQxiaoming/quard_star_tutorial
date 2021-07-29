// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/service_worker/respond_with_observer.h"

#include "third_party/blink/public/platform/modules/service_worker/web_service_worker_response.h"
#include "third_party/blink/renderer/bindings/core/v8/script_function.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_value.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_core.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/modules/service_worker/wait_until_observer.h"
#include "third_party/blink/renderer/platform/wtf/functional.h"
#include "v8/include/v8.h"

using blink::mojom::ServiceWorkerResponseError;

namespace blink {

void RespondWithObserver::ContextDestroyed(ExecutionContext*) {
  if (observer_) {
    DCHECK_EQ(kPending, state_);
    observer_.Clear();
  }
  state_ = kDone;
}

void RespondWithObserver::WillDispatchEvent() {
  event_dispatch_time_ = WTF::CurrentTime();
}

void RespondWithObserver::DidDispatchEvent(
    DispatchEventResult dispatch_result) {
  DCHECK(GetExecutionContext());
  if (state_ != kInitial)
    return;

  if (dispatch_result == DispatchEventResult::kNotCanceled) {
    OnNoResponse();
  } else {
    OnResponseRejected(ServiceWorkerResponseError::kDefaultPrevented);
  }

  state_ = kDone;
  observer_.Clear();
}

void RespondWithObserver::RespondWith(ScriptState* script_state,
                                      ScriptPromise script_promise,
                                      ExceptionState& exception_state) {
  if (state_ != kInitial) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kInvalidStateError,
        "The event has already been responded to.");
    return;
  }

  state_ = kPending;
  observer_->WaitUntil(
      script_state, script_promise, exception_state,
      WTF::BindRepeating(&RespondWithObserver::ResponseWasFulfilled,
                         WrapPersistent(this), exception_state.Context(),
                         WTF::Unretained(exception_state.InterfaceName()),
                         WTF::Unretained(exception_state.PropertyName())),
      WTF::BindRepeating(&RespondWithObserver::ResponseWasRejected,
                         WrapPersistent(this),
                         ServiceWorkerResponseError::kPromiseRejected));
}

void RespondWithObserver::ResponseWasRejected(ServiceWorkerResponseError error,
                                              const ScriptValue& value) {
  OnResponseRejected(error);
  state_ = kDone;
  observer_.Clear();
}

void RespondWithObserver::ResponseWasFulfilled(
    ExceptionState::ContextType context_type,
    const char* interface_name,
    const char* property_name,
    const ScriptValue& value) {
  OnResponseFulfilled(value, context_type, interface_name, property_name);
  state_ = kDone;
  observer_.Clear();
}

RespondWithObserver::RespondWithObserver(ExecutionContext* context,
                                         int event_id,
                                         WaitUntilObserver* observer)
    : ContextLifecycleObserver(context),
      event_id_(event_id),
      state_(kInitial),
      observer_(observer) {}

void RespondWithObserver::Trace(blink::Visitor* visitor) {
  visitor->Trace(observer_);
  ContextLifecycleObserver::Trace(visitor);
}

}  // namespace blink
