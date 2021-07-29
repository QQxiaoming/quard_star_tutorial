/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
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

#include "qwaylandxdgsurfacev5_p.h"
#include "qwaylandxdgshellv5_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandextendedsurface_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgSurfaceV5::QWaylandXdgSurfaceV5(QWaylandXdgShellV5 *shell, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , QtWayland::xdg_surface_v5(shell->get_xdg_surface(window->object()))
    , m_window(window)
    , m_shell(shell)
{
    if (window->display()->windowExtension())
        m_extendedWindow = new QWaylandExtendedSurface(window);

    updateTransientParent(window->transientParent());
}

QWaylandXdgSurfaceV5::~QWaylandXdgSurfaceV5()
{
    if (m_acked.states & Qt::WindowActive)
        window()->display()->handleWindowDeactivated(m_window);

    xdg_surface_destroy(object());
    delete m_extendedWindow;
}

void QWaylandXdgSurfaceV5::resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges)
{
    // May need some conversion if types get incompatibles, ATM they're identical
    enum resize_edge const * const arg = reinterpret_cast<enum resize_edge const *>(&edges);
    resize(inputDevice, *arg);
}

void QWaylandXdgSurfaceV5::resize(QWaylandInputDevice *inputDevice, enum resize_edge edges)
{
    resize(inputDevice->wl_seat(),
           inputDevice->serial(),
           edges);
}

bool QWaylandXdgSurfaceV5::move(QWaylandInputDevice *inputDevice)
{
    move(inputDevice->wl_seat(),
         inputDevice->serial());
    return true;
}

void QWaylandXdgSurfaceV5::updateTransientParent(QWaylandWindow *parent)
{
    if (!parent)
        return;
    auto parentXdgSurface = qobject_cast<QWaylandXdgSurfaceV5 *>(parent->shellSurface());
    Q_ASSERT(parentXdgSurface);
    set_parent(parentXdgSurface->object());
}

void QWaylandXdgSurfaceV5::setTitle(const QString & title)
{
    return QtWayland::xdg_surface_v5::set_title(title);
}

void QWaylandXdgSurfaceV5::setAppId(const QString & appId)
{
    return QtWayland::xdg_surface_v5::set_app_id(appId);
}

void QWaylandXdgSurfaceV5::raise()
{
    if (m_extendedWindow)
        m_extendedWindow->raise();
}

void QWaylandXdgSurfaceV5::lower()
{
    if (m_extendedWindow)
        m_extendedWindow->lower();
}

void QWaylandXdgSurfaceV5::setContentOrientationMask(Qt::ScreenOrientations orientation)
{
    if (m_extendedWindow)
        m_extendedWindow->setContentOrientationMask(orientation);
}

void QWaylandXdgSurfaceV5::setWindowFlags(Qt::WindowFlags flags)
{
    if (m_extendedWindow)
        m_extendedWindow->setWindowFlags(flags);
}

void QWaylandXdgSurfaceV5::sendProperty(const QString &name, const QVariant &value)
{
    if (m_extendedWindow)
        m_extendedWindow->updateGenericProperty(name, value);
}

void QWaylandXdgSurfaceV5::applyConfigure()
{
    if (m_pending.isResizing)
        m_normalSize = m_pending.size;
    else if (!(m_acked.states & (Qt::WindowMaximized|Qt::WindowFullScreen)))
        m_normalSize = m_window->window()->frameGeometry().size();

    if ((m_pending.states & Qt::WindowActive) && !(m_acked.states & Qt::WindowActive))
        m_window->display()->handleWindowActivated(m_window);

    if (!(m_pending.states & Qt::WindowActive) && (m_acked.states & Qt::WindowActive))
        m_window->display()->handleWindowDeactivated(m_window);

    // TODO: none of the other plugins send WindowActive either, but is it on purpose?
    Qt::WindowStates statesWithoutActive = m_pending.states & ~Qt::WindowActive;

    m_window->handleWindowStatesChanged(statesWithoutActive);
    if (!m_pending.size.isEmpty())
        m_window->resizeFromApplyConfigure(m_pending.size);
    else if (!m_normalSize.isEmpty())
        m_window->resizeFromApplyConfigure(m_normalSize);
    ack_configure(m_pending.serial);
    m_acked = m_pending;
}

void QWaylandXdgSurfaceV5::requestWindowStates(Qt::WindowStates states)
{
    Qt::WindowStates changedStates = m_acked.states ^ states;

    if (changedStates & Qt::WindowMaximized) {
        if (states & Qt::WindowMaximized)
            set_maximized();
        else
            unset_maximized();
    }

    if (changedStates & Qt::WindowFullScreen) {
        if (states & Qt::WindowFullScreen)
            set_fullscreen(nullptr);
        else
            unset_fullscreen();
    }

    // Minimized state is not reported by the protocol, so always send it
    if (states & Qt::WindowMinimized) {
        set_minimized();
        window()->handleWindowStatesChanged(states & ~Qt::WindowMinimized);
    }
}

bool QWaylandXdgSurfaceV5::wantsDecorations() const
{
    return !(m_pending.states & Qt::WindowFullScreen);
}

void QWaylandXdgSurfaceV5::xdg_surface_configure(int32_t width, int32_t height, struct wl_array *states,uint32_t serial)
{
    uint32_t *xdgStates = reinterpret_cast<uint32_t*>(states->data);
    size_t numStates = states->size / sizeof(uint32_t);
    m_pending.serial = serial;
    m_pending.size = QSize(width, height);
    m_pending.isResizing = false;
    m_pending.states = Qt::WindowNoState;
    for (size_t i = 0; i < numStates; i++) {
        switch (xdgStates[i]) {
        case XDG_SURFACE_STATE_MAXIMIZED:
            m_pending.states |= Qt::WindowMaximized;
            break;
        case XDG_SURFACE_STATE_FULLSCREEN:
            m_pending.states |= Qt::WindowFullScreen;
            break;
        case XDG_SURFACE_STATE_RESIZING:
            m_pending.isResizing = true;
            break;
        case XDG_SURFACE_STATE_ACTIVATED:
            m_pending.states |= Qt::WindowActive;
            break;
        default:
            break;
        }
    }
    m_window->applyConfigureWhenPossible();
}

void QWaylandXdgSurfaceV5::xdg_surface_close()
{
    m_window->window()->close();
}

}

QT_END_NAMESPACE
