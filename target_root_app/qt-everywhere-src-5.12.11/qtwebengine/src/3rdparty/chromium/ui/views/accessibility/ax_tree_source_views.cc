// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/accessibility/ax_tree_source_views.h"

#include <vector>

#include "ui/accessibility/ax_action_data.h"
#include "ui/accessibility/platform/ax_unique_id.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/transform.h"
#include "ui/views/accessibility/ax_aura_obj_cache.h"
#include "ui/views/accessibility/ax_aura_obj_wrapper.h"

namespace views {

void AXTreeSourceViews::HandleAccessibleAction(const ui::AXActionData& action) {
  int id = action.target_node_id;

  // In Views, we only support setting the selection within a single node,
  // not across multiple nodes like on the web.
  if (action.action == ax::mojom::Action::kSetSelection) {
    if (action.anchor_node_id != action.focus_node_id) {
      NOTREACHED();
      return;
    }
    id = action.anchor_node_id;
  }

  AXAuraObjWrapper* obj = AXAuraObjCache::GetInstance()->Get(id);
  if (obj)
    obj->HandleAccessibleAction(action);
}

bool AXTreeSourceViews::GetTreeData(ui::AXTreeData* tree_data) const {
  tree_data->loaded = true;
  tree_data->loading_progress = 1.0;
  AXAuraObjWrapper* focus = AXAuraObjCache::GetInstance()->GetFocus();
  if (focus)
    tree_data->focus_id = focus->GetUniqueId().Get();
  return true;
}

AXAuraObjWrapper* AXTreeSourceViews::GetFromId(int32_t id) const {
  AXAuraObjWrapper* root = GetRoot();
  // Root might not be in the cache.
  if (id == root->GetUniqueId().Get())
    return root;
  return AXAuraObjCache::GetInstance()->Get(id);
}

int32_t AXTreeSourceViews::GetId(AXAuraObjWrapper* node) const {
  return node->GetUniqueId().Get();
}

void AXTreeSourceViews::GetChildren(
    AXAuraObjWrapper* node,
    std::vector<AXAuraObjWrapper*>* out_children) const {
  node->GetChildren(out_children);
}

AXAuraObjWrapper* AXTreeSourceViews::GetParent(AXAuraObjWrapper* node) const {
  AXAuraObjWrapper* root = GetRoot();
  // The root has no parent by definition.
  if (node->GetUniqueId() == root->GetUniqueId())
    return nullptr;
  AXAuraObjWrapper* parent = node->GetParent();
  // A top-level widget doesn't have a parent, so return the root.
  if (!parent)
    return root;
  return parent;
}

bool AXTreeSourceViews::IsValid(AXAuraObjWrapper* node) const {
  return node && !node->IsIgnored();
}

bool AXTreeSourceViews::IsEqual(AXAuraObjWrapper* node1,
                                AXAuraObjWrapper* node2) const {
  return node1 && node2 && node1->GetUniqueId() == node2->GetUniqueId();
}

AXAuraObjWrapper* AXTreeSourceViews::GetNull() const {
  return nullptr;
}

void AXTreeSourceViews::SerializeNode(AXAuraObjWrapper* node,
                                      ui::AXNodeData* out_data) const {
  node->Serialize(out_data);

  // Converts the global coordinates reported by each AXAuraObjWrapper
  // into parent-relative coordinates to be used in the accessibility
  // tree. That way when any Window, Widget, or View moves (and fires
  // a location changed event), its descendants all move relative to
  // it by default.
  AXAuraObjWrapper* parent = node->GetParent();
  if (!parent)
    return;
  ui::AXNodeData parent_data;
  parent->Serialize(&parent_data);
  out_data->location.Offset(-parent_data.location.OffsetFromOrigin());
  out_data->offset_container_id = parent->GetUniqueId().Get();
}

std::string AXTreeSourceViews::ToString(AXAuraObjWrapper* root,
                                        std::string prefix) {
  ui::AXNodeData data;
  root->Serialize(&data);
  std::string output = prefix + data.ToString() + '\n';

  std::vector<AXAuraObjWrapper*> children;
  root->GetChildren(&children);

  prefix += prefix[0];
  for (AXAuraObjWrapper* child : children)
    output += ToString(child, prefix);

  return output;
}

AXTreeSourceViews::AXTreeSourceViews() = default;

AXTreeSourceViews::~AXTreeSourceViews() = default;

}  // namespace views
