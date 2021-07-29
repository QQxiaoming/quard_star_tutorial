// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/cursor/cursor_data.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/cursor/cursor_type.h"

#if defined(USE_AURA)
#include "ui/base/cursor/cursor.h"
#endif

#if defined(USE_OZONE)
#include "ui/ozone/public/cursor_factory_ozone.h"
#endif

namespace ui {

CursorData::CursorData()
    : cursor_type_(CursorType::kNull), scale_factor_(0.0f) {}

CursorData::CursorData(CursorType type)
    : cursor_type_(type), scale_factor_(0.0f) {}

CursorData::CursorData(const gfx::Point& hotspot_point,
                       const std::vector<SkBitmap>& cursor_frames,
                       float scale_factor,
                       const base::TimeDelta& frame_delay)
    : cursor_type_(CursorType::kCustom),
      frame_delay_(frame_delay),
      scale_factor_(scale_factor),
      hotspot_(hotspot_point),
      cursor_frames_(cursor_frames) {
  for (SkBitmap& bitmap : cursor_frames_)
    generator_ids_.push_back(bitmap.getGenerationID());
}

CursorData::CursorData(const CursorData& cursor) = default;

CursorData::CursorData(CursorData&& cursor) = default;

CursorData::~CursorData() {}

CursorData& CursorData::operator=(const CursorData& cursor) = default;

CursorData& CursorData::operator=(CursorData&& cursor) = default;

bool CursorData::IsType(CursorType cursor_type) const {
  return cursor_type_ == cursor_type;
}

bool CursorData::IsSameAs(const CursorData& rhs) const {
  return cursor_type_ == rhs.cursor_type_ && frame_delay_ == rhs.frame_delay_ &&
         hotspot_ == rhs.hotspot_ && scale_factor_ == rhs.scale_factor_ &&
         generator_ids_ == rhs.generator_ids_;
}

#if defined(USE_AURA)
gfx::NativeCursor CursorData::ToNativeCursor() const {
  Cursor cursor(cursor_type_);

#if defined(USE_OZONE)
  auto* factory = CursorFactoryOzone::GetInstance();
  if (cursor_type_ != CursorType::kCustom) {
    cursor.SetPlatformCursor(factory->GetDefaultCursor(cursor_type_));
  } else if (cursor_frames_.size() > 1U) {
    cursor.SetPlatformCursor(factory->CreateAnimatedCursor(
        cursor_frames_, hotspot_, frame_delay_.InMilliseconds(),
        scale_factor_));
    // CreateAnimatedCursor() and CreateImageCursor() below both return a cursor
    // with a ref count of 1, which we need to account for after storing it in
    // |cursor|.
    cursor.UnrefCustomCursor();
  } else {
    DCHECK_EQ(1U, cursor_frames_.size());
    cursor.SetPlatformCursor(
        factory->CreateImageCursor(cursor_frames_[0], hotspot_, scale_factor_));
    cursor.UnrefCustomCursor();
  }
#else
  NOTIMPLEMENTED();
#endif

  if (!cursor_frames_.empty()) {
    cursor.set_custom_bitmap(cursor_frames_[0]);
    cursor.set_custom_hotspot(hotspot_);
  }
  return cursor;
}
#endif

}  // namespace ui
