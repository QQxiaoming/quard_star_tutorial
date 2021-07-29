// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "testing/gtest/include/gtest/gtest.h"

#include "third_party/blink/renderer/platform/heap/name_traits.h"

namespace blink {

namespace {

class ClassWithoutName {
 public:
  ClassWithoutName() = default;
};

class ClassWithName : public NameClient {
 public:
  ClassWithName(const char* name) : name_(name) {}

  const char* NameInHeapSnapshot() const final { return name_; };

 private:
  const char* name_;
};

}  // namespace

TEST(NameTraitTest, DefaultName) {
  ClassWithoutName no_name;
  const char* name = NameTrait<ClassWithoutName>::GetName(&no_name);
  EXPECT_EQ(0, strcmp(name, "InternalNode"));
}

TEST(NameTraitTest, CustomName) {
  ClassWithName with_name("CustomName");
  const char* name = NameTrait<ClassWithName>::GetName(&with_name);
  EXPECT_EQ(0, strcmp(name, "CustomName"));
}

}  // namespace blink
