// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/blacklist/opt_out_blacklist/opt_out_blacklist_item.h"

#include <algorithm>
#include <tuple>

#include "components/blacklist/opt_out_blacklist/opt_out_store.h"

namespace blacklist {

OptOutBlacklistItem::OptOutRecord::OptOutRecord(base::Time entry_time,
                                                bool opt_out)
    : entry_time_(entry_time), opt_out_(opt_out) {}

OptOutBlacklistItem::OptOutRecord::~OptOutRecord() {}

OptOutBlacklistItem::OptOutRecord::OptOutRecord(OptOutRecord&&) noexcept =
    default;
OptOutBlacklistItem::OptOutRecord& OptOutBlacklistItem::OptOutRecord::operator=(
    OptOutRecord&&) noexcept = default;

bool OptOutBlacklistItem::OptOutRecord::operator<(
    const OptOutRecord& other) const {
  // Fresher entries are lower priority to evict, as are non-opt-outs.
  return std::tie(entry_time_, opt_out_) >
         std::tie(other.entry_time_, other.opt_out_);
}

OptOutBlacklistItem::OptOutBlacklistItem(size_t stored_history_length,
                                         int opt_out_black_list_threshold,
                                         base::TimeDelta black_list_duration)
    : max_stored_history_length_(stored_history_length),
      opt_out_black_list_threshold_(opt_out_black_list_threshold),
      max_black_list_duration_(black_list_duration),
      total_opt_out_(0) {}

OptOutBlacklistItem::~OptOutBlacklistItem() {}

void OptOutBlacklistItem::AddEntry(bool opt_out, base::Time entry_time) {
  DCHECK_LE(opt_out_records_.size(), max_stored_history_length_);

  opt_out_records_.emplace(entry_time, opt_out);

  if (opt_out && (!most_recent_opt_out_time_ ||
                  entry_time > most_recent_opt_out_time_.value())) {
    most_recent_opt_out_time_ = entry_time;
  }
  total_opt_out_ += opt_out ? 1 : 0;

  // Remove the oldest entry if the size exceeds the max history size.
  if (opt_out_records_.size() > max_stored_history_length_) {
    DCHECK_EQ(opt_out_records_.size(), max_stored_history_length_ + 1);
    DCHECK_LE(opt_out_records_.top().entry_time(), entry_time);
    total_opt_out_ -= opt_out_records_.top().opt_out() ? 1 : 0;
    opt_out_records_.pop();
  }
  DCHECK_LE(opt_out_records_.size(), max_stored_history_length_);
}

bool OptOutBlacklistItem::IsBlackListed(base::Time now) const {
  DCHECK_LE(opt_out_records_.size(), max_stored_history_length_);
  return most_recent_opt_out_time_ &&
         now - most_recent_opt_out_time_.value() < max_black_list_duration_ &&
         total_opt_out_ >= opt_out_black_list_threshold_;
}

size_t OptOutBlacklistItem::OptOutRecordsSizeForTesting() const {
  return opt_out_records_.size();
}

}  // namespace blacklist
