// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/platform/ax_platform_node_delegate_base.h"

#include "base/macros.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/accessibility/ax_tree_data.h"

namespace ui {

const AXNodeData& AXPlatformNodeDelegateBase::GetData() const {
  CR_DEFINE_STATIC_LOCAL(AXNodeData, empty_data, ());
  return empty_data;
}

const AXTreeData& AXPlatformNodeDelegateBase::GetTreeData() const {
  CR_DEFINE_STATIC_LOCAL(AXTreeData, empty_data, ());
  return empty_data;
}

gfx::NativeWindow AXPlatformNodeDelegateBase::GetTopLevelWidget() {
  return nullptr;
}

gfx::NativeViewAccessible AXPlatformNodeDelegateBase::GetParent() {
  return nullptr;
}

int AXPlatformNodeDelegateBase::GetChildCount() {
  return 0;
}

gfx::NativeViewAccessible AXPlatformNodeDelegateBase::ChildAtIndex(int index) {
  return nullptr;
}

gfx::Rect AXPlatformNodeDelegateBase::GetClippedScreenBoundsRect() const {
  return gfx::Rect();
}

gfx::Rect AXPlatformNodeDelegateBase::GetUnclippedScreenBoundsRect() const {
  return gfx::Rect();
}

gfx::NativeViewAccessible AXPlatformNodeDelegateBase::HitTestSync(int x,
                                                                  int y) {
  return nullptr;
}

gfx::NativeViewAccessible AXPlatformNodeDelegateBase::GetFocus() {
  return nullptr;
}

AXPlatformNode* AXPlatformNodeDelegateBase::GetFromNodeID(int32_t id) {
  return nullptr;
}

int AXPlatformNodeDelegateBase::GetIndexInParent() const {
  return -1;
}

gfx::AcceleratedWidget
AXPlatformNodeDelegateBase::GetTargetForNativeAccessibilityEvent() {
  return gfx::kNullAcceleratedWidget;
}

int AXPlatformNodeDelegateBase::GetTableRowCount() const {
  return 0;
}

int AXPlatformNodeDelegateBase::GetTableColCount() const {
  return 0;
}

std::vector<int32_t> AXPlatformNodeDelegateBase::GetColHeaderNodeIds() const {
  return std::vector<int32_t>();
}

std::vector<int32_t> AXPlatformNodeDelegateBase::GetColHeaderNodeIds(
    int32_t col_index) const {
  return std::vector<int32_t>();
}

std::vector<int32_t> AXPlatformNodeDelegateBase::GetRowHeaderNodeIds() const {
  return std::vector<int32_t>();
}

std::vector<int32_t> AXPlatformNodeDelegateBase::GetRowHeaderNodeIds(
    int32_t row_index) const {
  return std::vector<int32_t>();
}

int32_t AXPlatformNodeDelegateBase::GetCellId(int32_t row_index,
                                              int32_t col_index) const {
  return -1;
}

int32_t AXPlatformNodeDelegateBase::CellIdToIndex(int32_t cell_id) const {
  return 0;
}

int32_t AXPlatformNodeDelegateBase::CellIndexToId(int32_t cell_index) const {
  return -1;
}

bool AXPlatformNodeDelegateBase::AccessibilityPerformAction(
    const ui::AXActionData& data) {
  return false;
}

bool AXPlatformNodeDelegateBase::ShouldIgnoreHoveredStateForTesting() {
  return true;
}

bool AXPlatformNodeDelegateBase::IsOffscreen() const {
  return false;
}

std::set<int32_t> AXPlatformNodeDelegateBase::GetReverseRelations(
    ax::mojom::IntAttribute attr,
    int32_t dst_id) {
  return std::set<int32_t>();
}

std::set<int32_t> AXPlatformNodeDelegateBase::GetReverseRelations(
    ax::mojom::IntListAttribute attr,
    int32_t dst_id) {
  return std::set<int32_t>();
}

const AXUniqueId& AXPlatformNodeDelegateBase::GetUniqueId() const {
  CR_DEFINE_STATIC_LOCAL(AXUniqueId, dummy_unique_id, ());
  return dummy_unique_id;
}

}  // namespace ui
