/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "mockkeyboard.h"

void keyboardKeymap(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(format);
    Q_UNUSED(fd);
    Q_UNUSED(size);
}

void keyboardEnter(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(surface);
    Q_UNUSED(keys);

    static_cast<MockKeyboard *>(keyboard)->m_enteredSurface = surface;
}

void keyboardLeave(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface)
{
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(surface);

    static_cast<MockKeyboard *>(keyboard)->m_enteredSurface = nullptr;
}

void keyboardKey(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(time);
    Q_UNUSED(key);
    Q_UNUSED(state);
    auto kb = static_cast<MockKeyboard *>(keyboard);
    kb->m_lastKeyCode = key;
    kb->m_lastKeyState = state;
}

void keyboardModifiers(void *keyboard, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    Q_UNUSED(keyboard);
    Q_UNUSED(wl_keyboard);
    Q_UNUSED(serial);
    Q_UNUSED(mods_depressed);
    Q_UNUSED(mods_latched);
    Q_UNUSED(mods_locked);
    auto kb = static_cast<MockKeyboard *>(keyboard);
    kb->m_group = group;
}

static const struct wl_keyboard_listener keyboardListener = {
    keyboardKeymap,
    keyboardEnter,
    keyboardLeave,
    keyboardKey,
    keyboardModifiers
};

MockKeyboard::MockKeyboard(wl_seat *seat)
    : m_keyboard(wl_seat_get_keyboard(seat))
{
    wl_keyboard_add_listener(m_keyboard, &keyboardListener, this);
}

MockKeyboard::~MockKeyboard()
{
    wl_keyboard_destroy(m_keyboard);
}
