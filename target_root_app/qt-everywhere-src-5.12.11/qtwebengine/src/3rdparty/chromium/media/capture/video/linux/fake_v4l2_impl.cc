// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/capture/video/linux/fake_v4l2_impl.h"

#include <linux/videodev2.h>
#include <string.h>

#include "base/stl_util.h"

#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

namespace media {

static const int kInvalidId = -1;
static const int kSuccessReturnValue = 0;
static const int kErrorReturnValue = -1;

class FakeV4L2Impl::OpenedDevice {
 public:
  explicit OpenedDevice(const std::string& device_id, int open_flags)
      : device_id_(device_id), open_flags_(open_flags) {}

  const std::string& device_id() const { return device_id_; }
  int open_flags() const { return open_flags_; }

 private:
  const std::string device_id_;
  const int open_flags_;
};

FakeV4L2Impl::FakeV4L2Impl() : next_id_to_return_from_open_(1) {}

FakeV4L2Impl::~FakeV4L2Impl() = default;

void FakeV4L2Impl::AddDevice(const std::string& device_name,
                             const FakeV4L2DeviceConfig& config) {
  device_configs_.emplace(device_name, config);
}

int FakeV4L2Impl::open(const char* device_name, int flags) {
  std::string device_name_as_string(device_name);
  if (!base::ContainsKey(device_configs_, device_name_as_string))
    return kInvalidId;

  auto id_iter = device_name_to_open_id_map_.find(device_name_as_string);
  if (id_iter != device_name_to_open_id_map_.end()) {
    // Device is already open
    return kInvalidId;
  }

  auto device_id = next_id_to_return_from_open_++;
  device_name_to_open_id_map_.emplace(device_name_as_string, device_id);
  opened_devices_.emplace(
      device_id, std::make_unique<OpenedDevice>(device_name_as_string, flags));
  return device_id;
}

int FakeV4L2Impl::close(int fd) {
  auto device_iter = opened_devices_.find(fd);
  if (device_iter == opened_devices_.end())
    return kErrorReturnValue;
  device_name_to_open_id_map_.erase(device_iter->second->device_id());
  opened_devices_.erase(device_iter->first);
  return kSuccessReturnValue;
}

int FakeV4L2Impl::ioctl(int fd, int request, void* argp) {
  auto device_iter = opened_devices_.find(fd);
  if (device_iter == opened_devices_.end())
    return EBADF;
  auto& opened_device = device_iter->second;
  auto& device_config = device_configs_.at(opened_device->device_id());

  switch (request) {
    case VIDIOC_ENUM_FMT: {
      auto* fmtdesc = reinterpret_cast<v4l2_fmtdesc*>(argp);
      if (fmtdesc->index > 0u) {
        // We only support a single format for now.
        return EINVAL;
      }
      if (fmtdesc->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        // We only support video capture.
        return EINVAL;
      }
      fmtdesc->flags = 0u;
      strcpy(reinterpret_cast<char*>(fmtdesc->description), "YUV420");
      fmtdesc->pixelformat = V4L2_PIX_FMT_YUV420;
      memset(fmtdesc->reserved, 0, 4);
      return kSuccessReturnValue;
    }
    case VIDIOC_QUERYCAP: {
      auto* cap = reinterpret_cast<v4l2_capability*>(argp);
      strcpy(reinterpret_cast<char*>(cap->driver), "FakeV4L2");
      CHECK(device_config.descriptor.display_name().size() < 31);
      strcpy(reinterpret_cast<char*>(cap->driver),
             device_config.descriptor.display_name().c_str());
      cap->bus_info[0] = 0;
      // Provide arbitrary version info
      cap->version = KERNEL_VERSION(1, 0, 0);
      cap->capabilities = V4L2_CAP_VIDEO_CAPTURE;
      memset(cap->reserved, 0, 4);
      return kSuccessReturnValue;
    }
    case VIDIOC_STREAMON:
    case VIDIOC_STREAMOFF:
      NOTIMPLEMENTED();
      return kSuccessReturnValue;
    case VIDIOC_CROPCAP:
    case VIDIOC_DBG_G_REGISTER:
    case VIDIOC_DBG_S_REGISTER:
    case VIDIOC_ENCODER_CMD:
    case VIDIOC_TRY_ENCODER_CMD:
    case VIDIOC_ENUMAUDIO:
    case VIDIOC_ENUMAUDOUT:
    case VIDIOC_ENUM_FRAMESIZES:
    case VIDIOC_ENUM_FRAMEINTERVALS:
    case VIDIOC_ENUMINPUT:
    case VIDIOC_ENUMOUTPUT:
    case VIDIOC_ENUMSTD:
    case VIDIOC_G_AUDIO:
    case VIDIOC_S_AUDIO:
    case VIDIOC_G_AUDOUT:
    case VIDIOC_S_AUDOUT:
    case VIDIOC_G_CROP:
    case VIDIOC_S_CROP:
    case VIDIOC_G_CTRL:
    case VIDIOC_S_CTRL:
    case VIDIOC_G_ENC_INDEX:
    case VIDIOC_G_EXT_CTRLS:
    case VIDIOC_S_EXT_CTRLS:
    case VIDIOC_TRY_EXT_CTRLS:
    case VIDIOC_G_FBUF:
    case VIDIOC_S_FBUF:
    case VIDIOC_G_FMT:
    case VIDIOC_S_FMT:
    case VIDIOC_TRY_FMT:
    case VIDIOC_G_FREQUENCY:
    case VIDIOC_S_FREQUENCY:
    case VIDIOC_G_INPUT:
    case VIDIOC_S_INPUT:
    case VIDIOC_G_JPEGCOMP:
    case VIDIOC_S_JPEGCOMP:
    case VIDIOC_G_MODULATOR:
    case VIDIOC_S_MODULATOR:
    case VIDIOC_G_OUTPUT:
    case VIDIOC_S_OUTPUT:
    case VIDIOC_G_PARM:
    case VIDIOC_S_PARM:
    case VIDIOC_G_PRIORITY:
    case VIDIOC_S_PRIORITY:
    case VIDIOC_G_SLICED_VBI_CAP:
    case VIDIOC_G_STD:
    case VIDIOC_S_STD:
    case VIDIOC_G_TUNER:
    case VIDIOC_S_TUNER:
    case VIDIOC_LOG_STATUS:
    case VIDIOC_OVERLAY:
    case VIDIOC_QBUF:
    case VIDIOC_DQBUF:
    case VIDIOC_QUERYBUF:
    case VIDIOC_QUERYCTRL:
    case VIDIOC_QUERYMENU:
    case VIDIOC_QUERYSTD:
    case VIDIOC_REQBUFS:
    case VIDIOC_S_HW_FREQ_SEEK:
      // Unsupported |request| code.
      NOTREACHED() << "Unsupported request code " << request;
      return kErrorReturnValue;
  }

  // Invalid |request|.
  NOTREACHED();
  return kErrorReturnValue;
}

void* FakeV4L2Impl::mmap(void* start,
                         size_t length,
                         int prot,
                         int flags,
                         int fd,
                         off_t offset) {
  NOTREACHED();
  return nullptr;
}

int FakeV4L2Impl::munmap(void* start, size_t length) {
  NOTREACHED();
  return kErrorReturnValue;
}

int FakeV4L2Impl::poll(struct pollfd* ufds, unsigned int nfds, int timeout) {
  NOTREACHED();
  return kErrorReturnValue;
}

}  // namespace media
