// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_bitrate_allocation.h"

#include <limits>
#include <numeric>

#include "base/logging.h"
#include "base/numerics/checked_math.h"

namespace media {

constexpr size_t VideoBitrateAllocation::kMaxSpatialLayers;
constexpr size_t VideoBitrateAllocation::kMaxTemporalLayers;

VideoBitrateAllocation::VideoBitrateAllocation() : sum_(0), bitrates_{} {}

bool VideoBitrateAllocation::SetBitrate(size_t spatial_index,
                                        size_t temporal_index,
                                        int bitrate_bps) {
  CHECK_LT(spatial_index, kMaxSpatialLayers);
  CHECK_LT(temporal_index, kMaxTemporalLayers);
  CHECK_GE(bitrate_bps, 0);

  base::CheckedNumeric<int> checked_sum = sum_;
  checked_sum -= bitrates_[spatial_index][temporal_index];
  checked_sum += bitrate_bps;
  if (!checked_sum.IsValid()) {
    return false;  // Would cause overflow of the sum.
  }

  sum_ = checked_sum.ValueOrDie();
  bitrates_[spatial_index][temporal_index] = bitrate_bps;
  return true;
}

int VideoBitrateAllocation::GetBitrateBps(size_t spatial_index,
                                          size_t temporal_index) const {
  CHECK_LT(spatial_index, kMaxSpatialLayers);
  CHECK_LT(temporal_index, kMaxTemporalLayers);
  return bitrates_[spatial_index][temporal_index];
}

int VideoBitrateAllocation::GetSumBps() const {
  return sum_;
}

bool VideoBitrateAllocation::operator==(
    const VideoBitrateAllocation& other) const {
  if (sum_ != other.sum_) {
    return false;
  }
  return memcmp(bitrates_, other.bitrates_, sizeof(bitrates_)) == 0;
}

}  // namespace media
