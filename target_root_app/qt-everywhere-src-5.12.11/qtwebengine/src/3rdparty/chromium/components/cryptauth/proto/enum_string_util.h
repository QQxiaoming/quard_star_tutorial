// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CRYPTAUTH_PROTO_ENUM_STRING_UTIL_H_
#define COMPONENTS_CRYPTAUTH_PROTO_ENUM_STRING_UTIL_H_

#include <ostream>

#include "components/cryptauth/proto/cryptauth_api.pb.h"

namespace cryptauth {

std::ostream& operator<<(std::ostream& stream,
                         const SoftwareFeature& software_fature);

}  // namespace cryptauth

#endif  // COMPONENTS_CRYPTAUTH_PROTO_ENUM_STRING_UTIL_H_
