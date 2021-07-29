// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webusb/worker_navigator_usb.h"

#include "third_party/blink/renderer/core/workers/worker_navigator.h"
#include "third_party/blink/renderer/modules/webusb/usb.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

namespace blink {

WorkerNavigatorUSB& WorkerNavigatorUSB::From(
    WorkerNavigator& worker_navigator) {
  WorkerNavigatorUSB* supplement =
      Supplement<WorkerNavigator>::From<WorkerNavigatorUSB>(worker_navigator);
  if (!supplement) {
    supplement = new WorkerNavigatorUSB(worker_navigator);
    ProvideTo(worker_navigator, supplement);
  }
  return *supplement;
}

// static
USB* WorkerNavigatorUSB::usb(ScriptState* script_state,
                             WorkerNavigator& worker_navigator) {
  return WorkerNavigatorUSB::From(worker_navigator).usb(script_state);
}

USB* WorkerNavigatorUSB::usb(ScriptState* script_state) {
  // A bug in the WebIDL compiler causes this attribute to be incorrectly
  // exposed in the other worker contexts if one of the RuntimeEnabled flags is
  // enabled. Therefore, we will just return the empty usb_ member if the
  // appropriate flag is not enabled for the current context, or if the
  // current context is a ServiceWorkerGlobalScope.
  // TODO(https://crbug.com/839117): Once this attribute stops being incorrectly
  // exposed to the worker contexts, remove these checks.
  if (!usb_) {
    ExecutionContext* context = ExecutionContext::From(script_state);
    DCHECK(context);

    bool isDedicatedWorkerAndEnabled =
        context->IsDedicatedWorkerGlobalScope() &&
        RuntimeEnabledFeatures::WebUSBOnDedicatedWorkersEnabled();
    bool isSharedWorkerAndEnabled =
        context->IsSharedWorkerGlobalScope() &&
        RuntimeEnabledFeatures::WebUSBOnSharedWorkersEnabled();

    if (isDedicatedWorkerAndEnabled || isSharedWorkerAndEnabled) {
      usb_ = USB::Create(*context);
    }
  }
  return usb_;
}

void WorkerNavigatorUSB::Trace(blink::Visitor* visitor) {
  visitor->Trace(usb_);
  Supplement<WorkerNavigator>::Trace(visitor);
}

WorkerNavigatorUSB::WorkerNavigatorUSB(WorkerNavigator& worker_navigator)
    : Supplement<WorkerNavigator>(worker_navigator) {}

const char WorkerNavigatorUSB::kSupplementName[] = "WorkerNavigatorUSB";

}  // namespace blink
