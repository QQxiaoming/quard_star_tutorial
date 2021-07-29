// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "device/base/features.h"
#include "device/fido/authenticator_make_credential_response.h"
#include "device/fido/authenticator_selection_criteria.h"
#include "device/fido/ctap_make_credential_request.h"
#include "device/fido/fake_fido_discovery.h"
#include "device/fido/fido_constants.h"
#include "device/fido/fido_device.h"
#include "device/fido/fido_parsing_utils.h"
#include "device/fido/fido_test_data.h"
#include "device/fido/fido_transport_protocol.h"
#include "device/fido/make_credential_request_handler.h"
#include "device/fido/mock_fido_device.h"
#include "device/fido/test_callback_receiver.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;

namespace device {

namespace {

using TestMakeCredentialRequestCallback = test::StatusAndValueCallbackReceiver<
    FidoReturnCode,
    base::Optional<AuthenticatorMakeCredentialResponse>>;

}  // namespace

class FidoMakeCredentialHandlerTest : public ::testing::Test {
 public:
  void ForgeNextHidDiscovery() {
    discovery_ = scoped_fake_discovery_factory_.ForgeNextHidDiscovery();
  }

  std::unique_ptr<MakeCredentialRequestHandler> CreateMakeCredentialHandler() {
    return CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
        AuthenticatorSelectionCriteria());
  }

  std::unique_ptr<MakeCredentialRequestHandler>
  CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
      AuthenticatorSelectionCriteria authenticator_selection_criteria) {
    ForgeNextHidDiscovery();
    PublicKeyCredentialRpEntity rp(test_data::kRelyingPartyId);
    PublicKeyCredentialUserEntity user(
        fido_parsing_utils::Materialize(test_data::kUserId));
    PublicKeyCredentialParams credential_params(
        std::vector<PublicKeyCredentialParams::CredentialInfo>(1));

    auto request_parameter = CtapMakeCredentialRequest(
        test_data::kClientDataHash, std::move(rp), std::move(user),
        std::move(credential_params));

    return std::make_unique<MakeCredentialRequestHandler>(
        nullptr,
        base::flat_set<FidoTransportProtocol>(
            {FidoTransportProtocol::kUsbHumanInterfaceDevice}),
        std::move(request_parameter),
        std::move(authenticator_selection_criteria), cb_.callback());
  }

  void InitFeatureListAndDisableCtapFlag() {
    scoped_feature_list_.InitAndDisableFeature(kNewCtap2Device);
  }

  test::FakeFidoDiscovery* discovery() const { return discovery_; }
  TestMakeCredentialRequestCallback& callback() { return cb_; }

 protected:
  base::test::ScopedFeatureList scoped_feature_list_;
  base::test::ScopedTaskEnvironment scoped_task_environment_{
      base::test::ScopedTaskEnvironment::MainThreadType::MOCK_TIME};
  test::ScopedFakeFidoDiscoveryFactory scoped_fake_discovery_factory_;
  test::FakeFidoDiscovery* discovery_;
  TestMakeCredentialRequestCallback cb_;
};

TEST_F(FidoMakeCredentialHandlerTest, TestCtap2MakeCredentialWithFlagEnabled) {
  auto request_handler = CreateMakeCredentialHandler();
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestAuthenticatorGetInfoResponse);
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorMakeCredential,
      test_data::kTestMakeCredentialResponse);

  discovery()->AddDevice(std::move(device));
  callback().WaitForCallback();
  EXPECT_EQ(FidoReturnCode::kSuccess, callback().status());
  EXPECT_TRUE(request_handler->is_complete());
}

// Test a scenario where the connected authenticator is a U2F device.
TEST_F(FidoMakeCredentialHandlerTest, TestU2fRegisterWithFlagEnabled) {
  auto request_handler = CreateMakeCredentialHandler();
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);
  device->ExpectRequestAndRespondWith(
      test_data::kU2fRegisterCommandApdu,
      test_data::kApduEncodedNoErrorRegisterResponse);

  discovery()->AddDevice(std::move(device));
  callback().WaitForCallback();
  EXPECT_EQ(FidoReturnCode::kSuccess, callback().status());
  EXPECT_TRUE(request_handler->is_complete());
}

// Test a scenario where the connected authenticator is a U2F device using a
// logic that defaults to handling U2F devices.
TEST_F(FidoMakeCredentialHandlerTest, TestU2fRegisterWithoutFlagEnabled) {
  InitFeatureListAndDisableCtapFlag();
  auto request_handler = CreateMakeCredentialHandler();
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectRequestAndRespondWith(
      test_data::kU2fRegisterCommandApdu,
      test_data::kApduEncodedNoErrorRegisterResponse);

  discovery()->AddDevice(std::move(device));
  callback().WaitForCallback();
  EXPECT_EQ(FidoReturnCode::kSuccess, callback().status());
  EXPECT_TRUE(request_handler->is_complete());
}

TEST_F(FidoMakeCredentialHandlerTest, U2fRegisterWithUserVerificationRequired) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::kAny,
              false /* require_resident_key */,
              UserVerificationRequirement::kRequired));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

TEST_F(FidoMakeCredentialHandlerTest,
       U2fRegisterWithPlatformDeviceRequirement) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::
                  kPlatform,
              false /* require_resident_key */,
              UserVerificationRequirement::kPreferred));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

TEST_F(FidoMakeCredentialHandlerTest, U2fRegisterWithResidentKeyRequirement) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::kAny,
              true /* require_resident_key */,
              UserVerificationRequirement::kPreferred));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

TEST_F(FidoMakeCredentialHandlerTest,
       UserVerificationAuthenticatorSelectionCriteria) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::kAny,
              false /* require_resident_key */,
              UserVerificationRequirement::kRequired));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestGetInfoResponseWithoutUvSupport);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

TEST_F(FidoMakeCredentialHandlerTest,
       PlatformDeviceAuthenticatorSelectionCriteria) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::
                  kPlatform,
              false /* require_resident_key */,
              UserVerificationRequirement::kRequired));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestGetInfoResponseCrossPlatformDevice);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

TEST_F(FidoMakeCredentialHandlerTest,
       ResidentKeyAuthenticatorSelectionCriteria) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::kAny,
              true /* require_resident_key */,
              UserVerificationRequirement::kPreferred));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestGetInfoResponseWithoutResidentKeySupport);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

TEST_F(FidoMakeCredentialHandlerTest,
       SatisfyAllAuthenticatorSelectionCriteria) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::
                  kPlatform,
              true /* require_resident_key */,
              UserVerificationRequirement::kRequired));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestAuthenticatorGetInfoResponse);
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorMakeCredential,
      test_data::kTestMakeCredentialResponse);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  callback().WaitForCallback();
  EXPECT_EQ(FidoReturnCode::kSuccess, callback().status());
}

TEST_F(FidoMakeCredentialHandlerTest, IncompatibleUserVerificationSetting) {
  auto request_handler =
      CreateMakeCredentialHandlerWithAuthenticatorSelectionCriteria(
          AuthenticatorSelectionCriteria(
              AuthenticatorSelectionCriteria::AuthenticatorAttachment::kAny,
              false /* require_resident_key */,
              UserVerificationRequirement::kRequired));
  discovery()->WaitForCallToStartAndSimulateSuccess();

  auto device = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(*device, GetId()).WillRepeatedly(testing::Return("device0"));
  device->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestGetInfoResponseWithoutUvSupport);

  discovery()->AddDevice(std::move(device));

  scoped_task_environment_.FastForwardUntilNoTasksRemain();
  EXPECT_FALSE(callback().was_called());
}

}  // namespace device
