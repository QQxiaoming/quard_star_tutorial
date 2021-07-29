// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/previews/core/previews_black_list.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/histogram.h"
#include "base/optional.h"
#include "base/strings/stringprintf.h"
#include "base/time/clock.h"
#include "components/previews/core/previews_experiments.h"
#include "url/gurl.h"

namespace previews {

namespace {

PreviewsEligibilityReason BlacklistReasonToPreviewsReason(
    blacklist::BlacklistReason reason) {
  switch (reason) {
    case blacklist::BlacklistReason::kBlacklistNotLoaded:
      return PreviewsEligibilityReason::BLACKLIST_DATA_NOT_LOADED;
    case blacklist::BlacklistReason::kUserOptedOutInSession:
      return PreviewsEligibilityReason::USER_RECENTLY_OPTED_OUT;
    case blacklist::BlacklistReason::kUserOptedOutInGeneral:
      return PreviewsEligibilityReason::USER_BLACKLISTED;
    case blacklist::BlacklistReason::kUserOptedOutOfHost:
      return PreviewsEligibilityReason::HOST_BLACKLISTED;
    case blacklist::BlacklistReason::kUserOptedOutOfType:
      NOTREACHED() << "Previews does not support type-base blacklisting";
      return PreviewsEligibilityReason::ALLOWED;
    case blacklist::BlacklistReason::kAllowed:
      return PreviewsEligibilityReason::ALLOWED;
  }
}

}

PreviewsBlackList::PreviewsBlackList(
    std::unique_ptr<blacklist::OptOutStore> opt_out_store,
    base::Clock* clock,
    blacklist::OptOutBlacklistDelegate* blacklist_delegate,
    blacklist::BlacklistData::AllowedTypesAndVersions allowed_types)
    : blacklist::OptOutBlacklist(std::move(opt_out_store),
                                 clock,
                                 blacklist_delegate),
      allowed_types_(std::move(allowed_types)) {
  DCHECK(blacklist_delegate);
  Init();
}

bool PreviewsBlackList::ShouldUseSessionPolicy(base::TimeDelta* duration,
                                               size_t* history,
                                               int* threshold) const {
  *duration = params::SingleOptOutDuration();
  *history = 1;
  *threshold = 1;
  return true;
}

bool PreviewsBlackList::ShouldUsePersistentPolicy(base::TimeDelta* duration,
                                                  size_t* history,
                                                  int* threshold) const {
  *history = params::MaxStoredHistoryLengthForHostIndifferentBlackList();
  *threshold = params::HostIndifferentBlackListOptOutThreshold();
  *duration = params::HostIndifferentBlackListPerHostDuration();
  return true;
}
bool PreviewsBlackList::ShouldUseHostPolicy(base::TimeDelta* duration,
                                            size_t* history,
                                            int* threshold,
                                            size_t* max_hosts) const {
  *max_hosts = params::MaxInMemoryHostsInBlackList();
  *history = params::MaxStoredHistoryLengthForPerHostBlackList();
  *threshold = params::PerHostBlackListOptOutThreshold();
  *duration = params::PerHostBlackListDuration();
  return true;
}
bool PreviewsBlackList::ShouldUseTypePolicy(base::TimeDelta* duration,
                                            size_t* history,
                                            int* threshold) const {
  return false;
}

blacklist::BlacklistData::AllowedTypesAndVersions
PreviewsBlackList::GetAllowedTypes() const {
  return allowed_types_;
}

PreviewsBlackList::~PreviewsBlackList() {}

base::Time PreviewsBlackList::AddPreviewNavigation(const GURL& url,
                                                   bool opt_out,
                                                   PreviewsType type) {
  DCHECK(url.has_host());

  base::BooleanHistogram::FactoryGet(
      base::StringPrintf("Previews.OptOut.UserOptedOut.%s",
                         GetStringNameForType(type).c_str()),
      base::HistogramBase::kUmaTargetedHistogramFlag)
      ->Add(opt_out);

  return blacklist::OptOutBlacklist::AddEntry(url.host(), opt_out,
                                              static_cast<int>(type));
}

PreviewsEligibilityReason PreviewsBlackList::IsLoadedAndAllowed(
    const GURL& url,
    PreviewsType type,
    bool ignore_long_term_black_list_rules,
    std::vector<PreviewsEligibilityReason>* passed_reasons) const {
  DCHECK(url.has_host());

  std::vector<blacklist::BlacklistReason> passed_blacklist_reasons;
  blacklist::BlacklistReason reason =
      blacklist::OptOutBlacklist::IsLoadedAndAllowed(
          url.host(), static_cast<int>(type), ignore_long_term_black_list_rules,
          &passed_blacklist_reasons);
  for (auto passed_reason : passed_blacklist_reasons) {
    passed_reasons->push_back(BlacklistReasonToPreviewsReason(passed_reason));
  }

  return BlacklistReasonToPreviewsReason(reason);
}

}  // namespace previews
