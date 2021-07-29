#!/usr/bin/env python
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""code generator for Vulkan function pointers."""

import optparse
import os
import platform
import sys
from string import Template
from subprocess import call

VULKAN_UNASSOCIATED_FUNCTIONS = [
# vkGetInstanceProcAddr belongs here but is handled specially.
{ 'name': 'vkCreateInstance' },
{ 'name': 'vkEnumerateInstanceExtensionProperties' },
{ 'name': 'vkEnumerateInstanceLayerProperties' },
]

VULKAN_INSTANCE_FUNCTIONS = [
{ 'name': 'vkDestroyInstance' },
# vkDestroySurfaceKHR belongs here but is handled specially.
{ 'name': 'vkEnumeratePhysicalDevices' },
{ 'name': 'vkGetDeviceProcAddr' },
]

VULKAN_PHYSICAL_DEVICE_FUNCTIONS = [
{ 'name': 'vkCreateDevice' },
{ 'name': 'vkEnumerateDeviceLayerProperties' },
{ 'name': 'vkGetPhysicalDeviceQueueFamilyProperties' },
# The following functions belong here but are handled specially:
# vkGetPhysicalDeviceSurfaceCapabilitiesKHR
# vkGetPhysicalDeviceSurfaceFormatsKHR
# vkGetPhysicalDeviceSurfaceSupportKHR
]

VULKAN_DEVICE_FUNCTIONS = [
{ 'name': 'vkAllocateCommandBuffers' },
{ 'name': 'vkAllocateDescriptorSets' },
{ 'name': 'vkCreateCommandPool' },
{ 'name': 'vkCreateDescriptorPool' },
{ 'name': 'vkCreateDescriptorSetLayout' },
{ 'name': 'vkCreateFence' },
{ 'name': 'vkCreateFramebuffer' },
{ 'name': 'vkCreateImageView' },
{ 'name': 'vkCreateRenderPass' },
{ 'name': 'vkCreateSampler' },
{ 'name': 'vkCreateSemaphore' },
{ 'name': 'vkCreateShaderModule' },
{ 'name': 'vkDestroyCommandPool' },
{ 'name': 'vkDestroyDescriptorPool' },
{ 'name': 'vkDestroyDescriptorSetLayout' },
{ 'name': 'vkDestroyDevice' },
{ 'name': 'vkDestroyFramebuffer' },
{ 'name': 'vkDestroyFence' },
{ 'name': 'vkDestroyImage' },
{ 'name': 'vkDestroyImageView' },
{ 'name': 'vkDestroyRenderPass' },
{ 'name': 'vkDestroySampler' },
{ 'name': 'vkDestroySemaphore' },
{ 'name': 'vkDestroyShaderModule' },
{ 'name': 'vkFreeCommandBuffers' },
{ 'name': 'vkFreeDescriptorSets' },
{ 'name': 'vkFreeMemory' },
{ 'name': 'vkGetDeviceQueue' },
{ 'name': 'vkGetFenceStatus' },
{ 'name': 'vkResetFences' },
{ 'name': 'vkUpdateDescriptorSets' },
{ 'name': 'vkWaitForFences' },
]

VULKAN_QUEUE_FUNCTIONS = [
{ 'name': 'vkQueueSubmit' },
{ 'name': 'vkQueueWaitIdle' },
]

VULKAN_COMMAND_BUFFER_FUNCTIONS = [
{ 'name': 'vkBeginCommandBuffer' },
{ 'name': 'vkCmdBeginRenderPass' },
{ 'name': 'vkCmdEndRenderPass' },
{ 'name': 'vkCmdExecuteCommands' },
{ 'name': 'vkCmdNextSubpass' },
{ 'name': 'vkCmdPipelineBarrier' },
{ 'name': 'vkEndCommandBuffer' },
{ 'name': 'vkResetCommandBuffer' },
]

VULKAN_SWAPCHAIN_FUNCTIONS = [
{ 'name': 'vkAcquireNextImageKHR' },
{ 'name': 'vkCreateSwapchainKHR' },
{ 'name': 'vkDestroySwapchainKHR' },
{ 'name': 'vkGetSwapchainImagesKHR' },
{ 'name': 'vkQueuePresentKHR' },
]

SELF_LOCATION = os.path.dirname(os.path.abspath(__file__))

LICENSE_AND_HEADER = """\
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file is auto-generated from
// gpu/vulkan/generate_bindings.py
// It's formatted by clang-format using chromium coding style:
//    clang-format -i -style=chromium filename
// DO NOT EDIT!

"""

def WriteFunctionDeclarations(file, functions):
  template = Template('  PFN_${name} ${name}Fn = nullptr;\n')
  for func in functions:
    file.write(template.substitute(func))

def WriteMacros(file, functions):
  template = Template(
      '#define $name gpu::GetVulkanFunctionPointers()->${name}Fn\n')
  for func in functions:
    file.write(template.substitute(func))

def GenerateHeaderFile(file, unassociated_functions, instance_functions,
                       physical_device_functions, device_functions,
                       queue_functions, command_buffer_functions,
                       swapchain_functions):
  """Generates gpu/vulkan/vulkan_function_pointers.h"""

  file.write(LICENSE_AND_HEADER +
"""

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
""")

  WriteFunctionDeclarations(file, unassociated_functions)

  file.write("""\

  // Instance functions
""")

  WriteFunctionDeclarations(file, instance_functions)

  file.write("""\
  PFN_vkDestroySurfaceKHR vkDestroySurfaceKHRFn = nullptr;

  // Physical Device functions
""")

  WriteFunctionDeclarations(file, physical_device_functions)

  file.write("""\
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
      vkGetPhysicalDeviceSurfaceCapabilitiesKHRFn = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR
      vkGetPhysicalDeviceSurfaceFormatsKHRFn = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR
      vkGetPhysicalDeviceSurfaceSupportKHRFn = nullptr;

  // Device functions
""")

  WriteFunctionDeclarations(file, device_functions)

  file.write("""\

  // Queue functions
""")

  WriteFunctionDeclarations(file, queue_functions)

  file.write("""\

  // Command Buffer functions
""")

  WriteFunctionDeclarations(file, command_buffer_functions)

  file.write("""\

  // Swapchain functions
""")

  WriteFunctionDeclarations(file, swapchain_functions)

  file.write("""\
};

}  // namespace gpu

// Unassociated functions
""")

  WriteMacros(file, [ { 'name': 'vkGetInstanceProcAddr' } ])
  WriteMacros(file, unassociated_functions)

  file.write("""\

// Instance functions
""")

  WriteMacros(file, instance_functions)
  WriteMacros(file, [ { 'name': 'vkDestroySurfaceKHR' } ])

  file.write("""\

// Physical Device functions
""")

  WriteMacros(file, physical_device_functions)
  WriteMacros(file, [
      { 'name': 'vkGetPhysicalDeviceSurfaceCapabilitiesKHR' },
      { 'name': 'vkGetPhysicalDeviceSurfaceFormatsKHR' },
      { 'name': 'vkGetPhysicalDeviceSurfaceSupportKHR' },
  ])

  file.write("""\

// Device functions
""")

  WriteMacros(file, device_functions)

  file.write("""\

// Queue functions
""")

  WriteMacros(file, queue_functions)

  file.write("""\

// Command buffer functions
""")

  WriteMacros(file, command_buffer_functions)

  file.write("""\

// Swapchain functions
""")

  WriteMacros(file, swapchain_functions)

  file.write("""\

#endif  // GPU_VULKAN_VULKAN_FUNCTION_POINTERS_H_
""")

def WriteFunctionPointerInitialization(file, proc_addr_function, parent,
                                       functions):
  template = Template("""  ${name}Fn = reinterpret_cast<PFN_${name}>(
    $get_proc_addr($parent, "$name"));
  if (!${name}Fn)
    return false;

""")
  for func in functions:
    file.write(template.substitute(name=func['name'], get_proc_addr =
                                   proc_addr_function, parent=parent))

def WriteUnassociatedFunctionPointerInitialization(file, functions):
  WriteFunctionPointerInitialization(file, 'vkGetInstanceProcAddrFn', 'nullptr',
                                     functions)

def WriteInstanceFunctionPointerInitialization(file, functions):
  WriteFunctionPointerInitialization(file, 'vkGetInstanceProcAddrFn',
                                     'vk_instance', functions)

def WriteDeviceFunctionPointerInitialization(file, functions):
  WriteFunctionPointerInitialization(file, 'vkGetDeviceProcAddrFn', 'vk_device',
                                     functions)

def GenerateSourceFile(file, unassociated_functions, instance_functions,
                       physical_device_functions, device_functions,
                       queue_functions, command_buffer_functions,
                       swapchain_functions):
  """Generates gpu/vulkan/vulkan_function_pointers.cc"""

  file.write(LICENSE_AND_HEADER +
"""

#include "gpu/vulkan/vulkan_function_pointers.h"

#include "base/no_destructor.h"

namespace gpu {

VulkanFunctionPointers* GetVulkanFunctionPointers() {
  static base::NoDestructor<VulkanFunctionPointers> vulkan_function_pointers;
  return vulkan_function_pointers.get();
}

VulkanFunctionPointers::VulkanFunctionPointers() = default;
VulkanFunctionPointers::~VulkanFunctionPointers() = default;

bool VulkanFunctionPointers::BindUnassociatedFunctionPointers() {
  // vkGetInstanceProcAddr must be handled specially since it gets its function
  // pointer through base::GetFunctionPOinterFromNativeLibrary(). Other Vulkan
  // functions don't do this.
  vkGetInstanceProcAddrFn = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      base::GetFunctionPointerFromNativeLibrary(vulkan_loader_library_,
                                                "vkGetInstanceProcAddr"));
  if (!vkGetInstanceProcAddrFn)
    return false;

""")

  WriteUnassociatedFunctionPointerInitialization(file, unassociated_functions)

  file.write("""\

  return true;
}

bool VulkanFunctionPointers::BindInstanceFunctionPointers(
    VkInstance vk_instance) {
""")

  WriteInstanceFunctionPointerInitialization(file, instance_functions)

  file.write("""\

  return true;
}

bool VulkanFunctionPointers::BindPhysicalDeviceFunctionPointers(
    VkInstance vk_instance) {
""")

  WriteInstanceFunctionPointerInitialization(file, physical_device_functions)

  file.write("""\

  return true;
}

bool VulkanFunctionPointers::BindDeviceFunctionPointers(VkDevice vk_device) {
  // Device functions
""")
  WriteDeviceFunctionPointerInitialization(file, device_functions)

  file.write("""\

  // Queue functions
""")
  WriteDeviceFunctionPointerInitialization(file, queue_functions)

  file.write("""\

  // Command Buffer functions
""")
  WriteDeviceFunctionPointerInitialization(file, command_buffer_functions)

  file.write("""\


  return true;
}

bool VulkanFunctionPointers::BindSwapchainFunctionPointers(VkDevice vk_device) {
""")

  WriteDeviceFunctionPointerInitialization(file, swapchain_functions)

  file.write("""\


  return true;
}

}  // namespace gpu
""")

def main(argv):
  """This is the main function."""

  parser = optparse.OptionParser()
  _, args = parser.parse_args(argv)

  directory = SELF_LOCATION
  if len(args) >= 1:
    directory = args[0]

  def ClangFormat(filename):
    formatter = "clang-format"
    if platform.system() == "Windows":
      formatter += ".bat"
    call([formatter, "-i", "-style=chromium", filename])

  header_file = open(
      os.path.join(directory, 'vulkan_function_pointers.h'), 'wb')
  GenerateHeaderFile(header_file, VULKAN_UNASSOCIATED_FUNCTIONS,
                     VULKAN_INSTANCE_FUNCTIONS,
                     VULKAN_PHYSICAL_DEVICE_FUNCTIONS, VULKAN_DEVICE_FUNCTIONS,
                     VULKAN_QUEUE_FUNCTIONS, VULKAN_COMMAND_BUFFER_FUNCTIONS,
                     VULKAN_SWAPCHAIN_FUNCTIONS)
  header_file.close()
  ClangFormat(header_file.name)

  source_file = open(
      os.path.join(directory, 'vulkan_function_pointers.cc'), 'wb')
  GenerateSourceFile(source_file, VULKAN_UNASSOCIATED_FUNCTIONS,
                     VULKAN_INSTANCE_FUNCTIONS,
                     VULKAN_PHYSICAL_DEVICE_FUNCTIONS, VULKAN_DEVICE_FUNCTIONS,
                     VULKAN_QUEUE_FUNCTIONS, VULKAN_COMMAND_BUFFER_FUNCTIONS,
                     VULKAN_SWAPCHAIN_FUNCTIONS)
  source_file.close()
  ClangFormat(source_file.name)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
