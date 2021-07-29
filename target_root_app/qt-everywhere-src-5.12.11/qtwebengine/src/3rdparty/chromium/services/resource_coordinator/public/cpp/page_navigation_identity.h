// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_RESOURCE_COORDINATOR_PUBLIC_CPP_PAGE_NAVIGATION_IDENTITY_H_
#define SERVICES_RESOURCE_COORDINATOR_PUBLIC_CPP_PAGE_NAVIGATION_IDENTITY_H_

#include <stdint.h>
#include <string>
#include "services/resource_coordinator/public/cpp/coordination_unit_id.h"

namespace resource_coordinator {

struct PageNavigationIdentity {
  // The coordination ID of the page this event relates to.
  CoordinationUnitID page_cu_id;
  // The unique ID of the NavigationHandle of the page at the time the event
  // relates to.
  int64_t navigation_id;
  // The URL of the last navigation.
  std::string url;
};

}  // namespace resource_coordinator

#endif  // SERVICES_RESOURCE_COORDINATOR_PUBLIC_CPP_PAGE_NAVIGATION_IDENTITY_H_
