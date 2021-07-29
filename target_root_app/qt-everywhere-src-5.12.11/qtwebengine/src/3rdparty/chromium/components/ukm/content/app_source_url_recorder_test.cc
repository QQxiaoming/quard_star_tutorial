// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/ukm/content/app_source_url_recorder.h"

#include "base/test/scoped_feature_list.h"
#include "components/ukm/test_ukm_recorder.h"
#include "components/ukm/ukm_source.h"
#include "content/public/test/test_renderer_host.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace ukm {

class AppSourceUrlRecorderTest : public content::RenderViewHostTestHarness {
 public:
  void SetUp() override {
    scoped_feature_list_.InitAndEnableFeature(kUkmAppLogging);
    content::RenderViewHostTestHarness::SetUp();
  }

 protected:
  SourceId GetSourceIdForApp(AppType type, const std::string& id) {
    return AppSourceUrlRecorder::GetSourceIdForApp(type, id);
  }

  SourceId GetSourceIdForPWA(const GURL& url) {
    return AppSourceUrlRecorder::GetSourceIdForPWA(url);
  }

  base::test::ScopedFeatureList scoped_feature_list_;
  TestAutoSetUkmRecorder test_ukm_recorder_;
};

TEST_F(AppSourceUrlRecorderTest, CheckPlay) {
  SourceId id_play =
      GetSourceIdForApp(AppType::kArc, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

  GURL expected_url_play("app://play/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

  const auto& sources = test_ukm_recorder_.GetSources();
  EXPECT_EQ(1ul, sources.size());

  ASSERT_NE(kInvalidSourceId, id_play);
  auto it = sources.find(id_play);
  ASSERT_NE(sources.end(), it);
  EXPECT_EQ(expected_url_play, it->second->url());
  EXPECT_TRUE(it->second->initial_url().is_empty());
}

TEST_F(AppSourceUrlRecorderTest, CheckPWA) {
  GURL url("https://pwa_example_url.com");
  SourceId id = GetSourceIdForPWA(url);

  const auto& sources = test_ukm_recorder_.GetSources();
  EXPECT_EQ(1ul, sources.size());

  ASSERT_NE(kInvalidSourceId, id);
  auto it = sources.find(id);
  ASSERT_NE(sources.end(), it);
  EXPECT_EQ(url, it->second->url());
  EXPECT_TRUE(it->second->initial_url().is_empty());
}

}  // namespace ukm
