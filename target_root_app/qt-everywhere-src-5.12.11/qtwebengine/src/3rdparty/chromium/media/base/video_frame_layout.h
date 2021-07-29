// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_VIDEO_FRAME_LAYOUT_H_
#define MEDIA_BASE_VIDEO_FRAME_LAYOUT_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

#include "media/base/media_export.h"
#include "media/base/video_types.h"
#include "ui/gfx/geometry/size.h"

namespace media {

// A class to describes how physical buffer is allocated for video frame.
// In stores format, coded size of the frame and size of physical buffers
// which can be used to allocate buffer(s) hardware expected.
// Also, it stores stride (bytes per line) per color plane to calculate each
// color plane's size (note that a buffer may contains multiple color planes.)
class MEDIA_EXPORT VideoFrameLayout {
 public:
  // Constructor with strides and buffers' size.
  VideoFrameLayout(VideoPixelFormat format,
                   const gfx::Size& coded_size,
                   std::vector<int32_t> strides = std::vector<int32_t>(),
                   std::vector<size_t> buffer_sizes = std::vector<size_t>());

  VideoFrameLayout(const VideoFrameLayout& layout);

  ~VideoFrameLayout();

  VideoPixelFormat format() const { return format_; }
  const gfx::Size& coded_size() const { return coded_size_; }

  // Return number of buffers. Note that num_strides >= num_buffers.
  size_t num_buffers() const { return buffer_sizes_.size(); }

  // Returns number of strides. Note that num_strides >= num_buffers.
  size_t num_strides() const { return strides_.size(); }

  const std::vector<int32_t>& strides() const { return strides_; }
  const std::vector<size_t>& buffer_sizes() const { return buffer_sizes_; }

  // Sets strides.
  void set_strides(std::vector<int32_t> strides) {
    strides_ = std::move(strides);
  }

  // Sets buffer_sizes.
  void set_buffer_sizes(std::vector<size_t> buffer_sizes) {
    buffer_sizes_ = std::move(buffer_sizes);
  }

  // Returns sum of bytes of all buffers.
  size_t GetTotalBufferSize() const;

  // Composes VideoFrameLayout as human readable string.
  std::string ToString() const;

 private:
  const VideoPixelFormat format_;

  // Width and height of the video frame in pixels. This must include pixel
  // data for the whole image; i.e. for YUV formats with subsampled chroma
  // planes, in the case that the visible portion of the image does not line up
  // on a sample boundary, |coded_size_| must be rounded up appropriately and
  // the pixel data provided for the odd pixels.
  const gfx::Size coded_size_;

  // Vector of strides for each buffer, typically greater or equal to the
  // width of the surface divided by the horizontal sampling period. Note that
  // strides can be negative if the image layout is bottom-up.
  std::vector<int32_t> strides_;

  // Vector of sizes for each buffer, typically greater or equal to the area of
  // |coded_size_|.
  std::vector<size_t> buffer_sizes_;
};

}  // namespace media

#endif  // MEDIA_BASE_VIDEO_FRAME_LAYOUT_H_
