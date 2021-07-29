// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/origin_policy/origin_policy.h"

#include "testing/gtest/include/gtest/gtest.h"

// Unit tests for OriginPolicy / OriginPolicyParser.
//
// These are fairly simple "smoke tests". The majority of test coverage is
// expected from wpt/origin-policy/ end-to-end tests.

TEST(OriginPolicy, Empty) {
  auto policy = blink::OriginPolicy::From("");
  ASSERT_FALSE(policy);
}

TEST(OriginPolicy, Invalid) {
  auto policy = blink::OriginPolicy::From("potato potato potato");
  ASSERT_FALSE(policy);
}

TEST(OriginPolicy, ValidButEmpty) {
  auto policy = blink::OriginPolicy::From("{}");
  ASSERT_TRUE(policy);
  ASSERT_TRUE(policy->GetContentSecurityPolicies().empty());
}

TEST(OriginPolicy, SimpleCSP) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self' 'unsafe-inline'"
      }] }
  )");
  ASSERT_TRUE(policy);
  ASSERT_EQ(policy->GetContentSecurityPolicies().size(), 1U);
  ASSERT_EQ(policy->GetContentSecurityPolicies()[0].policy,
            "script-src 'self' 'unsafe-inline'");
}

TEST(OriginPolicy, DoubleCSP) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self' 'unsafe-inline'",
          "report-only": false
        },{
          "policy": "script-src 'self' 'https://example.com/'",
          "report-only": true
      }] }
  )");
  ASSERT_TRUE(policy);
  ASSERT_EQ(policy->GetContentSecurityPolicies().size(), 2U);
  ASSERT_EQ(policy->GetContentSecurityPolicies()[0].policy,
            "script-src 'self' 'unsafe-inline'");
  ASSERT_FALSE(policy->GetContentSecurityPolicies()[0].report_only);
  ASSERT_EQ(policy->GetContentSecurityPolicies()[1].policy,
            "script-src 'self' 'https://example.com/'");
  ASSERT_TRUE(policy->GetContentSecurityPolicies()[1].report_only);
}

TEST(OriginPolicy, HalfDoubleCSP) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self' 'unsafe-inline'",
        },{
          "policies": "script-src 'self' 'https://example.com/'",
      }] }
  )");
  ASSERT_FALSE(policy);
}

TEST(OriginPolicy, CSPWithoutCSP) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "police": "script-src 'self' 'unsafe-inline'",
          "report-only": false
        }] }
  )");
  ASSERT_FALSE(policy);
}

TEST(OriginPolicy, ExtraFieldsDontBreakParsing) {
  auto policy = blink::OriginPolicy::From(R"(
      { "potatoes": "are better than kale",
        "content-security-policy": [{
          "report-only": false,
          "potatoes": "are best",
          "policy": "script-src 'self' 'unsafe-inline'"
        }],
        "other": {
          "name": "Sieglinde",
          "value": "best of potatoes"
      }}
  )");
  ASSERT_TRUE(policy);
  ASSERT_EQ(policy->GetContentSecurityPolicies().size(), 1U);
  ASSERT_EQ(policy->GetContentSecurityPolicies()[0].policy,
            "script-src 'self' 'unsafe-inline'");
}

TEST(OriginPolicy, CSPDispositionEnforce) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self'",
          "report-only": false
        }] }
  )");
  ASSERT_FALSE(policy->GetContentSecurityPolicies()[0].report_only);
}

TEST(OriginPolicy, CSPDispositionReport) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self'",
          "report-only": true
        }] }
  )");
  ASSERT_TRUE(policy->GetContentSecurityPolicies()[0].report_only);
}

TEST(OriginPolicy, CSPDispositionInvalid) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self'",
          "report-only": "potato"
        }] }
  )");
  ASSERT_FALSE(policy->GetContentSecurityPolicies()[0].report_only);
}

TEST(OriginPolicy, CSPDispositionAbsent) {
  auto policy = blink::OriginPolicy::From(R"(
      { "content-security-policy": [{
          "policy": "script-src 'self'"
        }] }
  )");
  ASSERT_FALSE(policy->GetContentSecurityPolicies()[0].report_only);
}
