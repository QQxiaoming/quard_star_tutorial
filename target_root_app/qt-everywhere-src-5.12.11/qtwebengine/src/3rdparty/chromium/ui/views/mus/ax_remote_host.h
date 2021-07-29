// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_MUS_AX_REMOTE_HOST_H_
#define UI_VIEWS_MUS_AX_REMOTE_HOST_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "ui/accessibility/ax_tree_serializer.h"
#include "ui/accessibility/mojom/ax_host.mojom.h"
#include "ui/views/accessibility/ax_aura_obj_cache.h"
#include "ui/views/mus/mus_export.h"
#include "ui/views/widget/widget_observer.h"

namespace service_manager {
class Connector;
}

namespace ui {
struct AXActionData;
struct AXNodeData;
struct AXTreeData;
}  // namespace ui

namespace views {

class AXAuraObjWrapper;
class AXTreeSourceMus;
class View;
class Widget;

// Manages a tree of automation nodes for a mojo app outside the browser process
// (e.g. the keyboard shortcut viewer app).
class VIEWS_MUS_EXPORT AXRemoteHost : public ax::mojom::AXRemoteHost,
                                      public WidgetObserver,
                                      public AXAuraObjCache::Delegate {
 public:
  // Well-known tree ID for the remote client.
  // TODO(jamescook): Support different IDs for different clients.
  static constexpr int kRemoteAXTreeID = -2;

  AXRemoteHost();
  ~AXRemoteHost() override;

  // Initializes and adds ourself as a client of the host service.
  void Init(service_manager::Connector* connector);

  // Initializes with a fake host.
  void InitForTesting(ax::mojom::AXHostPtr host_ptr);

  // Sends the initial AX node tree to the host then starts monitoring for AX
  // events and tree changes.
  void StartMonitoringWidget(Widget* widget);
  void StopMonitoringWidget();

  // Handles an event fired upon a |view|.
  void HandleEvent(View* view, ax::mojom::Event event_type);

  // ax::mojom::AXRemoteHost:
  void OnAutomationEnabled(bool enabled) override;
  void PerformAction(const ui::AXActionData& action) override;

  // WidgetObserver:
  void OnWidgetDestroying(Widget* widget) override;

  // AXAuraObjCache::Delegate:
  void OnChildWindowRemoved(AXAuraObjWrapper* parent) override;
  void OnEvent(AXAuraObjWrapper* aura_obj,
               ax::mojom::Event event_type) override;

  void FlushForTesting();

 private:
  // Registers this object as a remote host for the parent AXHost.
  void BindAndSetRemote();

  void Enable();
  void Disable();

  // Sends an event to the host.
  void SendEvent(AXAuraObjWrapper* aura_obj, ax::mojom::Event event_type);

  // Accessibility host service in the browser.
  ax::mojom::AXHostPtr ax_host_ptr_;

  mojo::Binding<ax::mojom::AXRemoteHost> binding_{this};

  // Whether accessibility automation support is enabled.
  bool enabled_ = false;

  // Top-level widget being tracked.
  Widget* widget_ = nullptr;

  // Holds the active views-based accessibility tree. A tree consists of all
  // views descendant to a Widget's content area.
  std::unique_ptr<AXTreeSourceMus> tree_source_;

  // Serializes incremental updates on the currently active |tree_source_|.
  using AuraAXTreeSerializer =
      ui::AXTreeSerializer<AXAuraObjWrapper*, ui::AXNodeData, ui::AXTreeData>;
  std::unique_ptr<AuraAXTreeSerializer> tree_serializer_;

  DISALLOW_COPY_AND_ASSIGN(AXRemoteHost);
};

}  // namespace views

#endif  // UI_VIEWS_MUS_AX_REMOTE_HOST_H_
