// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/get_assertion_task.h"

#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "crypto/ec_private_key.h"
#include "device/base/features.h"
#include "device/fido/authenticator_get_assertion_response.h"
#include "device/fido/ctap_get_assertion_request.h"
#include "device/fido/device_response_converter.h"
#include "device/fido/fido_constants.h"
#include "device/fido/fido_parsing_utils.h"
#include "device/fido/fido_test_data.h"
#include "device/fido/mock_fido_device.h"
#include "device/fido/test_callback_receiver.h"
#include "device/fido/virtual_ctap2_device.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;

namespace device {
namespace {

using TestGetAssertionTaskCallbackReceiver =
    ::device::test::StatusAndValueCallbackReceiver<
        CtapDeviceResponseCode,
        base::Optional<AuthenticatorGetAssertionResponse>>;

class FidoGetAssertionTaskTest : public testing::Test {
 public:
  FidoGetAssertionTaskTest() { scoped_feature_list_.emplace(); }

  TestGetAssertionTaskCallbackReceiver& get_assertion_callback_receiver() {
    return cb_;
  }

  void RemoveCtapFlag() {
    scoped_feature_list_.emplace();
    scoped_feature_list_->InitAndDisableFeature(kNewCtap2Device);
  }

 private:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  base::Optional<base::test::ScopedFeatureList> scoped_feature_list_;
  TestGetAssertionTaskCallbackReceiver cb_;
};

TEST_F(FidoGetAssertionTaskTest, TestGetAssertionSuccess) {
  auto device = MockFidoDevice::MakeCtap();
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetAssertion,
      test_data::kTestGetAssertionResponse);

  CtapGetAssertionRequest request_param(test_data::kRelyingPartyId,
                                        test_data::kClientDataHash);
  request_param.SetAllowList({{CredentialType::kPublicKey,
                               fido_parsing_utils::Materialize(
                                   test_data::kTestGetAssertionCredentialId)}});

  auto task = std::make_unique<GetAssertionTask>(
      device.get(), std::move(request_param),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kSuccess,
            get_assertion_callback_receiver().status());
  EXPECT_TRUE(get_assertion_callback_receiver().value());
}

TEST_F(FidoGetAssertionTaskTest, TestU2fSignSuccess) {
  auto device = MockFidoDevice::MakeU2f();
  device->ExpectRequestAndRespondWith(
      test_data::kU2fCheckOnlySignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);
  device->ExpectRequestAndRespondWith(
      test_data::kU2fSignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);

  CtapGetAssertionRequest request_param(test_data::kRelyingPartyId,
                                        test_data::kClientDataHash);
  request_param.SetAllowList(
      {{CredentialType::kPublicKey,
        fido_parsing_utils::Materialize(test_data::kU2fSignKeyHandle)}});

  auto task = std::make_unique<GetAssertionTask>(
      device.get(), std::move(request_param),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kSuccess,
            get_assertion_callback_receiver().status());
  EXPECT_TRUE(get_assertion_callback_receiver().value());
}

TEST_F(FidoGetAssertionTaskTest, TestSignSuccessWithFake) {
  auto private_key = crypto::ECPrivateKey::Create();
  std::string public_key;
  private_key->ExportRawPublicKey(&public_key);
  auto hash = fido_parsing_utils::CreateSHA256Hash(public_key);
  std::vector<uint8_t> key_handle(hash.begin(), hash.end());
  CtapGetAssertionRequest request_param(test_data::kRelyingPartyId,
                                        test_data::kClientDataHash);
  request_param.SetAllowList({{CredentialType::kPublicKey, key_handle}});

  auto device = std::make_unique<VirtualCtap2Device>();
  device->mutable_state()->registrations.emplace(
      key_handle,
      VirtualFidoDevice::RegistrationData(
          std::move(private_key),
          fido_parsing_utils::CreateSHA256Hash(test_data::kRelyingPartyId),
          42 /* counter */));
  test::TestCallbackReceiver<> done;
  device->DiscoverSupportedProtocolAndDeviceInfo(done.callback());
  done.WaitForCallback();

  auto task = std::make_unique<GetAssertionTask>(
      device.get(), std::move(request_param),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kSuccess,
            get_assertion_callback_receiver().status());

  // Just a sanity check, we don't verify the actual signature.
  ASSERT_GE(32u + 1u + 4u + 8u,  // Minimal ECDSA signature is 8 bytes
            get_assertion_callback_receiver()
                .value()
                ->auth_data()
                .SerializeToByteArray()
                .size());
  EXPECT_EQ(0x01,
            get_assertion_callback_receiver()
                .value()
                ->auth_data()
                .SerializeToByteArray()[32]);  // UP flag
  // Counter is incremented for every sign request.
  EXPECT_EQ(43, get_assertion_callback_receiver()
                    .value()
                    ->auth_data()
                    .SerializeToByteArray()[36]);  // counter
}

TEST_F(FidoGetAssertionTaskTest, TestU2fSignWithoutFlag) {
  RemoveCtapFlag();
  auto device = MockFidoDevice::MakeU2f();
  device->ExpectRequestAndRespondWith(
      test_data::kU2fCheckOnlySignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);
  device->ExpectRequestAndRespondWith(
      test_data::kU2fSignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);

  CtapGetAssertionRequest request_param(test_data::kRelyingPartyId,
                                        test_data::kClientDataHash);
  request_param.SetAllowList(
      {{CredentialType::kPublicKey,
        fido_parsing_utils::Materialize(test_data::kU2fSignKeyHandle)}});

  auto task = std::make_unique<GetAssertionTask>(
      device.get(), std::move(request_param),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kSuccess,
            get_assertion_callback_receiver().status());
  EXPECT_TRUE(get_assertion_callback_receiver().value());
}

// Tests a scenario where the authenticator responds with credential ID that
// is not included in the allowed list.
TEST_F(FidoGetAssertionTaskTest, TestGetAssertionInvalidCredential) {
  auto device = MockFidoDevice::MakeCtap();
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetAssertion,
      test_data::kTestGetAssertionResponse);

  auto task = std::make_unique<GetAssertionTask>(
      device.get(),
      CtapGetAssertionRequest(test_data::kRelyingPartyId,
                              test_data::kClientDataHash),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kCtap2ErrOther,
            get_assertion_callback_receiver().status());
  EXPECT_FALSE(get_assertion_callback_receiver().value());
}

// Tests a scenario where authenticator responds without user entity in its
// response but client is expecting a resident key credential.
TEST_F(FidoGetAssertionTaskTest, TestGetAsserionIncorrectUserEntity) {
  auto device = MockFidoDevice::MakeCtap();
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetAssertion,
      test_data::kTestGetAssertionResponse);

  auto task = std::make_unique<GetAssertionTask>(
      device.get(),
      CtapGetAssertionRequest(test_data::kRelyingPartyId,
                              test_data::kClientDataHash),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kCtap2ErrOther,
            get_assertion_callback_receiver().status());
  EXPECT_FALSE(get_assertion_callback_receiver().value());
}

TEST_F(FidoGetAssertionTaskTest, TestGetAsserionIncorrectRpIdHash) {
  auto device = MockFidoDevice::MakeCtap();
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetAssertion,
      test_data::kTestGetAssertionResponseWithIncorrectRpIdHash);

  auto task = std::make_unique<GetAssertionTask>(
      device.get(),
      CtapGetAssertionRequest(test_data::kRelyingPartyId,
                              test_data::kClientDataHash),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kCtap2ErrOther,
            get_assertion_callback_receiver().status());
  EXPECT_FALSE(get_assertion_callback_receiver().value());
}

TEST_F(FidoGetAssertionTaskTest, TestIncorrectGetAssertionResponse) {
  auto device = MockFidoDevice::MakeCtap();
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetAssertion, base::nullopt);

  auto task = std::make_unique<GetAssertionTask>(
      device.get(),
      CtapGetAssertionRequest(test_data::kRelyingPartyId,
                              test_data::kClientDataHash),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kCtap2ErrOther,
            get_assertion_callback_receiver().status());
  EXPECT_FALSE(get_assertion_callback_receiver().value());
}

TEST_F(FidoGetAssertionTaskTest, TestU2fSignRequestWithEmptyAllowedList) {
  auto request = CtapGetAssertionRequest(test_data::kRelyingPartyId,
                                         test_data::kClientDataHash);

  auto device = MockFidoDevice::MakeU2f();
  device->ExpectRequestAndRespondWith(
      test_data::kU2fFakeRegisterCommand,
      test_data::kApduEncodedNoErrorSignResponse);

  auto task = std::make_unique<GetAssertionTask>(
      device.get(), std::move(request),
      get_assertion_callback_receiver().callback());

  get_assertion_callback_receiver().WaitForCallback();
  EXPECT_EQ(CtapDeviceResponseCode::kCtap2ErrNoCredentials,
            get_assertion_callback_receiver().status());
  EXPECT_FALSE(get_assertion_callback_receiver().value());
}

}  // namespace
}  // namespace device
