// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/get_assertion_request_handler.h"

#include <utility>

#include "base/bind.h"
#include "device/fido/authenticator_get_assertion_response.h"
#include "device/fido/fido_authenticator.h"
#include "device/fido/fido_cable_discovery.h"
#include "device/fido/get_assertion_task.h"

namespace device {

GetAssertionRequestHandler::GetAssertionRequestHandler(
    service_manager::Connector* connector,
    const base::flat_set<FidoTransportProtocol>& protocols,
    CtapGetAssertionRequest request,
    SignResponseCallback completion_callback)
    : GetAssertionRequestHandler(connector,
                                 protocols,
                                 std::move(request),
                                 std::move(completion_callback),
                                 AddPlatformAuthenticatorCallback()) {}

GetAssertionRequestHandler::GetAssertionRequestHandler(
    service_manager::Connector* connector,
    const base::flat_set<FidoTransportProtocol>& protocols,
    CtapGetAssertionRequest request,
    SignResponseCallback completion_callback,
    AddPlatformAuthenticatorCallback add_platform_authenticator)
    : FidoRequestHandler(connector,
                         protocols,
                         std::move(completion_callback),
                         std::move(add_platform_authenticator)),
      request_(std::move(request)),
      weak_factory_(this) {
  if (base::ContainsKey(
          protocols, FidoTransportProtocol::kCloudAssistedBluetoothLowEnergy) &&
      request_.cable_extension()) {
    auto discovery =
        std::make_unique<FidoCableDiscovery>(*request_.cable_extension());
    discovery->set_observer(this);
    discoveries().push_back(std::move(discovery));
  }

  Start();
}

GetAssertionRequestHandler::~GetAssertionRequestHandler() = default;

namespace {

// Checks UserVerificationRequirement enum passed from the relying party is
// compatible with the authenticator, and updates the request to the
// "effective" user verification requirement.
// https://w3c.github.io/webauthn/#effective-user-verification-requirement-for-assertion
bool CheckUserVerificationCompatible(FidoAuthenticator* authenticator,
                                     CtapGetAssertionRequest* request) {
  const auto uv_availability =
      authenticator->Options().user_verification_availability();

  switch (request->user_verification()) {
    case UserVerificationRequirement::kRequired:
      return uv_availability ==
             AuthenticatorSupportedOptions::UserVerificationAvailability::
                 kSupportedAndConfigured;

    case UserVerificationRequirement::kDiscouraged:
      return true;

    case UserVerificationRequirement::kPreferred:
      if (uv_availability ==
          AuthenticatorSupportedOptions::UserVerificationAvailability::
              kSupportedAndConfigured) {
        request->SetUserVerification(UserVerificationRequirement::kRequired);
      } else {
        request->SetUserVerification(UserVerificationRequirement::kDiscouraged);
      }
      return true;
  }

  NOTREACHED();
  return false;
}

}  // namespace

void GetAssertionRequestHandler::DispatchRequest(
    FidoAuthenticator* authenticator) {
  // The user verification field of the request may be adjusted to the
  // authenticator, so we need to make a copy.
  CtapGetAssertionRequest request_copy = request_;
  if (!CheckUserVerificationCompatible(authenticator, &request_copy)) {
    return;
  }

  authenticator->GetAssertion(
      std::move(request_copy),
      base::BindOnce(&GetAssertionRequestHandler::OnAuthenticatorResponse,
                     weak_factory_.GetWeakPtr(), authenticator));
}

}  // namespace device
