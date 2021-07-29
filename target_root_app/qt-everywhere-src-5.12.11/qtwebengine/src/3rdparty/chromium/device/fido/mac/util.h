// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_FIDO_MAC_UTIL_H_
#define DEVICE_FIDO_MAC_UTIL_H_

#include <memory>
#include <string>
#include <vector>

#import <Security/Security.h>

#include "base/callback.h"
#include "base/component_export.h"
#include "base/containers/span.h"
#include "base/mac/availability.h"
#include "device/fido/authenticator_data.h"
#include "device/fido/ec_public_key.h"
#include "device/fido/fido_constants.h"

namespace device {
namespace fido {
namespace mac {

// MakeAuthenticatorData returns an AuthenticatorData instance for the Touch ID
// authenticator with the given Relying Party ID, credential ID and public key.
// It returns |base::nullopt| on failure.
COMPONENT_EXPORT(DEVICE_FIDO)
base::Optional<AuthenticatorData> MakeAuthenticatorData(
    const std::string& rp_id,
    std::vector<uint8_t> credential_id,
    std::unique_ptr<ECPublicKey> public_key);

// GenerateSignature signs the concatenation of the serialization of the given
// authenticator data and the given client data hash, as required for
// (self-)attestation and assertion. Returns |base::nullopt| if the operation
// fails.
base::Optional<std::vector<uint8_t>> GenerateSignature(
    const AuthenticatorData& authenticator_data,
    base::span<const uint8_t, kClientDataHashLength> client_data_hash,
    SecKeyRef private_key) API_AVAILABLE(macosx(10.12.2));

// SecKeyRefToECPublicKey converts a SecKeyRef for a public key into an
// equivalent |ECPublicKey| instance. It returns |nullptr| if the key cannot be
// converted.
std::unique_ptr<ECPublicKey> SecKeyRefToECPublicKey(SecKeyRef public_key_ref)
    API_AVAILABLE(macosx(10.12.2));

std::vector<uint8_t> TouchIdAaguid();

}  // namespace mac
}  // namespace fido
}  // namespace device

#endif  // DEVICE_FIDO_MAC_UTIL_H_
