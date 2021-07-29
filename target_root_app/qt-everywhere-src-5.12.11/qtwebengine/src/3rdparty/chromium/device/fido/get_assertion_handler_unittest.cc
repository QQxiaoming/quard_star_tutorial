// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "device/base/features.h"
#include "device/fido/authenticator_get_assertion_response.h"
#include "device/fido/ctap_get_assertion_request.h"
#include "device/fido/fake_fido_discovery.h"
#include "device/fido/fido_constants.h"
#include "device/fido/fido_parsing_utils.h"
#include "device/fido/fido_test_data.h"
#include "device/fido/fido_transport_protocol.h"
#include "device/fido/get_assertion_request_handler.h"
#include "device/fido/mock_fido_device.h"
#include "device/fido/test_callback_receiver.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace device {

namespace {

using TestGetAssertionRequestCallback = test::StatusAndValueCallbackReceiver<
    FidoReturnCode,
    base::Optional<AuthenticatorGetAssertionResponse>>;

}  // namespace

class FidoGetAssertionHandlerTest : public ::testing::Test {
 public:
  void ForgeNextHidDiscovery() {
    discovery_ = scoped_fake_discovery_factory_.ForgeNextHidDiscovery();
  }

  std::unique_ptr<GetAssertionRequestHandler> CreateGetAssertionHandler() {
    CtapGetAssertionRequest request(test_data::kRelyingPartyId,
                                    test_data::kClientDataHash);
    request.SetAllowList(
        {{CredentialType::kPublicKey,
          fido_parsing_utils::Materialize(test_data::kU2fSignKeyHandle)}});

    return CreateGetAssertionHandlerWithRequest(std::move(request));
  }

  std::unique_ptr<GetAssertionRequestHandler>
  CreateGetAssertionHandlerWithRequest(CtapGetAssertionRequest request) {
    ForgeNextHidDiscovery();

    return std::make_unique<GetAssertionRequestHandler>(
        nullptr /* connector */,
        base::flat_set<FidoTransportProtocol>(
            {FidoTransportProtocol::kUsbHumanInterfaceDevice}),
        std::move(request), get_assertion_cb_.callback());
  }

  void InitFeatureListAndDisableCtapFlag() {
    scoped_feature_list_.InitAndDisableFeature(kNewCtap2Device);
  }

  test::FakeFidoDiscovery* discovery() const { return discovery_; }
  TestGetAssertionRequestCallback& get_assertion_callback() {
    return get_assertion_cb_;
  }

 protected:
  base::test::ScopedTaskEnvironment scoped_task_environment_{
      base::test::ScopedTaskEnvironment::MainThreadType::MOCK_TIME};
  base::test::ScopedFeatureList scoped_feature_list_;
  test::ScopedFakeFidoDiscoveryFactory scoped_fake_discovery_factory_;
  test::FakeFidoDiscovery* discovery_;
  TestGetAssertionRequestCallback get_assertion_cb_;
};

TEST_F(FidoGetAssertionHandlerTest, TestGetAssertionRequestOnSingleDevice) {
  auto request_handler = CreateGetAssertionHandler();
  discovery()->WaitForCallToStartAndSimulateSuccess();
  auto device = std::make_unique<MockFidoDevice>();

  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestAuthenticatorGetInfoResponse);
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetAssertion,
      test_data::kTestGetAssertionResponse);

  discovery()->AddDevice(std::move(device));
  get_assertion_callback().WaitForCallback();

  EXPECT_EQ(FidoReturnCode::kSuccess, get_assertion_callback().status());
  EXPECT_TRUE(get_assertion_callback().value());
  EXPECT_TRUE(request_handler->is_complete());
}

// Test a scenario where the connected authenticator is a U2F device.
TEST_F(FidoGetAssertionHandlerTest, TestU2fSign) {
  auto request_handler = CreateGetAssertionHandler();
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);
  device->ExpectRequestAndRespondWith(
      test_data::kU2fCheckOnlySignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);
  device->ExpectRequestAndRespondWith(
      test_data::kU2fSignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);

  discovery()->AddDevice(std::move(device));
  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_EQ(FidoReturnCode::kSuccess, get_assertion_callback().status());
  EXPECT_TRUE(get_assertion_callback().value());
  EXPECT_TRUE(request_handler->is_complete());
}

// Test a scenario where the connected authenticator is a U2F device and
// "WebAuthenticationCtap2" flag is not enabled.
TEST_F(FidoGetAssertionHandlerTest, TestU2fSignWithoutCtapFlag) {
  InitFeatureListAndDisableCtapFlag();
  auto request_handler = CreateGetAssertionHandler();
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectRequestAndRespondWith(
      test_data::kU2fCheckOnlySignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);
  device->ExpectRequestAndRespondWith(
      test_data::kU2fSignCommandApdu,
      test_data::kApduEncodedNoErrorSignResponse);

  discovery()->AddDevice(std::move(device));
  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_EQ(FidoReturnCode::kSuccess, get_assertion_callback().status());
  EXPECT_TRUE(get_assertion_callback().value());
  EXPECT_TRUE(request_handler->is_complete());
}

TEST_F(FidoGetAssertionHandlerTest, TestIncompatibleUserVerificationSetting) {
  auto request = CtapGetAssertionRequest(test_data::kRelyingPartyId,
                                         test_data::kClientDataHash);
  request.SetUserVerification(UserVerificationRequirement::kRequired);
  auto request_handler =
      CreateGetAssertionHandlerWithRequest(std::move(request));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestGetInfoResponseWithoutUvSupport);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(get_assertion_callback().was_called());
}

TEST_F(FidoGetAssertionHandlerTest,
       TestU2fSignRequestWithUserVerificationRequired) {
  auto request = CtapGetAssertionRequest(test_data::kRelyingPartyId,
                                         test_data::kClientDataHash);
  request.SetAllowList(
      {{CredentialType::kPublicKey,
        fido_parsing_utils::Materialize(test_data::kU2fSignKeyHandle)}});
  request.SetUserVerification(UserVerificationRequirement::kRequired);
  auto request_handler =
      CreateGetAssertionHandlerWithRequest(std::move(request));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(get_assertion_callback().was_called());
}

}  // namespace device
