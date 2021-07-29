
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_PUBLIC_COMMON_BLOB_BLOB_UTILS_H_
#define THIRD_PARTY_BLINK_PUBLIC_COMMON_BLOB_BLOB_UTILS_H_

#include "third_party/blink/common/common_export.h"

namespace blink {

class BlobUtils {
 public:
  // Whether the new Blob URL glue for NetworkService is enabled (i.e.,
  // the NetworkService or MojoBlobURLs feature is enabled).
  static bool BLINK_COMMON_EXPORT MojoBlobURLsEnabled();
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_PUBLIC_COMMON_BLOB_BLOB_UTILS_H_
