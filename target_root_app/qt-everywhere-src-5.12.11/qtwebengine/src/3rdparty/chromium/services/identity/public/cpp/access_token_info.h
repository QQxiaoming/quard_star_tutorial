// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_IDENTITY_PUBLIC_CPP_ACCESS_TOKEN_INFO_H_
#define SERVICES_IDENTITY_PUBLIC_CPP_ACCESS_TOKEN_INFO_H_

#include <string>

#include "base/macros.h"
#include "base/time/time.h"

namespace identity {

// Container for a valid access token plus associated metadata.
struct AccessTokenInfo {
  // The access token itself.
  std::string token;

  // The time at which this access token will expire.
  base::Time expiration_time;

  AccessTokenInfo() = default;
  AccessTokenInfo(const std::string& token_param,
                  const base::Time& expiration_time_param)
      : token(token_param), expiration_time(expiration_time_param) {}
};

// Defined for testing purposes only.
bool operator==(const AccessTokenInfo& lhs, const AccessTokenInfo& rhs);

}  // namespace identity

#endif  // SERVICES_IDENTITY_PUBLIC_CPP_ACCESS_TOKEN_INFO_H_
