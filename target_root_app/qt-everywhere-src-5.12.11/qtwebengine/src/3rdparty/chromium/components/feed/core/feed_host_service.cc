// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/feed/core/feed_host_service.h"

#include <utility>

namespace feed {

FeedHostService::FeedHostService(
    std::unique_ptr<FeedImageManager> image_manager,
    std::unique_ptr<FeedNetworkingHost> networking_host,
    std::unique_ptr<FeedSchedulerHost> scheduler_host,
    std::unique_ptr<FeedStorageDatabase> storage_database)
    : image_manager_(std::move(image_manager)),
      networking_host_(std::move(networking_host)),
      scheduler_host_(std::move(scheduler_host)),
      storage_database_(std::move(storage_database)) {}

FeedHostService::~FeedHostService() = default;

FeedImageManager* FeedHostService::GetImageManager() {
  return image_manager_.get();
}

FeedNetworkingHost* FeedHostService::GetNetworkingHost() {
  return networking_host_.get();
}

FeedSchedulerHost* FeedHostService::GetSchedulerHost() {
  return scheduler_host_.get();
}

FeedStorageDatabase* FeedHostService::GetStorageDatabase() {
  return storage_database_.get();
}

}  // namespace feed
