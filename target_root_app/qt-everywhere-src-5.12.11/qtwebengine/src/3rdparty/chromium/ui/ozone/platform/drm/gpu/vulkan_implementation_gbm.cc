// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/drm/gpu/vulkan_implementation_gbm.h"

#include "base/files/file_path.h"
#include "base/native_library.h"
#include "gpu/vulkan/vulkan_function_pointers.h"
#include "gpu/vulkan/vulkan_instance.h"
#include "gpu/vulkan/vulkan_surface.h"
#include "ui/gfx/gpu_fence.h"

namespace ui {

VulkanImplementationGbm::VulkanImplementationGbm() {}

VulkanImplementationGbm::~VulkanImplementationGbm() {}

bool VulkanImplementationGbm::InitializeVulkanInstance() {
  gpu::VulkanFunctionPointers* vulkan_function_pointers =
      gpu::GetVulkanFunctionPointers();

  base::NativeLibraryLoadError native_library_load_error;
  vulkan_function_pointers->vulkan_loader_library_ = base::LoadNativeLibrary(
      base::FilePath("libvulkan.so.1"), &native_library_load_error);
  if (!vulkan_function_pointers->vulkan_loader_library_)
    return false;

  std::vector<const char*> required_extensions = {
      "VK_KHR_external_fence_capabilities",
      "VK_KHR_get_physical_device_properties2",
  };
  if (!vulkan_instance_.Initialize(required_extensions)) {
    vulkan_instance_.Destroy();
    return false;
  }

  vkGetPhysicalDeviceExternalFencePropertiesKHR_ =
      reinterpret_cast<PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR>(
          vkGetInstanceProcAddr(
              vulkan_instance_.vk_instance(),
              "vkGetPhysicalDeviceExternalFencePropertiesKHR"));
  if (!vkGetPhysicalDeviceExternalFencePropertiesKHR_) {
    vulkan_instance_.Destroy();
    return false;
  }

  vkGetFenceFdKHR_ = reinterpret_cast<PFN_vkGetFenceFdKHR>(
      vkGetInstanceProcAddr(vulkan_instance_.vk_instance(), "vkGetFenceFdKHR"));
  if (!vkGetFenceFdKHR_) {
    vulkan_instance_.Destroy();
    return false;
  }

  return true;
}

VkInstance VulkanImplementationGbm::GetVulkanInstance() {
  return vulkan_instance_.vk_instance();
}

std::unique_ptr<gpu::VulkanSurface> VulkanImplementationGbm::CreateViewSurface(
    gfx::AcceleratedWidget window) {
  return nullptr;
}

bool VulkanImplementationGbm::GetPhysicalDevicePresentationSupport(
    VkPhysicalDevice physical_device,
    const std::vector<VkQueueFamilyProperties>& queue_family_properties,
    uint32_t queue_family_index) {
  VkPhysicalDeviceExternalFenceInfo external_fence_info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO,
      .handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT};
  VkExternalFenceProperties external_fence_properties;
  vkGetPhysicalDeviceExternalFencePropertiesKHR_(
      physical_device, &external_fence_info, &external_fence_properties);
  if (!(external_fence_properties.externalFenceFeatures &
        VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT)) {
    return false;
  }

  return true;
}

std::vector<const char*>
VulkanImplementationGbm::GetRequiredDeviceExtensions() {
  return {
      "VK_KHR_external_fence", "VK_KHR_external_fence_fd",
  };
}

VkFence VulkanImplementationGbm::CreateVkFenceForGpuFence(VkDevice vk_device) {
  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkExportFenceCreateInfo fence_export_create_info = {};
  fence_create_info.pNext = &fence_export_create_info;
  fence_export_create_info.sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
  fence_export_create_info.handleTypes =
      VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;

  VkFence fence;
  VkResult result =
      vkCreateFence(vk_device, &fence_create_info, nullptr, &fence);
  if (result != VK_SUCCESS) {
    DLOG(ERROR) << "vkCreateFence failed: " << result;
    return VK_NULL_HANDLE;
  }

  return fence;
}

std::unique_ptr<gfx::GpuFence> VulkanImplementationGbm::ExportVkFenceToGpuFence(
    VkDevice vk_device,
    VkFence vk_fence) {
  VkFenceGetFdInfoKHR fence_get_fd_info = {};
  fence_get_fd_info.sType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR;
  fence_get_fd_info.fence = vk_fence;
  fence_get_fd_info.handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;
  int fence_fd = -1;
  VkResult result = vkGetFenceFdKHR_(vk_device, &fence_get_fd_info, &fence_fd);
  if (result != VK_SUCCESS) {
    DLOG(ERROR) << "vkGetFenceFdKHR failed: " << result;
    return nullptr;
  }

  gfx::GpuFenceHandle gpu_fence_handle;
  gpu_fence_handle.type = gfx::GpuFenceHandleType::kAndroidNativeFenceSync;
  gpu_fence_handle.native_fd =
      base::FileDescriptor(fence_fd, true /* auto_close */);
  return std::make_unique<gfx::GpuFence>(gpu_fence_handle);
}

}  // namespace ui
