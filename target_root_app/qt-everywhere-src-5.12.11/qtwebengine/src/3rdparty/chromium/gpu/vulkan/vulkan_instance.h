// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_VULKAN_VULKAN_INSTANCE_H_
#define GPU_VULKAN_VULKAN_INSTANCE_H_

#include <vulkan/vulkan.h>
#include <memory>

#include "base/logging.h"
#include "base/macros.h"
#include "gpu/vulkan/vulkan_export.h"
#include "ui/gfx/extension_set.h"

namespace gpu {

class VULKAN_EXPORT VulkanInstance {
 public:
  VulkanInstance();

  ~VulkanInstance();

  bool Initialize(const std::vector<const char*>& required_extensions);

  void Destroy();

  const gfx::ExtensionSet& enabled_extensions() const {
    return enabled_extensions_;
  }

  VkInstance vk_instance() { return vk_instance_; }

 private:
  VkInstance vk_instance_ = VK_NULL_HANDLE;
  gfx::ExtensionSet enabled_extensions_;
  bool debug_report_enabled_ = false;
#if DCHECK_IS_ON()
  VkDebugReportCallbackEXT error_callback_ = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT warning_callback_ = VK_NULL_HANDLE;
#endif

  DISALLOW_COPY_AND_ASSIGN(VulkanInstance);
};

}  // namespace gpu

#endif  // GPU_VULKAN_VULKAN_INSTANCE_H_
