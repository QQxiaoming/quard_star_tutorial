// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/run_loop.h"
#include "base/test/bind_test_util.h"
#include "build/build_config.h"
#if defined(OS_CHROMEOS)
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/network/geolocation_handler.h"
#endif
#include "mojo/public/cpp/bindings/interface_ptr.h"
#include "services/device/device_service_test_base.h"
#include "services/device/geolocation/geolocation_provider_impl.h"
#include "services/device/geolocation/network_location_request.h"
#include "services/device/public/mojom/constants.mojom.h"
#include "services/device/public/mojom/geolocation.mojom.h"
#include "services/device/public/mojom/geolocation_config.mojom.h"
#include "services/device/public/mojom/geolocation_context.mojom.h"
#include "services/device/public/mojom/geolocation_control.mojom.h"

namespace device {

namespace {

void CheckBoolReturnValue(base::OnceClosure quit_closure,
                          bool expect,
                          bool result) {
  EXPECT_EQ(expect, result);
  std::move(quit_closure).Run();
}

class GeolocationServiceUnitTest : public DeviceServiceTestBase {
 public:
  GeolocationServiceUnitTest() = default;
  ~GeolocationServiceUnitTest() override = default;

 protected:
  void SetUp() override {
    DeviceServiceTestBase::SetUp();

#if defined(OS_CHROMEOS)
    chromeos::DBusThreadManager::Initialize();
    chromeos::NetworkHandler::Initialize();
#endif

    connector()->BindInterface(mojom::kServiceName, &geolocation_control_);
    geolocation_control_->UserDidOptIntoLocationServices();

    connector()->BindInterface(mojom::kServiceName, &geolocation_context_);
    geolocation_context_->BindGeolocation(MakeRequest(&geolocation_));
  }

  void TearDown() override {
    DeviceServiceTestBase::TearDown();

#if defined(OS_CHROMEOS)
    chromeos::NetworkHandler::Shutdown();
    chromeos::DBusThreadManager::Shutdown();
#endif

    // Let the GeolocationImpl destruct earlier than GeolocationProviderImpl to
    // make sure the base::CallbackList<> member in GeolocationProviderImpl is
    // empty.
    geolocation_.reset();
    GeolocationProviderImpl::GetInstance()
        ->clear_user_did_opt_into_location_services_for_testing();
    base::RunLoop().RunUntilIdle();
  }

  void BindGeolocationConfig() {
    connector()->BindInterface(mojom::kServiceName, &geolocation_config_);
  }

  mojom::GeolocationControlPtr geolocation_control_;
  mojom::GeolocationContextPtr geolocation_context_;
  mojom::GeolocationPtr geolocation_;
  mojom::GeolocationConfigPtr geolocation_config_;

  DISALLOW_COPY_AND_ASSIGN(GeolocationServiceUnitTest);
};

#if defined(OS_CHROMEOS) || defined(OS_ANDROID)
// ChromeOS fails to perform network geolocation when zero wifi networks are
// detected in a scan: https://crbug.com/767300.
#else
TEST_F(GeolocationServiceUnitTest, UrlWithApiKey) {
  base::RunLoop loop;
  test_url_loader_factory_.SetInterceptor(base::BindLambdaForTesting(
      [&loop](const network::ResourceRequest& request) {
        // Verify the full URL including a fake Google API key.
        std::string expected_url =
            "https://www.googleapis.com/geolocation/v1/geolocate?key=";
        expected_url.append(kTestGeolocationApiKey);

        if (request.url == expected_url)
          loop.Quit();
      }));

  geolocation_->SetHighAccuracy(true);
  loop.Run();
}
#endif

TEST_F(GeolocationServiceUnitTest, GeolocationConfig) {
  BindGeolocationConfig();
  {
    base::RunLoop run_loop;
    geolocation_config_->IsHighAccuracyLocationBeingCaptured(
        base::BindOnce(&CheckBoolReturnValue, run_loop.QuitClosure(), false));
    run_loop.Run();
  }

  geolocation_->SetHighAccuracy(true);
  {
    base::RunLoop run_loop;
    geolocation_config_->IsHighAccuracyLocationBeingCaptured(
        base::BindOnce(&CheckBoolReturnValue, run_loop.QuitClosure(), true));
    run_loop.Run();
  }
}

}  // namespace

}  // namespace device
