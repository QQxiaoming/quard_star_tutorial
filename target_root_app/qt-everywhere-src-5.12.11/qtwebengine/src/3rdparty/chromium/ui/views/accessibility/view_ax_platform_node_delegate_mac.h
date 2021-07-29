// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_ACCESSIBILITY_VIEW_AX_PLATFORM_NODE_DELEGATE_MAC_H_
#define UI_VIEWS_ACCESSIBILITY_VIEW_AX_PLATFORM_NODE_DELEGATE_MAC_H_

#include "base/macros.h"
#include "ui/views/accessibility/view_ax_platform_node_delegate.h"

namespace views {

// Mac-specific accessibility class for |ViewAXPlatformNodeDelegate|.
class ViewAXPlatformNodeDelegateMac : public ViewAXPlatformNodeDelegate {
 public:
  explicit ViewAXPlatformNodeDelegateMac(View* view);
  ~ViewAXPlatformNodeDelegateMac() override;

  // |ViewAXPlatformNodeDelegate| overrides:
  gfx::NativeViewAccessible GetParent() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewAXPlatformNodeDelegateMac);
};

}  // namespace views

#endif  // UI_VIEWS_ACCESSIBILITY_VIEW_AX_PLATFORM_NODE_DELEGATE_MAC_H_
