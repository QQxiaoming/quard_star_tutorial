// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/previews/content/activation_list.h"

#include <string>
#include <vector>

#include "base/macros.h"
#include "components/previews/core/previews_experiments.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace previews {

namespace {

class ActivationListTest : public testing::Test {
 public:
  ActivationListTest() {}

  ~ActivationListTest() override {}

 protected:
  std::unique_ptr<ActivationList> page_host_activation_list_;

  DISALLOW_COPY_AND_ASSIGN(ActivationListTest);
};

TEST_F(ActivationListTest, TestInitialization) {
  std::vector<std::string> keys;
  keys.push_back("example.com");
  keys.push_back("m.foo.com");
  page_host_activation_list_ = std::make_unique<ActivationList>(keys);

  struct {
    GURL url;
    bool expect_whitelisted;
  } test_cases[] = {
      {GURL("https://www.example.com"), true},
      {GURL("https://www.example.co.in"), false},
      {GURL("https://www.example.com/index.html"), true},
      {GURL("https://www.example.com/a.html"), true},
      {GURL("https://www.example.com/foo/a.html"), true},
      {GURL("https://m.example.com"), true},
      {GURL("https://m.example.com/foo.html"), true},
      {GURL("https://m.example.com/foo/bar/baz.html"), true},
      {GURL("https://example.com"), true},
      {GURL("https://example2.com"), false},
      {GURL("https://m.example2.com"), false},
      {GURL("https://example2.com/foo.html"), false},

      {GURL("https://m.foo.com"), true},
      {GURL("https://en.m.foo.com"), true},
      {GURL("https://m.en.foo.com"), false},
      {GURL("https://m.foo.com/foo.html"), true},
      {GURL("https://m.foo.com/foo/bar/baz.html"), true},
      {GURL("https://foo.com"), false},
      {GURL("https://en.foo.com"), false},
      {GURL("https://m.foo2.com"), false},
      {GURL("https://en.foo2.com"), false},
  };

  for (const auto& test : test_cases) {
    EXPECT_TRUE(test.url.is_valid());
    EXPECT_EQ(test.expect_whitelisted,
              page_host_activation_list_->IsHostWhitelistedAtNavigation(
                  test.url, PreviewsType::RESOURCE_LOADING_HINTS))
        << " url=" << test.url.spec();
  }
}

}  // namespace

}  // namespace previews