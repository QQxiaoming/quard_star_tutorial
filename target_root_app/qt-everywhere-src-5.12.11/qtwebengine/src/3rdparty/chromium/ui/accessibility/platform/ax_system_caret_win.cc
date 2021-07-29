// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/platform/ax_system_caret_win.h"

#include <windows.h>

#include "base/logging.h"
#include "ui/accessibility/ax_enums.mojom.h"
#include "ui/accessibility/platform/ax_platform_node_win.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/rect_f.h"

namespace ui {

AXSystemCaretWin::AXSystemCaretWin(gfx::AcceleratedWidget event_target)
    : event_target_(event_target) {
  caret_ = static_cast<AXPlatformNodeWin*>(AXPlatformNodeWin::Create(this));
  // The caret object is not part of the accessibility tree and so doesn't need
  // a node ID. A globally unique ID is used when firing Win events, retrieved
  // via |unique_id|.
  data_.id = -1;
  data_.role = ax::mojom::Role::kCaret;
  // |get_accState| should return 0 which means that the caret is visible.
  data_.state = 0;
  // According to MSDN, "Edit" should be the name of the caret object.
  data_.SetName(L"Edit");
  data_.offset_container_id = -1;

  if (event_target_) {
    ::NotifyWinEvent(EVENT_OBJECT_CREATE, event_target_, OBJID_CARET,
                     -caret_->GetUniqueId());
  }
}

AXSystemCaretWin::~AXSystemCaretWin() {
  if (event_target_) {
    ::NotifyWinEvent(EVENT_OBJECT_DESTROY, event_target_, OBJID_CARET,
                     -caret_->GetUniqueId());
  }
  caret_->Destroy();
}

Microsoft::WRL::ComPtr<IAccessible> AXSystemCaretWin::GetCaret() const {
  Microsoft::WRL::ComPtr<IAccessible> caret_accessible;
  HRESULT hr = caret_->QueryInterface(
      IID_IAccessible,
      reinterpret_cast<void**>(caret_accessible.GetAddressOf()));
  DCHECK(SUCCEEDED(hr));
  return caret_accessible;
}

void AXSystemCaretWin::MoveCaretTo(const gfx::Rect& bounds) {
  if (bounds.IsEmpty())
    return;
  data_.location = gfx::RectF(bounds);
  if (event_target_) {
    ::NotifyWinEvent(EVENT_OBJECT_LOCATIONCHANGE, event_target_, OBJID_CARET,
                     -caret_->GetUniqueId());
  }
}

const AXNodeData& AXSystemCaretWin::GetData() const {
  return data_;
}

gfx::NativeViewAccessible AXSystemCaretWin::GetParent() {
  if (!event_target_)
    return nullptr;

  gfx::NativeViewAccessible parent;
  HRESULT hr =
      ::AccessibleObjectFromWindow(event_target_, OBJID_WINDOW, IID_IAccessible,
                                   reinterpret_cast<void**>(&parent));
  if (SUCCEEDED(hr))
    return parent;
  return nullptr;
}

gfx::Rect AXSystemCaretWin::GetClippedScreenBoundsRect() const {
  // We could optionally add clipping here if ever needed.
  return ToEnclosingRect(data_.location);
}

gfx::Rect AXSystemCaretWin::GetUnclippedScreenBoundsRect() const {
  return ToEnclosingRect(data_.location);
}

gfx::AcceleratedWidget
AXSystemCaretWin::GetTargetForNativeAccessibilityEvent() {
  return event_target_;
}

bool AXSystemCaretWin::ShouldIgnoreHoveredStateForTesting() {
  return false;
}

const ui::AXUniqueId& AXSystemCaretWin::GetUniqueId() const {
  return unique_id_;
}

}  // namespace ui
