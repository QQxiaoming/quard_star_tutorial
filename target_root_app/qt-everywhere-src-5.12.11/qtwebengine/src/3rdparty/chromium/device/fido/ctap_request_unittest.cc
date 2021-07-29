// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/ctap_empty_authenticator_request.h"
#include "device/fido/ctap_get_assertion_request.h"
#include "device/fido/ctap_make_credential_request.h"
#include "device/fido/fido_constants.h"
#include "device/fido/fido_parsing_utils.h"
#include "device/fido/fido_test_data.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace device {

// Leveraging example 4 of section 6.1 of the spec
// https://fidoalliance.org/specs/fido-v2.0-rd-20170927/fido-client-to-authenticator-protocol-v2.0-rd-20170927.html
TEST(CTAPRequestTest, TestConstructMakeCredentialRequestParam) {
  PublicKeyCredentialRpEntity rp("acme.com");
  rp.SetRpName("Acme");

  PublicKeyCredentialUserEntity user(
      fido_parsing_utils::Materialize(test_data::kUserId));
  user.SetUserName("johnpsmith@example.com")
      .SetDisplayName("John P. Smith")
      .SetIconUrl(GURL("https://pics.acme.com/00/p/aBjjjpqPb.png"));

  CtapMakeCredentialRequest make_credential_param(
      test_data::kClientDataHash, std::move(rp), std::move(user),
      PublicKeyCredentialParams({{CredentialType::kPublicKey, 7},
                                 {CredentialType::kPublicKey, 257}}));
  auto serialized_data = make_credential_param.SetResidentKeySupported(true)
                             .SetUserVerificationRequired(true)
                             .EncodeAsCBOR();
  EXPECT_THAT(serialized_data, ::testing::ElementsAreArray(
                                   test_data::kCtapMakeCredentialRequest));
}

TEST(CTAPRequestTest, TestConstructGetAssertionRequest) {
  CtapGetAssertionRequest get_assertion_req("acme.com",
                                            test_data::kClientDataHash);

  std::vector<PublicKeyCredentialDescriptor> allowed_list;
  allowed_list.push_back(PublicKeyCredentialDescriptor(
      CredentialType::kPublicKey,
      {0xf2, 0x20, 0x06, 0xde, 0x4f, 0x90, 0x5a, 0xf6, 0x8a, 0x43, 0x94,
       0x2f, 0x02, 0x4f, 0x2a, 0x5e, 0xce, 0x60, 0x3d, 0x9c, 0x6d, 0x4b,
       0x3d, 0xf8, 0xbe, 0x08, 0xed, 0x01, 0xfc, 0x44, 0x26, 0x46, 0xd0,
       0x34, 0x85, 0x8a, 0xc7, 0x5b, 0xed, 0x3f, 0xd5, 0x80, 0xbf, 0x98,
       0x08, 0xd9, 0x4f, 0xcb, 0xee, 0x82, 0xb9, 0xb2, 0xef, 0x66, 0x77,
       0xaf, 0x0a, 0xdc, 0xc3, 0x58, 0x52, 0xea, 0x6b, 0x9e}));
  allowed_list.push_back(PublicKeyCredentialDescriptor(
      CredentialType::kPublicKey,
      {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
       0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
       0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
       0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
       0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}));

  get_assertion_req.SetAllowList(std::move(allowed_list))
      .SetUserPresenceRequired(false)
      .SetUserVerification(UserVerificationRequirement::kRequired);

  auto serialized_data = get_assertion_req.EncodeAsCBOR();
  EXPECT_THAT(serialized_data,
              ::testing::ElementsAreArray(test_data::kCtapGetAssertionRequest));
}

TEST(CTAPRequestTest, TestConstructCtapAuthenticatorRequestParam) {
  static constexpr uint8_t kSerializedGetInfoCmd = 0x04;
  static constexpr uint8_t kSerializedGetNextAssertionCmd = 0x08;
  static constexpr uint8_t kSerializedResetCmd = 0x07;

  EXPECT_THAT(AuthenticatorGetInfoRequest().Serialize(),
              ::testing::ElementsAre(kSerializedGetInfoCmd));
  EXPECT_THAT(AuthenticatorGetNextAssertionRequest().Serialize(),
              ::testing::ElementsAre(kSerializedGetNextAssertionCmd));
  EXPECT_THAT(AuthenticatorResetRequest().Serialize(),
              ::testing::ElementsAre(kSerializedResetCmd));
}

TEST(CTAPRequestTest, ParseMakeCredentialRequestForVirtualCtapKey) {
  const auto request = ParseCtapMakeCredentialRequest(
      base::make_span(test_data::kCtapMakeCredentialRequest).subspan(1));
  ASSERT_TRUE(request);
  EXPECT_THAT(request->client_data_hash(),
              ::testing::ElementsAreArray(test_data::kClientDataHash));
  EXPECT_EQ(test_data::kRelyingPartyId, request->rp().rp_id());
  EXPECT_EQ("Acme", request->rp().rp_name());
  EXPECT_THAT(request->user().user_id(),
              ::testing::ElementsAreArray(test_data::kUserId));
  ASSERT_TRUE(request->user().user_name());
  EXPECT_EQ("johnpsmith@example.com", *request->user().user_name());
  ASSERT_TRUE(request->user().user_display_name());
  EXPECT_EQ("John P. Smith", *request->user().user_display_name());
  ASSERT_TRUE(request->user().user_icon_url());
  EXPECT_EQ("https://pics.acme.com/00/p/aBjjjpqPb.png",
            request->user().user_icon_url()->spec());
  ASSERT_EQ(2u, request->public_key_credential_params()
                    .public_key_credential_params()
                    .size());
  EXPECT_EQ(7, request->public_key_credential_params()
                   .public_key_credential_params()
                   .at(0)
                   .algorithm);
  EXPECT_EQ(257, request->public_key_credential_params()
                     .public_key_credential_params()
                     .at(1)
                     .algorithm);
  EXPECT_TRUE(request->user_verification_required());
  EXPECT_TRUE(request->resident_key_supported());
}

TEST(CTAPRequestTest, ParseGetAssertionRequestForVirtualCtapKey) {
  constexpr uint8_t kAllowedCredentialOne[] = {
      0xf2, 0x20, 0x06, 0xde, 0x4f, 0x90, 0x5a, 0xf6, 0x8a, 0x43, 0x94,
      0x2f, 0x02, 0x4f, 0x2a, 0x5e, 0xce, 0x60, 0x3d, 0x9c, 0x6d, 0x4b,
      0x3d, 0xf8, 0xbe, 0x08, 0xed, 0x01, 0xfc, 0x44, 0x26, 0x46, 0xd0,
      0x34, 0x85, 0x8a, 0xc7, 0x5b, 0xed, 0x3f, 0xd5, 0x80, 0xbf, 0x98,
      0x08, 0xd9, 0x4f, 0xcb, 0xee, 0x82, 0xb9, 0xb2, 0xef, 0x66, 0x77,
      0xaf, 0x0a, 0xdc, 0xc3, 0x58, 0x52, 0xea, 0x6b, 0x9e};
  constexpr uint8_t kAllowedCredentialTwo[] = {
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
      0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};

  const auto request = ParseCtapGetAssertionRequest(
      base::make_span(test_data::kCtapGetAssertionRequest).subspan(1));
  ASSERT_TRUE(request);
  EXPECT_THAT(request->client_data_hash(),
              ::testing::ElementsAreArray(test_data::kClientDataHash));
  EXPECT_EQ(test_data::kRelyingPartyId, request->rp_id());
  EXPECT_EQ(UserVerificationRequirement::kRequired,
            request->user_verification());
  EXPECT_FALSE(request->user_presence_required());
  ASSERT_TRUE(request->allow_list());
  ASSERT_EQ(2u, request->allow_list()->size());

  EXPECT_THAT(request->allow_list()->at(0).id(),
              ::testing::ElementsAreArray(kAllowedCredentialOne));
  EXPECT_THAT(request->allow_list()->at(1).id(),
              ::testing::ElementsAreArray(kAllowedCredentialTwo));
}
}  // namespace device
