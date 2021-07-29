// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_WTF_TEXT_MOVABLE_STRING_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_WTF_TEXT_MOVABLE_STRING_H_

#include <map>
#include <set>
#include <utility>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "third_party/blink/renderer/platform/wtf/wtf_thread_data.h"

// MovableString represents a string that may be moved in memory, that it its
// underlying memory address may change. Its content can be retrieved with the
// |ToString()| method.
// As a consequence, the inner pointer should never be cached, and only touched
// through a string returned by the |ToString()| method.
//
// As with WTF::AtomicString, this class is *not* thread-safe, and strings
// created on a thread must always be used on the same thread.
//
// Implementation note: the string content is not moved yet, it is merely
// used to gather statistics.

namespace WTF {

class WTF_EXPORT MovableStringImpl final
    : public RefCounted<MovableStringImpl> {
 public:
  // Histogram buckets, exported for testing.
  enum class ParkingAction {
    kParkedInBackground = 0,
    kUnparkedInBackground = 1,
    kUnparkedInForeground = 2,
    kMaxValue = kUnparkedInForeground
  };

  MovableStringImpl();
  explicit MovableStringImpl(scoped_refptr<StringImpl>&& impl);
  ~MovableStringImpl();

  // The returned string may be used as a normal one, as long as the
  // returned value (or a copy of it) is alive.
  const String& ToString();

  // See the matching String methods.
  bool Is8Bit() const;
  bool IsNull() const;
  unsigned length() const { return string_.length(); }
  unsigned CharactersSizeInBytes() const;

  // A parked string cannot be accessed until it has been |Unpark()|-ed.
  // Returns true iff the string has been parked.
  bool Park();
  // Returns true if the string is parked.
  bool is_parked() const { return is_parked_; }

 private:
  void Unpark();

  String string_;
  bool is_parked_;

  DISALLOW_COPY_AND_ASSIGN(MovableStringImpl);
};

class WTF_EXPORT MovableString final {
 public:
  MovableString() : impl_(base::MakeRefCounted<MovableStringImpl>(nullptr)) {}
  explicit MovableString(scoped_refptr<StringImpl>&& impl);
  MovableString(const MovableString& rhs) : impl_(rhs.impl_) {}
  ~MovableString();

  // See the matching String methods.
  bool Is8Bit() const;
  bool IsNull() const;
  unsigned length() const { return impl_->length(); }
  MovableStringImpl* Impl() const { return impl_.get(); }
  // Returns an unparked version of the string.
  // The string is guaranteed to be valid for
  // max(lifetime of a copy of the returned reference, current thread task).
  const String& ToString() const;
  unsigned CharactersSizeInBytes() const;

  // Causes the string to be unparked. Note that the pointer must not be
  // cached.
  const LChar* Characters8() const { return ToString().Characters8(); }
  const UChar* Characters16() const { return ToString().Characters16(); }

 private:
  scoped_refptr<MovableStringImpl> impl_;
};

// Per-thread registry of all MovableString instances. NOT thread-safe.
class WTF_EXPORT MovableStringTable final {
 public:
  MovableStringTable();
  ~MovableStringTable();

  static MovableStringTable& Instance() {
    return WtfThreadData().GetMovableStringTable();
  }

  scoped_refptr<MovableStringImpl> Add(scoped_refptr<StringImpl>&&);

  // This is for ~MovableStringImpl to unregister a string before
  // destruction since the table is holding raw pointers. It should not be used
  // directly.
  void Remove(StringImpl*);

  void SetRendererBackgrounded(bool backgrounded);
  bool IsRendererBackgrounded() const;

  // Parks all the strings in the table if currently in background.
  void MaybeParkAll();

 private:
  bool backgrounded_;
  // Could bet a set where the hashing function is
  // MovableStringImpl::string_.Impl(), but clearer this way.
  std::map<StringImpl*, MovableStringImpl*> table_;

  FRIEND_TEST_ALL_PREFIXES(MovableStringTest, TableSimple);
  FRIEND_TEST_ALL_PREFIXES(MovableStringTest, TableMultiple);

  DISALLOW_COPY_AND_ASSIGN(MovableStringTable);
};

}  // namespace WTF

using WTF::MovableStringTable;
using WTF::MovableString;
using WTF::MovableStringImpl;

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_WTF_TEXT_MOVABLE_STRING_H_
