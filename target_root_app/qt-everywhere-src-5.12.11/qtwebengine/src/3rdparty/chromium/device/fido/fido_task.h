// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_FIDO_FIDO_TASK_H_
#define DEVICE_FIDO_FIDO_TASK_H_

#include <stdint.h>

#include <vector>

#include "base/callback.h"
#include "base/component_export.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "device/fido/fido_device.h"

namespace device {

// Encapsulates per-device request logic shared between MakeCredential and
// GetAssertion. Handles issuing the AuthenticatorGetInfo command to tokens,
// caching device info, and distinguishing U2F tokens from CTAP tokens.
//
// FidoTask is owned by FidoRequestHandler and manages all interaction with
// |device_|. It is created when a new device is discovered by FidoDiscovery and
// destroyed when the device is removed or when a successful response has been
// issued to the relying party from another authenticator.
class COMPONENT_EXPORT(DEVICE_FIDO) FidoTask {
 public:
  // The |device| must outlive the FidoTask instance.
  explicit FidoTask(FidoDevice* device);
  virtual ~FidoTask();

  // Invokes the AuthenticatorCancel method on |device_| if it supports the
  // CTAP protocol. Upon receiving AuthenticatorCancel request, authenticators
  // cancel ongoing requests (if any) immediately. Calling this method itself
  // neither destructs |this| instance nor destroys |device_|.
  void CancelTask();

 protected:
  // Asynchronously initiates CTAP request operation for a single device.
  virtual void StartTask() = 0;

  FidoDevice* device() const {
    DCHECK(device_);
    return device_;
  }

 private:
  FidoDevice* const device_;
  base::WeakPtrFactory<FidoTask> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(FidoTask);
};

}  // namespace device

#endif  // DEVICE_FIDO_FIDO_TASK_H_
