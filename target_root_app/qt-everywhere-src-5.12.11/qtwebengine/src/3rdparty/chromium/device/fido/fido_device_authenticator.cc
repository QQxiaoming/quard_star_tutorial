// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/fido_device_authenticator.h"

#include <utility>

#include "base/logging.h"
#include "device/fido/authenticator_supported_options.h"
#include "device/fido/ctap_get_assertion_request.h"
#include "device/fido/ctap_make_credential_request.h"
#include "device/fido/fido_device.h"
#include "device/fido/get_assertion_task.h"
#include "device/fido/make_credential_task.h"

namespace device {

FidoDeviceAuthenticator::FidoDeviceAuthenticator(FidoDevice* device)
    : device_(device) {
  DCHECK(device_->SupportedProtocolIsInitialized());
}
FidoDeviceAuthenticator::~FidoDeviceAuthenticator() = default;

void FidoDeviceAuthenticator::MakeCredential(CtapMakeCredentialRequest request,
                                             MakeCredentialCallback callback) {
  DCHECK(!task_);
  // TODO(martinkr): Change FidoTasks to take all request parameters by const
  // reference, so we can avoid copying these from the RequestHandler.
  task_ = std::make_unique<MakeCredentialTask>(device_, std::move(request),
                                               std::move(callback));
}

void FidoDeviceAuthenticator::GetAssertion(CtapGetAssertionRequest request,
                                           GetAssertionCallback callback) {
  task_ = std::make_unique<GetAssertionTask>(device_, std::move(request),
                                             std::move(callback));
}

void FidoDeviceAuthenticator::Cancel() {
  if (!task_)
    return;

  task_->CancelTask();
}

std::string FidoDeviceAuthenticator::GetId() const {
  return device_->GetId();
}

const AuthenticatorSupportedOptions& FidoDeviceAuthenticator::Options() const {
  static const AuthenticatorSupportedOptions default_options;
  switch (device_->supported_protocol()) {
    case ProtocolVersion::kU2f:
      return default_options;
    case ProtocolVersion::kCtap:
      DCHECK(device_->device_info()) << "uninitialized device";
      return device_->device_info()->options();
    case ProtocolVersion::kUnknown:
      NOTREACHED() << "uninitialized device";
  }
  NOTREACHED();
  return default_options;
}

void FidoDeviceAuthenticator::SetTaskForTesting(
    std::unique_ptr<FidoTask> task) {
  task_ = std::move(task);
}

}  // namespace device
