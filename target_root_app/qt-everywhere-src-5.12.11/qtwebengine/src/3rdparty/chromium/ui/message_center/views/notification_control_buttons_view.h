
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_MESSAGE_CENTER_VIEWS_NOTIFICATION_CONTROL_BUTTONS_VIEW_H_
#define UI_MESSAGE_CENTER_VIEWS_NOTIFICATION_CONTROL_BUTTONS_VIEW_H_

#include "base/macros.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/message_center/message_center_export.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"

namespace ui {
class Event;
}

namespace views {
class Button;
}

namespace message_center {

class MessageView;
class PaddedButton;

class MESSAGE_CENTER_EXPORT NotificationControlButtonsView
    : public views::View,
      public views::ButtonListener {
 public:
  // String to be returned by GetClassName() method.
  static const char kViewClassName[];

  explicit NotificationControlButtonsView(MessageView* message_view);
  ~NotificationControlButtonsView() override;

  // Change the visibility of the close button. True to show, false to hide.
  void ShowCloseButton(bool show);
  // Change the visibility of the settings button. True to show, false to hide.
  void ShowSettingsButton(bool show);
  // Change the visibility of the settings button. True to show, false to hide.
  // Default: hidden.
  void ShowSnoozeButton(bool show);

  // Request the focus on the close button.
  void RequestFocusOnCloseButton();

  // Return the focus status of the close button. True if the focus is on the
  // close button, false otherwise.
  bool IsCloseButtonFocused() const;
  // Return the focus status of the settings button. True if the focus is on the
  // close button, false otherwise.
  bool IsSettingsButtonFocused() const;

  // Methods for retrieving the control buttons directly.
  views::Button* close_button() const;
  views::Button* settings_button() const;
  views::Button* snooze_button() const;

  // views::View
  const char* GetClassName() const override;
  void SetVisible(bool visible) override;

  // views::ButtonListener
  void ButtonPressed(views::Button* sender, const ui::Event& event) override;

 private:
  MessageView* message_view_;

  std::unique_ptr<PaddedButton> close_button_;
  std::unique_ptr<PaddedButton> settings_button_;
  std::unique_ptr<PaddedButton> snooze_button_;

  DISALLOW_COPY_AND_ASSIGN(NotificationControlButtonsView);
};

}  // namespace message_center

#endif  // UI_MESSAGE_CENTER_VIEWS_NOTIFICATION_CONTROL_BUTTONS_VIEW_H_
