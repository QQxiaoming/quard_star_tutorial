// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/capture/video/linux/video_capture_device_factory_linux.h"
#include "base/run_loop.h"
#include "base/test/scoped_task_environment.h"
#include "media/capture/video/linux/fake_v4l2_impl.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;

namespace media {

class DescriptorDeviceProvider
    : public VideoCaptureDeviceFactoryLinux::DeviceProvider {
 public:
  void AddDevice(const VideoCaptureDeviceDescriptor& descriptor) {
    descriptors_.emplace_back(descriptor);
  }

  void GetDeviceIds(std::vector<std::string>* target_container) override {
    for (const auto& entry : descriptors_) {
      target_container->emplace_back(entry.device_id);
    }
  }

  std::string GetDeviceModelId(const std::string& device_id) override {
    auto iter =
        std::find_if(descriptors_.begin(), descriptors_.end(),
                     [&device_id](const VideoCaptureDeviceDescriptor& val) {
                       return val.device_id == device_id;
                     });
    if (iter == descriptors_.end())
      CHECK(false) << "Unknown device_id " << device_id;

    return iter->model_id;
  }

  std::string GetDeviceDisplayName(const std::string& device_id) override {
    auto iter =
        std::find_if(descriptors_.begin(), descriptors_.end(),
                     [&device_id](const VideoCaptureDeviceDescriptor& val) {
                       return val.device_id == device_id;
                     });
    if (iter == descriptors_.end())
      CHECK(false) << "Unknown device_id " << device_id;

    return iter->display_name();
  }

  VideoFacingMode GetCameraFacing(const std::string& device_id,
                                  const std::string& model_id) override {
    return MEDIA_VIDEO_FACING_NONE;
  }

  int GetOrientation(const std::string& device_id,
                     const std::string& model_id) override {
    return 0;
  }

 private:
  std::vector<VideoCaptureDeviceDescriptor> descriptors_;
};

class VideoCaptureDeviceFactoryLinuxTest : public ::testing::Test {
 public:
  VideoCaptureDeviceFactoryLinuxTest() {}
  ~VideoCaptureDeviceFactoryLinuxTest() override = default;

  void SetUp() override {
    factory_ = std::make_unique<VideoCaptureDeviceFactoryLinux>(
        base::ThreadTaskRunnerHandle::Get());
    scoped_refptr<FakeV4L2Impl> fake_v4l2(new FakeV4L2Impl());
    fake_v4l2_ = fake_v4l2.get();
    auto fake_device_provider = std::make_unique<DescriptorDeviceProvider>();
    fake_device_provider_ = fake_device_provider.get();
    factory_->SetV4L2EnvironmentForTesting(std::move(fake_v4l2),
                                           std::move(fake_device_provider));
  }

  base::test::ScopedTaskEnvironment scoped_task_environment_;
  FakeV4L2Impl* fake_v4l2_;
  DescriptorDeviceProvider* fake_device_provider_;
  std::unique_ptr<VideoCaptureDeviceFactoryLinux> factory_;
};

TEST_F(VideoCaptureDeviceFactoryLinuxTest, EnumerateSingleFakeV4L2Device) {
  // Setup
  const std::string stub_display_name = "Fake Device 0";
  const std::string stub_device_id = "/dev/video0";
  VideoCaptureDeviceDescriptor descriptor(stub_display_name, stub_device_id);
  fake_device_provider_->AddDevice(descriptor);
  fake_v4l2_->AddDevice(stub_device_id, FakeV4L2DeviceConfig(descriptor));

  // Exercise
  VideoCaptureDeviceDescriptors descriptors;
  factory_->GetDeviceDescriptors(&descriptors);

  // Verification
  ASSERT_EQ(1u, descriptors.size());
  ASSERT_EQ(stub_device_id, descriptors[0].device_id);
  ASSERT_EQ(stub_display_name, descriptors[0].display_name());
}

};  // namespace media
