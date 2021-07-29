// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "google_apis/gaia/ubertoken_fetcher.h"

#include <vector>

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/rand_util.h"
#include "base/time/time.h"
#include "google_apis/gaia/gaia_auth_fetcher.h"
#include "google_apis/gaia/gaia_constants.h"
#include "google_apis/gaia/google_service_auth_error.h"
#include "google_apis/gaia/oauth2_token_service.h"
#include "services/network/public/cpp/wrapper_shared_url_loader_factory.h"

namespace {
std::unique_ptr<GaiaAuthFetcher> CreateGaiaAuthFetcher(
    GaiaAuthConsumer* consumer,
    const std::string& source,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory) {
  return std::make_unique<GaiaAuthFetcher>(consumer, source,
                                           url_loader_factory);
}
}

const int UbertokenFetcher::kMaxRetries = 3;

UbertokenFetcher::UbertokenFetcher(
    OAuth2TokenService* token_service,
    UbertokenConsumer* consumer,
    const std::string& source,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : UbertokenFetcher(token_service,
                       consumer,
                       source,
                       url_loader_factory,
                       base::BindRepeating(CreateGaiaAuthFetcher)) {}

UbertokenFetcher::UbertokenFetcher(
    OAuth2TokenService* token_service,
    UbertokenConsumer* consumer,
    const std::string& source,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    GaiaAuthFetcherFactory factory)
    : OAuth2TokenService::Consumer("uber_token_fetcher"),
      token_service_(token_service),
      consumer_(consumer),
      source_(source),
      url_loader_factory_(url_loader_factory),
      is_bound_to_channel_id_(true),
      gaia_auth_fetcher_factory_(factory),
      retry_number_(0),
      second_access_token_request_(false) {
  DCHECK(token_service);
  DCHECK(consumer);
  DCHECK(url_loader_factory);
}

UbertokenFetcher::~UbertokenFetcher() {
}

void UbertokenFetcher::StartFetchingToken(const std::string& account_id) {
  DCHECK(!account_id.empty());
  account_id_ = account_id;
  second_access_token_request_ = false;
  RequestAccessToken();
}

void UbertokenFetcher::StartFetchingTokenWithAccessToken(
    const std::string& account_id, const std::string& access_token) {
  DCHECK(!account_id.empty());
  DCHECK(!access_token.empty());

  account_id_ = account_id;
  access_token_ = access_token;
  ExchangeTokens();
}

void UbertokenFetcher::OnUberAuthTokenSuccess(const std::string& token) {
  consumer_->OnUbertokenSuccess(token);
}

void UbertokenFetcher::OnUberAuthTokenFailure(
    const GoogleServiceAuthError& error) {
  // Retry only transient errors.
  bool should_retry =
      error.state() == GoogleServiceAuthError::CONNECTION_FAILED ||
      error.state() == GoogleServiceAuthError::SERVICE_UNAVAILABLE;
  if (should_retry) {
    if (retry_number_ < kMaxRetries) {
      // Calculate an exponential backoff with randomness of less than 1 sec.
      double backoff = base::RandDouble() + (1 << retry_number_);
      ++retry_number_;
      UMA_HISTOGRAM_ENUMERATION("Signin.UberTokenRetry",
          error.state(), GoogleServiceAuthError::NUM_STATES);
      retry_timer_.Stop();
      retry_timer_.Start(FROM_HERE,
                         base::TimeDelta::FromSecondsD(backoff),
                         this,
                         &UbertokenFetcher::ExchangeTokens);
      return;
    }
  } else {
    // The access token is invalid.  Tell the token service.
    OAuth2TokenService::ScopeSet scopes;
    scopes.insert(GaiaConstants::kOAuth1LoginScope);
    token_service_->InvalidateAccessToken(account_id_, scopes, access_token_);

    // In case the access was just stale, try one more time.
    if (!second_access_token_request_) {
      second_access_token_request_ = true;
      RequestAccessToken();
      return;
    }
  }

  UMA_HISTOGRAM_ENUMERATION("Signin.UberTokenFailure",
      error.state(), GoogleServiceAuthError::NUM_STATES);
  consumer_->OnUbertokenFailure(error);
}

void UbertokenFetcher::OnGetTokenSuccess(
    const OAuth2TokenService::Request* request,
    const std::string& access_token,
    const base::Time& expiration_time) {
  DCHECK(!access_token.empty());
  access_token_ = access_token;
  access_token_request_.reset();
  ExchangeTokens();
}

void UbertokenFetcher::OnGetTokenFailure(
    const OAuth2TokenService::Request* request,
    const GoogleServiceAuthError& error) {
  access_token_request_.reset();
  consumer_->OnUbertokenFailure(error);
}

void UbertokenFetcher::RequestAccessToken() {
  retry_number_ = 0;
  gaia_auth_fetcher_.reset();
  retry_timer_.Stop();

  OAuth2TokenService::ScopeSet scopes;
  scopes.insert(GaiaConstants::kOAuth1LoginScope);
  access_token_request_ =
      token_service_->StartRequest(account_id_, scopes, this);
}

void UbertokenFetcher::ExchangeTokens() {
  gaia_auth_fetcher_ =
      gaia_auth_fetcher_factory_.Run(this, source_, url_loader_factory_);
  gaia_auth_fetcher_->StartTokenFetchForUberAuthExchange(
      access_token_, is_bound_to_channel_id_);
}
