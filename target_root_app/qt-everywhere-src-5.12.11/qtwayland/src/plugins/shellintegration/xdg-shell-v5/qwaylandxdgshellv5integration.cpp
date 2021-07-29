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

#include "qwaylandxdgshellv5integration_p.h"
#include "qwaylandxdgsurfacev5_p.h"
#include "qwaylandxdgpopupv5_p.h"
#include "qwaylandxdgshellv5_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandXdgShellV5Integration::initialize(QWaylandDisplay *display)
{
    Q_FOREACH (QWaylandDisplay::RegistryGlobal global, display->globals()) {
        if (global.interface == QLatin1String("xdg_shell")) {
            m_xdgShell.reset(new QWaylandXdgShellV5(display->wl_registry(), global.id));
            break;
        }
    }

    if (!m_xdgShell) {
        qWarning() << "Couldn't find global xdg_shell for xdg-shell unstable v5";
        return false;
    }

    qCWarning(lcQpaWayland) << "\"xdg-shell-v5\" is a deprecated shell extension, prefer using"
                            << "\"xdg-shell-v6\" or \"xdg-shell\" if supported by the compositor"
                            << "by setting the environment variable QT_WAYLAND_SHELL_INTEGRATION";

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandXdgShellV5Integration::createShellSurface(QWaylandWindow *window)
{
    QWaylandInputDevice *inputDevice = window->display()->lastInputDevice();
    if (window->window()->type() == Qt::WindowType::Popup && inputDevice) {
        if (auto *popup = m_xdgShell->createXdgPopup(window, inputDevice))
            return popup;

        qWarning(lcQpaWayland) << "Failed to create xdg-popup v5 for window" << window->window()
                               << "falling back to creating an xdg-surface";
    }

    return m_xdgShell->createXdgSurface(window);
}

void QWaylandXdgShellV5Integration::handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus) {
    if (newFocus && qobject_cast<QWaylandXdgPopupV5 *>(newFocus->shellSurface()))
        m_display->handleWindowActivated(newFocus);
    if (oldFocus && qobject_cast<QWaylandXdgPopupV5 *>(oldFocus->shellSurface()))
        m_display->handleWindowDeactivated(oldFocus);
}

}

QT_END_NAMESPACE
