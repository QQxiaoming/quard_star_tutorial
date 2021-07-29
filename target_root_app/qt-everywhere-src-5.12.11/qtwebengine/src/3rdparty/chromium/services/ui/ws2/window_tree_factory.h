// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_WS2_WINDOW_TREE_FACTORY_H_
#define SERVICES_UI_WS2_WINDOW_TREE_FACTORY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/component_export.h"
#include "base/macros.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/ui/public/interfaces/window_tree.mojom.h"

namespace ui {
namespace ws2 {

class WindowService;
class WindowTreeBinding;

// Implementation of mojom::WindowTreeFactory. This creates a WindowTreeBinding
// for each request for a WindowTree. Any WindowTreeBindings created by
// WindowTreeFactory are owned by the WindowTreeFactory.
class COMPONENT_EXPORT(WINDOW_SERVICE) WindowTreeFactory
    : public mojom::WindowTreeFactory {
 public:
  explicit WindowTreeFactory(WindowService* window_service);
  ~WindowTreeFactory() override;

  // |client_name| is the name of the client requesting the factory.
  void AddBinding(mojom::WindowTreeFactoryRequest request,
                  const std::string& client_name);

  // mojom::WindowTreeFactory:
  void CreateWindowTree(mojom::WindowTreeRequest tree_request,
                        mojom::WindowTreeClientPtr client) override;

 private:
  void OnLostConnectionToClient(WindowTreeBinding* binding);

  using WindowTreeBindings = std::vector<std::unique_ptr<WindowTreeBinding>>;

  WindowService* window_service_;

  // The |string| parameter is the name of the client that created by binding.
  mojo::BindingSet<mojom::WindowTreeFactory, std::string> bindings_;

  WindowTreeBindings window_tree_bindings_;

  DISALLOW_COPY_AND_ASSIGN(WindowTreeFactory);
};

}  // namespace ws2
}  // namespace ui

#endif  // SERVICES_UI_WS2_WINDOW_TREE_FACTORY_H_
