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

#include "qwaylandwlshellintegration_p.h"
#include "qwaylandwlshellsurface_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

bool QWaylandWlShellIntegration::initialize(QWaylandDisplay *display)
{
    Q_FOREACH (QWaylandDisplay::RegistryGlobal global, display->globals()) {
        if (global.interface == QLatin1String("wl_shell")) {
            m_wlShell = new QtWayland::wl_shell(display->wl_registry(), global.id, 1);
            break;
        }
    }

    if (!m_wlShell) {
        qCDebug(lcQpaWayland) << "Couldn't find global wl_shell";
        return false;
    }

    qCWarning(lcQpaWayland) << "\"wl-shell\" is a deprecated shell extension, prefer using"
                            << "\"xdg-shell-v6\" or \"xdg-shell\" if supported by the compositor"
                            << "by setting the environment variable QT_WAYLAND_SHELL_INTEGRATION";

    return QWaylandShellIntegration::initialize(display);
}

QWaylandShellSurface *QWaylandWlShellIntegration::createShellSurface(QWaylandWindow *window)
{
    return new QWaylandWlShellSurface(m_wlShell->get_shell_surface(window->object()), window);
}

void *QWaylandWlShellIntegration::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    QByteArray lowerCaseResource = resource.toLower();
    if (lowerCaseResource == "wl_shell_surface") {
        if (auto waylandWindow = static_cast<QWaylandWindow *>(window->handle())) {
            if (auto shellSurface = qobject_cast<QWaylandWlShellSurface *>(waylandWindow->shellSurface())) {
                return shellSurface->object();
            }
        }
    }
    return nullptr;
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
