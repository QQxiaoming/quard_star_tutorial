/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PLATFORM_WINDOW_QT_H
#define PLATFORM_WINDOW_QT_H

#if defined(USE_OZONE)

#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/platform_window/platform_window.h"

namespace ui {

class PlatformWindowDelegate;

class PlatformWindowQt : public PlatformWindow, public PlatformEventDispatcher
{
public:
    PlatformWindowQt(PlatformWindowDelegate* delegate, const gfx::Rect& bounds);
    ~PlatformWindowQt() override;
    // PlatformWindow:
    gfx::Rect GetBounds() override;
    void SetBounds(const gfx::Rect& bounds) override;
    void Show() override { }
    void Hide() override { }
    void Close() override { }
    void SetTitle(const base::string16&) override { }
    void SetCapture() override { }
    void ReleaseCapture() override { }
    bool HasCapture() const override { return false; }
    void ToggleFullscreen() override { }
    void Maximize() override { }
    void Minimize() override { }
    void Restore() override { }
    PlatformWindowState GetPlatformWindowState() const override { return PLATFORM_WINDOW_STATE_UNKNOWN; }
    void SetCursor(PlatformCursor) override { }
    void MoveCursorTo(const gfx::Point&) override { }
    void ConfineCursorToBounds(const gfx::Rect&) override { }
    PlatformImeController* GetPlatformImeController() override { return nullptr; }
    // PlatformEventDispatcher:
    bool CanDispatchEvent(const PlatformEvent& event) override;
    uint32_t DispatchEvent(const PlatformEvent& event) override;
    void PrepareForShutdown() override;

private:
    PlatformWindowDelegate* delegate_;
    gfx::Rect bounds_;

    DISALLOW_COPY_AND_ASSIGN(PlatformWindowQt);
};

}

#endif // defined(USE_OZONE)
#endif //PLATFORM_WINDOW_QT_H
