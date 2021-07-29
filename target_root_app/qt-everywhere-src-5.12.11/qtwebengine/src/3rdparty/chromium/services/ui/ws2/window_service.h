// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_WS2_WINDOW_SERVICE_H_
#define SERVICES_UI_WS2_WINDOW_SERVICE_H_

#include <stdint.h>

#include <memory>
#include <set>
#include <string>

#include "base/component_export.h"
#include "base/macros.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/service.h"
#include "services/ui/ime/ime_driver_bridge.h"
#include "services/ui/ime/ime_registrar_impl.h"
#include "services/ui/input_devices/input_device_server.h"
#include "services/ui/public/interfaces/ime/ime.mojom.h"
#include "services/ui/public/interfaces/remoting_event_injector.mojom.h"
#include "services/ui/public/interfaces/user_activity_monitor.mojom.h"
#include "services/ui/public/interfaces/window_server_test.mojom.h"
#include "services/ui/public/interfaces/window_tree.mojom.h"
#include "services/ui/ws2/ids.h"
#include "ui/aura/mus/property_converter.h"
#include "ui/base/mojo/clipboard.mojom.h"

namespace aura {
class Window;
namespace client {
class FocusClient;
}
}

namespace display {
class Display;
}

namespace gfx {
class Insets;
}

namespace ui {

class ClipboardHost;

namespace mojom {
class WindowTreeClient;
}

namespace ws2 {

class GpuInterfaceProvider;
class RemotingEventInjector;
class ScreenProvider;
class ServerWindow;
class UserActivityMonitor;
class WindowServiceDelegate;
class WindowTree;
class WindowTreeFactory;

// WindowService is the entry point into providing an implementation of
// the ui::mojom::WindowTree related mojoms on top of an aura Window hierarchy.
// A WindowTree is created for each client.
class COMPONENT_EXPORT(WINDOW_SERVICE) WindowService
    : public service_manager::Service {
 public:
  WindowService(WindowServiceDelegate* delegate,
                std::unique_ptr<GpuInterfaceProvider> gpu_support,
                aura::client::FocusClient* focus_client,
                bool decrement_client_ids = false);
  ~WindowService() override;

  // Gets the ServerWindow for |window|, creating if necessary.
  ServerWindow* GetServerWindowForWindowCreateIfNecessary(aura::Window* window);

  // Creates a new WindowTree, caller must call one of the Init() functions on
  // the returned object.
  std::unique_ptr<WindowTree> CreateWindowTree(
      mojom::WindowTreeClient* window_tree_client,
      const std::string& client_name = std::string());

  // Sets the window frame metrics.
  void SetFrameDecorationValues(const gfx::Insets& client_area_insets,
                                int max_title_bar_button_width);

  // Whether |window| hosts a remote client.
  static bool HasRemoteClient(const aura::Window* window);

  WindowServiceDelegate* delegate() { return delegate_; }

  aura::PropertyConverter* property_converter() { return &property_converter_; }

  aura::client::FocusClient* focus_client() { return focus_client_; }

  const std::set<WindowTree*>& window_trees() const { return window_trees_; }

  service_manager::BinderRegistry* registry() { return &registry_; }

  // Called when the surface of |client_name| is first activated.
  void OnFirstSurfaceActivation(const std::string& client_name);

  // Called when a WindowServiceClient is about to be destroyed.
  void OnWillDestroyWindowTree(WindowTree* tree);

  // Asks the client that created |window| to close |window|. |window| must be
  // a top-level window.
  void RequestClose(aura::Window* window);

  // Called when the metrics of a display changes. It is expected the local
  // environment call this *before* the change is applied to any
  // WindowTreeHosts. This is necessary to ensure clients are notified of the
  // change before the client is notified of a bounds change. To do otherwise
  // results in the client applying bounds change with the wrong scale factor.
  // |changed_metrics| is a bitmask of the DisplayObserver::DisplayMetric.
  void OnDisplayMetricsChanged(const display::Display& display,
                               uint32_t changed_metrics);

  // Returns an id useful for debugging. See ServerWindow::GetIdForDebugging()
  // for details.
  std::string GetIdForDebugging(aura::Window* window);

  ScreenProvider* screen_provider() { return screen_provider_.get(); }

  // service_manager::Service:
  void OnStart() override;
  void OnBindInterface(const service_manager::BindSourceInfo& remote_info,
                       const std::string& interface_name,
                       mojo::ScopedMessagePipeHandle handle) override;

 private:
  friend class WindowServerTestImpl;

  // Sets a callback to be called whenever a surface is activated. This
  // corresponds to a client submitting a new CompositorFrame for a Window. This
  // should only be called in a test configuration.
  void SetSurfaceActivationCallback(
      base::OnceCallback<void(const std::string&)> callback);

  void BindClipboardHostRequest(mojom::ClipboardHostRequest request);
  void BindImeRegistrarRequest(mojom::IMERegistrarRequest request);
  void BindImeDriverRequest(mojom::IMEDriverRequest request);
  void BindInputDeviceServerRequest(mojom::InputDeviceServerRequest request);
  void BindRemotingEventInjectorRequest(
      mojom::RemotingEventInjectorRequest request);
  void BindUserActivityMonitorRequest(
      mojom::UserActivityMonitorRequest request);
  void BindWindowServerTestRequest(mojom::WindowServerTestRequest request);

  void BindWindowTreeFactoryRequest(
      mojom::WindowTreeFactoryRequest request,
      const service_manager::BindSourceInfo& source_info);

  WindowServiceDelegate* delegate_;

  // GpuInterfaceProvider may be null in tests.
  std::unique_ptr<GpuInterfaceProvider> gpu_interface_provider_;

  std::unique_ptr<ScreenProvider> screen_provider_;

  aura::client::FocusClient* focus_client_;

  service_manager::BinderRegistryWithArgs<
      const service_manager::BindSourceInfo&>
      registry_with_source_info_;
  service_manager::BinderRegistry registry_;

  std::unique_ptr<UserActivityMonitor> user_activity_monitor_;

  std::unique_ptr<WindowTreeFactory> window_tree_factory_;

  // Helper used to serialize and deserialize window properties.
  aura::PropertyConverter property_converter_;

  // Provides info to InputDeviceClient users, via InputDeviceManager.
  ui::InputDeviceServer input_device_server_;

  std::unique_ptr<RemotingEventInjector> remoting_event_injector_;

  std::unique_ptr<ClipboardHost> clipboard_host_;

  // Id for the next WindowTree.
  ClientSpecificId next_client_id_;

  // If true, client ids are decremented, not incremented; the default is false.
  const bool decrement_client_ids_;

  // Id used for the next window created locally that is exposed to clients.
  ClientSpecificId next_window_id_ = 1;

  IMERegistrarImpl ime_registrar_;
  IMEDriverBridge ime_driver_;

  // All WindowTrees created by the WindowService.
  std::set<WindowTree*> window_trees_;

  // Returns true if various test interfaces are exposed.
  bool test_config_ = false;

  base::OnceCallback<void(const std::string&)> surface_activation_callback_;

  DISALLOW_COPY_AND_ASSIGN(WindowService);
};

}  // namespace ws2
}  // namespace ui

#endif  // SERVICES_UI_WS2_WINDOW_SERVICE_H_
