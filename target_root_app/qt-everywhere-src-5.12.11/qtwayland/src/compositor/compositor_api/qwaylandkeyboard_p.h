/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#ifndef QTWAYLAND_QWLKEYBOARD_P_H
#define QTWAYLAND_QWLKEYBOARD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#include <QtWaylandCompositor/qwaylandseat.h>
#include <QtWaylandCompositor/qwaylandkeyboard.h>
#include <QtWaylandCompositor/qwaylanddestroylistener.h>

#include <QtCore/private/qobject_p.h>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QVector>

#if QT_CONFIG(xkbcommon)
#include <xkbcommon/xkbcommon.h>
#endif


QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandKeyboardPrivate : public QObjectPrivate
                                                  , public QtWaylandServer::wl_keyboard
{
public:
    Q_DECLARE_PUBLIC(QWaylandKeyboard)

    static QWaylandKeyboardPrivate *get(QWaylandKeyboard *keyboard);

    QWaylandKeyboardPrivate(QWaylandSeat *seat);
    ~QWaylandKeyboardPrivate() override;

    QWaylandCompositor *compositor() const { return seat->compositor(); }

    void focused(QWaylandSurface* surface);
    void modifiers(uint32_t serial, uint32_t mods_depressed,
                   uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

#if QT_CONFIG(xkbcommon)
    struct xkb_state *xkbState() const { return xkb_state; }
    uint32_t xkbModsMask() const { return modsDepressed | modsLatched | modsLocked; }
    void maybeUpdateXkbScanCodeTable();
#endif

    void keyEvent(uint code, uint32_t state);
    void sendKeyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void maybeUpdateKeymap();

    void checkFocusResource(Resource *resource);
    void sendEnter(QWaylandSurface *surface, Resource *resource);

protected:
    void keyboard_bind_resource(Resource *resource) override;
    void keyboard_destroy_resource(Resource *resource) override;
    void keyboard_release(Resource *resource) override;

private:
#if QT_CONFIG(xkbcommon)
    void initXKB();
    void createXKBKeymap();
    void createXKBState(xkb_keymap *keymap);
#endif
    static uint toWaylandXkbV1Key(const uint nativeScanCode);

    void sendRepeatInfo();

    QWaylandSeat *seat = nullptr;

    QWaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    QWaylandDestroyListener focusDestroyListener;

    QVector<uint32_t> keys;
    uint32_t modsDepressed = 0;
    uint32_t modsLatched = 0;
    uint32_t modsLocked = 0;
    uint32_t group = 0;

    bool pendingKeymap = false;
#if QT_CONFIG(xkbcommon)
    size_t keymap_size;
    int keymap_fd = -1;
    char *keymap_area = nullptr;
    using ScanCodeKey = std::pair<uint,int>; // group/layout and QtKey
    QMap<ScanCodeKey, uint> scanCodesByQtKey;
    struct xkb_context *xkb_context = nullptr;
    struct xkb_state *xkb_state = nullptr;
#endif

    quint32 repeatRate = 40;
    quint32 repeatDelay = 400;
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLKEYBOARD_P_H
