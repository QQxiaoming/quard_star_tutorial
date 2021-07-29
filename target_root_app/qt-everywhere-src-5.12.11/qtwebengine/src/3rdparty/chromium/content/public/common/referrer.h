// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_REFERRER_H_
#define CONTENT_PUBLIC_COMMON_REFERRER_H_

#include "base/logging.h"
#include "content/common/content_export.h"
#include "net/url_request/url_request.h"
#include "third_party/blink/public/platform/web_referrer_policy.h"
#include "url/gurl.h"

namespace content {

// This struct holds a referrer URL, as well as the referrer policy to be
// applied to this URL. When passing around referrers that will eventually end
// up being used for URL requests, always use this struct.

struct CONTENT_EXPORT Referrer {
  // TODO(jam): convert this to hold the net enum
  Referrer(const GURL& url, blink::WebReferrerPolicy policy)
      : url(url), policy(policy) {}
  Referrer() : policy(blink::kWebReferrerPolicyDefault) {}

  GURL url;
  blink::WebReferrerPolicy policy;

  static Referrer SanitizeForRequest(const GURL& request,
                                     const Referrer& referrer);

  // Returns |initiator| origin sanitized by |policy| so that it can be used
  // when requesting |request| URL.
  //
  // Note that the URL-based sanitization (e.g. SanitizeForRequest above) cannot
  // be used for cases where the referrer URL is missing (e.g. about:blank) but
  // the initiator origin still needs to be used (e.g. when calculating the
  // value of the `Origin` header to use in a POST request).
  static url::Origin SanitizeOriginForRequest(
      const GURL& request,
      const url::Origin& initiator,
      blink::WebReferrerPolicy policy);

  static void SetReferrerForRequest(net::URLRequest* request,
                                    const Referrer& referrer);

  static net::URLRequest::ReferrerPolicy ReferrerPolicyForUrlRequest(
      blink::WebReferrerPolicy referrer_policy);

  static blink::WebReferrerPolicy NetReferrerPolicyToBlinkReferrerPolicy(
      net::URLRequest::ReferrerPolicy net_policy);

  static net::URLRequest::ReferrerPolicy GetDefaultReferrerPolicy();
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_REFERRER_H_
