// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_ACCESSIBILITY_VIEW_AX_PLATFORM_NODE_DELEGATE_AURALINUX_H_
#define UI_VIEWS_ACCESSIBILITY_VIEW_AX_PLATFORM_NODE_DELEGATE_AURALINUX_H_

#include "base/macros.h"
#include "ui/views/accessibility/view_ax_platform_node_delegate.h"

namespace views {

class View;

class ViewAXPlatformNodeDelegateAuraLinux : public ViewAXPlatformNodeDelegate {
 public:
  explicit ViewAXPlatformNodeDelegateAuraLinux(View* view);
  ~ViewAXPlatformNodeDelegateAuraLinux() override;

  // |ViewAXPlatformNodeDelegate| overrides:
  gfx::NativeViewAccessible GetParent() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewAXPlatformNodeDelegateAuraLinux);
};

}  // namespace views

#endif  // UI_VIEWS_ACCESSIBILITY_VIEW_AX_PLATFORM_NODE_DELEGATE_AURALINUX_H_
