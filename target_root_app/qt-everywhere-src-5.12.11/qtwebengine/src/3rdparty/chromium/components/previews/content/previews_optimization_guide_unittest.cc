// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/previews/content/previews_optimization_guide.h"

#include <memory>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/run_loop.h"
#include "base/test/gtest_util.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "components/optimization_guide/optimization_guide_service.h"
#include "components/optimization_guide/optimization_guide_service_observer.h"
#include "components/optimization_guide/proto/hints.pb.h"
#include "components/previews/core/previews_features.h"
#include "components/previews/core/previews_user_data.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace previews {

class TestOptimizationGuideService
    : public optimization_guide::OptimizationGuideService {
 public:
  explicit TestOptimizationGuideService(
      const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner)
      : OptimizationGuideService(io_task_runner),
        remove_observer_called_(false) {}

  void RemoveObserver(
      optimization_guide::OptimizationGuideServiceObserver* observer) override {
    remove_observer_called_ = true;
  }

  bool RemoveObserverCalled() { return remove_observer_called_; }

 private:
  bool remove_observer_called_;
};

class PreviewsOptimizationGuideTest : public testing::Test {
 public:
  PreviewsOptimizationGuideTest() {}

  ~PreviewsOptimizationGuideTest() override {}

  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    optimization_guide_service_ =
        std::make_unique<TestOptimizationGuideService>(
            scoped_task_environment_.GetMainThreadTaskRunner());
    guide_ = std::make_unique<PreviewsOptimizationGuide>(
        optimization_guide_service_.get(),
        scoped_task_environment_.GetMainThreadTaskRunner());
  }

  // Delete |guide_| if it hasn't been deleted.
  void TearDown() override { ResetGuide(); }

  PreviewsOptimizationGuide* guide() { return guide_.get(); }

  TestOptimizationGuideService* optimization_guide_service() {
    return optimization_guide_service_.get();
  }

  void ProcessHints(const optimization_guide::proto::Configuration& config,
                    std::string version) {
    optimization_guide::ComponentInfo info(
        base::Version(version),
        temp_dir().Append(FILE_PATH_LITERAL("somefile.pb")));
    guide_->OnHintsProcessed(config, info);
  }

  std::unique_ptr<net::URLRequest> CreateRequestWithURL(const GURL& url) const {
    return context_.CreateRequest(url, net::DEFAULT_PRIORITY, nullptr,
                                  TRAFFIC_ANNOTATION_FOR_TESTS);
  }

  void ResetGuide() {
    guide_.reset();
    RunUntilIdle();
  }

  base::FilePath temp_dir() const { return temp_dir_.GetPath(); }

 protected:
  void RunUntilIdle() {
    scoped_task_environment_.RunUntilIdle();
    base::RunLoop().RunUntilIdle();
  }

  void DoExperimentFlagTest(base::Optional<std::string> experiment_name,
                            bool expect_enabled);

 private:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  base::ScopedTempDir temp_dir_;

  std::unique_ptr<PreviewsOptimizationGuide> guide_;
  std::unique_ptr<TestOptimizationGuideService> optimization_guide_service_;

  net::TestURLRequestContext context_;

  DISALLOW_COPY_AND_ASSIGN(PreviewsOptimizationGuideTest);
};

TEST_F(PreviewsOptimizationGuideTest, IsWhitelistedWithoutHints) {
  std::unique_ptr<net::URLRequest> request =
      CreateRequestWithURL(GURL("https://m.facebook.com"));
  EXPECT_FALSE(guide()->IsWhitelisted(*request, PreviewsType::NOSCRIPT));
}

TEST_F(PreviewsOptimizationGuideTest,
       ProcessHintsWhitelistForNoScriptPopulatedCorrectly) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  // Add a second optimization to ensure that the applicable optimizations are
  // still whitelisted.
  optimization_guide::proto::Optimization* optimization2 =
      hint1->add_whitelisted_optimizations();
  optimization2->set_optimization_type(
      optimization_guide::proto::TYPE_UNSPECIFIED);
  // Add a second hint.
  optimization_guide::proto::Hint* hint2 = config.add_hints();
  hint2->set_key("twitter.com");
  hint2->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization3 =
      hint2->add_whitelisted_optimizations();
  optimization3->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  ProcessHints(config, "2.0.0");

  RunUntilIdle();

  // Twitter and Facebook should be whitelisted but not Google.
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::NOSCRIPT));
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.twitter.com/example")),
      PreviewsType::NOSCRIPT));
  EXPECT_FALSE(
      guide()->IsWhitelisted(*CreateRequestWithURL(GURL("https://google.com")),
                             PreviewsType::NOSCRIPT));

  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.facebook.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.twitter.com/example"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://google.com"), PreviewsType::RESOURCE_LOADING_HINTS));
}

// Test when resource loading hints are enabled.
TEST_F(PreviewsOptimizationGuideTest,
       ProcessHintsWhitelistForResourceLoadingHintsPopulatedCorrectly) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(
      optimization_guide::proto::RESOURCE_LOADING);
  // Add a second optimization to ensure that the applicable optimizations are
  // still whitelisted.
  optimization_guide::proto::Optimization* optimization2 =
      hint1->add_whitelisted_optimizations();
  optimization2->set_optimization_type(
      optimization_guide::proto::TYPE_UNSPECIFIED);
  // Add a second hint.
  optimization_guide::proto::Hint* hint2 = config.add_hints();
  hint2->set_key("twitter.com");
  hint2->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization3 =
      hint2->add_whitelisted_optimizations();
  optimization3->set_optimization_type(
      optimization_guide::proto::RESOURCE_LOADING);
  ProcessHints(config, "2.0.0");

  RunUntilIdle();

  // Twitter and Facebook should be whitelisted but not Google.
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.twitter.com/example")),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(
      guide()->IsWhitelisted(*CreateRequestWithURL(GURL("https://google.com")),
                             PreviewsType::RESOURCE_LOADING_HINTS));

  EXPECT_TRUE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.facebook.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_TRUE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.facebook.com/example.html"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_TRUE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.twitter.com/example"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://google.com"), PreviewsType::RESOURCE_LOADING_HINTS));
}

// Test when both NoScript and resource loading hints are enabled.
TEST_F(
    PreviewsOptimizationGuideTest,
    ProcessHintsWhitelistForNoScriptAndResourceLoadingHintsPopulatedCorrectly) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  // Add a second optimization to ensure that the applicable optimizations are
  // still whitelisted.
  optimization_guide::proto::Optimization* optimization2 =
      hint1->add_whitelisted_optimizations();
  optimization2->set_optimization_type(
      optimization_guide::proto::TYPE_UNSPECIFIED);
  // Add a second hint.
  optimization_guide::proto::Hint* hint2 = config.add_hints();
  hint2->set_key("twitter.com");
  hint2->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization3 =
      hint2->add_whitelisted_optimizations();
  optimization3->set_optimization_type(
      optimization_guide::proto::RESOURCE_LOADING);
  ProcessHints(config, "2.0.0");

  RunUntilIdle();

  // Twitter and Facebook should be whitelisted but not Google.
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::NOSCRIPT));
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com/example.html")),
      PreviewsType::NOSCRIPT));
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.twitter.com/example")),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(
      guide()->IsWhitelisted(*CreateRequestWithURL(GURL("https://google.com")),
                             PreviewsType::RESOURCE_LOADING_HINTS));

  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.facebook.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_TRUE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.twitter.com/example"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_TRUE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.twitter.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://google.com"), PreviewsType::RESOURCE_LOADING_HINTS));
}

// This is a helper function for testing the experiment flags on the config for
// the optimization guide. It creates a test config with a hint containing
// multiple optimizations. The optimization under test will be marked with an
// experiment name if one is provided in |experiment_name|. It will then be
// tested to see if it's enabled, the expectation found in |expect_enabled|.
void PreviewsOptimizationGuideTest::DoExperimentFlagTest(
    base::Optional<std::string> experiment_name,
    bool expect_enabled) {
  optimization_guide::proto::Configuration config;

  // Create a hint with two optimizations. One may be marked experimental
  // depending on test configuration. The other is never marked experimental.
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  // NOSCRIPT is the optimization under test and may be marked experimental.
  if (experiment_name.has_value()) {
    optimization1->set_experiment_name(experiment_name.value());
  }
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  // RESOURCE_LOADING is never marked experimental.
  optimization_guide::proto::Optimization* optimization2 =
      hint1->add_whitelisted_optimizations();
  optimization2->set_optimization_type(
      optimization_guide::proto::RESOURCE_LOADING);

  // Add a second, non-experimental hint.
  optimization_guide::proto::Hint* hint2 = config.add_hints();
  hint2->set_key("twitter.com");
  hint2->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization3 =
      hint2->add_whitelisted_optimizations();
  optimization3->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  ProcessHints(config, "2.0.0");

  RunUntilIdle();

  // Check to ensure the optimization under test (facebook noscript) is either
  // enabled or disabled, depending on what the caller told us to expect.
  EXPECT_EQ(expect_enabled,
            guide()->IsWhitelisted(
                *CreateRequestWithURL(GURL("https://m.facebook.com")),
                PreviewsType::NOSCRIPT));

  // RESOURCE_LOADING_HINTS for facebook should always be enabled.
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::RESOURCE_LOADING_HINTS));
  // Twitter's NOSCRIPT should always be enabled; RESOURCE_LOADING_HINTS is not
  // configured and should be disabled.
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.twitter.com/example")),
      PreviewsType::NOSCRIPT));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.twitter.com/example"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  // Google (which is not configured at all) should always have both NOSCRIPT
  // and RESOURCE_LOADING_HINTS disabled.
  EXPECT_FALSE(
      guide()->IsWhitelisted(*CreateRequestWithURL(GURL("https://google.com")),
                             PreviewsType::NOSCRIPT));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://google.com"), PreviewsType::RESOURCE_LOADING_HINTS));
}

TEST_F(PreviewsOptimizationGuideTest,
       HandlesExperimentalFlagWithNoExperimentFlaggedOrEnabled) {
  // With the optimization NOT flagged as experimental and no experiment
  // enabled, the optimization should be enabled.
  base::test::ScopedFeatureList scoped_list;
  scoped_list.InitAndDisableFeature(features::kOptimizationHintsExperiments);
  DoExperimentFlagTest(base::nullopt, true);
}

TEST_F(PreviewsOptimizationGuideTest,
       HandlesExperimentalFlagWithEmptyExperimentName) {
  // Empty experiment names should be equivalent to no experiment flag set.
  base::test::ScopedFeatureList scoped_list;
  scoped_list.InitAndDisableFeature(features::kOptimizationHintsExperiments);
  DoExperimentFlagTest("", true);
}

TEST_F(PreviewsOptimizationGuideTest,
       HandlesExperimentalFlagWithExperimentConfiguredAndNotRunning) {
  // With the optimization flagged as experimental and no experiment
  // enabled, the optimization should be disabled.
  base::test::ScopedFeatureList scoped_list;
  scoped_list.InitAndDisableFeature(features::kOptimizationHintsExperiments);
  DoExperimentFlagTest("foo_experiment", false);
}

TEST_F(PreviewsOptimizationGuideTest,
       HandlesExperimentalFlagWithExperimentConfiguredAndSameOneRunning) {
  // With the optimization flagged as experimental and an experiment with that
  // name running, the optimization should be enabled.
  base::test::ScopedFeatureList scoped_list;
  scoped_list.InitAndEnableFeatureWithParameters(
      features::kOptimizationHintsExperiments,
      {{"experiment_name", "foo_experiment"}});
  DoExperimentFlagTest("foo_experiment", true);
}

TEST_F(PreviewsOptimizationGuideTest,
       HandlesExperimentalFlagWithExperimentConfiguredAndDifferentOneRunning) {
  // With the optimization flagged as experimental and a *different* experiment
  // enabled, the optimization should be disabled.
  base::test::ScopedFeatureList scoped_list;
  scoped_list.InitAndEnableFeatureWithParameters(
      features::kOptimizationHintsExperiments,
      {{"experiment_name", "bar_experiment"}});
  DoExperimentFlagTest("foo_experiment", false);
}

TEST_F(PreviewsOptimizationGuideTest, EnsureExperimentsDisabledByDefault) {
  // Mark an optimization as experiment, and ensure it's disabled even though we
  // don't explicitly enable or disable the feature as part of the test. This
  // ensures the experiments feature is disabled by default.
  DoExperimentFlagTest("foo_experiment", false);
}

TEST_F(PreviewsOptimizationGuideTest, ProcessHintsUnsupportedKeyRepIsIgnored) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint = config.add_hints();
  hint->set_key("facebook.com");
  hint->set_key_representation(
      optimization_guide::proto::REPRESENTATION_UNSPECIFIED);
  optimization_guide::proto::Optimization* optimization =
      hint->add_whitelisted_optimizations();
  optimization->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  ProcessHints(config, "2.0.0");

  RunUntilIdle();

  std::unique_ptr<net::URLRequest> request =
      CreateRequestWithURL(GURL("https://m.facebook.com"));
  EXPECT_FALSE(guide()->IsWhitelisted(*request, PreviewsType::NOSCRIPT));
}

TEST_F(PreviewsOptimizationGuideTest,
       ProcessHintsUnsupportedOptimizationIsIgnored) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint = config.add_hints();
  hint->set_key("facebook.com");
  hint->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization =
      hint->add_whitelisted_optimizations();
  optimization->set_optimization_type(
      optimization_guide::proto::TYPE_UNSPECIFIED);
  ProcessHints(config, "2.0.0");

  RunUntilIdle();

  std::unique_ptr<net::URLRequest> request =
      CreateRequestWithURL(GURL("https://m.facebook.com"));
  EXPECT_FALSE(guide()->IsWhitelisted(*request, PreviewsType::NOSCRIPT));
}

TEST_F(PreviewsOptimizationGuideTest, ProcessHintsWithExistingSentinel) {
  base::HistogramTester histogram_tester;

  // Create valid config.
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);

  // Create sentinel file for version 2.0.0.
  const base::FilePath sentinel_path =
      temp_dir().Append(FILE_PATH_LITERAL("previews_config_sentinel.txt"));
  base::WriteFile(sentinel_path, "2.0.0", 5);

  // Verify config not processed for version 2.0.0 (same as sentinel).
  ProcessHints(config, "2.0.0");
  RunUntilIdle();
  EXPECT_FALSE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::NOSCRIPT));
  EXPECT_TRUE(base::PathExists(sentinel_path));
  histogram_tester.ExpectUniqueSample("Previews.ProcessHintsResult",
                                      2 /* FAILED_FINISH_PROCESSING */, 1);

  // Now verify config is processed for different version and sentinel cleared.
  ProcessHints(config, "3.0.0");
  RunUntilIdle();
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::NOSCRIPT));
  EXPECT_FALSE(base::PathExists(sentinel_path));
  histogram_tester.ExpectBucketCount("Previews.ProcessHintsResult",
                                     1 /* PROCESSED_PREVIEWS_HINTS */, 1);
}

TEST_F(PreviewsOptimizationGuideTest, ProcessHintsWithInvalidSentinelFile) {
  base::HistogramTester histogram_tester;

  // Create valid config.
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);

  // Create sentinel file with invalid contents.
  const base::FilePath sentinel_path =
      temp_dir().Append(FILE_PATH_LITERAL("previews_config_sentinel.txt"));
  base::WriteFile(sentinel_path, "bad-2.0.0", 5);

  // Verify config not processed for existing sentinel with bad value but
  // that the existinel sentinel file is deleted.
  ProcessHints(config, "2.0.0");
  RunUntilIdle();
  EXPECT_FALSE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::NOSCRIPT));
  EXPECT_FALSE(base::PathExists(sentinel_path));
  histogram_tester.ExpectUniqueSample("Previews.ProcessHintsResult",
                                      2 /* FAILED_FINISH_PROCESSING */, 1);

  // Now verify config is processed with sentinel cleared.
  ProcessHints(config, "2.0.0");
  RunUntilIdle();
  EXPECT_TRUE(guide()->IsWhitelisted(
      *CreateRequestWithURL(GURL("https://m.facebook.com")),
      PreviewsType::NOSCRIPT));
  EXPECT_FALSE(base::PathExists(sentinel_path));
  histogram_tester.ExpectBucketCount("Previews.ProcessHintsResult",
                                     1 /* PROCESSED_PREVIEWS_HINTS */, 1);
}

TEST_F(PreviewsOptimizationGuideTest, ProcessHintConfigWithNoKeyFailsDcheck) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint = config.add_hints();
  hint->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization =
      hint->add_whitelisted_optimizations();
  optimization->set_optimization_type(optimization_guide::proto::NOSCRIPT);

  EXPECT_DCHECK_DEATH({
    ProcessHints(config, "2.0.0");
    RunUntilIdle();
  });
}

TEST_F(PreviewsOptimizationGuideTest,
       ProcessHintsConfigWithDuplicateKeysFailsDcheck) {
  optimization_guide::proto::Configuration config;
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("facebook.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  optimization_guide::proto::Hint* hint2 = config.add_hints();
  hint2->set_key("facebook.com");
  hint2->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization2 =
      hint2->add_whitelisted_optimizations();
  optimization2->set_optimization_type(optimization_guide::proto::NOSCRIPT);

  EXPECT_DCHECK_DEATH({
    ProcessHints(config, "2.0.0");
    RunUntilIdle();
  });
}

TEST_F(PreviewsOptimizationGuideTest, IsWhitelistedWithMultipleHintMatches) {
  optimization_guide::proto::Configuration config;

  // Whitelist NoScript for indoor.sports.yahoo.com:
  optimization_guide::proto::Hint* hint1 = config.add_hints();
  hint1->set_key("indoor.sports.yahoo.com");
  hint1->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization1 =
      hint1->add_whitelisted_optimizations();
  optimization1->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  optimization1->set_inflation_percent(10);

  // No optimizations for sports.yahoo.com:
  optimization_guide::proto::Hint* hint2 = config.add_hints();
  hint2->set_key("sports.yahoo.com");
  hint2->set_key_representation(optimization_guide::proto::HOST_SUFFIX);

  // Whitelist NoScript for base domain yahoo.com:
  optimization_guide::proto::Hint* hint3 = config.add_hints();
  hint3->set_key("yahoo.com");
  hint3->set_key_representation(optimization_guide::proto::HOST_SUFFIX);
  optimization_guide::proto::Optimization* optimization3 =
      hint3->add_whitelisted_optimizations();
  optimization3->set_optimization_type(optimization_guide::proto::NOSCRIPT);
  optimization3->set_inflation_percent(30);

  // No optimizations for mail.yahoo.com:
  optimization_guide::proto::Hint* hint4 = config.add_hints();
  hint4->set_key("mail.yahoo.com");
  hint4->set_key_representation(optimization_guide::proto::HOST_SUFFIX);

  ProcessHints(config, "2.0.0");
  RunUntilIdle();

  std::unique_ptr<net::URLRequest> request1 =
      CreateRequestWithURL(GURL("https://yahoo.com"));
  previews::PreviewsUserData::Create(request1.get(), 1);
  EXPECT_TRUE(guide()->IsWhitelisted(*request1, PreviewsType::NOSCRIPT));
  EXPECT_EQ(30, previews::PreviewsUserData::GetData(*request1)
                    ->data_savings_inflation_percent());

  std::unique_ptr<net::URLRequest> request2 =
      CreateRequestWithURL(GURL("https://sports.yahoo.com"));
  // Uses "sports.yahoo.com" match before "yahoo.com" match.
  EXPECT_FALSE(guide()->IsWhitelisted(*request2, PreviewsType::NOSCRIPT));

  std::unique_ptr<net::URLRequest> request3 =
      CreateRequestWithURL(GURL("https://mail.yahoo.com"));
  previews::PreviewsUserData::Create(request3.get(), 3);
  // Uses "yahoo.com" match before "mail.yahoo.com" match.
  EXPECT_TRUE(guide()->IsWhitelisted(*request3, PreviewsType::NOSCRIPT));
  EXPECT_EQ(30, previews::PreviewsUserData::GetData(*request3)
                    ->data_savings_inflation_percent());

  std::unique_ptr<net::URLRequest> request4 =
      CreateRequestWithURL(GURL("https://indoor.sports.yahoo.com"));
  previews::PreviewsUserData::Create(request4.get(), 4);
  // Uses "indoor.sports.yahoo.com" match before "sports.yahoo.com" match.
  EXPECT_TRUE(guide()->IsWhitelisted(*request4, PreviewsType::NOSCRIPT));
  EXPECT_EQ(10, previews::PreviewsUserData::GetData(*request4)
                    ->data_savings_inflation_percent());

  std::unique_ptr<net::URLRequest> request5 =
      CreateRequestWithURL(GURL("https://outdoor.sports.yahoo.com"));
  // Uses "sports.yahoo.com" match before "yahoo.com" match.
  EXPECT_FALSE(guide()->IsWhitelisted(*request5, PreviewsType::NOSCRIPT));

  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://yahoo.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.facebook.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://m.twitter.com/example"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://google.com"), PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://outdoor.sports.yahoo.com"),
      PreviewsType::RESOURCE_LOADING_HINTS));
  EXPECT_FALSE(guide()->IsHostWhitelistedAtNavigation(
      GURL("https://outdoor.sports.yahoo.com/index.html"),
      PreviewsType::RESOURCE_LOADING_HINTS));
}

TEST_F(PreviewsOptimizationGuideTest, RemoveObserverCalledAtDestruction) {
  EXPECT_FALSE(optimization_guide_service()->RemoveObserverCalled());

  ResetGuide();

  EXPECT_TRUE(optimization_guide_service()->RemoveObserverCalled());
}

}  // namespace previews
