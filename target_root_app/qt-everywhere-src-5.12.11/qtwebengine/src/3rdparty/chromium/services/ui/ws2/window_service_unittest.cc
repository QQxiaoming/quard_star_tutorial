// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/window_service.h"

#include <memory>

#include "base/run_loop.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/cpp/test/test_connector_factory.h"
#include "services/ui/public/interfaces/constants.mojom.h"
#include "services/ui/public/interfaces/window_tree.mojom.h"
#include "services/ui/ws2/gpu_interface_provider.h"
#include "services/ui/ws2/window_service_test_setup.h"
#include "services/ui/ws2/window_tree.h"
#include "services/ui/ws2/window_tree_test_helper.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ui {
namespace ws2 {

TEST(WindowServiceTest, DeleteWithClients) {
  // Use |test_setup| to configure aura and other state.
  WindowServiceTestSetup test_setup;

  // Create another WindowService.
  TestWindowServiceDelegate test_window_service_delegate;
  std::unique_ptr<WindowService> window_service_ptr =
      std::make_unique<WindowService>(&test_window_service_delegate, nullptr,
                                      test_setup.focus_controller());
  WindowService* window_service = window_service_ptr.get();
  std::unique_ptr<service_manager::TestConnectorFactory> factory =
      service_manager::TestConnectorFactory::CreateForUniqueService(
          std::move(window_service_ptr));
  std::unique_ptr<service_manager::Connector> connector =
      factory->CreateConnector();

  // Connect to |window_service| and ask for a new WindowTree.
  ui::mojom::WindowTreeFactoryPtr window_tree_factory;
  connector->BindInterface(ui::mojom::kServiceName, &window_tree_factory);
  ui::mojom::WindowTreePtr window_tree;
  ui::mojom::WindowTreeClientPtr client;
  mojom::WindowTreeClientRequest client_request = MakeRequest(&client);
  window_tree_factory->CreateWindowTree(MakeRequest(&window_tree),
                                        std::move(client));

  // Use FlushForTesting() to ensure WindowService processes the request.
  window_tree_factory.FlushForTesting();

  // There should be at least one WindowTree.
  EXPECT_FALSE(window_service->window_trees().empty());

  // Destroying the |window_service| should remove all the WindowTrees and
  // ensure a DCHECK isn't hit in ~WindowTree.
}

// Test client ids assigned to window trees that connect to the window service.
TEST(WindowServiceTest, ClientIds) {
  // Use |test_setup| to configure aura and other state.
  WindowServiceTestSetup test_setup;

  // Create another WindowService.
  TestWindowServiceDelegate test_window_service_delegate;
  WindowService window_service(&test_window_service_delegate, nullptr,
                               test_setup.focus_controller());

  // The first window tree should have the initial client id.
  auto tree = window_service.CreateWindowTree(nullptr);
  EXPECT_EQ(kInitialClientId, WindowTreeTestHelper(tree.get()).client_id());

  // The second window tree should have an incremented client id.
  tree = window_service.CreateWindowTree(nullptr);
  EXPECT_EQ(kInitialClientId + 1, WindowTreeTestHelper(tree.get()).client_id());
}

// Test client ids assigned to window trees in the decrementing mode.
TEST(WindowServiceTest, ClientIdsDecrement) {
  // Use |test_setup| to configure aura and other state.
  WindowServiceTestSetup test_setup;

  // Create another WindowService that decrements window ids.
  const bool decrement = true;
  TestWindowServiceDelegate test_window_service_delegate;
  WindowService window_service(&test_window_service_delegate, nullptr,
                               test_setup.focus_controller(), decrement);

  // The first window tree should have the initial decrementing client id.
  auto tree = window_service.CreateWindowTree(nullptr);
  EXPECT_EQ(kInitialClientIdDecrement,
            WindowTreeTestHelper(tree.get()).client_id());

  // The second window tree should have a decremented client id.
  tree = window_service.CreateWindowTree(nullptr);
  EXPECT_EQ(kInitialClientIdDecrement - 1,
            WindowTreeTestHelper(tree.get()).client_id());
}

}  // namespace ws2
}  // namespace ui
