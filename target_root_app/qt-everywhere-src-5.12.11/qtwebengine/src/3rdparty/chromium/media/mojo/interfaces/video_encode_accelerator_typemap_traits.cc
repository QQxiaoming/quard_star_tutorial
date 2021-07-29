// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/interfaces/video_encode_accelerator_typemap_traits.h"

#include "base/logging.h"
#include "media/base/video_bitrate_allocation.h"
#include "mojo/public/cpp/base/time_mojom_traits.h"

namespace mojo {

// static
media::mojom::VideoEncodeAccelerator::Error
EnumTraits<media::mojom::VideoEncodeAccelerator::Error,
           media::VideoEncodeAccelerator::Error>::
    ToMojom(media::VideoEncodeAccelerator::Error error) {
  switch (error) {
    case media::VideoEncodeAccelerator::kIllegalStateError:
      return media::mojom::VideoEncodeAccelerator::Error::ILLEGAL_STATE;
    case media::VideoEncodeAccelerator::kInvalidArgumentError:
      return media::mojom::VideoEncodeAccelerator::Error::INVALID_ARGUMENT;
    case media::VideoEncodeAccelerator::kPlatformFailureError:
      return media::mojom::VideoEncodeAccelerator::Error::PLATFORM_FAILURE;
  }
  NOTREACHED();
  return media::mojom::VideoEncodeAccelerator::Error::INVALID_ARGUMENT;
}

// static
bool EnumTraits<media::mojom::VideoEncodeAccelerator::Error,
                media::VideoEncodeAccelerator::Error>::
    FromMojom(media::mojom::VideoEncodeAccelerator::Error error,
              media::VideoEncodeAccelerator::Error* out) {
  switch (error) {
    case media::mojom::VideoEncodeAccelerator::Error::ILLEGAL_STATE:
      *out = media::VideoEncodeAccelerator::kIllegalStateError;
      return true;
    case media::mojom::VideoEncodeAccelerator::Error::INVALID_ARGUMENT:
      *out = media::VideoEncodeAccelerator::kInvalidArgumentError;
      return true;
    case media::mojom::VideoEncodeAccelerator::Error::PLATFORM_FAILURE:
      *out = media::VideoEncodeAccelerator::kPlatformFailureError;
      return true;
  }
  NOTREACHED();
  return false;
}

// static
std::vector<int32_t> StructTraits<media::mojom::VideoBitrateAllocationDataView,
                                  media::VideoBitrateAllocation>::
    bitrates(const media::VideoBitrateAllocation& bitrate_allocation) {
  std::vector<int32_t> bitrates;
  int sum_bps = 0;
  for (size_t si = 0; si < media::VideoBitrateAllocation::kMaxSpatialLayers;
       ++si) {
    for (size_t ti = 0; ti < media::VideoBitrateAllocation::kMaxTemporalLayers;
         ++ti) {
      if (sum_bps == bitrate_allocation.GetSumBps()) {
        // The rest is all zeros, no need to iterate further.
        return bitrates;
      }
      const int layer_bitrate = bitrate_allocation.GetBitrateBps(si, ti);
      bitrates.emplace_back(layer_bitrate);
      sum_bps += layer_bitrate;
    }
  }
  return bitrates;
}

// static
bool StructTraits<media::mojom::VideoBitrateAllocationDataView,
                  media::VideoBitrateAllocation>::
    Read(media::mojom::VideoBitrateAllocationDataView data,
         media::VideoBitrateAllocation* out_bitrate_allocation) {
  ArrayDataView<int32_t> bitrates;
  data.GetBitratesDataView(&bitrates);
  size_t size = bitrates.size();
  if (size > media::VideoBitrateAllocation::kMaxSpatialLayers *
                 media::VideoBitrateAllocation::kMaxTemporalLayers) {
    return false;
  }
  for (size_t i = 0; i < size; ++i) {
    const int32_t bitrate = bitrates[i];
    const size_t si = i / media::VideoBitrateAllocation::kMaxTemporalLayers;
    const size_t ti = i % media::VideoBitrateAllocation::kMaxTemporalLayers;
    if (!out_bitrate_allocation->SetBitrate(si, ti, bitrate)) {
      return false;
    }
  }
  return true;
}

// static
bool StructTraits<media::mojom::BitstreamBufferMetadataDataView,
                  media::BitstreamBufferMetadata>::
    Read(media::mojom::BitstreamBufferMetadataDataView data,
         media::BitstreamBufferMetadata* out_metadata) {
  out_metadata->payload_size_bytes = data.payload_size_bytes();
  out_metadata->key_frame = data.key_frame();
  if (!data.ReadTimestamp(&out_metadata->timestamp)) {
    return false;
  }
  return data.ReadVp8(&out_metadata->vp8);
}

// static
bool StructTraits<media::mojom::Vp8MetadataDataView, media::Vp8Metadata>::Read(
    media::mojom::Vp8MetadataDataView data,
    media::Vp8Metadata* out_metadata) {
  out_metadata->non_reference = data.non_reference();
  out_metadata->temporal_idx = data.temporal_idx();
  out_metadata->layer_sync = data.layer_sync();
  return true;
}
}  // namespace mojo
