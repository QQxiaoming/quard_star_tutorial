// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file is auto-generated from
// gpu/vulkan/generate_bindings.py
// It's formatted by clang-format using chromium coding style:
//    clang-format -i -style=chromium filename
// DO NOT EDIT!

#ifndef GPU_VULKAN_VULKAN_FUNCTION_POINTERS_H_
#define GPU_VULKAN_VULKAN_FUNCTION_POINTERS_H_

#include <vulkan/vulkan.h>

#include "base/native_library.h"
#include "gpu/vulkan/vulkan_export.h"

namespace gpu {

struct VulkanFunctionPointers;

VULKAN_EXPORT VulkanFunctionPointers* GetVulkanFunctionPointers();

struct VulkanFunctionPointers {
  VulkanFunctionPointers();
  ~VulkanFunctionPointers();

  bool BindUnassociatedFunctionPointers();

  // These functions assume that vkGetInstanceProcAddr has been populated.
  bool BindInstanceFunctionPointers(VkInstance vk_instance);
  bool BindPhysicalDeviceFunctionPointers(VkInstance vk_instance);

  // These functions assume that vkGetDeviceProcAddr has been populated.
  bool BindDeviceFunctionPointers(VkDevice vk_device);
  bool BindSwapchainFunctionPointers(VkDevice vk_device);

  base::NativeLibrary vulkan_loader_library_ = nullptr;

  // Unassociated functions
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddrFn = nullptr;
  PFN_vkCreateInstance vkCreateInstanceFn = nullptr;
  PFN_vkEnumerateInstanceExtensionProperties
      vkEnumerateInstanceExtensionPropertiesFn = nullptr;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerPropertiesFn =
      nullptr;

  // Instance functions
  PFN_vkDestroyInstance vkDestroyInstanceFn = nullptr;
  PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevicesFn = nullptr;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddrFn = nullptr;
  PFN_vkDestroySurfaceKHR vkDestroySurfaceKHRFn = nullptr;

  // Physical Device functions
  PFN_vkCreateDevice vkCreateDeviceFn = nullptr;
  PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerPropertiesFn =
      nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties
      vkGetPhysicalDeviceQueueFamilyPropertiesFn = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
      vkGetPhysicalDeviceSurfaceCapabilitiesKHRFn = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR
      vkGetPhysicalDeviceSurfaceFormatsKHRFn = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR
      vkGetPhysicalDeviceSurfaceSupportKHRFn = nullptr;

  // Device functions
  PFN_vkAllocateCommandBuffers vkAllocateCommandBuffersFn = nullptr;
  PFN_vkAllocateDescriptorSets vkAllocateDescriptorSetsFn = nullptr;
  PFN_vkCreateCommandPool vkCreateCommandPoolFn = nullptr;
  PFN_vkCreateDescriptorPool vkCreateDescriptorPoolFn = nullptr;
  PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayoutFn = nullptr;
  PFN_vkCreateFence vkCreateFenceFn = nullptr;
  PFN_vkCreateFramebuffer vkCreateFramebufferFn = nullptr;
  PFN_vkCreateImageView vkCreateImageViewFn = nullptr;
  PFN_vkCreateRenderPass vkCreateRenderPassFn = nullptr;
  PFN_vkCreateSampler vkCreateSamplerFn = nullptr;
  PFN_vkCreateSemaphore vkCreateSemaphoreFn = nullptr;
  PFN_vkCreateShaderModule vkCreateShaderModuleFn = nullptr;
  PFN_vkDestroyCommandPool vkDestroyCommandPoolFn = nullptr;
  PFN_vkDestroyDescriptorPool vkDestroyDescriptorPoolFn = nullptr;
  PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayoutFn = nullptr;
  PFN_vkDestroyDevice vkDestroyDeviceFn = nullptr;
  PFN_vkDestroyFramebuffer vkDestroyFramebufferFn = nullptr;
  PFN_vkDestroyFence vkDestroyFenceFn = nullptr;
  PFN_vkDestroyImage vkDestroyImageFn = nullptr;
  PFN_vkDestroyImageView vkDestroyImageViewFn = nullptr;
  PFN_vkDestroyRenderPass vkDestroyRenderPassFn = nullptr;
  PFN_vkDestroySampler vkDestroySamplerFn = nullptr;
  PFN_vkDestroySemaphore vkDestroySemaphoreFn = nullptr;
  PFN_vkDestroyShaderModule vkDestroyShaderModuleFn = nullptr;
  PFN_vkFreeCommandBuffers vkFreeCommandBuffersFn = nullptr;
  PFN_vkFreeDescriptorSets vkFreeDescriptorSetsFn = nullptr;
  PFN_vkFreeMemory vkFreeMemoryFn = nullptr;
  PFN_vkGetDeviceQueue vkGetDeviceQueueFn = nullptr;
  PFN_vkGetFenceStatus vkGetFenceStatusFn = nullptr;
  PFN_vkResetFences vkResetFencesFn = nullptr;
  PFN_vkUpdateDescriptorSets vkUpdateDescriptorSetsFn = nullptr;
  PFN_vkWaitForFences vkWaitForFencesFn = nullptr;

  // Queue functions
  PFN_vkQueueSubmit vkQueueSubmitFn = nullptr;
  PFN_vkQueueWaitIdle vkQueueWaitIdleFn = nullptr;

  // Command Buffer functions
  PFN_vkBeginCommandBuffer vkBeginCommandBufferFn = nullptr;
  PFN_vkCmdBeginRenderPass vkCmdBeginRenderPassFn = nullptr;
  PFN_vkCmdEndRenderPass vkCmdEndRenderPassFn = nullptr;
  PFN_vkCmdExecuteCommands vkCmdExecuteCommandsFn = nullptr;
  PFN_vkCmdNextSubpass vkCmdNextSubpassFn = nullptr;
  PFN_vkCmdPipelineBarrier vkCmdPipelineBarrierFn = nullptr;
  PFN_vkEndCommandBuffer vkEndCommandBufferFn = nullptr;
  PFN_vkResetCommandBuffer vkResetCommandBufferFn = nullptr;

  // Swapchain functions
  PFN_vkAcquireNextImageKHR vkAcquireNextImageKHRFn = nullptr;
  PFN_vkCreateSwapchainKHR vkCreateSwapchainKHRFn = nullptr;
  PFN_vkDestroySwapchainKHR vkDestroySwapchainKHRFn = nullptr;
  PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHRFn = nullptr;
  PFN_vkQueuePresentKHR vkQueuePresentKHRFn = nullptr;
};

}  // namespace gpu

// Unassociated functions
#define vkGetInstanceProcAddr \
  gpu::GetVulkanFunctionPointers()->vkGetInstanceProcAddrFn
#define vkCreateInstance gpu::GetVulkanFunctionPointers()->vkCreateInstanceFn
#define vkEnumerateInstanceExtensionProperties \
  gpu::GetVulkanFunctionPointers()->vkEnumerateInstanceExtensionPropertiesFn
#define vkEnumerateInstanceLayerProperties \
  gpu::GetVulkanFunctionPointers()->vkEnumerateInstanceLayerPropertiesFn

// Instance functions
#define vkDestroyInstance gpu::GetVulkanFunctionPointers()->vkDestroyInstanceFn
#define vkEnumeratePhysicalDevices \
  gpu::GetVulkanFunctionPointers()->vkEnumeratePhysicalDevicesFn
#define vkGetDeviceProcAddr \
  gpu::GetVulkanFunctionPointers()->vkGetDeviceProcAddrFn
#define vkDestroySurfaceKHR \
  gpu::GetVulkanFunctionPointers()->vkDestroySurfaceKHRFn

// Physical Device functions
#define vkCreateDevice gpu::GetVulkanFunctionPointers()->vkCreateDeviceFn
#define vkEnumerateDeviceLayerProperties \
  gpu::GetVulkanFunctionPointers()->vkEnumerateDeviceLayerPropertiesFn
#define vkGetPhysicalDeviceQueueFamilyProperties \
  gpu::GetVulkanFunctionPointers()->vkGetPhysicalDeviceQueueFamilyPropertiesFn
#define vkGetPhysicalDeviceSurfaceCapabilitiesKHR \
  gpu::GetVulkanFunctionPointers()->vkGetPhysicalDeviceSurfaceCapabilitiesKHRFn
#define vkGetPhysicalDeviceSurfaceFormatsKHR \
  gpu::GetVulkanFunctionPointers()->vkGetPhysicalDeviceSurfaceFormatsKHRFn
#define vkGetPhysicalDeviceSurfaceSupportKHR \
  gpu::GetVulkanFunctionPointers()->vkGetPhysicalDeviceSurfaceSupportKHRFn

// Device functions
#define vkAllocateCommandBuffers \
  gpu::GetVulkanFunctionPointers()->vkAllocateCommandBuffersFn
#define vkAllocateDescriptorSets \
  gpu::GetVulkanFunctionPointers()->vkAllocateDescriptorSetsFn
#define vkCreateCommandPool \
  gpu::GetVulkanFunctionPointers()->vkCreateCommandPoolFn
#define vkCreateDescriptorPool \
  gpu::GetVulkanFunctionPointers()->vkCreateDescriptorPoolFn
#define vkCreateDescriptorSetLayout \
  gpu::GetVulkanFunctionPointers()->vkCreateDescriptorSetLayoutFn
#define vkCreateFence gpu::GetVulkanFunctionPointers()->vkCreateFenceFn
#define vkCreateFramebuffer \
  gpu::GetVulkanFunctionPointers()->vkCreateFramebufferFn
#define vkCreateImageView gpu::GetVulkanFunctionPointers()->vkCreateImageViewFn
#define vkCreateRenderPass \
  gpu::GetVulkanFunctionPointers()->vkCreateRenderPassFn
#define vkCreateSampler gpu::GetVulkanFunctionPointers()->vkCreateSamplerFn
#define vkCreateSemaphore gpu::GetVulkanFunctionPointers()->vkCreateSemaphoreFn
#define vkCreateShaderModule \
  gpu::GetVulkanFunctionPointers()->vkCreateShaderModuleFn
#define vkDestroyCommandPool \
  gpu::GetVulkanFunctionPointers()->vkDestroyCommandPoolFn
#define vkDestroyDescriptorPool \
  gpu::GetVulkanFunctionPointers()->vkDestroyDescriptorPoolFn
#define vkDestroyDescriptorSetLayout \
  gpu::GetVulkanFunctionPointers()->vkDestroyDescriptorSetLayoutFn
#define vkDestroyDevice gpu::GetVulkanFunctionPointers()->vkDestroyDeviceFn
#define vkDestroyFramebuffer \
  gpu::GetVulkanFunctionPointers()->vkDestroyFramebufferFn
#define vkDestroyFence gpu::GetVulkanFunctionPointers()->vkDestroyFenceFn
#define vkDestroyImage gpu::GetVulkanFunctionPointers()->vkDestroyImageFn
#define vkDestroyImageView \
  gpu::GetVulkanFunctionPointers()->vkDestroyImageViewFn
#define vkDestroyRenderPass \
  gpu::GetVulkanFunctionPointers()->vkDestroyRenderPassFn
#define vkDestroySampler gpu::GetVulkanFunctionPointers()->vkDestroySamplerFn
#define vkDestroySemaphore \
  gpu::GetVulkanFunctionPointers()->vkDestroySemaphoreFn
#define vkDestroyShaderModule \
  gpu::GetVulkanFunctionPointers()->vkDestroyShaderModuleFn
#define vkFreeCommandBuffers \
  gpu::GetVulkanFunctionPointers()->vkFreeCommandBuffersFn
#define vkFreeDescriptorSets \
  gpu::GetVulkanFunctionPointers()->vkFreeDescriptorSetsFn
#define vkFreeMemory gpu::GetVulkanFunctionPointers()->vkFreeMemoryFn
#define vkGetDeviceQueue gpu::GetVulkanFunctionPointers()->vkGetDeviceQueueFn
#define vkGetFenceStatus gpu::GetVulkanFunctionPointers()->vkGetFenceStatusFn
#define vkResetFences gpu::GetVulkanFunctionPointers()->vkResetFencesFn
#define vkUpdateDescriptorSets \
  gpu::GetVulkanFunctionPointers()->vkUpdateDescriptorSetsFn
#define vkWaitForFences gpu::GetVulkanFunctionPointers()->vkWaitForFencesFn

// Queue functions
#define vkQueueSubmit gpu::GetVulkanFunctionPointers()->vkQueueSubmitFn
#define vkQueueWaitIdle gpu::GetVulkanFunctionPointers()->vkQueueWaitIdleFn

// Command buffer functions
#define vkBeginCommandBuffer \
  gpu::GetVulkanFunctionPointers()->vkBeginCommandBufferFn
#define vkCmdBeginRenderPass \
  gpu::GetVulkanFunctionPointers()->vkCmdBeginRenderPassFn
#define vkCmdEndRenderPass \
  gpu::GetVulkanFunctionPointers()->vkCmdEndRenderPassFn
#define vkCmdExecuteCommands \
  gpu::GetVulkanFunctionPointers()->vkCmdExecuteCommandsFn
#define vkCmdNextSubpass gpu::GetVulkanFunctionPointers()->vkCmdNextSubpassFn
#define vkCmdPipelineBarrier \
  gpu::GetVulkanFunctionPointers()->vkCmdPipelineBarrierFn
#define vkEndCommandBuffer \
  gpu::GetVulkanFunctionPointers()->vkEndCommandBufferFn
#define vkResetCommandBuffer \
  gpu::GetVulkanFunctionPointers()->vkResetCommandBufferFn

// Swapchain functions
#define vkAcquireNextImageKHR \
  gpu::GetVulkanFunctionPointers()->vkAcquireNextImageKHRFn
#define vkCreateSwapchainKHR \
  gpu::GetVulkanFunctionPointers()->vkCreateSwapchainKHRFn
#define vkDestroySwapchainKHR \
  gpu::GetVulkanFunctionPointers()->vkDestroySwapchainKHRFn
#define vkGetSwapchainImagesKHR \
  gpu::GetVulkanFunctionPointers()->vkGetSwapchainImagesKHRFn
#define vkQueuePresentKHR gpu::GetVulkanFunctionPointers()->vkQueuePresentKHRFn

#endif  // GPU_VULKAN_VULKAN_FUNCTION_POINTERS_H_
