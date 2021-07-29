// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/previews/content/previews_decider_impl.h"

#include <algorithm>
#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "base/metrics/histogram.h"
#include "base/sequenced_task_runner.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "base/time/clock.h"
#include "components/blacklist/opt_out_blacklist/opt_out_store.h"
#include "components/previews/content/previews_ui_service.h"
#include "components/previews/core/previews_experiments.h"
#include "components/previews/core/previews_switches.h"
#include "components/previews/core/previews_user_data.h"
#include "net/base/load_flags.h"
#include "net/nqe/network_quality_estimator.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "url/gurl.h"

namespace previews {

namespace {

void LogPreviewsEligibilityReason(PreviewsEligibilityReason status,
                                  PreviewsType type) {
  int32_t max_limit = static_cast<int32_t>(PreviewsEligibilityReason::LAST);
  base::LinearHistogram::FactoryGet(
      base::StringPrintf("Previews.EligibilityReason.%s",
                         GetStringNameForType(type).c_str()),
      1, max_limit, max_limit + 1,
      base::HistogramBase::kUmaTargetedHistogramFlag)
      ->Add(static_cast<int>(status));
}

bool AllowedOnReload(PreviewsType type) {
  switch (type) {
    // These types return new content on refresh.
    case PreviewsType::LITE_PAGE:
    case PreviewsType::LOFI:
    case PreviewsType::NOSCRIPT:
    case PreviewsType::RESOURCE_LOADING_HINTS:
      return true;
    // Loading these types will always be stale when refreshed.
    case PreviewsType::OFFLINE:
      return false;
    case PreviewsType::NONE:
    case PreviewsType::UNSPECIFIED:
    case PreviewsType::DEPRECATED_AMP_REDIRECTION:
    case PreviewsType::LAST:
      break;
  }
  NOTREACHED();
  return false;
}

bool IsServerWhitelistedType(PreviewsType type) {
  switch (type) {
    // These types check server whitelist, if available.
    case PreviewsType::NOSCRIPT:
    case PreviewsType::RESOURCE_LOADING_HINTS:
      return true;
    case PreviewsType::OFFLINE:
    case PreviewsType::LITE_PAGE:
    case PreviewsType::LOFI:
      return false;
    case PreviewsType::NONE:
    case PreviewsType::UNSPECIFIED:
    case PreviewsType::DEPRECATED_AMP_REDIRECTION:
    case PreviewsType::LAST:
      break;
  }
  NOTREACHED();
  return false;
}

bool IsPreviewsBlacklistIgnoredViaFlag() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kIgnorePreviewsBlacklist);
}

}  // namespace

PreviewsDeciderImpl::PreviewsDeciderImpl(
    const scoped_refptr<base::SingleThreadTaskRunner>& ui_task_runner,
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner,
    base::Clock* clock)
    : blacklist_ignored_(IsPreviewsBlacklistIgnoredViaFlag()),
      clock_(clock),
      ui_task_runner_(ui_task_runner),
      io_task_runner_(io_task_runner),
      page_id_(1u),
      weak_factory_(this) {}

PreviewsDeciderImpl::~PreviewsDeciderImpl() {}

void PreviewsDeciderImpl::Initialize(
    base::WeakPtr<PreviewsUIService> previews_ui_service,
    std::unique_ptr<blacklist::OptOutStore> previews_opt_out_store,
    std::unique_ptr<PreviewsOptimizationGuide> previews_opt_guide,
    const PreviewsIsEnabledCallback& is_enabled_callback,
    blacklist::BlacklistData::AllowedTypesAndVersions allowed_previews) {
  DCHECK(ui_task_runner_->BelongsToCurrentThread());
  is_enabled_callback_ = is_enabled_callback;
  previews_ui_service_ = previews_ui_service;
  previews_opt_guide_ = std::move(previews_opt_guide);

  // Set up the IO thread portion of |this|.
  io_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&PreviewsDeciderImpl::InitializeOnIOThread,
                     base::Unretained(this), std::move(previews_opt_out_store),
                     std::move(allowed_previews)));
}

void PreviewsDeciderImpl::OnNewBlacklistedHost(const std::string& host,
                                               base::Time time) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  ui_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&PreviewsUIService::OnNewBlacklistedHost,
                                previews_ui_service_, host, time));
}

void PreviewsDeciderImpl::OnUserBlacklistedStatusChange(bool blacklisted) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  ui_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&PreviewsUIService::OnUserBlacklistedStatusChange,
                     previews_ui_service_, blacklisted));
}

void PreviewsDeciderImpl::OnBlacklistCleared(base::Time time) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  ui_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&PreviewsUIService::OnBlacklistCleared,
                                previews_ui_service_, time));
}

void PreviewsDeciderImpl::InitializeOnIOThread(
    std::unique_ptr<blacklist::OptOutStore> previews_opt_out_store,
    blacklist::BlacklistData::AllowedTypesAndVersions allowed_previews) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  previews_black_list_.reset(
      new PreviewsBlackList(std::move(previews_opt_out_store), clock_, this,
                            std::move(allowed_previews)));
  ui_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&PreviewsUIService::SetIOData, previews_ui_service_,
                     weak_factory_.GetWeakPtr()));
}

void PreviewsDeciderImpl::SetPreviewsBlacklistForTesting(
    std::unique_ptr<PreviewsBlackList> previews_back_list) {
  previews_black_list_ = std::move(previews_back_list);
}

void PreviewsDeciderImpl::LogPreviewNavigation(const GURL& url,
                                               bool opt_out,
                                               PreviewsType type,
                                               base::Time time,
                                               uint64_t page_id) const {
  ui_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&PreviewsUIService::LogPreviewNavigation,
                     previews_ui_service_, url, type, opt_out, time, page_id));
}

void PreviewsDeciderImpl::LogPreviewDecisionMade(
    PreviewsEligibilityReason reason,
    const GURL& url,
    base::Time time,
    PreviewsType type,
    std::vector<PreviewsEligibilityReason>&& passed_reasons,
    uint64_t page_id) const {
  LogPreviewsEligibilityReason(reason, type);
  ui_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&PreviewsUIService::LogPreviewDecisionMade,
                                previews_ui_service_, reason, url, time, type,
                                std::move(passed_reasons), page_id));
}

void PreviewsDeciderImpl::AddPreviewNavigation(const GURL& url,
                                               bool opt_out,
                                               PreviewsType type,
                                               uint64_t page_id) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  base::Time time =
      previews_black_list_->AddPreviewNavigation(url, opt_out, type);
  if (opt_out) {
    last_opt_out_time_ = time;
  }
  LogPreviewNavigation(url, opt_out, type, time, page_id);
}

void PreviewsDeciderImpl::ClearBlackList(base::Time begin_time,
                                         base::Time end_time) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  previews_black_list_->ClearBlackList(begin_time, end_time);
}

void PreviewsDeciderImpl::SetIgnorePreviewsBlacklistDecision(bool ignored) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  blacklist_ignored_ = ignored;
  ui_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&PreviewsUIService::OnIgnoreBlacklistDecisionStatusChanged,
                     previews_ui_service_, blacklist_ignored_));
}

bool PreviewsDeciderImpl::ShouldAllowPreview(const net::URLRequest& request,
                                             PreviewsType type) const {
  DCHECK(type == PreviewsType::OFFLINE || type == PreviewsType::NOSCRIPT ||
         type == PreviewsType::RESOURCE_LOADING_HINTS);
  // Consumers that need to specify a blacklist or ignore flag should use
  // ShouldAllowPreviewAtECT directly.
  return ShouldAllowPreviewAtECT(request, type,
                                 params::GetECTThresholdForPreview(type),
                                 std::vector<std::string>(), false);
}

bool PreviewsDeciderImpl::ShouldAllowPreviewAtECT(
    const net::URLRequest& request,
    PreviewsType type,
    net::EffectiveConnectionType effective_connection_type_threshold,
    const std::vector<std::string>& host_blacklist_from_server,
    bool ignore_long_term_black_list_rules) const {
  if (!previews::params::ArePreviewsAllowed()) {
    return false;
  }

  if (!request.url().has_host() || !PreviewsUserData::GetData(request)) {
    // Don't capture UMA on this case, as it is not important and can happen
    // when navigating to files on disk, etc.
    return false;
  }

  std::vector<PreviewsEligibilityReason> passed_reasons;
  uint64_t page_id = PreviewsUserData::GetData(request)->page_id();
  if (is_enabled_callback_.is_null() || !previews_black_list_) {
    LogPreviewDecisionMade(PreviewsEligibilityReason::BLACKLIST_UNAVAILABLE,
                           request.url(), clock_->Now(), type,
                           std::move(passed_reasons), page_id);
    return false;
  }
  passed_reasons.push_back(PreviewsEligibilityReason::BLACKLIST_UNAVAILABLE);

  if (!is_enabled_callback_.Run(type))
    return false;

  // In the case that the user has chosen to ignore the normal blacklist rules
  // (flags or interventions-internals), a preview should still not be served
  // for 5 seconds after the last opt out. This allows "show original" to
  // function correctly as the start of that navigation will be within 5 seconds
  // (we don't yet re-evaluate on redirects, so this is sufficient).
  if (blacklist_ignored_) {
    if (clock_->Now() < last_opt_out_time_ + base::TimeDelta::FromSeconds(5)) {
      LogPreviewDecisionMade(PreviewsEligibilityReason::USER_RECENTLY_OPTED_OUT,
                             request.url(), clock_->Now(), type,
                             std::move(passed_reasons), page_id);

      return false;
    }
  } else {
    // The blacklist will disallow certain hosts for periods of time based on
    // user's opting out of the preview.
    PreviewsEligibilityReason status = previews_black_list_->IsLoadedAndAllowed(
        request.url(), type, ignore_long_term_black_list_rules,
        &passed_reasons);

    if (status != PreviewsEligibilityReason::ALLOWED) {
      if (type == PreviewsType::LITE_PAGE) {
        PreviewsUserData::GetData(request)->set_black_listed_for_lite_page(
            true);
      }
      LogPreviewDecisionMade(status, request.url(), clock_->Now(), type,
                             std::move(passed_reasons), page_id);
      return false;
    }
  }

  if (effective_connection_type_threshold !=
      net::EFFECTIVE_CONNECTION_TYPE_LAST) {
    net::NetworkQualityEstimator* network_quality_estimator =
        request.context()->network_quality_estimator();
    const net::EffectiveConnectionType observed_effective_connection_type =
        network_quality_estimator
            ? network_quality_estimator->GetEffectiveConnectionType()
            : net::EFFECTIVE_CONNECTION_TYPE_UNKNOWN;
    // Network quality estimator may sometimes return effective connection type
    // as offline when the Android APIs incorrectly return device connectivity
    // as null. See https://crbug.com/838969. So, we do not trigger previews
    // when |observed_effective_connection_type| is
    // net::EFFECTIVE_CONNECTION_TYPE_OFFLINE.
    if (observed_effective_connection_type <=
        net::EFFECTIVE_CONNECTION_TYPE_OFFLINE) {
      LogPreviewDecisionMade(
          PreviewsEligibilityReason::NETWORK_QUALITY_UNAVAILABLE, request.url(),
          clock_->Now(), type, std::move(passed_reasons), page_id);
      return false;
    }
    passed_reasons.push_back(
        PreviewsEligibilityReason::NETWORK_QUALITY_UNAVAILABLE);

    if (observed_effective_connection_type >
        effective_connection_type_threshold) {
      LogPreviewDecisionMade(PreviewsEligibilityReason::NETWORK_NOT_SLOW,
                             request.url(), clock_->Now(), type,
                             std::move(passed_reasons), page_id);
      return false;
    }
    passed_reasons.push_back(PreviewsEligibilityReason::NETWORK_NOT_SLOW);
  }

  // LOAD_VALIDATE_CACHE or LOAD_BYPASS_CACHE mean the user reloaded the page.
  // If this is a query for offline previews, reloads should be disallowed.
  if (!AllowedOnReload(type) &&
      request.load_flags() &
          (net::LOAD_VALIDATE_CACHE | net::LOAD_BYPASS_CACHE)) {
    LogPreviewDecisionMade(PreviewsEligibilityReason::RELOAD_DISALLOWED,
                           request.url(), clock_->Now(), type,
                           std::move(passed_reasons), page_id);
    return false;
  }
  passed_reasons.push_back(PreviewsEligibilityReason::RELOAD_DISALLOWED);

  // Check provided blacklist, if any. This type of blacklist was added for
  // Finch provided blacklist for Client LoFi.
  if (base::ContainsValue(host_blacklist_from_server,
                          request.url().host_piece())) {
    LogPreviewDecisionMade(
        PreviewsEligibilityReason::HOST_BLACKLISTED_BY_SERVER, request.url(),
        clock_->Now(), type, std::move(passed_reasons), page_id);
    return false;
  }
  passed_reasons.push_back(
      PreviewsEligibilityReason::HOST_BLACKLISTED_BY_SERVER);

  // Check whitelist from the server, if provided.
  if (IsServerWhitelistedType(type)) {
    if (params::IsOptimizationHintsEnabled()) {
      // Optimization hints are configured, so require whitelist match.
      PreviewsEligibilityReason status =
          IsPreviewAllowedByOptmizationHints(request, type, &passed_reasons);
      if (status != PreviewsEligibilityReason::ALLOWED) {
        LogPreviewDecisionMade(status, request.url(), clock_->Now(), type,
                               std::move(passed_reasons), page_id);
        return false;
      }
    } else if (type == PreviewsType::RESOURCE_LOADING_HINTS) {
      // RESOURCE_LOADING_HINTS optimization can be applied only when a server
      // provided whitelist is available.
      LogPreviewDecisionMade(
          PreviewsEligibilityReason::HOST_NOT_WHITELISTED_BY_SERVER,
          request.url(), clock_->Now(), type, std::move(passed_reasons),
          page_id);
      return false;
    } else {
      DCHECK_EQ(PreviewsType::NOSCRIPT, type);
      // Since server optimization guidance not configured, allow the preview
      // but with qualified eligibility reason.
      LogPreviewDecisionMade(
          PreviewsEligibilityReason::ALLOWED_WITHOUT_OPTIMIZATION_HINTS,
          request.url(), clock_->Now(), type, std::move(passed_reasons),
          page_id);
      return true;
    }
  }

  LogPreviewDecisionMade(PreviewsEligibilityReason::ALLOWED, request.url(),
                         clock_->Now(), type, std::move(passed_reasons),
                         page_id);
  return true;
}

bool PreviewsDeciderImpl::IsURLAllowedForPreview(const net::URLRequest& request,
                                                 PreviewsType type) const {
  DCHECK(PreviewsType::NOSCRIPT == type ||
         PreviewsType::RESOURCE_LOADING_HINTS == type);
  if (previews_black_list_ && !blacklist_ignored_) {
    std::vector<PreviewsEligibilityReason> passed_reasons;
    // The blacklist will disallow certain hosts for periods of time based on
    // user's opting out of the preview.
    PreviewsEligibilityReason status = previews_black_list_->IsLoadedAndAllowed(
        request.url(), type, false, &passed_reasons);
    if (status != PreviewsEligibilityReason::ALLOWED) {
      if (type == PreviewsType::LITE_PAGE) {
        PreviewsUserData::GetData(request)->set_black_listed_for_lite_page(
            true);
      }
      LogPreviewDecisionMade(status, request.url(), clock_->Now(), type,
                             std::move(passed_reasons),
                             PreviewsUserData::GetData(request)->page_id());
      return false;
    }
  }

  // Check whitelist from the server, if provided.
  if (IsServerWhitelistedType(type)) {
    if (params::IsOptimizationHintsEnabled()) {
      std::vector<PreviewsEligibilityReason> passed_reasons;
      PreviewsEligibilityReason status =
          IsPreviewAllowedByOptmizationHints(request, type, &passed_reasons);
      if (status != PreviewsEligibilityReason::ALLOWED) {
        LogPreviewDecisionMade(status, request.url(), clock_->Now(), type,
                               std::move(passed_reasons),
                               PreviewsUserData::GetData(request)->page_id());
        return false;
      }
    }
  }
  return true;
}

PreviewsEligibilityReason
PreviewsDeciderImpl::IsPreviewAllowedByOptmizationHints(
    const net::URLRequest& request,
    PreviewsType type,
    std::vector<PreviewsEligibilityReason>* passed_reasons) const {
  DCHECK(type == PreviewsType::NOSCRIPT ||
         type == PreviewsType::RESOURCE_LOADING_HINTS);

  // For NoScript, if optimization guide is not present, assume that all URLs
  // are ALLOWED.
  if (!previews_opt_guide_ && type == PreviewsType::NOSCRIPT)
    return PreviewsEligibilityReason::ALLOWED;

  // Check optimization guide whitelist.
  if (!previews_opt_guide_->IsWhitelisted(request, type)) {
    return PreviewsEligibilityReason::HOST_NOT_WHITELISTED_BY_SERVER;
  }
  passed_reasons->push_back(
      PreviewsEligibilityReason::HOST_NOT_WHITELISTED_BY_SERVER);

  return PreviewsEligibilityReason::ALLOWED;
}

uint64_t PreviewsDeciderImpl::GeneratePageId() {
  return ++page_id_;
}

}  // namespace previews
