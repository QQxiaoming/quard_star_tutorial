// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/video/video_encode_accelerator.h"

#include "base/callback.h"

namespace media {

Vp8Metadata::Vp8Metadata()
    : non_reference(false), temporal_idx(0), layer_sync(false) {}
Vp8Metadata::Vp8Metadata(const Vp8Metadata& other) = default;
Vp8Metadata::Vp8Metadata(Vp8Metadata&& other) = default;
Vp8Metadata::~Vp8Metadata() = default;

BitstreamBufferMetadata::BitstreamBufferMetadata()
    : payload_size_bytes(0), key_frame(false) {}
BitstreamBufferMetadata::BitstreamBufferMetadata(
    BitstreamBufferMetadata&& other) = default;
BitstreamBufferMetadata::BitstreamBufferMetadata(size_t payload_size_bytes,
                                                 bool key_frame,
                                                 base::TimeDelta timestamp)
    : payload_size_bytes(payload_size_bytes),
      key_frame(key_frame),
      timestamp(timestamp) {}
BitstreamBufferMetadata::~BitstreamBufferMetadata() = default;

VideoEncodeAccelerator::~VideoEncodeAccelerator() = default;

VideoEncodeAccelerator::SupportedProfile::SupportedProfile()
    : profile(media::VIDEO_CODEC_PROFILE_UNKNOWN),
      max_framerate_numerator(0),
      max_framerate_denominator(0) {
}

VideoEncodeAccelerator::SupportedProfile::~SupportedProfile() = default;

void VideoEncodeAccelerator::Flush(FlushCallback flush_callback) {
  // TODO(owenlin): implements this https://crbug.com/755889.
  NOTIMPLEMENTED();
  std::move(flush_callback).Run(false);
}

void VideoEncodeAccelerator::RequestEncodingParametersChange(
    const VideoBitrateAllocation& bitrate_allocation,
    uint32_t framerate) {
  RequestEncodingParametersChange(bitrate_allocation.GetSumBps(), framerate);
}

}  // namespace media

namespace std {

void default_delete<media::VideoEncodeAccelerator>::operator()(
    media::VideoEncodeAccelerator* vea) const {
  vea->Destroy();
}

}  // namespace std
