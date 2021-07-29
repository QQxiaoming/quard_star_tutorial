/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#if defined(USE_OZONE)

#include "base/bind.h"
#include "ozone/platform_window_qt.h"
#include "ui/events/ozone/events_ozone.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

PlatformWindowQt::PlatformWindowQt(PlatformWindowDelegate* delegate, const gfx::Rect& bounds)
    : delegate_(delegate)
    , bounds_(bounds)
{
    ui::PlatformEventSource::GetInstance()->AddPlatformEventDispatcher(this);
}

PlatformWindowQt::~PlatformWindowQt()
{
    ui::PlatformEventSource::GetInstance()->RemovePlatformEventDispatcher(this);
}

gfx::Rect PlatformWindowQt::GetBounds()
{
    return bounds_;
}

void PlatformWindowQt::SetBounds(const gfx::Rect& bounds)
{
    if (bounds == bounds_)
        return;
    bounds_ = bounds;
    delegate_->OnBoundsChanged(bounds);
}

bool PlatformWindowQt::CanDispatchEvent(const ui::PlatformEvent& /*ne*/)
{
    return true;
}

uint32_t PlatformWindowQt::DispatchEvent(const ui::PlatformEvent& native_event)
{
    DispatchEventFromNativeUiEvent(
                native_event, base::Bind(&PlatformWindowDelegate::DispatchEvent,
                                         base::Unretained(delegate_)));

    return ui::POST_DISPATCH_STOP_PROPAGATION;
}

void PlatformWindowQt::PrepareForShutdown()
{
}

} // namespace ui

#endif // defined(USE_OZONE)
