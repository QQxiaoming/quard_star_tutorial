/*
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "third_party/blink/renderer/platform/shared_buffer.h"

#include <memory>

#include "third_party/blink/renderer/platform/instrumentation/tracing/web_process_memory_dump.h"
#include "third_party/blink/renderer/platform/wtf/text/unicode.h"
#include "third_party/blink/renderer/platform/wtf/text/utf8.h"
#include "third_party/skia/include/core/SkData.h"

namespace blink {

constexpr unsigned SharedBuffer::kSegmentSize;

static inline size_t SegmentIndex(size_t position) {
  return position / SharedBuffer::kSegmentSize;
}

static inline size_t OffsetInSegment(size_t position) {
  return position % SharedBuffer::kSegmentSize;
}

class SharedBuffer::Segment final {
 public:
  Segment() {
    ptr_.reset(static_cast<char*>(WTF::Partitions::FastMalloc(
        SharedBuffer::kSegmentSize, "blink::SharedBuffer")));
  }
  ~Segment() = default;
  Segment(Segment&&) = default;

  char* get() { return ptr_.get(); }
  const char* get() const { return ptr_.get(); }

 private:
  struct Deleter {
    void operator()(char* p) const { WTF::Partitions::FastFree(p); }
  };
  std::unique_ptr<char[], Deleter> ptr_;
};

SharedBuffer::Iterator& SharedBuffer::Iterator::operator++() {
  DCHECK(!IsEnd());
  ++index_;
  Init(0);
  return *this;
}

SharedBuffer::Iterator::Iterator(const SharedBuffer* buffer)
    : index_(buffer->segments_.size() + 1), buffer_(buffer) {
  DCHECK(IsEnd());
}

SharedBuffer::Iterator::Iterator(size_t offset, const SharedBuffer* buffer)
    : index_(0), buffer_(buffer) {
  Init(offset);
}

SharedBuffer::Iterator::Iterator(size_t segment_index,
                                 size_t offset,
                                 const SharedBuffer* buffer)
    : index_(segment_index + 1), buffer_(buffer) {
  Init(offset);
}

void SharedBuffer::Iterator::Init(size_t offset) {
  if (IsEnd()) {
    value_ = base::span<const char>();
    return;
  }

  if (index_ == 0) {
    DCHECK_LT(offset, buffer_->buffer_.size());
    value_ = base::make_span(buffer_->buffer_.data() + offset,
                             buffer_->buffer_.size() - offset);
    return;
  }
  const auto segment_index = index_ - 1;
  const auto& segment = buffer_->segments_[segment_index];
  size_t segment_size = segment_index == buffer_->segments_.size() - 1
                            ? buffer_->GetLastSegmentSize()
                            : kSegmentSize;
  DCHECK_LT(offset, segment_size);
  value_ = base::make_span(segment.get() + offset, segment_size - offset);
}

SharedBuffer::SharedBuffer() : size_(0) {}

SharedBuffer::SharedBuffer(size_t size) : size_(size), buffer_(size) {}

SharedBuffer::SharedBuffer(const char* data, size_t size) : size_(size) {
  buffer_.Append(data, size);
}

SharedBuffer::SharedBuffer(const unsigned char* data, size_t size)
    : SharedBuffer(reinterpret_cast<const char*>(data), size) {}

SharedBuffer::~SharedBuffer() = default;

scoped_refptr<SharedBuffer> SharedBuffer::AdoptVector(Vector<char>& vector) {
  scoped_refptr<SharedBuffer> buffer = Create();
  buffer->buffer_.swap(vector);
  buffer->size_ = buffer->buffer_.size();
  return buffer;
}

const char* SharedBuffer::Data() {
  MergeSegmentsIntoBuffer();
  return buffer_.data();
}

void SharedBuffer::Append(const SharedBuffer& data) {
  for (const auto& span : data)
    Append(span.data(), span.size());
}

void SharedBuffer::AppendInternal(const char* data, size_t length) {
  if (!length)
    return;

  DCHECK_GE(size_, buffer_.size());
  size_t position_in_segment = OffsetInSegment(size_ - buffer_.size());
  size_ += length;

  if (size_ <= kSegmentSize) {
    // No need to use segments for small resource data.
    buffer_.Append(data, length);
    return;
  }

  while (length > 0) {
    if (!position_in_segment)
      segments_.push_back(Segment());

    size_t bytes_to_copy = std::min(length, kSegmentSize - position_in_segment);
    memcpy(segments_.back().get() + position_in_segment, data, bytes_to_copy);

    data += bytes_to_copy;
    length -= bytes_to_copy;
    position_in_segment = 0;
  }
}

void SharedBuffer::Clear() {
  segments_.clear();
  size_ = 0;
  buffer_.clear();
}

SharedBuffer::Iterator SharedBuffer::begin() const {
  return GetIteratorAt(static_cast<size_t>(0));
}

SharedBuffer::Iterator SharedBuffer::end() const {
  return Iterator(this);
}

void SharedBuffer::MergeSegmentsIntoBuffer() {
  size_t bytes_left = size_ - buffer_.size();
  for (const auto& segment : segments_) {
    size_t bytes_to_copy = std::min<size_t>(bytes_left, kSegmentSize);
    buffer_.Append(segment.get(), bytes_to_copy);
    bytes_left -= bytes_to_copy;
  }
  segments_.clear();
}

SharedBuffer::Iterator SharedBuffer::GetIteratorAtInternal(
    size_t position) const {
  if (position >= size())
    return cend();

  if (position < buffer_.size())
    return Iterator(position, this);

  return Iterator(SegmentIndex(position - buffer_.size()),
                  OffsetInSegment(position - buffer_.size()), this);
}

bool SharedBuffer::GetBytesInternal(void* dest, size_t dest_size) const {
  if (!dest)
    return false;

  size_t offset = 0;
  for (const auto& span : *this) {
    if (offset >= dest_size)
      break;
    size_t to_be_written = std::min(span.size(), dest_size - offset);
    memcpy(static_cast<char*>(dest) + offset, span.data(), to_be_written);
    offset += to_be_written;
  }
  return offset == dest_size;
}

sk_sp<SkData> SharedBuffer::GetAsSkData() const {
  sk_sp<SkData> data = SkData::MakeUninitialized(size());
  char* buffer = static_cast<char*>(data->writable_data());
  size_t offset = 0;
  for (const auto& span : *this) {
    memcpy(buffer + offset, span.data(), span.size());
    offset += span.size();
  }

  DCHECK_EQ(offset, size());
  return data;
}

void SharedBuffer::OnMemoryDump(const String& dump_prefix,
                                WebProcessMemoryDump* memory_dump) const {
  if (buffer_.size()) {
    WebMemoryAllocatorDump* dump =
        memory_dump->CreateMemoryAllocatorDump(dump_prefix + "/shared_buffer");
    dump->AddScalar("size", "bytes", buffer_.size());
    memory_dump->AddSuballocation(
        dump->Guid(), String(WTF::Partitions::kAllocatedObjectPoolName));
  } else {
    // If there is data in the segments, then it should have been allocated
    // using fastMalloc.
    const String data_dump_name = dump_prefix + "/segments";
    auto* dump = memory_dump->CreateMemoryAllocatorDump(data_dump_name);
    dump->AddScalar("size", "bytes", size_);
    memory_dump->AddSuballocation(
        dump->Guid(), String(WTF::Partitions::kAllocatedObjectPoolName));
  }
}

SharedBuffer::DeprecatedFlatData::DeprecatedFlatData(
    scoped_refptr<const SharedBuffer> buffer)
    : buffer_(std::move(buffer)) {
  DCHECK(buffer_);

  if (buffer_->size() <= buffer_->buffer_.size()) {
    // The SharedBuffer is not segmented - just point to its data.
    data_ = buffer_->buffer_.data();
    return;
  }

  // Merge all segments.
  flat_buffer_.ReserveInitialCapacity(buffer_->size());
  for (const auto& span : *buffer_)
    flat_buffer_.Append(span.data(), span.size());

  data_ = flat_buffer_.data();
}

}  // namespace blink
