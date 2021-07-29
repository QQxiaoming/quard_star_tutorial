// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/previews/content/activation_list.h"

#include <set>

#include "base/metrics/histogram_macros.h"
#include "url/gurl.h"

namespace previews {

ActivationList::ActivationList(const std::vector<std::string>& url_suffixes) {
  url_matcher_ = std::make_unique<url_matcher::URLMatcher>();

  url_matcher::URLMatcherConditionSet::Vector all_conditions;
  url_matcher::URLMatcherConditionFactory* condition_factory =
      url_matcher_->condition_factory();

  int id = 0;

  for (const auto& url_suffix : url_suffixes) {
    DCHECK(!url_suffix.empty());
    DCHECK_EQ(std::string::npos, url_suffix.find("/"));

    url_matcher::URLMatcherCondition condition =
        condition_factory->CreateHostSuffixCondition(url_suffix);

    all_conditions.push_back(new url_matcher::URLMatcherConditionSet(
        id, std::set<url_matcher::URLMatcherCondition>{condition}));
    ++id;
  }
  url_matcher_->AddConditionSets(all_conditions);

  DETACH_FROM_SEQUENCE(sequence_checker_);
}

ActivationList::~ActivationList() {}

bool ActivationList::IsHostWhitelistedAtNavigation(const GURL& url,
                                                   PreviewsType type) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(PreviewsType::RESOURCE_LOADING_HINTS, type);
  DCHECK(url.is_valid());
  DCHECK(url.SchemeIsHTTPOrHTTPS());

  if (!url_matcher_)
    return false;

  const std::set<url_matcher::URLMatcherConditionSet::ID>& matches =
      url_matcher_->MatchURL(url);

  // Only consider the first match in iteration order as it takes precedence
  // if there are multiple matches.
  return (matches.begin() != matches.end());
}

}  // namespace previews