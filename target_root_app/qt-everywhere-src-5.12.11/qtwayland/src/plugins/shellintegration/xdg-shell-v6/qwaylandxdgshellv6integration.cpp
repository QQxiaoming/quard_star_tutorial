/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qwaylandxdgshellv6integration_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandXdgShellV6Integration::initialize(QWaylandDisplay *display)
{
    for (QWaylandDisplay::RegistryGlobal global : display->globals()) {
        if (global.interface == QLatin1String("zxdg_shell_v6")) {
            m_xdgShell.reset(new QWaylandXdgShellV6(display->wl_registry(), global.id, global.version));
            break;
        }
    }

    if (!m_xdgShell) {
        qCDebug(lcQpaWayland) << "Couldn't find global zxdg_shell_v6 for xdg-shell unstable v6";
        return false;
    }

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandXdgShellV6Integration::createShellSurface(QWaylandWindow *window)
{
    return m_xdgShell->getXdgSurface(window);
}

void QWaylandXdgShellV6Integration::handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus)
{
    if (newFocus) {
        auto *xdgSurface = qobject_cast<QWaylandXdgSurfaceV6 *>(newFocus->shellSurface());
        if (xdgSurface && !xdgSurface->handlesActiveState())
            m_display->handleWindowActivated(newFocus);
    }
    if (oldFocus && qobject_cast<QWaylandXdgSurfaceV6 *>(oldFocus->shellSurface())) {
        auto *xdgSurface = qobject_cast<QWaylandXdgSurfaceV6 *>(oldFocus->shellSurface());
        if (xdgSurface && !xdgSurface->handlesActiveState())
            m_display->handleWindowDeactivated(oldFocus);
    }
}

}

QT_END_NAMESPACE
