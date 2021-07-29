// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_THIRD_PARTY_QUIC_CORE_QUIC_DEBUG_INFO_PROVIDER_INTERFACE_H_
#define NET_THIRD_PARTY_QUIC_CORE_QUIC_DEBUG_INFO_PROVIDER_INTERFACE_H_

#include "net/third_party/quic/platform/api/quic_export.h"
#include "net/third_party/quic/platform/api/quic_string.h"

namespace quic {

class QUIC_EXPORT_PRIVATE QuicDebugInfoProviderInterface {
 public:
  virtual ~QuicDebugInfoProviderInterface() {}

  // Get ack processing related debug information of the connection.
  virtual QuicString DebugStringForAckProcessing() const = 0;
};

}  // namespace quic

#endif  // NET_THIRD_PARTY_QUIC_CORE_QUIC_DEBUG_INFO_PROVIDER_INTERFACE_H_
