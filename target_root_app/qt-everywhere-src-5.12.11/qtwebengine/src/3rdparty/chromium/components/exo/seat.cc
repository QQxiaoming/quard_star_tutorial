// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/exo/seat.h"

#include "ash/shell.h"
#include "base/auto_reset.h"
#include "base/strings/utf_string_conversions.h"
#include "components/exo/data_source.h"
#include "components/exo/keyboard.h"
#include "components/exo/shell_surface.h"
#include "components/exo/surface.h"
#include "components/exo/wm_helper.h"
#include "ui/aura/client/focus_client.h"
#include "ui/base/clipboard/clipboard_monitor.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#include "ui/events/event_utils.h"
#include "ui/events/platform/platform_event_source.h"

namespace exo {
namespace {

Surface* GetEffectiveFocus(aura::Window* window) {
  if (!window)
    return nullptr;
  Surface* const surface = Surface::AsSurface(window);
  if (surface)
    return surface;
  // Fallback to main surface.
  aura::Window* const top_level_window = window->GetToplevelWindow();
  if (!top_level_window)
    return nullptr;
  return ShellSurface::GetMainSurface(top_level_window);
}

}  // namespace

Seat::Seat() : changing_clipboard_data_to_selection_source_(false) {
  aura::client::GetFocusClient(ash::Shell::Get()->GetPrimaryRootWindow())
      ->AddObserver(this);
  // Prepend handler as it's critical that we see all events.
  WMHelper::GetInstance()->PrependPreTargetHandler(this);
  ui::ClipboardMonitor::GetInstance()->AddObserver(this);
  // TODO(reveman): Need to handle the mus case where PlatformEventSource is
  // null. https://crbug.com/856230
  if (ui::PlatformEventSource::GetInstance())
    ui::PlatformEventSource::GetInstance()->AddPlatformEventObserver(this);
}

Seat::~Seat() {
  DCHECK(!selection_source_) << "DataSource must be released before Seat";
  aura::client::GetFocusClient(ash::Shell::Get()->GetPrimaryRootWindow())
      ->RemoveObserver(this);
  WMHelper::GetInstance()->RemovePreTargetHandler(this);
  ui::ClipboardMonitor::GetInstance()->RemoveObserver(this);
  if (ui::PlatformEventSource::GetInstance())
    ui::PlatformEventSource::GetInstance()->RemovePlatformEventObserver(this);
}

void Seat::AddObserver(SeatObserver* observer) {
  observers_.AddObserver(observer);
}

void Seat::RemoveObserver(SeatObserver* observer) {
  observers_.RemoveObserver(observer);
}

Surface* Seat::GetFocusedSurface() {
  return GetEffectiveFocus(WMHelper::GetInstance()->GetFocusedWindow());
}

void Seat::SetSelection(DataSource* source) {
  if (!source) {
    ui::Clipboard::GetForCurrentThread()->Clear(ui::CLIPBOARD_TYPE_COPY_PASTE);
    // selection_source_ is Cancelled() and reset() in OnClipboardDataChanged().
    return;
  }

  if (selection_source_) {
    if (selection_source_->get() == source)
      return;
    selection_source_->get()->Cancelled();
  }
  selection_source_ = std::make_unique<ScopedDataSource>(source, this);

  // Unretained is safe as Seat always outlives DataSource.
  source->ReadData(base::Bind(&Seat::OnDataRead, base::Unretained(this)));
}

void Seat::OnDataRead(const std::vector<uint8_t>& data) {
  base::AutoReset<bool> auto_reset(
      &changing_clipboard_data_to_selection_source_, true);

  ui::ScopedClipboardWriter writer(ui::CLIPBOARD_TYPE_COPY_PASTE);
  writer.WriteText(base::UTF8ToUTF16(base::StringPiece(
      reinterpret_cast<const char*>(data.data()), data.size())));
}

////////////////////////////////////////////////////////////////////////////////
// aura::client::FocusChangeObserver overrides:

void Seat::OnWindowFocused(aura::Window* gained_focus,
                           aura::Window* lost_focus) {
  Surface* const surface = GetEffectiveFocus(gained_focus);
  for (auto& observer : observers_) {
    observer.OnSurfaceFocusing(surface);
  }
  for (auto& observer : observers_) {
    observer.OnSurfaceFocused(surface);
  }
}

////////////////////////////////////////////////////////////////////////////////
// ui::PlatformEventObserver overrides:

void Seat::WillProcessEvent(const ui::PlatformEvent& event) {
  switch (ui::EventTypeFromNative(event)) {
    case ui::ET_KEY_PRESSED:
    case ui::ET_KEY_RELEASED:
      physical_code_for_currently_processing_event_ = ui::CodeFromNative(event);
      break;
    default:
      break;
  }
}

void Seat::DidProcessEvent(const ui::PlatformEvent& event) {
  switch (ui::EventTypeFromNative(event)) {
    case ui::ET_KEY_PRESSED:
    case ui::ET_KEY_RELEASED:
      physical_code_for_currently_processing_event_ = ui::DomCode::NONE;
      break;
    default:
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
// ui::EventHandler overrides:

void Seat::OnKeyEvent(ui::KeyEvent* event) {
  // Ignore synthetic key repeat events.
  if (event->is_repeat())
    return;
  if (physical_code_for_currently_processing_event_ != ui::DomCode::NONE) {
    switch (event->type()) {
      case ui::ET_KEY_PRESSED:
        pressed_keys_.insert(
            {physical_code_for_currently_processing_event_, event->code()});
        break;
      case ui::ET_KEY_RELEASED:
        pressed_keys_.erase(physical_code_for_currently_processing_event_);
        break;
      default:
        NOTREACHED();
        break;
    }
  }
  modifier_flags_ = event->flags();
}

////////////////////////////////////////////////////////////////////////////////
// ui::ClipboardObserver overrides:

void Seat::OnClipboardDataChanged() {
  if (!selection_source_ || changing_clipboard_data_to_selection_source_)
    return;
  selection_source_->get()->Cancelled();
  selection_source_.reset();
}

////////////////////////////////////////////////////////////////////////////////
// DataSourceObserver overrides:

void Seat::OnDataSourceDestroying(DataSource* source) {
  if (selection_source_ && selection_source_->get() == source)
    selection_source_.reset();
}

}  // namespace exo
