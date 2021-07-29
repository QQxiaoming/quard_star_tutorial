// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "google_apis/gaia/ubertoken_fetcher.h"

#include <memory>

#include "base/memory/ref_counted.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread_task_runner_handle.h"
#include "google_apis/gaia/fake_oauth2_token_service.h"
#include "google_apis/gaia/gaia_constants.h"
#include "mojo/core/embedder/embedder.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/test/test_url_loader_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const char kTestAccountId[] = "test@gmail.com";

class MockUbertokenConsumer : public UbertokenConsumer {
 public:
  MockUbertokenConsumer()
      : nb_correct_token_(0),
        last_error_(GoogleServiceAuthError::AuthErrorNone()),
        nb_error_(0) {
  }
  ~MockUbertokenConsumer() override {}

  void OnUbertokenSuccess(const std::string& token) override {
    last_token_ = token;
    ++ nb_correct_token_;
  }

  void OnUbertokenFailure(const GoogleServiceAuthError& error) override {
    last_error_ = error;
    ++nb_error_;
  }

  std::string last_token_;
  int nb_correct_token_;
  GoogleServiceAuthError last_error_;
  int nb_error_;
};

}  // namespace

class UbertokenFetcherTest : public testing::Test {
 public:
  UbertokenFetcherTest()
      : scoped_task_environment_(
            base::test::ScopedTaskEnvironment::MainThreadType::UI),
        test_shared_loader_factory_(
            base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
                &url_loader_factory_)) {}

  void SetUp() override {
    mojo::core::Init();

    fetcher_ = std::make_unique<UbertokenFetcher>(&token_service_, &consumer_,
                                                  GaiaConstants::kChromeSource,
                                                  test_shared_loader_factory_);
  }

  void TearDown() override { fetcher_.reset(); }

 protected:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  FakeOAuth2TokenService token_service_;
  network::TestURLLoaderFactory url_loader_factory_;
  scoped_refptr<network::SharedURLLoaderFactory> test_shared_loader_factory_;
  MockUbertokenConsumer consumer_;
  std::unique_ptr<UbertokenFetcher> fetcher_;
};

TEST_F(UbertokenFetcherTest, Basic) {
}

TEST_F(UbertokenFetcherTest, Success) {
  fetcher_->StartFetchingToken(kTestAccountId);
  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());
  fetcher_->OnUberAuthTokenSuccess("uberToken");

  EXPECT_EQ(0, consumer_.nb_error_);
  EXPECT_EQ(1, consumer_.nb_correct_token_);
  EXPECT_EQ("uberToken", consumer_.last_token_);
}

TEST_F(UbertokenFetcherTest, NoRefreshToken) {
  fetcher_->StartFetchingToken(kTestAccountId);
  GoogleServiceAuthError error(GoogleServiceAuthError::USER_NOT_SIGNED_UP);
  fetcher_->OnGetTokenFailure(NULL, error);

  EXPECT_EQ(1, consumer_.nb_error_);
  EXPECT_EQ(0, consumer_.nb_correct_token_);
}

TEST_F(UbertokenFetcherTest, FailureToGetAccessToken) {
  fetcher_->StartFetchingToken(kTestAccountId);
  GoogleServiceAuthError error(GoogleServiceAuthError::USER_NOT_SIGNED_UP);
  fetcher_->OnGetTokenFailure(NULL, error);

  EXPECT_EQ(1, consumer_.nb_error_);
  EXPECT_EQ(0, consumer_.nb_correct_token_);
  EXPECT_EQ("", consumer_.last_token_);
}

TEST_F(UbertokenFetcherTest, TransientFailureEventualFailure) {
  fetcher_->StartFetchingToken(kTestAccountId);
  GoogleServiceAuthError error(GoogleServiceAuthError::CONNECTION_FAILED);
  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());

  for (int i = 0; i < UbertokenFetcher::kMaxRetries; ++i) {
    fetcher_->OnUberAuthTokenFailure(error);
    EXPECT_EQ(0, consumer_.nb_error_);
    EXPECT_EQ(0, consumer_.nb_correct_token_);
    EXPECT_EQ("", consumer_.last_token_);
  }

  fetcher_->OnUberAuthTokenFailure(error);
  EXPECT_EQ(1, consumer_.nb_error_);
  EXPECT_EQ(0, consumer_.nb_correct_token_);
  EXPECT_EQ("", consumer_.last_token_);
}

TEST_F(UbertokenFetcherTest, TransientFailureEventualSuccess) {
  fetcher_->StartFetchingToken(kTestAccountId);
  GoogleServiceAuthError error(GoogleServiceAuthError::CONNECTION_FAILED);
  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());

  for (int i = 0; i < UbertokenFetcher::kMaxRetries; ++i) {
    fetcher_->OnUberAuthTokenFailure(error);
    EXPECT_EQ(0, consumer_.nb_error_);
    EXPECT_EQ(0, consumer_.nb_correct_token_);
    EXPECT_EQ("", consumer_.last_token_);
  }

  fetcher_->OnUberAuthTokenSuccess("uberToken");
  EXPECT_EQ(0, consumer_.nb_error_);
  EXPECT_EQ(1, consumer_.nb_correct_token_);
  EXPECT_EQ("uberToken", consumer_.last_token_);
}

TEST_F(UbertokenFetcherTest, PermanentFailureEventualFailure) {
  fetcher_->StartFetchingToken(kTestAccountId);
  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());

  GoogleServiceAuthError error(GoogleServiceAuthError::USER_NOT_SIGNED_UP);
  fetcher_->OnUberAuthTokenFailure(error);
  EXPECT_EQ(0, consumer_.nb_error_);
  EXPECT_EQ(0, consumer_.nb_correct_token_);
  EXPECT_EQ("", consumer_.last_token_);

  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());
  fetcher_->OnUberAuthTokenFailure(error);
  EXPECT_EQ(1, consumer_.nb_error_);
  EXPECT_EQ(0, consumer_.nb_correct_token_);
  EXPECT_EQ("", consumer_.last_token_);
}

TEST_F(UbertokenFetcherTest, PermanentFailureEventualSuccess) {
  fetcher_->StartFetchingToken(kTestAccountId);
  GoogleServiceAuthError error(GoogleServiceAuthError::USER_NOT_SIGNED_UP);
  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());

  fetcher_->OnUberAuthTokenFailure(error);
  EXPECT_EQ(0, consumer_.nb_error_);
  EXPECT_EQ(0, consumer_.nb_correct_token_);
  EXPECT_EQ("", consumer_.last_token_);

  fetcher_->OnGetTokenSuccess(NULL, "accessToken", base::Time());
  fetcher_->OnUberAuthTokenSuccess("uberToken");
  EXPECT_EQ(0, consumer_.nb_error_);
  EXPECT_EQ(1, consumer_.nb_correct_token_);
  EXPECT_EQ("uberToken", consumer_.last_token_);
}
