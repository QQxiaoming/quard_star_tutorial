// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/public/cpp/network_quality_tracker.h"

#include "base/macros.h"
#include "base/run_loop.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread.h"
#include "base/threading/thread_checker.h"
#include "net/nqe/effective_connection_type.h"
#include "net/nqe/network_quality_estimator.h"
#include "services/network/network_service.h"
#include "services/network/public/mojom/network_change_manager.mojom.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace network {

namespace {

class TestEffectiveConnectionTypeObserver
    : public NetworkQualityTracker::EffectiveConnectionTypeObserver {
 public:
  explicit TestEffectiveConnectionTypeObserver(NetworkQualityTracker* tracker)
      : num_notifications_(0),
        tracker_(tracker),
        run_loop_(std::make_unique<base::RunLoop>()),
        effective_connection_type_(net::EFFECTIVE_CONNECTION_TYPE_UNKNOWN) {
    tracker_->AddEffectiveConnectionTypeObserver(this);
  }

  ~TestEffectiveConnectionTypeObserver() override {
    tracker_->RemoveEffectiveConnectionTypeObserver(this);
  }

  // Helper to synchronously get effective connection type from
  // NetworkQualityTracker.
  net::EffectiveConnectionType GetEffectiveConnectionTypeSync() {
    return tracker_->GetEffectiveConnectionType();
  }

  // NetworkConnectionObserver implementation:
  void OnEffectiveConnectionTypeChanged(
      net::EffectiveConnectionType type) override {
    EXPECT_EQ(type, GetEffectiveConnectionTypeSync());
    num_notifications_++;
    effective_connection_type_ = type;
    run_loop_->Quit();
  }

  size_t num_notifications() const { return num_notifications_; }

  void WaitForNotification() {
    run_loop_->Run();
    run_loop_.reset(new base::RunLoop());
  }

  net::EffectiveConnectionType effective_connection_type() const {
    return effective_connection_type_;
  }

 private:
  static void GetEffectiveConnectionTypeCallback(
      base::RunLoop* run_loop,
      net::EffectiveConnectionType* out,
      net::EffectiveConnectionType type) {
    *out = type;
    run_loop->Quit();
  }

  size_t num_notifications_;
  NetworkQualityTracker* tracker_;
  std::unique_ptr<base::RunLoop> run_loop_;
  net::EffectiveConnectionType effective_connection_type_;

  DISALLOW_COPY_AND_ASSIGN(TestEffectiveConnectionTypeObserver);
};

}  // namespace

class NetworkQualityTrackerTest : public testing::Test {
 public:
  NetworkQualityTrackerTest() {
    network::mojom::NetworkServicePtr network_service_ptr;
    network::mojom::NetworkServiceRequest network_service_request =
        mojo::MakeRequest(&network_service_ptr);
    network_service_ =
        network::NetworkService::Create(std::move(network_service_request),
                                        /*netlog=*/nullptr);
    tracker_ = std::make_unique<NetworkQualityTracker>(
        base::BindRepeating(&NetworkQualityTrackerTest::mojom_network_service,
                            base::Unretained(this)));
    observer_ =
        std::make_unique<TestEffectiveConnectionTypeObserver>(tracker_.get());
  }

  ~NetworkQualityTrackerTest() override {}

  network::NetworkService* network_service() { return network_service_.get(); }

  NetworkQualityTracker* network_quality_tracker() { return tracker_.get(); }

  TestEffectiveConnectionTypeObserver* effective_connection_type_observer() {
    return observer_.get();
  }

  // Simulates a connection type change and broadcast it to observers.
  void SimulateEffectiveConnectionTypeChange(
      net::EffectiveConnectionType type) {
    network_service()
        ->network_quality_estimator()
        ->SimulateNetworkQualityChangeForTesting(type);
  }

 private:
  network::mojom::NetworkService* mojom_network_service() const {
    return network_service_.get();
  }

  base::test::ScopedTaskEnvironment scoped_task_environment_;
  std::unique_ptr<network::NetworkService> network_service_;
  std::unique_ptr<NetworkQualityTracker> tracker_;
  std::unique_ptr<TestEffectiveConnectionTypeObserver> observer_;

  DISALLOW_COPY_AND_ASSIGN(NetworkQualityTrackerTest);
};

TEST_F(NetworkQualityTrackerTest, ObserverNotified) {
  EXPECT_EQ(net::EFFECTIVE_CONNECTION_TYPE_UNKNOWN,
            effective_connection_type_observer()->effective_connection_type());

  SimulateEffectiveConnectionTypeChange(net::EFFECTIVE_CONNECTION_TYPE_3G);

  effective_connection_type_observer()->WaitForNotification();
  EXPECT_EQ(net::EFFECTIVE_CONNECTION_TYPE_3G,
            effective_connection_type_observer()->effective_connection_type());
  // Typical RTT and downlink values when effective connection type is 3G. Taken
  // from net::NetworkQualityEstimatorParams.
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(450),
            network_quality_tracker()->GetHttpRTT());
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(400),
            network_quality_tracker()->GetTransportRTT());
  EXPECT_EQ(400, network_quality_tracker()->GetDownstreamThroughputKbps());
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(1u, effective_connection_type_observer()->num_notifications());
}

TEST_F(NetworkQualityTrackerTest, UnregisteredObserverNotNotified) {
  auto network_quality_observer2 =
      std::make_unique<TestEffectiveConnectionTypeObserver>(
          network_quality_tracker());

  // Simulate a network quality change.
  SimulateEffectiveConnectionTypeChange(net::EFFECTIVE_CONNECTION_TYPE_3G);

  network_quality_observer2->WaitForNotification();
  EXPECT_EQ(net::EFFECTIVE_CONNECTION_TYPE_3G,
            network_quality_observer2->effective_connection_type());
  effective_connection_type_observer()->WaitForNotification();
  EXPECT_EQ(net::EFFECTIVE_CONNECTION_TYPE_3G,
            effective_connection_type_observer()->effective_connection_type());
  // Typical RTT and downlink values when effective connection type is 3G. Taken
  // from net::NetworkQualityEstimatorParams.
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(450),
            network_quality_tracker()->GetHttpRTT());
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(400),
            network_quality_tracker()->GetTransportRTT());
  EXPECT_EQ(400, network_quality_tracker()->GetDownstreamThroughputKbps());
  base::RunLoop().RunUntilIdle();

  network_quality_observer2.reset();

  // Simulate an another network quality change.
  SimulateEffectiveConnectionTypeChange(net::EFFECTIVE_CONNECTION_TYPE_2G);
  effective_connection_type_observer()->WaitForNotification();
  EXPECT_EQ(net::EFFECTIVE_CONNECTION_TYPE_2G,
            effective_connection_type_observer()->effective_connection_type());
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(1800),
            network_quality_tracker()->GetHttpRTT());
  EXPECT_EQ(base::TimeDelta::FromMilliseconds(1500),
            network_quality_tracker()->GetTransportRTT());
  EXPECT_EQ(75, network_quality_tracker()->GetDownstreamThroughputKbps());
  EXPECT_EQ(2u, effective_connection_type_observer()->num_notifications());
}

}  // namespace network
