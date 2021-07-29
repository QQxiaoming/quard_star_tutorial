// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/wtf/text/movable_string.h"

#include "base/metrics/histogram_macros.h"

namespace WTF {

namespace {

bool IsLargeEnough(const StringImpl* impl) {
  // Don't attempt to park strings smaller than this size.
  static constexpr unsigned int kSizeThreshold = 10000;
  return impl && impl->length() > kSizeThreshold;
}

void RecordParkingAction(MovableStringImpl::ParkingAction action) {
  UMA_HISTOGRAM_ENUMERATION("Memory.MovableStringParkingAction", action);
}

}  // namespace

MovableStringImpl::MovableStringImpl() = default;

MovableStringImpl::MovableStringImpl(scoped_refptr<StringImpl>&& impl)
    : string_(std::move(impl)), is_parked_(false) {}

MovableStringImpl::~MovableStringImpl() {
  MovableStringTable::Instance().Remove(string_.Impl());
}

bool MovableStringImpl::Is8Bit() const {
  return string_.Is8Bit();
}

bool MovableStringImpl::IsNull() const {
  return string_.IsNull();
}

const String& MovableStringImpl::ToString() {
  Unpark();
  return string_;
}

unsigned MovableStringImpl::CharactersSizeInBytes() const {
  return string_.CharactersSizeInBytes();
}

bool MovableStringImpl::Park() {
  // Cannot park strings with several references.
  if (string_.Impl()->HasOneRef()) {
    RecordParkingAction(ParkingAction::kParkedInBackground);
    is_parked_ = true;
  }
  return is_parked_;
}

void MovableStringImpl::Unpark() {
  if (!is_parked_)
    return;

  bool backgrounded = MovableStringTable::Instance().IsRendererBackgrounded();
  RecordParkingAction(backgrounded ? ParkingAction::kUnparkedInBackground
                                   : ParkingAction::kUnparkedInForeground);
  is_parked_ = false;
}

MovableString::MovableString(scoped_refptr<StringImpl>&& impl) {
  // Don't move small strings.
  if (IsLargeEnough(impl.get())) {
    impl_ = MovableStringTable::Instance().Add(std::move(impl));
  } else {
    impl_ = base::MakeRefCounted<MovableStringImpl>(std::move(impl));
  }
}

MovableString::~MovableString() = default;

bool MovableString::Is8Bit() const {
  return impl_->Is8Bit();
}

bool MovableString::IsNull() const {
  return impl_->IsNull();
}
const String& MovableString::ToString() const {
  return impl_->ToString();
}

unsigned MovableString::CharactersSizeInBytes() const {
  return impl_->CharactersSizeInBytes();
}

MovableStringTable::MovableStringTable() = default;
MovableStringTable::~MovableStringTable() = default;

scoped_refptr<MovableStringImpl> MovableStringTable::Add(
    scoped_refptr<StringImpl>&& string) {
  StringImpl* raw_ptr = string.get();
  auto it = table_.find(raw_ptr);
  if (it != table_.end()) {
    return it->second;
  }
  auto new_movable_string =
      base::MakeRefCounted<MovableStringImpl>(std::move(string));
  table_.emplace(raw_ptr, new_movable_string.get());
  return new_movable_string;
}

void MovableStringTable::Remove(StringImpl* string) {
  if (!IsLargeEnough(string))
    return;

  auto it = table_.find(string);
  DCHECK(it != table_.end());
  table_.erase(it);
}

void MovableStringTable::SetRendererBackgrounded(bool backgrounded) {
  backgrounded_ = backgrounded;
}

bool MovableStringTable::IsRendererBackgrounded() const {
  return backgrounded_;
}

void MovableStringTable::MaybeParkAll() {
  if (!IsRendererBackgrounded())
    return;

  size_t total_size = 0, count = 0;
  for (auto& kv : table_) {
    MovableStringImpl* str = kv.second;
    str->Park();
    total_size += str->CharactersSizeInBytes();
    count += 1;
  }

  size_t total_size_kb = total_size / 1000;
  UMA_HISTOGRAM_COUNTS_100000("Memory.MovableStringsTotalSizeKb",
                              total_size_kb);
  UMA_HISTOGRAM_COUNTS_1000("Memory.MovableStringsCount", table_.size());
}

}  // namespace WTF
