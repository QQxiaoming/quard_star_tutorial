// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_MUS_AX_TREE_SOURCE_MUS_H_
#define UI_VIEWS_MUS_AX_TREE_SOURCE_MUS_H_

#include "base/macros.h"
#include "ui/views/accessibility/ax_tree_source_views.h"
#include "ui/views/mus/mus_export.h"

namespace views {

class AXAuraObjWrapper;

// This class exposes the views hierarchy as an accessibility tree permitting
// use with other accessibility classes. Only used for out-of-process views
// apps (e.g. Chrome OS shortcut_viewer app). The browser process uses
// AXTreeSourceAura.
class VIEWS_MUS_EXPORT AXTreeSourceMus : public AXTreeSourceViews {
 public:
  // |root| must outlive this object.
  explicit AXTreeSourceMus(AXAuraObjWrapper* root);
  ~AXTreeSourceMus() override;

  // AXTreeSource:
  bool GetTreeData(ui::AXTreeData* data) const override;
  AXAuraObjWrapper* GetRoot() const override;
  void SerializeNode(AXAuraObjWrapper* node,
                     ui::AXNodeData* out_data) const override;

 private:
  // The top-level object to use for the AX tree.
  AXAuraObjWrapper* root_;

  DISALLOW_COPY_AND_ASSIGN(AXTreeSourceMus);
};

}  // namespace views

#endif  // UI_VIEWS_MUS_AX_TREE_SOURCE_MUS_H_
