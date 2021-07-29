// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/input/scroll_snap_data.h"

#include <algorithm>
#include <cmath>

namespace cc {
namespace {

bool IsMutualVisible(const SnapSearchResult& a, const SnapSearchResult& b) {
  return a.visible_range().Contains(b.snap_offset()) &&
         b.visible_range().Contains(a.snap_offset());
}

bool SnappableOnAxis(const SnapAreaData& area, SearchAxis search_axis) {
  return search_axis == SearchAxis::kX
             ? area.scroll_snap_align.alignment_inline != SnapAlignment::kNone
             : area.scroll_snap_align.alignment_block != SnapAlignment::kNone;
}

}  // namespace

bool SnapVisibleRange::Contains(float value) const {
  if (start_ < end_)
    return start_ <= value && value <= end_;
  return end_ <= value && value <= start_;
}

SnapSearchResult::SnapSearchResult(float offset, const SnapVisibleRange& range)
    : snap_offset_(offset) {
  set_visible_range(range);
}

void SnapSearchResult::set_visible_range(const SnapVisibleRange& range) {
  DCHECK(range.start() <= range.end());
  visible_range_ = range;
}

void SnapSearchResult::Clip(float max_snap, float max_visible) {
  snap_offset_ = std::max(std::min(snap_offset_, max_snap), 0.0f);
  visible_range_ = SnapVisibleRange(
      std::max(std::min(visible_range_.start(), max_visible), 0.0f),
      std::max(std::min(visible_range_.end(), max_visible), 0.0f));
}

void SnapSearchResult::Union(const SnapSearchResult& other) {
  DCHECK(snap_offset_ == other.snap_offset_);
  visible_range_ = SnapVisibleRange(
      std::min(visible_range_.start(), other.visible_range_.start()),
      std::max(visible_range_.end(), other.visible_range_.end()));
}

SnapContainerData::SnapContainerData()
    : proximity_range_(gfx::ScrollOffset(std::numeric_limits<float>::max(),
                                         std::numeric_limits<float>::max())) {}

SnapContainerData::SnapContainerData(ScrollSnapType type)
    : scroll_snap_type_(type),
      proximity_range_(gfx::ScrollOffset(std::numeric_limits<float>::max(),
                                         std::numeric_limits<float>::max())) {}

SnapContainerData::SnapContainerData(ScrollSnapType type,
                                     const gfx::RectF& rect,
                                     const gfx::ScrollOffset& max)
    : scroll_snap_type_(type),
      rect_(rect),
      max_position_(max),
      proximity_range_(gfx::ScrollOffset(std::numeric_limits<float>::max(),
                                         std::numeric_limits<float>::max())) {}

SnapContainerData::SnapContainerData(const SnapContainerData& other) = default;

SnapContainerData::SnapContainerData(SnapContainerData&& other) = default;

SnapContainerData::~SnapContainerData() = default;

SnapContainerData& SnapContainerData::operator=(
    const SnapContainerData& other) = default;

SnapContainerData& SnapContainerData::operator=(SnapContainerData&& other) =
    default;

void SnapContainerData::AddSnapAreaData(SnapAreaData snap_area_data) {
  snap_area_list_.push_back(snap_area_data);
}

bool SnapContainerData::FindSnapPosition(
    const gfx::ScrollOffset& current_position,
    bool should_snap_on_x,
    bool should_snap_on_y,
    gfx::ScrollOffset* snap_position) const {
  SnapAxis axis = scroll_snap_type_.axis;
  should_snap_on_x &= (axis == SnapAxis::kX || axis == SnapAxis::kBoth);
  should_snap_on_y &= (axis == SnapAxis::kY || axis == SnapAxis::kBoth);
  if (!should_snap_on_x && !should_snap_on_y)
    return false;

  base::Optional<SnapSearchResult> closest_x, closest_y;
  // A region that includes every reachable scroll position.
  gfx::RectF scrollable_region(0, 0, max_position_.x(), max_position_.y());
  if (should_snap_on_x) {
    // Start from current offset in the cross axis and assume it's always
    // visible.
    SnapSearchResult initial_snap_position_y = {
        current_position.y(), SnapVisibleRange(0, max_position_.x())};
    closest_x = FindClosestValidArea(SearchAxis::kX, current_position.x(),
                                     initial_snap_position_y);
  }
  if (should_snap_on_y) {
    SnapSearchResult initial_snap_position_x = {
        current_position.x(), SnapVisibleRange(0, max_position_.y())};
    closest_y = FindClosestValidArea(SearchAxis::kY, current_position.y(),
                                     initial_snap_position_x);
  }

  if (!closest_x.has_value() && !closest_y.has_value())
    return false;

  // If snapping in one axis pushes off-screen the other snap area, this snap
  // position is invalid. https://drafts.csswg.org/css-scroll-snap-1/#snap-scope
  // In this case, we choose the axis whose snap area is closer, and find a
  // mutual visible snap area on the other axis.
  if (closest_x.has_value() && closest_y.has_value() &&
      !IsMutualVisible(closest_x.value(), closest_y.value())) {
    bool candidate_on_x_axis_is_closer =
        std::abs(closest_x.value().snap_offset() - current_position.x()) <=
        std::abs(closest_y.value().snap_offset() - current_position.y());
    if (candidate_on_x_axis_is_closer)
      closest_y = FindClosestValidArea(SearchAxis::kY, current_position.y(),
                                       closest_x.value());
    else
      closest_x = FindClosestValidArea(SearchAxis::kX, current_position.x(),
                                       closest_y.value());
  }

  *snap_position = current_position;
  if (closest_x.has_value())
    snap_position->set_x(closest_x.value().snap_offset());
  if (closest_y.has_value())
    snap_position->set_y(closest_y.value().snap_offset());
  return true;
}

base::Optional<SnapSearchResult> SnapContainerData::FindClosestValidArea(
    SearchAxis axis,
    float current_offset,
    const SnapSearchResult& cros_axis_snap_result) const {
  base::Optional<SnapSearchResult> result;
  base::Optional<SnapSearchResult> inplace;
  // The valid snap offsets immediately before and after the current offset.
  float prev = std::numeric_limits<float>::lowest();
  float next = std::numeric_limits<float>::max();
  float smallest_distance =
      axis == SearchAxis::kX ? proximity_range_.x() : proximity_range_.y();
  for (const SnapAreaData& area : snap_area_list_) {
    if (!SnappableOnAxis(area, axis))
      continue;

    SnapSearchResult candidate = GetSnapSearchResult(axis, area);
    if (IsSnapportCoveredOnAxis(axis, current_offset, area.rect)) {
      // Since snap area is currently covering the snapport, we consider the
      // current offset as a valid snap position.
      SnapSearchResult inplace_candidate = candidate;
      inplace_candidate.set_snap_offset(current_offset);
      if (IsMutualVisible(inplace_candidate, cros_axis_snap_result)) {
        // If we've already found a valid overflowing area before, we enlarge
        // the area's visible region.
        if (inplace.has_value())
          inplace.value().Union(inplace_candidate);
        else
          inplace = inplace_candidate;
        // Even if a snap area covers the snapport, we need to continue this
        // search to find previous and next snap positions and also to have
        // alternative snap candidates if this inplace candidate is ultimately
        // rejected. And this covering snap area has its own alignment that may
        // generates a snap position rejecting the current inplace candidate.
      }
    }
    if (!IsMutualVisible(candidate, cros_axis_snap_result))
      continue;
    float distance = std::abs(candidate.snap_offset() - current_offset);
    if (distance < smallest_distance) {
      smallest_distance = distance;
      result = candidate;
    }
    if (candidate.snap_offset() < current_offset &&
        candidate.snap_offset() > prev)
      prev = candidate.snap_offset();
    if (candidate.snap_offset() > current_offset &&
        candidate.snap_offset() < next)
      next = candidate.snap_offset();
  }
  // According to the spec [1], if the snap area is covering the snapport, the
  // scroll offset is a valid snap position only if the distance between the
  // geometrically previous and subsequent snap positions in that axis is larger
  // than size of the snapport in that axis.
  // [1] https://drafts.csswg.org/css-scroll-snap-1/#snap-overflow
  float size = axis == SearchAxis::kX ? rect_.width() : rect_.height();
  if (inplace.has_value() &&
      (prev == std::numeric_limits<float>::lowest() ||
       next == std::numeric_limits<float>::max() || next - prev > size)) {
    return inplace;
  }

  return result;
}

SnapSearchResult SnapContainerData::GetSnapSearchResult(
    SearchAxis axis,
    const SnapAreaData& area) const {
  SnapSearchResult result;
  if (axis == SearchAxis::kX) {
    result.set_visible_range(SnapVisibleRange(area.rect.y() - rect_.bottom(),
                                              area.rect.bottom() - rect_.y()));
    // https://www.w3.org/TR/css-scroll-snap-1/#scroll-snap-align
    switch (area.scroll_snap_align.alignment_inline) {
      case SnapAlignment::kStart:
        result.set_snap_offset(area.rect.x() - rect_.x());
        break;
      case SnapAlignment::kCenter:
        result.set_snap_offset(area.rect.CenterPoint().x() -
                               rect_.CenterPoint().x());
        break;
      case SnapAlignment::kEnd:
        result.set_snap_offset(area.rect.right() - rect_.right());
        break;
      default:
        NOTREACHED();
    }
    result.Clip(max_position_.x(), max_position_.y());
  } else {
    result.set_visible_range(SnapVisibleRange(area.rect.x() - rect_.right(),
                                              area.rect.right() - rect_.x()));
    switch (area.scroll_snap_align.alignment_block) {
      case SnapAlignment::kStart:
        result.set_snap_offset(area.rect.y() - rect_.y());
        break;
      case SnapAlignment::kCenter:
        result.set_snap_offset(area.rect.CenterPoint().y() -
                               rect_.CenterPoint().y());
        break;
      case SnapAlignment::kEnd:
        result.set_snap_offset(area.rect.bottom() - rect_.bottom());
        break;
      default:
        NOTREACHED();
    }
    result.Clip(max_position_.y(), max_position_.x());
  }
  return result;
}

bool SnapContainerData::IsSnapportCoveredOnAxis(
    SearchAxis axis,
    float current_offset,
    const gfx::RectF& area_rect) const {
  if (axis == SearchAxis::kX) {
    if (area_rect.width() < rect_.width())
      return false;
    float left = area_rect.x() - rect_.x();
    float right = area_rect.right() - rect_.right();
    return current_offset >= left && current_offset <= right;
  } else {
    if (area_rect.height() < rect_.height())
      return false;
    float top = area_rect.y() - rect_.y();
    float bottom = area_rect.bottom() - rect_.bottom();
    return current_offset >= top && current_offset <= bottom;
  }
}

std::ostream& operator<<(std::ostream& ostream, const SnapAreaData& area_data) {
  return ostream << area_data.rect.ToString();
}

std::ostream& operator<<(std::ostream& ostream,
                         const SnapContainerData& container_data) {
  ostream << "container_rect: " << container_data.rect().ToString();
  ostream << "area_rects: ";
  for (size_t i = 0; i < container_data.size(); ++i) {
    ostream << container_data.at(i) << "\n";
  }
  return ostream;
}

}  // namespace cc
