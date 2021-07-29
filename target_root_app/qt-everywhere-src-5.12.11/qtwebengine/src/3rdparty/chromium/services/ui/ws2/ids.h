// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_WS2_IDS_H_
#define SERVICES_UI_WS2_IDS_H_

#include <stdint.h>

#include "components/viz/common/surfaces/frame_sink_id.h"
#include "services/ui/common/types.h"

namespace ui {
namespace ws2 {

// A client id used to indicate no client. That is, no WindowTree ever gets this
// id.
constexpr ClientSpecificId kInvalidClientId = 0;

constexpr Id kInvalidTransportId = 0;

// A client id used to indicate WindowServer.
constexpr ClientSpecificId kWindowServerClientId = 1;

// The initial id assigned to window service clients, incremented thereafter.
constexpr ClientSpecificId kInitialClientId = 2;

// The initial id assigned to window service clients in a decrementing mode.
// Used when Chrome hosts Ash in the browser process; in that configuration,
// Content connects directly to Viz and assigns incrementing FrameSinkIds to
// renderers, so the window service uses decrementing ids to avoid conflicts.
constexpr ClientSpecificId kInitialClientIdDecrement =
    std::numeric_limits<ClientSpecificId>::max();

// Used for ids assigned by the client.
using ClientWindowId = viz::FrameSinkId;

using ClientWindowIdHash = viz::FrameSinkIdHash;

}  // namespace ws2
}  // namespace ui

#endif  // SERVICES_UI_WS2_IDS_H_
