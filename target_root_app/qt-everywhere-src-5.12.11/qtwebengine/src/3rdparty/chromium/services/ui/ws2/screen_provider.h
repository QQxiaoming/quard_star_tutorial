// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_WS2_SCREEN_PROVIDER_H_
#define SERVICES_UI_WS2_SCREEN_PROVIDER_H_

#include <stdint.h>

#include "base/component_export.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "services/ui/public/interfaces/screen_provider.mojom.h"
#include "services/ui/public/interfaces/window_tree_constants.mojom.h"
#include "ui/display/display_observer.h"
#include "ui/gfx/geometry/insets.h"

namespace display {
class Display;
}

namespace ui {
namespace ws2 {

// Provides information about displays to window service clients.
// display::Screen must outlive this object. Exported for test.
class COMPONENT_EXPORT(WINDOW_SERVICE) ScreenProvider
    : public display::DisplayObserver {
 public:
  ScreenProvider();
  ~ScreenProvider() override;

  void AddObserver(mojom::ScreenProviderObserver* observer);
  void RemoveObserver(mojom::ScreenProviderObserver* observer);

  // Sets the window frame metrics.
  void SetFrameDecorationValues(const gfx::Insets& client_area_insets,
                                int max_title_bar_button_width);

  // See comment in WindowService as to why this is special cased.
  void DisplayMetricsChanged(const display::Display& display,
                             uint32_t changed_metrics);

  // display::DisplayObserver:
  void OnDidProcessDisplayChanges() override;

 private:
  void NotifyAllObservers();

  void NotifyObserver(mojom::ScreenProviderObserver* observer);

  std::vector<mojom::WsDisplayPtr> GetAllDisplays();

  // Returns the window frame metrics as a mojo struct.
  mojom::FrameDecorationValuesPtr GetFrameDecorationValues();

  // See mojom::FrameDecorationValuesPtr documentation.
  gfx::Insets client_area_insets_;
  int max_title_bar_button_width_ = 0;

  base::ObserverList<mojom::ScreenProviderObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(ScreenProvider);
};

}  // namespace ws2
}  // namespace ui

#endif  // SERVICES_UI_WS2_SCREEN_PROVIDER_H_
