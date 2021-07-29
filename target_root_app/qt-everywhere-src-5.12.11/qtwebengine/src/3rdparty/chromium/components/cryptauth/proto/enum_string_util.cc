// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/cryptauth/proto/enum_string_util.h"

namespace cryptauth {

std::ostream& operator<<(std::ostream& stream,
                         const SoftwareFeature& software_feature) {
  switch (software_feature) {
    case SoftwareFeature::BETTER_TOGETHER_HOST:
      stream << "[Better Together host]";
      break;
    case SoftwareFeature::BETTER_TOGETHER_CLIENT:
      stream << "[Better Together client]";
      break;
    case SoftwareFeature::EASY_UNLOCK_HOST:
      stream << "[EasyUnlock host]";
      break;
    case SoftwareFeature::EASY_UNLOCK_CLIENT:
      stream << "[EasyUnlock client]";
      break;
    case SoftwareFeature::MAGIC_TETHER_HOST:
      stream << "[Instant Tethering host]";
      break;
    case SoftwareFeature::MAGIC_TETHER_CLIENT:
      stream << "[Instant Tethering client]";
      break;
    case SoftwareFeature::SMS_CONNECT_HOST:
      stream << "[SMS Connect host]";
      break;
    case SoftwareFeature::SMS_CONNECT_CLIENT:
      stream << "[SMS Connect client]";
      break;
    default:
      stream << "[unknown software feature]";
      break;
  }
  return stream;
}

}  // namespace cryptauth
