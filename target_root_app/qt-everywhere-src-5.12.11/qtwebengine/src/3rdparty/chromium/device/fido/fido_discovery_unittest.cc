// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/fido_discovery.h"

#include <utility>

#include "base/macros.h"
#include "base/run_loop.h"
#include "base/test/scoped_task_environment.h"
#include "device/fido/fido_test_data.h"
#include "device/fido/mock_fido_device.h"
#include "device/fido/mock_fido_discovery_observer.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace device {

namespace {

using ::testing::_;
using ::testing::Return;
using ::testing::UnorderedElementsAre;

// A minimal implementation of FidoDiscovery that is no longer abstract.
class ConcreteFidoDiscovery : public FidoDiscovery {
 public:
  explicit ConcreteFidoDiscovery(FidoTransportProtocol transport)
      : FidoDiscovery(transport) {}
  ~ConcreteFidoDiscovery() override = default;

  MOCK_METHOD0(StartInternal, void());

  using FidoDiscovery::AddDevice;
  using FidoDiscovery::RemoveDevice;

  using FidoDiscovery::NotifyDiscoveryStarted;
  using FidoDiscovery::NotifyDeviceAdded;
  using FidoDiscovery::NotifyDeviceRemoved;

 private:
  DISALLOW_COPY_AND_ASSIGN(ConcreteFidoDiscovery);
};

}  // namespace

TEST(FidoDiscoveryTest, TestAddAndRemoveObserver) {
  ConcreteFidoDiscovery discovery(FidoTransportProtocol::kBluetoothLowEnergy);
  MockFidoDiscoveryObserver observer;
  EXPECT_EQ(nullptr, discovery.observer());

  discovery.set_observer(&observer);
  EXPECT_EQ(&observer, discovery.observer());

  discovery.set_observer(nullptr);
  EXPECT_EQ(nullptr, discovery.observer());
}

TEST(FidoDiscoveryTest, TestNotificationsOnSuccessfulStart) {
  ConcreteFidoDiscovery discovery(FidoTransportProtocol::kBluetoothLowEnergy);
  MockFidoDiscoveryObserver observer;
  discovery.set_observer(&observer);

  EXPECT_FALSE(discovery.is_start_requested());
  EXPECT_FALSE(discovery.is_running());

  EXPECT_CALL(discovery, StartInternal());
  discovery.Start();
  EXPECT_TRUE(discovery.is_start_requested());
  EXPECT_FALSE(discovery.is_running());
  ::testing::Mock::VerifyAndClearExpectations(&discovery);

  EXPECT_CALL(observer, DiscoveryStarted(&discovery, true));
  discovery.NotifyDiscoveryStarted(true);
  EXPECT_TRUE(discovery.is_start_requested());
  EXPECT_TRUE(discovery.is_running());
  ::testing::Mock::VerifyAndClearExpectations(&observer);
}

TEST(FidoDiscoveryTest, TestNotificationsOnFailedStart) {
  ConcreteFidoDiscovery discovery(FidoTransportProtocol::kBluetoothLowEnergy);
  MockFidoDiscoveryObserver observer;
  discovery.set_observer(&observer);

  discovery.Start();

  EXPECT_CALL(observer, DiscoveryStarted(&discovery, false));
  discovery.NotifyDiscoveryStarted(false);
  EXPECT_TRUE(discovery.is_start_requested());
  EXPECT_FALSE(discovery.is_running());
  ::testing::Mock::VerifyAndClearExpectations(&observer);
}

TEST(FidoDiscoveryTest, TestAddRemoveDevices) {
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  ConcreteFidoDiscovery discovery(FidoTransportProtocol::kBluetoothLowEnergy);
  MockFidoDiscoveryObserver observer;
  discovery.set_observer(&observer);
  discovery.Start();

  // Expect successful insertion.
  auto device0 = std::make_unique<MockFidoDevice>();
  auto* device0_raw = device0.get();
  device0->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo,
      test_data::kTestAuthenticatorGetInfoResponse);
  base::RunLoop device0_done;
  EXPECT_CALL(observer, DeviceAdded(&discovery, device0_raw))
      .WillOnce(testing::InvokeWithoutArgs(
          [&device0_done]() { device0_done.Quit(); }));
  EXPECT_CALL(*device0, GetId()).WillOnce(Return("device0"));
  EXPECT_TRUE(discovery.AddDevice(std::move(device0)));
  device0_done.Run();
  ::testing::Mock::VerifyAndClearExpectations(&observer);
  // Device should have been initialized as a CTAP device.
  EXPECT_EQ(ProtocolVersion::kCtap, device0_raw->supported_protocol());
  EXPECT_TRUE(device0_raw->device_info());

  // Expect successful insertion.
  base::RunLoop device1_done;
  auto device1 = std::make_unique<MockFidoDevice>();
  auto* device1_raw = device1.get();
  device1->ExpectCtap2CommandAndRespondWith(
      CtapRequestCommand::kAuthenticatorGetInfo, base::nullopt);
  EXPECT_CALL(observer, DeviceAdded(&discovery, device1_raw))
      .WillOnce(testing::InvokeWithoutArgs(
          [&device1_done]() { device1_done.Quit(); }));
  EXPECT_CALL(*device1, GetId()).WillOnce(Return("device1"));
  EXPECT_TRUE(discovery.AddDevice(std::move(device1)));
  device1_done.Run();
  ::testing::Mock::VerifyAndClearExpectations(&observer);
  // Device should have been initialized as a U2F device.
  EXPECT_EQ(ProtocolVersion::kU2f, device1_raw->supported_protocol());
  EXPECT_FALSE(device1_raw->device_info());

  // Inserting a device with an already present id should be prevented.
  auto device1_dup = std::make_unique<MockFidoDevice>();
  EXPECT_CALL(observer, DeviceAdded(_, _)).Times(0);
  EXPECT_CALL(*device1_dup, GetId()).WillOnce(Return("device1"));
  EXPECT_FALSE(discovery.AddDevice(std::move(device1_dup)));
  ::testing::Mock::VerifyAndClearExpectations(&observer);

  EXPECT_EQ(device0_raw, discovery.GetDevice("device0"));
  EXPECT_EQ(device1_raw, discovery.GetDevice("device1"));
  EXPECT_THAT(discovery.GetDevices(),
              UnorderedElementsAre(device0_raw, device1_raw));

  const FidoDiscovery& const_discovery = discovery;
  EXPECT_EQ(device0_raw, const_discovery.GetDevice("device0"));
  EXPECT_EQ(device1_raw, const_discovery.GetDevice("device1"));
  EXPECT_THAT(const_discovery.GetDevices(),
              UnorderedElementsAre(device0_raw, device1_raw));

  // Trying to remove a non-present device should fail.
  EXPECT_CALL(observer, DeviceRemoved(_, _)).Times(0);
  EXPECT_FALSE(discovery.RemoveDevice("device2"));
  ::testing::Mock::VerifyAndClearExpectations(&observer);

  EXPECT_CALL(observer, DeviceRemoved(&discovery, device1_raw));
  EXPECT_TRUE(discovery.RemoveDevice("device1"));
  ::testing::Mock::VerifyAndClearExpectations(&observer);

  EXPECT_CALL(observer, DeviceRemoved(&discovery, device0_raw));
  EXPECT_TRUE(discovery.RemoveDevice("device0"));
  ::testing::Mock::VerifyAndClearExpectations(&observer);
}

}  // namespace device
