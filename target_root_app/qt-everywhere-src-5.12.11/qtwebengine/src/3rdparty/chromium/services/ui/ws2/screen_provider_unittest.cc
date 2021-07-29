// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws2/screen_provider.h"

#include <stdint.h>

#include "base/run_loop.h"
#include "services/service_manager/public/cpp/test/test_connector_factory.h"
#include "services/ui/public/interfaces/constants.mojom.h"
#include "services/ui/public/interfaces/screen_provider.mojom.h"
#include "services/ui/public/interfaces/window_tree.mojom.h"
#include "services/ui/ws2/gpu_interface_provider.h"
#include "services/ui/ws2/test_screen_provider_observer.h"
#include "services/ui/ws2/window_service.h"
#include "services/ui/ws2/window_service_test_setup.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/display/display.h"
#include "ui/display/screen_base.h"

using display::Display;
using display::DisplayList;
using display::DisplayObserver;

namespace ui {
namespace ws2 {
namespace {

// A testing screen that generates the OnDidProcessDisplayChanges() notification
// similar to production code.
class TestScreen : public display::ScreenBase {
 public:
  TestScreen() { display::Screen::SetScreenInstance(this); }

  ~TestScreen() override { display::Screen::SetScreenInstance(nullptr); }

  void AddDisplay(const Display& display, DisplayList::Type type) {
    display_list().AddDisplay(display, type);
    for (DisplayObserver& observer : *display_list().observers())
      observer.OnDidProcessDisplayChanges();
  }

  void UpdateDisplay(const Display& display, DisplayList::Type type) {
    display_list().UpdateDisplay(display, type);
    for (DisplayObserver& observer : *display_list().observers())
      observer.OnDidProcessDisplayChanges();
  }

  void RemoveDisplay(int64_t display_id) {
    display_list().RemoveDisplay(display_id);
    for (DisplayObserver& observer : *display_list().observers())
      observer.OnDidProcessDisplayChanges();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(TestScreen);
};

TEST(ScreenProviderTest, AddRemoveDisplay) {
  TestScreen screen;
  screen.AddDisplay(Display(111, gfx::Rect(0, 0, 640, 480)),
                    DisplayList::Type::PRIMARY);
  Display::SetInternalDisplayId(111);

  ScreenProvider screen_provider;
  TestScreenProviderObserver observer;

  // Adding an observer triggers an update.
  screen_provider.AddObserver(&observer);
  EXPECT_EQ("111", observer.display_ids());
  EXPECT_EQ(111, observer.primary_display_id());
  EXPECT_EQ(111, observer.internal_display_id());
  observer.display_ids().clear();

  // Adding a display triggers an update.
  screen.AddDisplay(Display(222, gfx::Rect(640, 0, 640, 480)),
                    DisplayList::Type::NOT_PRIMARY);
  EXPECT_EQ("111 222", observer.display_ids());
  EXPECT_EQ(111, observer.primary_display_id());
  observer.display_ids().clear();

  // Updating which display is primary triggers an update.
  screen.UpdateDisplay(Display(222, gfx::Rect(640, 0, 800, 600)),
                       DisplayList::Type::PRIMARY);
  EXPECT_EQ("111 222", observer.display_ids());
  EXPECT_EQ(222, observer.primary_display_id());
  observer.display_ids().clear();

  // Removing a display triggers an update.
  screen.RemoveDisplay(111);
  EXPECT_EQ("222", observer.display_ids());
  EXPECT_EQ(222, observer.primary_display_id());
}

TEST(ScreenProviderTest, SetFrameDecorationValues) {
  // Set up a single display.
  TestScreen screen;
  screen.AddDisplay(Display(111, gfx::Rect(0, 0, 640, 480)),
                    DisplayList::Type::PRIMARY);

  // Set up custom frame decoration values.
  ScreenProvider screen_provider;
  screen_provider.SetFrameDecorationValues(gfx::Insets(1, 2, 3, 4), 55u);

  // Add an observer to the screen provider.
  TestScreenProviderObserver observer;
  screen_provider.AddObserver(&observer);

  // The screen information contains the frame decoration values.
  ASSERT_EQ(1u, observer.displays().size());
  const mojom::FrameDecorationValuesPtr& values =
      observer.displays()[0]->frame_decoration_values;
  EXPECT_EQ(gfx::Insets(1, 2, 3, 4), values->normal_client_area_insets);
  EXPECT_EQ(gfx::Insets(1, 2, 3, 4), values->maximized_client_area_insets);
  EXPECT_EQ(55u, values->max_title_bar_button_width);
}

TEST(ScreenProviderTest, DisplaysSentOnConnection) {
  // Use |test_setup| to configure aura and other state.
  WindowServiceTestSetup test_setup;

  // Create another WindowService.
  TestWindowServiceDelegate test_window_service_delegate;
  std::unique_ptr<WindowService> window_service_ptr =
      std::make_unique<WindowService>(&test_window_service_delegate, nullptr,
                                      test_setup.focus_controller());
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

  TestWindowTreeClient window_tree_client;
  mojo::Binding<mojom::WindowTreeClient> binding(&window_tree_client);
  binding.Bind(std::move(client_request));

  // Wait for all mojo messages to be processed.
  base::RunLoop().RunUntilIdle();

  // Should have gotten display information.
  EXPECT_FALSE(
      window_tree_client.screen_provider_observer()->display_ids().empty());
}

}  // namespace
}  // namespace ws2
}  // namespace ui
