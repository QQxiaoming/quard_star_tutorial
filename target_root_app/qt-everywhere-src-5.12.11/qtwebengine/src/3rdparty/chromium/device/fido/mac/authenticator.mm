// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/mac/authenticator.h"

#import <LocalAuthentication/LocalAuthentication.h>

#include "base/feature_list.h"
#include "base/logging.h"
#include "base/mac/mac_util.h"
#include "base/memory/ptr_util.h"
#include "base/optional.h"
#include "base/strings/string_piece.h"
#include "device/base/features.h"
#include "device/fido/authenticator_supported_options.h"
#include "device/fido/ctap_get_assertion_request.h"
#include "device/fido/ctap_make_credential_request.h"
#include "device/fido/fido_constants.h"
#include "device/fido/mac/get_assertion_operation.h"
#include "device/fido/mac/make_credential_operation.h"
#include "device/fido/mac/util.h"

namespace device {
namespace fido {
namespace mac {

// static
bool TouchIdAuthenticator::IsAvailable() {
  if (base::FeatureList::IsEnabled(device::kWebAuthTouchId)) {
    if (base::mac::IsAtLeastOS10_13()) {
      base::scoped_nsobject<LAContext> context([[LAContext alloc] init]);
      return [context
          canEvaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics
                      error:nil];
    }
  }
  return false;
}

// static
std::unique_ptr<TouchIdAuthenticator> TouchIdAuthenticator::CreateIfAvailable(
    std::string keychain_access_group,
    std::string metadata_secret) {
  // N.B. IsAvailable also checks for the feature flag being set.
  return IsAvailable() ? base::WrapUnique(new TouchIdAuthenticator(
                             std::move(keychain_access_group),
                             std::move(metadata_secret)))
                       : nullptr;
}

// static
std::unique_ptr<TouchIdAuthenticator> TouchIdAuthenticator::CreateForTesting(
    std::string keychain_access_group,
    std::string metadata_secret) {
  return base::WrapUnique(new TouchIdAuthenticator(
      std::move(keychain_access_group), std::move(metadata_secret)));
}

TouchIdAuthenticator::~TouchIdAuthenticator() = default;

void TouchIdAuthenticator::MakeCredential(CtapMakeCredentialRequest request,
                                          MakeCredentialCallback callback) {
  if (base::mac::IsAtLeastOS10_13()) {
    DCHECK(!operation_);
    operation_ = std::make_unique<MakeCredentialOperation>(
        std::move(request), metadata_secret_, keychain_access_group_,
        std::move(callback));
    operation_->Run();
    return;
  }
  NOTREACHED();
}

void TouchIdAuthenticator::GetAssertion(CtapGetAssertionRequest request,
                                        GetAssertionCallback callback) {
  if (base::mac::IsAtLeastOS10_13()) {
    DCHECK(!operation_);
    operation_ = std::make_unique<GetAssertionOperation>(
        std::move(request), metadata_secret_, keychain_access_group_,
        std::move(callback));
    operation_->Run();
    return;
  }
  NOTREACHED();
}

void TouchIdAuthenticator::Cancel() {
  // If there is an operation pending, delete it, which will clean up any
  // pending callbacks, e.g. if the operation is waiting for a response from
  // the Touch ID prompt. Note that we cannot cancel the actual prompt once it
  // has been shown.
  operation_.reset();
}

std::string TouchIdAuthenticator::GetId() const {
  return "TouchIdAuthenticator";
}

namespace {

AuthenticatorSupportedOptions TouchIdAuthenticatorOptions() {
  AuthenticatorSupportedOptions options;
  options.SetIsPlatformDevice(true);
  options.SetSupportsResidentKey(true);
  options.SetUserVerificationAvailability(
      AuthenticatorSupportedOptions::UserVerificationAvailability::
          kSupportedAndConfigured);
  options.SetUserPresenceRequired(true);
  return options;
}

}  // namespace

const AuthenticatorSupportedOptions& TouchIdAuthenticator::Options() const {
  static const AuthenticatorSupportedOptions options =
      TouchIdAuthenticatorOptions();
  return options;
}

TouchIdAuthenticator::TouchIdAuthenticator(std::string keychain_access_group,
                                           std::string metadata_secret)
    : keychain_access_group_(std::move(keychain_access_group)),
      metadata_secret_(std::move(metadata_secret)) {}

}  // namespace mac
}  // namespace fido
}  // namespace device
