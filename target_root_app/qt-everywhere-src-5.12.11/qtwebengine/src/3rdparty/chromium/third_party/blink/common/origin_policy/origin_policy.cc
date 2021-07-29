// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/origin_policy/origin_policy.h"

#include "third_party/blink/common/origin_policy/origin_policy_parser.h"

namespace blink {

OriginPolicy::~OriginPolicy() {}

std::unique_ptr<OriginPolicy> OriginPolicy::From(
    base::StringPiece manifest_text) {
  return OriginPolicyParser::Parse(manifest_text);
}

OriginPolicy::OriginPolicy() {}

}  // namespace blink
