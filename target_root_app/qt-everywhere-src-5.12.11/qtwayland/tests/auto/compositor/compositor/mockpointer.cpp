/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockpointer.h"

static void pointerEnter(void *pointer, struct wl_pointer *wlPointer, uint serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(wlPointer);
    Q_UNUSED(serial);
    Q_UNUSED(x);
    Q_UNUSED(y);

    static_cast<MockPointer *>(pointer)->m_enteredSurface = surface;
}

static void pointerLeave(void *pointer, struct wl_pointer *wlPointer, uint32_t serial, struct wl_surface *surface)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(serial);

    Q_ASSERT(surface);

    static_cast<MockPointer *>(pointer)->m_enteredSurface = nullptr;
}

static void pointerMotion(void *pointer, struct wl_pointer *wlPointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(time);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

static void pointerButton(void *pointer, struct wl_pointer *wlPointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    Q_UNUSED(button);
    Q_UNUSED(state);
}

static void pointerAxis(void *pointer, struct wl_pointer *wlPointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    Q_UNUSED(pointer);
    Q_UNUSED(wlPointer);
    Q_UNUSED(time);
    Q_UNUSED(axis);
    Q_UNUSED(value);
}

static const struct wl_pointer_listener pointerListener = {
    pointerEnter,
    pointerLeave,
    pointerMotion,
    pointerButton,
    pointerAxis,
};

MockPointer::MockPointer(wl_seat *seat)
    : m_pointer(wl_seat_get_pointer(seat))
{
    wl_pointer_add_listener(m_pointer, &pointerListener, this);
}

MockPointer::~MockPointer()
{
    wl_pointer_destroy(m_pointer);
}
