// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PREVIEWS_CONTENT_ACTIVATION_LIST_H_
#define COMPONENTS_PREVIEWS_CONTENT_ACTIVATION_LIST_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/sequence_checker.h"
#include "components/optimization_guide/proto/hints.pb.h"
#include "components/previews/core/previews_experiments.h"
#include "components/previews/core/previews_user_data.h"
#include "components/url_matcher/url_matcher.h"

class GURL;

namespace previews {

// Holds the activation list of hosts extracted from the server hints
// configuration.
class ActivationList {
 public:
  // Adds |url_suffixes| to |url_matcher_| for resource loading hints
  // optimization.
  explicit ActivationList(const std::vector<std::string>& url_suffixes);
  ~ActivationList();

  // Whether |url| is whitelisted for previews type |type|. The method may
  // make the decision based only on a partial comparison (e.g., only the
  // hostname of |url|). As such, the method may return true even if |type|
  // can't be used for the previews.
  bool IsHostWhitelistedAtNavigation(const GURL& url, PreviewsType type) const;

 private:
  // The URLMatcher used to match whether a URL has any previews whitelisted
  // with it.
  std::unique_ptr<url_matcher::URLMatcher> url_matcher_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(ActivationList);
};

}  // namespace previews

#endif  // COMPONENTS_PREVIEWS_CONTENT_ACTIVATION_LIST_H_
