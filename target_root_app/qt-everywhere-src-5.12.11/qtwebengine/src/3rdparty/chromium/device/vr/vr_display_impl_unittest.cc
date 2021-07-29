// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/vr/vr_display_impl.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "device/vr/public/mojom/vr_service.mojom.h"
#include "device/vr/test/fake_vr_device.h"
#include "device/vr/test/fake_vr_service_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace device {

class VRDisplayImplTest : public testing::Test {
 public:
  VRDisplayImplTest() {}
  ~VRDisplayImplTest() override {}
  void onDisplaySynced() {}
  void onPresentComplete(
      device::mojom::XRPresentationConnectionPtr connection,
      mojom::XRSessionControllerPtr immersive_session_controller) {
    is_request_presenting_success_ = connection ? true : false;

    immersive_session_controller_ = std::move(immersive_session_controller);
  }

 protected:
  void SetUp() override {
    device_ = std::make_unique<FakeVRDevice>(1);
    device_->SetPose(mojom::VRPose::New());
    mojom::VRServiceClientPtr proxy;
    client_ = std::make_unique<FakeVRServiceClient>(mojo::MakeRequest(&proxy));
  }

  std::unique_ptr<VRDisplayImpl> MakeDisplay(
      mojom::XRSessionControllerPtr* controller) {
    mojom::VRMagicWindowProviderPtr session;
    auto display = std::make_unique<VRDisplayImpl>(
        device(), mojo::MakeRequest(&session), mojo::MakeRequest(controller));
    static_cast<mojom::XRSessionController*>(display.get())
        ->SetFrameDataRestricted(true);
    return display;
  }

  void RequestSession(VRDisplayImpl* display_impl) {
    device_->RequestSession(
        mojom::XRDeviceRuntimeSessionOptionsPtr(),
        base::BindOnce(&VRDisplayImplTest::onPresentComplete,
                       base::Unretained(this)));
  }

  void ExitPresent() {
    device_->StopSession();
    immersive_session_controller_ = nullptr;
  }

  bool presenting() { return device_->IsPresenting(); }
  VRDeviceBase* device() { return device_.get(); }
  FakeVRServiceClient* client() { return client_.get(); }

  base::MessageLoop message_loop_;
  bool is_request_presenting_success_ = false;
  std::unique_ptr<FakeVRDevice> device_;
  std::unique_ptr<FakeVRServiceClient> client_;
  mojom::XRSessionControllerPtr immersive_session_controller_;

  DISALLOW_COPY_AND_ASSIGN(VRDisplayImplTest);
};

TEST_F(VRDisplayImplTest, DevicePresentationIsolation) {
  mojom::XRSessionControllerPtr controller1;
  std::unique_ptr<VRDisplayImpl> display_1 = MakeDisplay(&controller1);
  static_cast<mojom::XRSessionController*>(display_1.get())
      ->SetFrameDataRestricted(false);
  mojom::XRSessionControllerPtr controller2;
  std::unique_ptr<VRDisplayImpl> display_2 = MakeDisplay(&controller2);
  static_cast<mojom::XRSessionController*>(display_2.get())
      ->SetFrameDataRestricted(false);

  // When not presenting either service should be able to access the device.
  EXPECT_FALSE(device()->HasExclusiveSession());

  bool was_called = false;
  auto callback = [](bool expect_null, bool* was_called,
                     mojom::XRFrameDataPtr data) {
    *was_called = true;
    EXPECT_EQ(expect_null, !data);
  };

  static_cast<mojom::VRMagicWindowProvider*>(display_1.get())
      ->GetFrameData(base::BindOnce(callback, false, &was_called));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(was_called);
  was_called = false;
  static_cast<mojom::VRMagicWindowProvider*>(display_2.get())
      ->GetFrameData(base::BindOnce(callback, false, &was_called));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(was_called);
  was_called = false;

  // Attempt to present.
  RequestSession(display_1.get());
  EXPECT_TRUE(is_request_presenting_success_);
  EXPECT_TRUE(presenting());
  EXPECT_TRUE(device()->HasExclusiveSession());

  // While a device is presenting, noone should have access to magic window.
  static_cast<mojom::VRMagicWindowProvider*>(display_1.get())
      ->GetFrameData(base::BindOnce(callback, true, &was_called));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(was_called);
  was_called = false;
  static_cast<mojom::VRMagicWindowProvider*>(display_2.get())
      ->GetFrameData(base::BindOnce(callback, true, &was_called));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(was_called);
  was_called = false;

  // Service 1 should be able to exit the presentation it initiated.
  ExitPresent();
  EXPECT_FALSE(presenting());
  EXPECT_FALSE(device()->HasExclusiveSession());

  // Once presentation had ended both services should be able to access the
  // device.
  static_cast<mojom::VRMagicWindowProvider*>(display_1.get())
      ->GetFrameData(base::BindOnce(callback, false, &was_called));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(was_called);
  was_called = false;
  static_cast<mojom::VRMagicWindowProvider*>(display_2.get())
      ->GetFrameData(base::BindOnce(callback, false, &was_called));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(was_called);
  was_called = false;
}

}
