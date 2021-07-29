// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/capture/video/chromeos/video_capture_device_factory_chromeos.h"

#include "base/memory/ptr_util.h"
#include "media/capture/video/chromeos/camera_hal_dispatcher_impl.h"

namespace media {

namespace {

gpu::GpuMemoryBufferManager* g_gpu_buffer_manager = nullptr;

}  // namespace

VideoCaptureDeviceFactoryChromeOS::VideoCaptureDeviceFactoryChromeOS(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner_for_screen_observer)
    : task_runner_for_screen_observer_(task_runner_for_screen_observer),
      camera_hal_ipc_thread_("CameraHalIpcThread"),
      initialized_(Init()) {}

VideoCaptureDeviceFactoryChromeOS::~VideoCaptureDeviceFactoryChromeOS() {
  camera_hal_delegate_->Reset();
  camera_hal_ipc_thread_.Stop();
}

std::unique_ptr<VideoCaptureDevice>
VideoCaptureDeviceFactoryChromeOS::CreateDevice(
    const VideoCaptureDeviceDescriptor& device_descriptor) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (!initialized_) {
    return std::unique_ptr<VideoCaptureDevice>();
  }
  return camera_hal_delegate_->CreateDevice(task_runner_for_screen_observer_,
                                            device_descriptor);
}

void VideoCaptureDeviceFactoryChromeOS::GetSupportedFormats(
    const VideoCaptureDeviceDescriptor& device_descriptor,
    VideoCaptureFormats* supported_formats) {
  DCHECK(thread_checker_.CalledOnValidThread());
  camera_hal_delegate_->GetSupportedFormats(device_descriptor,
                                            supported_formats);
}

void VideoCaptureDeviceFactoryChromeOS::GetDeviceDescriptors(
    VideoCaptureDeviceDescriptors* device_descriptors) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (!initialized_) {
    return;
  }
  camera_hal_delegate_->GetDeviceDescriptors(device_descriptors);
}

// static
gpu::GpuMemoryBufferManager*
VideoCaptureDeviceFactoryChromeOS::GetBufferManager() {
  return g_gpu_buffer_manager;
}

// static
void VideoCaptureDeviceFactoryChromeOS::SetGpuBufferManager(
    gpu::GpuMemoryBufferManager* buffer_manager) {
  g_gpu_buffer_manager = buffer_manager;
}

bool VideoCaptureDeviceFactoryChromeOS::Init() {
  if (!camera_hal_ipc_thread_.Start()) {
    LOG(ERROR) << "Module thread failed to start";
    return false;
  }

  if (!CameraHalDispatcherImpl::GetInstance()->IsStarted()) {
    LOG(ERROR) << "CameraHalDispatcherImpl is not started";
    return false;
  }

  camera_hal_delegate_ =
      new CameraHalDelegate(camera_hal_ipc_thread_.task_runner());
  camera_hal_delegate_->RegisterCameraClient();
  return true;
}

}  // namespace media
