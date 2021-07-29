// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/cryptauth/cryptauth_client_impl.h"

#include "base/command_line.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/test/gtest_util.h"
#include "base/test/null_task_runner.h"
#include "base/test/scoped_task_environment.h"
#include "components/cryptauth/cryptauth_api_call_flow.h"
#include "components/cryptauth/proto/cryptauth_api.pb.h"
#include "components/cryptauth/switches.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/test_url_fetcher_factory.h"
#include "net/url_request/url_request_test_util.h"
#include "services/identity/public/cpp/identity_test_environment.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

using testing::_;
using testing::DoAll;
using testing::Return;
using testing::SaveArg;
using testing::StrictMock;

namespace cryptauth {

namespace {

const char kTestGoogleApisUrl[] = "https://www.testgoogleapis.com";
const char kAccessToken[] = "access_token";
const char kEmail[] = "test@gmail.com";
const char kPublicKey1[] = "public_key1";
const char kPublicKey2[] = "public_key2";
const char kBluetoothAddress1[] = "AA:AA:AA:AA:AA:AA";
const char kBluetoothAddress2[] = "BB:BB:BB:BB:BB:BB";

// Values for the DeviceClassifier field.
const int kDeviceOsVersionCode = 100;
const int kDeviceSoftwareVersionCode = 200;
const char kDeviceSoftwarePackage[] = "cryptauth_client_unittest";
const DeviceType kDeviceType = CHROME;

// Mock CryptAuthApiCallFlow, which handles the HTTP requests to CryptAuth.
class MockCryptAuthApiCallFlow : public CryptAuthApiCallFlow {
 public:
  MockCryptAuthApiCallFlow() : CryptAuthApiCallFlow() {
    SetPartialNetworkTrafficAnnotation(PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  }
  virtual ~MockCryptAuthApiCallFlow() {}

  MOCK_METHOD6(Start,
               void(const GURL&,
                    net::URLRequestContextGetter* context,
                    const std::string& access_token,
                    const std::string& serialized_request,
                    const ResultCallback& result_callback,
                    const ErrorCallback& error_callback));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockCryptAuthApiCallFlow);
};

// Callback that should never be invoked.
template <class T>
void NotCalled(T type) {
  EXPECT_TRUE(false);
}

// Callback that should never be invoked.
template <class T>
void NotCalledConstRef(const T& type) {
  EXPECT_TRUE(false);
}

// Callback that saves the result returned by CryptAuthClient.
template <class T>
void SaveResult(T* out, T result) {
  *out = result;
}

// Callback that saves the result returned by CryptAuthClient.
template <class T>
void SaveResultConstRef(T* out, const T& result) {
  *out = result;
}

}  // namespace

class CryptAuthClientTest : public testing::Test {
 protected:
  CryptAuthClientTest()
      : api_call_flow_(new StrictMock<MockCryptAuthApiCallFlow>()),
        url_request_context_(
            new net::TestURLRequestContextGetter(new base::NullTaskRunner())),
        serialized_request_(std::string()) {}

  void SetUp() override {
    base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(
        switches::kCryptAuthHTTPHost, kTestGoogleApisUrl);

    DeviceClassifier device_classifier;
    device_classifier.set_device_os_version_code(kDeviceOsVersionCode);
    device_classifier.set_device_software_version_code(
        kDeviceSoftwareVersionCode);
    device_classifier.set_device_software_package(kDeviceSoftwarePackage);
    device_classifier.set_device_type(kDeviceType);

    identity_test_environment_.MakePrimaryAccountAvailable(kEmail);

    client_.reset(
        new CryptAuthClientImpl(base::WrapUnique(api_call_flow_),
                                identity_test_environment_.identity_manager(),
                                url_request_context_, device_classifier));
  }

  // Sets up an expectation and captures a CryptAuth API request to
  // |request_url|.
  void ExpectRequest(const std::string& request_url) {
    GURL url(request_url);
    EXPECT_CALL(*api_call_flow_,
                Start(url, url_request_context_.get(), kAccessToken, _, _, _))
        .WillOnce(DoAll(SaveArg<3>(&serialized_request_),
                        SaveArg<4>(&flow_result_callback_),
                        SaveArg<5>(&flow_error_callback_)));
  }

  // Returns |response_proto| as the result to the current API request.
  // ExpectResult() must have been called first.
  void FinishApiCallFlow(const google::protobuf::MessageLite* response_proto) {
    flow_result_callback_.Run(response_proto->SerializeAsString());
  }

  // Ends the current API request with |error|. ExpectResult() must have been
  // called first.
  void FailApiCallFlow(NetworkRequestError error) {
    flow_error_callback_.Run(error);
  }

 protected:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  identity::IdentityTestEnvironment identity_test_environment_;
  // Owned by |client_|.
  StrictMock<MockCryptAuthApiCallFlow>* api_call_flow_;

  scoped_refptr<net::URLRequestContextGetter> url_request_context_;
  std::unique_ptr<CryptAuthClient> client_;

  std::string serialized_request_;
  CryptAuthApiCallFlow::ResultCallback flow_result_callback_;
  CryptAuthApiCallFlow::ErrorCallback flow_error_callback_;
};

TEST_F(CryptAuthClientTest, GetMyDevicesSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "getmydevices?alt=proto");

  GetMyDevicesResponse result_proto;
  GetMyDevicesRequest request_proto;
  request_proto.set_allow_stale_read(true);
  client_->GetMyDevices(
      request_proto,
      base::Bind(&SaveResultConstRef<GetMyDevicesResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>),
      PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  GetMyDevicesRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));
  EXPECT_TRUE(expected_request.allow_stale_read());

  // Return two devices, one unlock key and one unlockable device.
  {
    GetMyDevicesResponse response_proto;
    response_proto.add_devices();
    response_proto.mutable_devices(0)->set_public_key(kPublicKey1);
    response_proto.mutable_devices(0)->set_unlock_key(true);
    response_proto.mutable_devices(0)
        ->set_bluetooth_address(kBluetoothAddress1);
    response_proto.add_devices();
    response_proto.mutable_devices(1)->set_public_key(kPublicKey2);
    response_proto.mutable_devices(1)->set_unlockable(true);
    FinishApiCallFlow(&response_proto);
  }

  // Check that the result received in callback is the same as the response.
  ASSERT_EQ(2, result_proto.devices_size());
  EXPECT_EQ(kPublicKey1, result_proto.devices(0).public_key());
  EXPECT_TRUE(result_proto.devices(0).unlock_key());
  EXPECT_EQ(kBluetoothAddress1, result_proto.devices(0).bluetooth_address());
  EXPECT_EQ(kPublicKey2, result_proto.devices(1).public_key());
  EXPECT_TRUE(result_proto.devices(1).unlockable());
}

TEST_F(CryptAuthClientTest, GetMyDevicesFailure) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "getmydevices?alt=proto");

  NetworkRequestError error;
  client_->GetMyDevices(GetMyDevicesRequest(),
                        base::Bind(&NotCalledConstRef<GetMyDevicesResponse>),
                        base::Bind(&SaveResult<NetworkRequestError>, &error),
                        PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  FailApiCallFlow(NetworkRequestError::kInternalServerError);
  EXPECT_EQ(NetworkRequestError::kInternalServerError, error);
}

TEST_F(CryptAuthClientTest, FindEligibleUnlockDevicesSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "findeligibleunlockdevices?alt=proto");

  FindEligibleUnlockDevicesResponse result_proto;
  FindEligibleUnlockDevicesRequest request_proto;
  request_proto.set_callback_bluetooth_address(kBluetoothAddress2);
  client_->FindEligibleUnlockDevices(
      request_proto,
      base::Bind(&SaveResultConstRef<FindEligibleUnlockDevicesResponse>,
                 &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>));
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  FindEligibleUnlockDevicesRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));
  EXPECT_EQ(kBluetoothAddress2, expected_request.callback_bluetooth_address());

  // Return a response proto with one eligible and one ineligible device.
  FindEligibleUnlockDevicesResponse response_proto;
  response_proto.add_eligible_devices();
  response_proto.mutable_eligible_devices(0)->set_public_key(kPublicKey1);

  const cryptauth::IneligibilityReason kIneligibilityReason =
      cryptauth::IneligibilityReason::UNKNOWN_INELIGIBILITY_REASON;
  response_proto.add_ineligible_devices();
  response_proto.mutable_ineligible_devices(0)
      ->mutable_device()
      ->set_public_key(kPublicKey2);
  response_proto.mutable_ineligible_devices(0)
      ->add_reasons(kIneligibilityReason);
  FinishApiCallFlow(&response_proto);

  // Check that the result received in callback is the same as the response.
  ASSERT_EQ(1, result_proto.eligible_devices_size());
  EXPECT_EQ(kPublicKey1, result_proto.eligible_devices(0).public_key());
  ASSERT_EQ(1, result_proto.ineligible_devices_size());
  EXPECT_EQ(kPublicKey2,
            result_proto.ineligible_devices(0).device().public_key());
  ASSERT_EQ(1, result_proto.ineligible_devices(0).reasons_size());
  EXPECT_EQ(kIneligibilityReason,
            result_proto.ineligible_devices(0).reasons(0));
}

TEST_F(CryptAuthClientTest, FindEligibleUnlockDevicesFailure) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "findeligibleunlockdevices?alt=proto");

  NetworkRequestError error;
  FindEligibleUnlockDevicesRequest request_proto;
  request_proto.set_callback_bluetooth_address(kBluetoothAddress1);
  client_->FindEligibleUnlockDevices(
      request_proto,
      base::Bind(&NotCalledConstRef<FindEligibleUnlockDevicesResponse>),
      base::Bind(&SaveResult<NetworkRequestError>, &error));
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  FailApiCallFlow(NetworkRequestError::kAuthenticationError);
  EXPECT_EQ(NetworkRequestError::kAuthenticationError, error);
}

TEST_F(CryptAuthClientTest, FindEligibleForPromotionSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "findeligibleforpromotion?alt=proto");

  FindEligibleForPromotionResponse result_proto;
  client_->FindEligibleForPromotion(
      FindEligibleForPromotionRequest(),
      base::Bind(&SaveResultConstRef<FindEligibleForPromotionResponse>,
                 &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>));
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  FindEligibleForPromotionRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));

  FindEligibleForPromotionResponse response_proto;
  FinishApiCallFlow(&response_proto);
}

TEST_F(CryptAuthClientTest, SendDeviceSyncTickleSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "senddevicesynctickle?alt=proto");

  SendDeviceSyncTickleResponse result_proto;
  client_->SendDeviceSyncTickle(
      SendDeviceSyncTickleRequest(),
      base::Bind(&SaveResultConstRef<SendDeviceSyncTickleResponse>,
                 &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>),
      PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  SendDeviceSyncTickleRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));

  SendDeviceSyncTickleResponse response_proto;
  FinishApiCallFlow(&response_proto);
}

TEST_F(CryptAuthClientTest, ToggleEasyUnlockSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "toggleeasyunlock?alt=proto");

  ToggleEasyUnlockResponse result_proto;
  ToggleEasyUnlockRequest request_proto;
  request_proto.set_enable(true);
  request_proto.set_apply_to_all(false);
  request_proto.set_public_key(kPublicKey1);
  client_->ToggleEasyUnlock(
      request_proto,
      base::Bind(&SaveResultConstRef<ToggleEasyUnlockResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>));
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  ToggleEasyUnlockRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));
  EXPECT_TRUE(expected_request.enable());
  EXPECT_EQ(kPublicKey1, expected_request.public_key());
  EXPECT_FALSE(expected_request.apply_to_all());

  ToggleEasyUnlockResponse response_proto;
  FinishApiCallFlow(&response_proto);
}

TEST_F(CryptAuthClientTest, SetupEnrollmentSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/enrollment/"
      "setup?alt=proto");

  std::string kApplicationId = "mkaes";
  std::vector<std::string> supported_protocols;
  supported_protocols.push_back("gcmV1");
  supported_protocols.push_back("testProtocol");

  SetupEnrollmentResponse result_proto;
  SetupEnrollmentRequest request_proto;
  request_proto.set_application_id(kApplicationId);
  request_proto.add_types("gcmV1");
  request_proto.add_types("testProtocol");
  client_->SetupEnrollment(
      request_proto,
      base::Bind(&SaveResultConstRef<SetupEnrollmentResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>));
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  SetupEnrollmentRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));
  EXPECT_EQ(kApplicationId, expected_request.application_id());
  ASSERT_EQ(2, expected_request.types_size());
  EXPECT_EQ("gcmV1", expected_request.types(0));
  EXPECT_EQ("testProtocol", expected_request.types(1));

  // Return a fake enrollment session.
  {
    SetupEnrollmentResponse response_proto;
    response_proto.set_status("OK");
    response_proto.add_infos();
    response_proto.mutable_infos(0)->set_type("gcmV1");
    response_proto.mutable_infos(0)->set_enrollment_session_id("session_id");
    response_proto.mutable_infos(0)->set_server_ephemeral_key("ephemeral_key");
    FinishApiCallFlow(&response_proto);
  }

  // Check that the returned proto is the same as the one just created.
  EXPECT_EQ("OK", result_proto.status());
  ASSERT_EQ(1, result_proto.infos_size());
  EXPECT_EQ("gcmV1", result_proto.infos(0).type());
  EXPECT_EQ("session_id", result_proto.infos(0).enrollment_session_id());
  EXPECT_EQ("ephemeral_key", result_proto.infos(0).server_ephemeral_key());
}

TEST_F(CryptAuthClientTest, FinishEnrollmentSuccess) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/enrollment/"
      "finish?alt=proto");

  const char kEnrollmentSessionId[] = "enrollment_session_id";
  const char kEnrollmentMessage[] = "enrollment_message";
  const char kDeviceEphemeralKey[] = "device_ephermal_key";
  FinishEnrollmentResponse result_proto;
  FinishEnrollmentRequest request_proto;
  request_proto.set_enrollment_session_id(kEnrollmentSessionId);
  request_proto.set_enrollment_message(kEnrollmentMessage);
  request_proto.set_device_ephemeral_key(kDeviceEphemeralKey);
  client_->FinishEnrollment(
      request_proto,
      base::Bind(&SaveResultConstRef<FinishEnrollmentResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>));
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  FinishEnrollmentRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));
  EXPECT_EQ(kEnrollmentSessionId, expected_request.enrollment_session_id());
  EXPECT_EQ(kEnrollmentMessage, expected_request.enrollment_message());
  EXPECT_EQ(kDeviceEphemeralKey, expected_request.device_ephemeral_key());

  {
    FinishEnrollmentResponse response_proto;
    response_proto.set_status("OK");
    FinishApiCallFlow(&response_proto);
  }
  EXPECT_EQ("OK", result_proto.status());
}

TEST_F(CryptAuthClientTest, FetchAccessTokenFailure) {
  NetworkRequestError error;
  client_->GetMyDevices(GetMyDevicesRequest(),
                        base::Bind(&NotCalledConstRef<GetMyDevicesResponse>),
                        base::Bind(&SaveResult<NetworkRequestError>, &error),
                        PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithError(
          GoogleServiceAuthError(GoogleServiceAuthError::SERVICE_UNAVAILABLE));

  EXPECT_EQ(NetworkRequestError::kAuthenticationError, error);
}

TEST_F(CryptAuthClientTest, ParseResponseProtoFailure) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "getmydevices?alt=proto");

  NetworkRequestError error;
  client_->GetMyDevices(GetMyDevicesRequest(),
                        base::Bind(&NotCalledConstRef<GetMyDevicesResponse>),
                        base::Bind(&SaveResult<NetworkRequestError>, &error),
                        PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  flow_result_callback_.Run("Not a valid serialized response message.");
  EXPECT_EQ(NetworkRequestError::kResponseMalformed, error);
}

TEST_F(CryptAuthClientTest,
       MakeSecondRequestBeforeFirstRequestSucceeds) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "getmydevices?alt=proto");

  // Make first request.
  GetMyDevicesResponse result_proto;
  client_->GetMyDevices(
      GetMyDevicesRequest(),
      base::Bind(&SaveResultConstRef<GetMyDevicesResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>),
      PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  // With request pending, make second request.
  {
    NetworkRequestError error;
    EXPECT_DCHECK_DEATH(client_->FindEligibleUnlockDevices(
        FindEligibleUnlockDevicesRequest(),
        base::Bind(&NotCalledConstRef<FindEligibleUnlockDevicesResponse>),
        base::Bind(&SaveResult<NetworkRequestError>, &error)));
  }

  // Complete first request.
  {
    GetMyDevicesResponse response_proto;
    response_proto.add_devices();
    response_proto.mutable_devices(0)->set_public_key(kPublicKey1);
    FinishApiCallFlow(&response_proto);
  }

  ASSERT_EQ(1, result_proto.devices_size());
  EXPECT_EQ(kPublicKey1, result_proto.devices(0).public_key());
}

TEST_F(CryptAuthClientTest,
       MakeSecondRequestAfterFirstRequestSucceeds) {
  // Make first request successfully.
  {
    ExpectRequest(
        "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
        "getmydevices?alt=proto");
    GetMyDevicesResponse result_proto;
    client_->GetMyDevices(
        GetMyDevicesRequest(),
        base::Bind(&SaveResultConstRef<GetMyDevicesResponse>, &result_proto),
        base::Bind(&NotCalled<NetworkRequestError>),
        PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
    identity_test_environment_
        .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
            kAccessToken, base::Time::Max());

    GetMyDevicesResponse response_proto;
    response_proto.add_devices();
    response_proto.mutable_devices(0)->set_public_key(kPublicKey1);
    FinishApiCallFlow(&response_proto);
    ASSERT_EQ(1, result_proto.devices_size());
    EXPECT_EQ(kPublicKey1, result_proto.devices(0).public_key());
  }

  // Second request fails.
  {
    NetworkRequestError error;
    EXPECT_DCHECK_DEATH(client_->FindEligibleUnlockDevices(
        FindEligibleUnlockDevicesRequest(),
        base::Bind(&NotCalledConstRef<FindEligibleUnlockDevicesResponse>),
        base::Bind(&SaveResult<NetworkRequestError>, &error)));
  }
}

TEST_F(CryptAuthClientTest, DeviceClassifierIsSet) {
  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "getmydevices?alt=proto");

  GetMyDevicesResponse result_proto;
  GetMyDevicesRequest request_proto;
  request_proto.set_allow_stale_read(true);
  client_->GetMyDevices(
      request_proto,
      base::Bind(&SaveResultConstRef<GetMyDevicesResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>),
      PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());
  GetMyDevicesRequest expected_request;
  EXPECT_TRUE(expected_request.ParseFromString(serialized_request_));

  const DeviceClassifier& device_classifier =
      expected_request.device_classifier();
  EXPECT_EQ(kDeviceOsVersionCode, device_classifier.device_os_version_code());
  EXPECT_EQ(kDeviceSoftwareVersionCode,
            device_classifier.device_software_version_code());
  EXPECT_EQ(kDeviceSoftwarePackage,
            device_classifier.device_software_package());
  EXPECT_EQ(kDeviceType, device_classifier.device_type());
}

TEST_F(CryptAuthClientTest, GetAccessTokenUsed) {
  EXPECT_TRUE(client_->GetAccessTokenUsed().empty());

  ExpectRequest(
      "https://www.testgoogleapis.com/cryptauth/v1/deviceSync/"
      "getmydevices?alt=proto");

  GetMyDevicesResponse result_proto;
  GetMyDevicesRequest request_proto;
  request_proto.set_allow_stale_read(true);
  client_->GetMyDevices(
      request_proto,
      base::Bind(&SaveResultConstRef<GetMyDevicesResponse>, &result_proto),
      base::Bind(&NotCalled<NetworkRequestError>),
      PARTIAL_TRAFFIC_ANNOTATION_FOR_TESTS);
  identity_test_environment_
      .WaitForAccessTokenRequestIfNecessaryAndRespondWithToken(
          kAccessToken, base::Time::Max());

  EXPECT_EQ(kAccessToken, client_->GetAccessTokenUsed());
}

}  // namespace cryptauth
