// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/video_frame_layout.h"

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <utility>

#include "media/base/video_types.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/size.h"

namespace media {

TEST(VideoFrameLayout, Constructor) {
  gfx::Size coded_size = gfx::Size(320, 180);
  std::vector<int32_t> strides = {384, 192, 192};
  std::vector<size_t> buffer_sizes = {73728, 18432, 18432};
  VideoFrameLayout layout(PIXEL_FORMAT_I420, coded_size, strides, buffer_sizes);

  EXPECT_EQ(layout.format(), PIXEL_FORMAT_I420);
  EXPECT_EQ(layout.coded_size(), coded_size);
  EXPECT_EQ(layout.num_strides(), 3u);
  EXPECT_EQ(layout.num_buffers(), 3u);
  EXPECT_EQ(layout.GetTotalBufferSize(), 110592u);
  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(layout.strides()[i], strides[i]);
    EXPECT_EQ(layout.buffer_sizes()[i], buffer_sizes[i]);
  }
}

TEST(VideoFrameLayout, ConstructorNoStrideBufferSize) {
  gfx::Size coded_size = gfx::Size(320, 180);
  VideoFrameLayout layout(PIXEL_FORMAT_I420, coded_size);

  EXPECT_EQ(layout.format(), PIXEL_FORMAT_I420);
  EXPECT_EQ(layout.coded_size(), coded_size);
  EXPECT_EQ(layout.GetTotalBufferSize(), 0u);
  EXPECT_EQ(layout.num_strides(), 0u);
  EXPECT_EQ(layout.num_buffers(), 0u);
}

TEST(VideoFrameLayout, CopyConstructor) {
  gfx::Size coded_size = gfx::Size(320, 180);
  std::vector<int32_t> strides = {384, 192, 192};
  std::vector<size_t> buffer_sizes = {73728, 18432, 18432};
  VideoFrameLayout layout(PIXEL_FORMAT_I420, coded_size, strides, buffer_sizes);

  VideoFrameLayout layout_copy(layout);

  EXPECT_EQ(layout_copy.format(), PIXEL_FORMAT_I420);
  EXPECT_EQ(layout_copy.coded_size(), coded_size);
  EXPECT_EQ(layout_copy.num_strides(), 3u);
  EXPECT_EQ(layout_copy.num_buffers(), 3u);
  EXPECT_EQ(layout_copy.GetTotalBufferSize(), 110592u);
  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(layout_copy.strides()[i], strides[i]);
    EXPECT_EQ(layout_copy.buffer_sizes()[i], buffer_sizes[i]);
  }
}

TEST(VideoFrameLayout, ToString) {
  gfx::Size coded_size = gfx::Size(320, 180);
  std::vector<int32_t> strides = {384, 192, 192};
  std::vector<size_t> buffer_sizes = {73728, 18432, 18432};
  VideoFrameLayout layout(PIXEL_FORMAT_I420, coded_size, strides, buffer_sizes);

  EXPECT_EQ(layout.ToString(),
            "VideoFrameLayout format:PIXEL_FORMAT_I420 coded_size:320x180 "
            "num_buffers:3 buffer_sizes:[73728, 18432, 18432] num_strides:3 "
            "strides:[384, 192, 192]");
}

TEST(VideoFrameLayout, ToStringOneBuffer) {
  gfx::Size coded_size = gfx::Size(320, 180);
  std::vector<int32_t> strides = {384};
  std::vector<size_t> buffer_sizes = {122880};
  VideoFrameLayout layout(PIXEL_FORMAT_NV12, coded_size, strides, buffer_sizes);

  EXPECT_EQ(layout.ToString(),
            "VideoFrameLayout format:PIXEL_FORMAT_NV12 coded_size:320x180 "
            "num_buffers:1 buffer_sizes:[122880] num_strides:1 strides:[384]");
}

TEST(VideoFrameLayout, ToStringNoBufferInfo) {
  gfx::Size coded_size = gfx::Size(320, 180);
  VideoFrameLayout layout(PIXEL_FORMAT_NV12, coded_size);

  EXPECT_EQ(layout.ToString(),
            "VideoFrameLayout format:PIXEL_FORMAT_NV12 coded_size:320x180 "
            "num_buffers:0 buffer_sizes:[] num_strides:0 strides:[]");
}

TEST(VideoFrameLayout, SetStrideBufferSize) {
  gfx::Size coded_size = gfx::Size(320, 180);
  VideoFrameLayout layout(PIXEL_FORMAT_NV12, coded_size);

  std::vector<int32_t> strides = {384, 192, 192};
  layout.set_strides(std::move(strides));
  std::vector<size_t> buffer_sizes = {122880};
  layout.set_buffer_sizes(std::move(buffer_sizes));

  EXPECT_EQ(layout.ToString(),
            "VideoFrameLayout format:PIXEL_FORMAT_NV12 coded_size:320x180 "
            "num_buffers:1 buffer_sizes:[122880] num_strides:3 "
            "strides:[384, 192, 192]");
}

}  // namespace media
