// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/video_frame_layout.h"

#include <numeric>
#include <sstream>

namespace {

template <class T>
std::string VectorToString(const std::vector<T>& vec) {
  std::ostringstream result;
  std::string delim;
  result << "[";
  for (auto v : vec) {
    result << delim << v;
    if (delim.size() == 0)
      delim = ", ";
  }
  result << "]";
  return result.str();
}

}  // namespace

namespace media {

VideoFrameLayout::VideoFrameLayout(VideoPixelFormat format,
                                   const gfx::Size& coded_size,
                                   std::vector<int32_t> strides,
                                   std::vector<size_t> buffer_sizes)
    : format_(format),
      coded_size_(coded_size),
      strides_(std::move(strides)),
      buffer_sizes_(std::move(buffer_sizes)) {}

VideoFrameLayout::VideoFrameLayout(const VideoFrameLayout& layout) = default;
VideoFrameLayout::~VideoFrameLayout() = default;

size_t VideoFrameLayout::GetTotalBufferSize() const {
  return std::accumulate(buffer_sizes_.begin(), buffer_sizes_.end(), 0u);
}

std::string VideoFrameLayout::ToString() const {
  std::ostringstream s;
  s << "VideoFrameLayout format:" << VideoPixelFormatToString(format_)
    << " coded_size:" << coded_size_.ToString()
    << " num_buffers:" << num_buffers()
    << " buffer_sizes:" << VectorToString(buffer_sizes_)
    << " num_strides:" << num_strides()
    << " strides:" << VectorToString(strides_);
  return s.str();
}

}  // namespace media
