// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_COMMON_SURFACES_SURFACE_RANGE_H_
#define COMPONENTS_VIZ_COMMON_SURFACES_SURFACE_RANGE_H_

#include "base/optional.h"
#include "components/viz/common/surfaces/surface_id.h"
#include "components/viz/common/viz_common_export.h"
#include "mojo/public/cpp/bindings/struct_traits.h"

namespace viz {

namespace mojom {
class SurfaceRangeDataView;
}

// SurfaceRange consists of two SurfaceIds representing a range of surfaces
// [start,end] ordered by SurfaceId where |start| is an optional surface
// which acts like a "fallback" surface in case it exists.
class VIZ_COMMON_EXPORT SurfaceRange {
 public:
  SurfaceRange();

  SurfaceRange(const base::Optional<SurfaceId>& start, const SurfaceId& end);

  explicit SurfaceRange(const SurfaceId& surface_id);

  SurfaceRange(const SurfaceRange& other);

  bool operator==(const SurfaceRange& other) const;

  bool operator!=(const SurfaceRange& other) const;

  bool operator<(const SurfaceRange& other) const;

  bool IsValid() const;

  const base::Optional<SurfaceId>& start() const { return start_; }

  const SurfaceId& end() const { return end_; }

  std::string ToString() const;

 private:
  friend struct mojo::StructTraits<mojom::SurfaceRangeDataView, SurfaceRange>;

  base::Optional<SurfaceId> start_;
  SurfaceId end_;
};

VIZ_COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                           const SurfaceRange& surface_range);

}  // namespace viz

#endif  // COMPONENTS_VIZ_COMMON_SURFACES_SURFACE_RANGE_H_
