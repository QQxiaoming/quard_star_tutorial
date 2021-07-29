// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_CONTENT_SIMPLE_BROWSER_WINDOW_H_
#define SERVICES_CONTENT_SIMPLE_BROWSER_WINDOW_H_

#include "base/macros.h"

namespace service_manager {
class Connector;
}

namespace views {
class Widget;
}

namespace simple_browser {

class Window {
 public:
  explicit Window(service_manager::Connector* connector);
  ~Window();

 private:
  views::Widget* window_widget_;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace simple_browser

#endif  // SERVICES_CONTENT_SIMPLE_BROWSER_WINDOW_H_
