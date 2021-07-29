// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_FIDO_MAC_BROWSING_DATA_DELETION_H_
#define DEVICE_FIDO_MAC_BROWSING_DATA_DELETION_H_

#include <string>

#include "base/component_export.h"
#include "base/time/time.h"

namespace device {
namespace fido {
namespace mac {

// DeleteWebAuthnCredentiuals deletes Touch ID authenticator credentials from
// the macOS keychain that were created within the time interval `[begin, end)`
// and with the given metadata secret (which is tied to a browser profile).
// The |keychain_access_group| parameter is an identifier tied to Chrome's code
// signing identity that identifies the set of all keychain items associated
// with the Touch ID WebAuthentication authenticator.
//
// Returns false if any attempt to delete a credential failed (but others may
// still have succeeded), and true otherwise.
//
// On platforms where Touch ID is not supported, or when the Touch ID WebAuthn
// authenticator feature flag is disabled, this method does nothing and returns
// true.
bool COMPONENT_EXPORT(DEVICE_FIDO)
    DeleteWebAuthnCredentials(const std::string& keychain_access_group,
                              const std::string& profile_metadata_secret,
                              base::Time begin,
                              base::Time end);

}  // namespace mac
}  // namespace fido
}  // namespace device

#endif  // DEVICE_FIDO_MAC_BROWSING_DATA_DELETION_H_
