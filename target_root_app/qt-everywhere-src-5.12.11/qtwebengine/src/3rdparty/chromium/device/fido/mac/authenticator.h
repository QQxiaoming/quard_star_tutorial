// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_FIDO_MAC_AUTHENTICATOR_H_
#define DEVICE_FIDO_MAC_AUTHENTICATOR_H_

#include "base/component_export.h"
#include "base/mac/availability.h"
#include "base/macros.h"
#include "base/strings/string_piece_forward.h"
#include "device/fido/fido_authenticator.h"
#include "device/fido/mac/operation.h"

namespace device {
namespace fido {
namespace mac {

class COMPONENT_EXPORT(DEVICE_FIDO) TouchIdAuthenticator
    : public FidoAuthenticator {
 public:
  // IsAvailable returns whether Touch ID is available and enrolled on the
  // current device.
  static bool IsAvailable();

  // CreateIfAvailable returns a TouchIdAuthenticator if IsAvailable() returns
  // true and nullptr otherwise.
  static std::unique_ptr<TouchIdAuthenticator> CreateIfAvailable(
      std::string keychain_access_group,
      std::string metadata_secret);

  static std::unique_ptr<TouchIdAuthenticator> CreateForTesting(
      std::string keychain_access_group,
      std::string metadata_secret);

  ~TouchIdAuthenticator() override;

  // FidoAuthenticator
  void MakeCredential(
      CtapMakeCredentialRequest request,
      MakeCredentialCallback callback) override;
  void GetAssertion(CtapGetAssertionRequest request,
                    GetAssertionCallback callback) override;
  void Cancel() override;
  std::string GetId() const override;
  const AuthenticatorSupportedOptions& Options() const override;

 private:
  TouchIdAuthenticator(std::string keychain_access_group,
                       std::string metadata_secret);

  // The keychain access group under which credentials are stored in the macOS
  // keychain for access control. The set of all access groups that the
  // application belongs to is stored in the entitlements file that gets
  // embedded into the application during code signing. For more information
  // see
  // https://developer.apple.com/documentation/security/ksecattraccessgroup?language=objc.
  std::string keychain_access_group_;

  std::string metadata_secret_;

  std::unique_ptr<Operation> operation_;

 private:
  DISALLOW_COPY_AND_ASSIGN(TouchIdAuthenticator);
};

}  // namespace mac
}  // namespace fido
}  // namespace device

#endif  // DEVICE_FIDO_MAC_AUTHENTICATOR_H_
