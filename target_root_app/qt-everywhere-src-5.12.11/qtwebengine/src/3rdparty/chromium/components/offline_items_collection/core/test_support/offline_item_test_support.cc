// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_items_collection/core/offline_item.h"

#include <iostream>

namespace offline_items_collection {

// All of these methods are not provided in core so that they can't be
// accidentally called and linked into Chrome. The declarations are provided
// in the core headers to avoid ODR violation that can occur if, for instance,
// one test includes these operators and one test does not.

std::ostream& operator<<(std::ostream& os, const OfflineItem& item) {
  os << "OfflineItem(";
  os << "id: " << item.id.name_space << "." << item.id.id;
  os << ", title: " << item.title;
  os << ", description: " << item.description;
  os << ", filter: " << item.filter;
  os << ", is_transient: " << item.is_transient;
  os << ", is_suggested: " << item.is_suggested;
  os << ", is_accelerated: " << item.is_accelerated;
  os << ", total_size_bytes: " << item.total_size_bytes;
  os << ", externally_removed: " << item.externally_removed;
  os << ", creation_time: " << item.creation_time;
  os << ", last_accessed_time: " << item.last_accessed_time;
  os << ", is_openable: " << item.is_openable;
  os << ", file_path: " << item.file_path;
  os << ", mime_type: " << item.mime_type;
  os << ", page_url: " << item.page_url;
  os << ", original_url: " << item.original_url;
  os << ", is_off_the_record: " << item.is_off_the_record;
  os << ", state: " << item.state;
  os << ", fail_state: " << item.fail_state;
  os << ", pending_state: " << item.pending_state;
  os << ", is_resumable: " << item.is_resumable;
  os << ", allow_metered: " << item.allow_metered;
  os << ", received_bytes: " << item.received_bytes;
  os << ", progress: " << item.progress.value;
  if (item.progress.max)
    os << "/" << item.progress.max.value();
  os << ", time_remaining_ms: " << item.time_remaining_ms;
  os << ", is_dangerous: " << item.is_dangerous;
  os << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const OfflineItemState& state) {
  switch (state) {
    case IN_PROGRESS:
      return os << "IN_PROGRESS";
    case PENDING:
      return os << "PENDING";
    case COMPLETE:
      return os << "COMPLETE";
    case CANCELLED:
      return os << "CANCELLED";
    case INTERRUPTED:
      return os << "INTERRUPTED";
    case FAILED:
      return os << "FAILED";
    case PAUSED:
      return os << "PAUSED";
    case MAX_DOWNLOAD_STATE:
      return os << "MAX_DOWNLOAD_STATE";
  }
  CHECK(false) << "state=" << static_cast<int>(state);
  return os;
}

std::ostream& operator<<(std::ostream& os, FailState state) {
  switch (state) {
    case FailState::NO_FAILURE:
      return os << "NO_FAILURE";
    case FailState::CANNOT_DOWNLOAD:
      return os << "CANNOT_DOWNLOAD";
    case FailState::NETWORK_INSTABILITY:
      return os << "NETWORK_INSTABILITY";
  }
  CHECK(false) << "state=" << static_cast<int>(state);
  return os;
}

std::ostream& operator<<(std::ostream& os, PendingState state) {
  switch (state) {
    case PendingState::NOT_PENDING:
      return os << "NOT_PENDING";
    case PendingState::PENDING_NETWORK:
      return os << "PENDING_NETWORK";
    case PendingState::PENDING_ANOTHER_DOWNLOAD:
      return os << "PENDING_ANOTHER_DOWNLOAD";
  }
  CHECK(false) << "state=" << static_cast<int>(state);
  return os;
}

std::ostream& operator<<(std::ostream& os, OfflineItemFilter state) {
  switch (state) {
    case FILTER_PAGE:
      return os << "FILTER_PAGE";
    case FILTER_VIDEO:
      return os << "FILTER_VIDEO";
    case FILTER_AUDIO:
      return os << "FILTER_AUDIO";
    case FILTER_IMAGE:
      return os << "FILTER_IMAGE";
    case FILTER_DOCUMENT:
      return os << "FILTER_DOCUMENT";
    case FILTER_OTHER:
      return os << "FILTER_OTHER";
    case FILTER_BOUNDARY:
      return os << "FILTER_BOUNDARY";
  }
  CHECK(false) << "state=" << static_cast<int>(state);
  return os;
}

}  // namespace offline_items_collection
