// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_UTILITY_UTILITY_BLINK_PLATFORM_WITH_SANDBOX_SUPPORT_IMPL_H_
#define CONTENT_UTILITY_UTILITY_BLINK_PLATFORM_WITH_SANDBOX_SUPPORT_IMPL_H_

#include <memory>

#include "base/macros.h"
#include "build/build_config.h"
#include "content/utility/utility_blink_platform_impl.h"

#if defined(OS_POSIX) && !defined(OS_ANDROID)
#include "components/services/font/public/cpp/font_loader.h"  // nogncheck
#include "third_party/skia/include/core/SkRefCnt.h"           // nogncheck
#endif

namespace blink {
class WebSandboxSupport;
}

namespace service_manager {
class Connector;
}

namespace content {

// This class extends from UtilityBlinkPlatformImpl with added blink web
// sandbox support.
class UtilityBlinkPlatformWithSandboxSupportImpl
    : public UtilityBlinkPlatformImpl {
 public:
  UtilityBlinkPlatformWithSandboxSupportImpl() = delete;
  explicit UtilityBlinkPlatformWithSandboxSupportImpl(
      service_manager::Connector*);
  ~UtilityBlinkPlatformWithSandboxSupportImpl() override;

  // BlinkPlatformImpl
  blink::WebSandboxSupport* GetSandboxSupport() override;

 private:
#if defined(OS_POSIX) && !defined(OS_ANDROID)
  class SandboxSupport;
  std::unique_ptr<SandboxSupport> sandbox_support_;
#endif
#if defined(OS_LINUX)
  sk_sp<font_service::FontLoader> font_loader_;
#endif

  DISALLOW_COPY_AND_ASSIGN(UtilityBlinkPlatformWithSandboxSupportImpl);
};

}  // namespace content

#endif  // CONTENT_UTILITY_UTILITY_BLINK_PLATFORM_WITH_SANDBOX_SUPPORT_IMPL_H_
