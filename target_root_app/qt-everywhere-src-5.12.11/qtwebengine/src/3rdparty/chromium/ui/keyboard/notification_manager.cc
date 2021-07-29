// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/keyboard/notification_manager.h"
#include "base/observer_list.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/keyboard/keyboard_controller_observer.h"

namespace keyboard {

template <typename T>
bool ValueNotificationConsolidator<T>::ShouldSendNotification(
    const T new_value) {
  if (never_sent_) {
    value_ = new_value;
    never_sent_ = false;
    return true;
  }
  const bool value_changed = new_value != value_;
  if (value_changed) {
    value_ = new_value;
  }
  return value_changed;
}

NotificationManager::NotificationManager() {}

void NotificationManager::SendNotifications(
    bool bounds_affect_layout,
    bool is_locked,
    const gfx::Rect& visual_bounds,
    const gfx::Rect& occluded_bounds,
    const base::ObserverList<KeyboardControllerObserver>& observers) {
  bool is_visible = !visual_bounds.IsEmpty();
  bool send_visibility_notification =
      ShouldSendVisibilityNotification(is_visible);

  bool send_visual_bounds_notification =
      ShouldSendVisualBoundsNotification(visual_bounds);

  bool send_occluded_bounds_notification =
      ShouldSendOccludedBoundsNotification(occluded_bounds);

  const gfx::Rect workspace_layout_offset_region =
      bounds_affect_layout ? visual_bounds : gfx::Rect();
  bool send_displaced_bounds_notification =
      ShouldSendWorkspaceDisplacementBoundsNotification(
          workspace_layout_offset_region);

  KeyboardStateDescriptor state;
  state.is_visible = is_visible;
  state.is_locked = is_locked;
  state.visual_bounds = visual_bounds;
  state.occluded_bounds = occluded_bounds;
  state.displaced_bounds = workspace_layout_offset_region;

  for (KeyboardControllerObserver& observer : observers) {
    if (send_visibility_notification)
      observer.OnKeyboardVisibilityStateChanged(is_visible);

    if (send_visual_bounds_notification)
      observer.OnKeyboardVisibleBoundsChanged(visual_bounds);

    if (send_occluded_bounds_notification)
      observer.OnKeyboardWorkspaceOccludedBoundsChanged(occluded_bounds);

    if (send_displaced_bounds_notification) {
      observer.OnKeyboardWorkspaceDisplacingBoundsChanged(
          workspace_layout_offset_region);
    }

    observer.OnKeyboardAppearanceChanged(state);
  }
}

bool NotificationManager::ShouldSendVisibilityNotification(
    bool current_visibility) {
  return visibility_.ShouldSendNotification(current_visibility);
}

bool NotificationManager::ShouldSendVisualBoundsNotification(
    const gfx::Rect& new_bounds) {
  return visual_bounds_.ShouldSendNotification(
      CanonicalizeEmptyRectangles(new_bounds));
}

bool NotificationManager::ShouldSendOccludedBoundsNotification(
    const gfx::Rect& new_bounds) {
  return occluded_bounds_.ShouldSendNotification(
      CanonicalizeEmptyRectangles(new_bounds));
}

bool NotificationManager::ShouldSendWorkspaceDisplacementBoundsNotification(
    const gfx::Rect& new_bounds) {
  return workspace_displaced_bounds_.ShouldSendNotification(
      CanonicalizeEmptyRectangles(new_bounds));
}

gfx::Rect NotificationManager::CanonicalizeEmptyRectangles(
    const gfx::Rect& rect) const {
  if (rect.IsEmpty()) {
    return gfx::Rect();
  }
  return rect;
}

}  // namespace keyboard
