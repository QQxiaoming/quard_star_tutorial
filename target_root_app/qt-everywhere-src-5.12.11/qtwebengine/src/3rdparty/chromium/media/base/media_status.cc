// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "media/base/media_status.h"

namespace media {

MediaStatus::MediaStatus() = default;

MediaStatus::MediaStatus(const MediaStatus& other) = default;

MediaStatus::~MediaStatus() = default;

MediaStatus& MediaStatus::operator=(const MediaStatus& other) = default;

bool MediaStatus::operator==(const MediaStatus& other) const {
  return std::tie(title, can_play_pause, can_mute, can_set_volume, can_seek,
                  play_state, is_muted, volume, duration, current_time) ==
         std::tie(other.title, other.can_play_pause, other.can_mute,
                  other.can_set_volume, other.can_seek, play_state,
                  other.is_muted, other.volume, other.duration,
                  other.current_time);
}

}  // namespace media
