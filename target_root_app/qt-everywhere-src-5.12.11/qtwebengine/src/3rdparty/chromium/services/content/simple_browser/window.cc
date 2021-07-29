// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/content/simple_browser/window.h"

#include <memory>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "services/content/public/cpp/navigable_contents.h"
#include "services/content/public/mojom/constants.mojom.h"
#include "services/content/public/mojom/navigable_contents_factory.mojom.h"
#include "services/service_manager/public/cpp/connector.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace simple_browser {

namespace {

class SimpleBrowserUI : public views::WidgetDelegateView,
                        public views::TextfieldController {
 public:
  explicit SimpleBrowserUI(service_manager::Connector* connector)
      : connector_(connector), location_bar_(new views::Textfield) {
    SetBackground(views::CreateStandardPanelBackground());
    location_bar_->set_controller(this);
    AddChildView(location_bar_);

    connector_->BindInterface(content::mojom::kServiceName,
                              MakeRequest(&navigable_contents_factory_));
    navigable_contents_ = std::make_unique<content::NavigableContents>(
        navigable_contents_factory_.get());
    content_view_ = navigable_contents_->GetView();
    content_view_->SetBorder(views::CreateSolidBorder(2, SK_ColorGREEN));
    AddChildView(content_view_);
  }

  ~SimpleBrowserUI() override = default;

 private:
  // views::WidgetDelegate:
  base::string16 GetWindowTitle() const override {
    return base::ASCIIToUTF16("Simple Browser");
  }

  // views::View:
  void Layout() override {
    gfx::Rect location_bar_bounds{GetLocalBounds().width(), 20};
    location_bar_bounds.Inset(5, 0);
    location_bar_->SetBoundsRect(location_bar_bounds);

    gfx::Rect content_view_bounds = GetLocalBounds();
    content_view_bounds.Inset(5, 25, 5, 5);
    content_view_->SetBoundsRect(content_view_bounds);
  }

  gfx::Size CalculatePreferredSize() const override {
    return gfx::Size(640, 480);
  }

  // views::TextFieldController:
  bool HandleKeyEvent(views::Textfield* sender,
                      const ui::KeyEvent& key_event) override {
    if (key_event.type() != ui::ET_KEY_PRESSED)
      return false;

    if (key_event.key_code() == ui::VKEY_RETURN) {
      navigable_contents_->Navigate(
          GURL(base::UTF16ToUTF8(location_bar_->text())));
    }

    return false;
  }

  service_manager::Connector* const connector_;

  content::mojom::NavigableContentsFactoryPtr navigable_contents_factory_;
  std::unique_ptr<content::NavigableContents> navigable_contents_;

  views::Textfield* location_bar_;
  views::View* content_view_;

  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserUI);
};

}  // namespace

Window::Window(service_manager::Connector* connector) {
  window_widget_ = views::Widget::CreateWindowWithContextAndBounds(
      new SimpleBrowserUI(connector), nullptr, gfx::Rect(10, 640, 0, 0));
  window_widget_->GetNativeWindow()->GetHost()->window()->SetName(
      "SimpleBrowser");
  window_widget_->Show();
}

Window::~Window() = default;

}  // namespace simple_browser
