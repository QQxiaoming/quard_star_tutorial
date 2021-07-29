// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PREVIEWS_CONTENT_PREVIEWS_HINTS_H_
#define COMPONENTS_PREVIEWS_CONTENT_PREVIEWS_HINTS_H_

#include <map>
#include <memory>
#include <set>

#include "base/macros.h"
#include "base/sequence_checker.h"
#include "components/optimization_guide/proto/hints.pb.h"
#include "components/previews/content/activation_list.h"
#include "components/previews/content/previews_hints.h"
#include "components/previews/core/previews_user_data.h"
#include "components/url_matcher/url_matcher.h"

class GURL;

namespace optimization_guide {
struct ComponentInfo;
}

namespace previews {

// Holds previews hints extracted from the configuration.
class PreviewsHints {
 public:
  ~PreviewsHints();

  // Creates a Hints instance from the provided configuration.
  static std::unique_ptr<PreviewsHints> CreateFromConfig(
      const optimization_guide::proto::Configuration& config,
      const optimization_guide::ComponentInfo& info);

  // Whether the URL is whitelisted for the given previews type. If so,
  // |out_inflation_percent| will be populated if meta data available for it.
  bool IsWhitelisted(const GURL& url,
                     PreviewsType type,
                     int* out_inflation_percent);

  // Whether |url| is whitelisted for previews type |type|. The method may
  // make the decision based only on a partial comparison (e.g., only the
  // hostname of |url|). As such, the method may return true even if |type|
  // can't be used for the previews.
  bool IsHostWhitelistedAtNavigation(const GURL& url,
                                     previews::PreviewsType type) const;

 private:
  PreviewsHints();

  // The URLMatcher used to match whether a URL has any hints associated with
  // it.
  url_matcher::URLMatcher url_matcher_;

  // Holds the activation list of hosts extracted from the server hints
  // configuration.
  std::unique_ptr<ActivationList> activation_list_;

  // A map from the condition set ID to associated whitelist Optimization
  // details.
  std::map<url_matcher::URLMatcherConditionSet::ID,
           std::set<std::pair<PreviewsType, int>>>
      whitelist_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(PreviewsHints);
};

}  // namespace previews

#endif  // COMPONENTS_PREVIEWS_CONTENT_PREVIEWS_HINTS_H_