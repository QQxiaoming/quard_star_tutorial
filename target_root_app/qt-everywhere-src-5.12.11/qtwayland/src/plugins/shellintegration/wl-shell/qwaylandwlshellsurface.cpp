/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qwaylandwlshellsurface_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandextendedsurface_p.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandWlShellSurface::QWaylandWlShellSurface(struct ::wl_shell_surface *shell_surface, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , QtWayland::wl_shell_surface(shell_surface)
    , m_window(window)
{
    if (window->display()->windowExtension())
        m_extendedWindow = new QWaylandExtendedSurface(window);

    Qt::WindowType type = window->window()->type();
    auto *transientParent = window->transientParent();
    if (type == Qt::Popup && transientParent && transientParent->object())
        setPopup(transientParent, m_window->display()->lastInputDevice(), m_window->display()->lastInputSerial());
    else if (transientParent && transientParent->object())
        updateTransientParent(transientParent->window());
    else
        setTopLevel();
}

QWaylandWlShellSurface::~QWaylandWlShellSurface()
{
    wl_shell_surface_destroy(object());
    delete m_extendedWindow;
}

void QWaylandWlShellSurface::resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges)
{
    resize(inputDevice->wl_seat(),
           inputDevice->serial(),
           edges);
}

bool QWaylandWlShellSurface::move(QWaylandInputDevice *inputDevice)
{
    move(inputDevice->wl_seat(),
         inputDevice->serial());
    return true;
}

void QWaylandWlShellSurface::setTitle(const QString & title)
{
    return QtWayland::wl_shell_surface::set_title(title);
}

void QWaylandWlShellSurface::setAppId(const QString & appId)
{
    return QtWayland::wl_shell_surface::set_class(appId);
}

void QWaylandWlShellSurface::raise()
{
    if (m_extendedWindow)
        m_extendedWindow->raise();
}

void QWaylandWlShellSurface::lower()
{
    if (m_extendedWindow)
        m_extendedWindow->lower();
}

void QWaylandWlShellSurface::setContentOrientationMask(Qt::ScreenOrientations orientation)
{
    if (m_extendedWindow)
        m_extendedWindow->setContentOrientationMask(orientation);
}

void QWaylandWlShellSurface::setWindowFlags(Qt::WindowFlags flags)
{
    if (m_extendedWindow)
        m_extendedWindow->setWindowFlags(flags);
}

void QWaylandWlShellSurface::sendProperty(const QString &name, const QVariant &value)
{
    if (m_extendedWindow)
        m_extendedWindow->updateGenericProperty(name, value);
}

void QWaylandWlShellSurface::applyConfigure()
{
    if ((m_pending.states & (Qt::WindowMaximized|Qt::WindowFullScreen))
            && !(m_applied.states & (Qt::WindowMaximized|Qt::WindowFullScreen))) {
        m_normalSize = m_window->window()->frameGeometry().size();
    }

    if (m_pending.states != m_applied.states)
        m_window->handleWindowStatesChanged(m_pending.states);

    if (!m_pending.size.isEmpty()) {
        int x = 0;
        int y = 0;
        if (m_pending.edges & resize_left)
            x = m_applied.size.width() - m_pending.size.width();
        if (m_pending.edges & resize_top)
            y = m_applied.size.height() - m_pending.size.height();
        QPoint offset(x, y);
        m_window->resizeFromApplyConfigure(m_pending.size, offset);
    } else if (m_pending.size.isValid() && !m_normalSize.isEmpty()) {
        m_window->resizeFromApplyConfigure(m_normalSize);
    }

    m_applied = m_pending;
}

bool QWaylandWlShellSurface::wantsDecorations() const
{
    return !(m_pending.states & Qt::WindowFullScreen);
}

void QWaylandWlShellSurface::requestWindowStates(Qt::WindowStates states)
{
    // On wl-shell the client is in charge of states, so diff from the pending state
    Qt::WindowStates changedStates = m_pending.states ^ states;
    Qt::WindowStates addedStates = changedStates & states;

    if (addedStates & Qt::WindowMinimized)
        qCWarning(lcQpaWayland) << "Minimizing is not supported on wl-shell. Consider using xdg-shell instead.";

    if (addedStates & Qt::WindowMaximized) {
        set_maximized(nullptr);
        m_window->applyConfigureWhenPossible();
    }

    if (addedStates & Qt::WindowFullScreen) {
        set_fullscreen(WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, nullptr);
        m_window->applyConfigureWhenPossible();
    }

    bool isNormal = !(states & Qt::WindowMaximized) && !(states & Qt::WindowFullScreen);
    if (isNormal && (changedStates & (Qt::WindowMaximized | Qt::WindowFullScreen))) {
        setTopLevel(); // set normal window
        // There's usually no configure event after this, so just clear the rest of the pending
        // configure here and queue the applyConfigure call
        m_pending.size = {0, 0};
        m_pending.edges = resize_none;
        m_window->applyConfigureWhenPossible();
    }

    m_pending.states = states & ~Qt::WindowMinimized;
}


void QWaylandWlShellSurface::setTopLevel()
{
    set_toplevel();
}

static inline bool testShowWithoutActivating(const QWindow *window)
{
    // QWidget-attribute Qt::WA_ShowWithoutActivating.
    const QVariant showWithoutActivating = window->property("_q_showWithoutActivating");
    return showWithoutActivating.isValid() && showWithoutActivating.toBool();
}

void QWaylandWlShellSurface::updateTransientParent(QWindow *parent)
{
    QWaylandWindow *parent_wayland_window = static_cast<QWaylandWindow *>(parent->handle());
    if (!parent_wayland_window)
        return;

    // set_transient expects a position relative to the parent
    QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
    transientPos -= parent->geometry().topLeft();
    if (parent_wayland_window->decoration()) {
        transientPos.setX(transientPos.x() + parent_wayland_window->decoration()->margins().left());
        transientPos.setY(transientPos.y() + parent_wayland_window->decoration()->margins().top());
    }

    uint32_t flags = 0;
    Qt::WindowFlags wf = m_window->window()->flags();
    if (wf.testFlag(Qt::ToolTip)
            || wf.testFlag(Qt::WindowTransparentForInput)
            || testShowWithoutActivating(m_window->window()))
        flags |= WL_SHELL_SURFACE_TRANSIENT_INACTIVE;

    Q_ASSERT(parent_wayland_window->object());
    set_transient(parent_wayland_window->object(),
                  transientPos.x(),
                  transientPos.y(),
                  flags);
}

void QWaylandWlShellSurface::setPopup(QWaylandWindow *parent, QWaylandInputDevice *device, uint serial)
{
    QWaylandWindow *parent_wayland_window = parent;
    if (!parent_wayland_window) {
        qCWarning(lcQpaWayland) << "setPopup called without a parent window";
        return;
    }
    if (!device) {
        qCWarning(lcQpaWayland) << "setPopup called without an input device";
        return;
    }

    // set_popup expects a position relative to the parent
    QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
    transientPos -= parent_wayland_window->geometry().topLeft();
    if (parent_wayland_window->decoration()) {
        transientPos.setX(transientPos.x() + parent_wayland_window->decoration()->margins().left());
        transientPos.setY(transientPos.y() + parent_wayland_window->decoration()->margins().top());
    }

    Q_ASSERT(parent_wayland_window->object());
    set_popup(device->wl_seat(), serial, parent_wayland_window->object(),
              transientPos.x(), transientPos.y(), 0);
}

void QWaylandWlShellSurface::shell_surface_ping(uint32_t serial)
{
    pong(serial);
}

void QWaylandWlShellSurface::shell_surface_configure(uint32_t edges, int32_t width, int32_t height)
{
    m_pending.size = QSize(width, height);
    m_pending.edges = static_cast<enum resize>(edges);
    if (m_pending.edges && !m_pending.size.isEmpty())
        m_normalSize = m_pending.size;
    m_window->applyConfigureWhenPossible();
}

void QWaylandWlShellSurface::shell_surface_popup_done()
{
    QCoreApplication::postEvent(m_window->window(), new QCloseEvent());
}

}

QT_END_NAMESPACE
