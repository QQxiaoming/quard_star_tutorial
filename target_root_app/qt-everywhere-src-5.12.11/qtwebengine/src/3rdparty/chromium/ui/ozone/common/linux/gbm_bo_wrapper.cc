// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/common/linux/gbm_bo_wrapper.h"

#include <gbm.h>

#include "ui/gfx/native_pixmap_handle.h"

namespace ui {

GbmBoWrapper::GbmBoWrapper(gbm_bo* bo,
                           uint32_t format,
                           uint32_t flags,
                           uint64_t modifier,
                           std::vector<base::ScopedFD> fds,
                           const gfx::Size& size,
                           std::vector<gfx::NativePixmapPlane> planes)
    : bo_(bo),
      format_modifier_(modifier),
      format_(format),
      flags_(flags),
      fds_(std::move(fds)),
      size_(size),
      planes_(std::move(planes)) {}

GbmBoWrapper::~GbmBoWrapper() {
  if (bo_)
    gbm_bo_destroy(bo_);
}

bool GbmBoWrapper::AreFdsValid() const {
  if (fds_.empty())
    return false;

  for (const auto& fd : fds_) {
    if (fd.get() == -1)
      return false;
  }
  return true;
}

int GbmBoWrapper::GetFd(size_t index) const {
  DCHECK_LT(index, fds_.size());
  return fds_[index].get();
}

int GbmBoWrapper::GetStride(size_t index) const {
  DCHECK_LT(index, planes_.size());
  return planes_[index].stride;
}

int GbmBoWrapper::GetOffset(size_t index) const {
  DCHECK_LT(index, planes_.size());
  return planes_[index].offset;
}

size_t GbmBoWrapper::GetPlaneSize(size_t index) const {
  DCHECK_LT(index, planes_.size());
  return planes_[index].size;
}

uint32_t GbmBoWrapper::GetBoHandle() const {
  return bo_ ? gbm_bo_get_handle(bo_).u32 : 0;
}

}  // namespace ui
