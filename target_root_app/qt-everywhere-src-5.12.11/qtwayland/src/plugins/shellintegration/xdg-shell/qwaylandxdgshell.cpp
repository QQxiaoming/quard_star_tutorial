/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Eurogiciel, author: <philippe.coval@eurogiciel.fr>
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

#include "qwaylandxdgshell_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgSurface::Toplevel::Toplevel(QWaylandXdgSurface *xdgSurface)
    : QtWayland::xdg_toplevel(xdgSurface->get_toplevel())
    , m_xdgSurface(xdgSurface)
{
    if (auto *decorationManager = m_xdgSurface->m_shell->decorationManager())
        m_decoration = decorationManager->createToplevelDecoration(object());

    QWindow *window = xdgSurface->window()->window();
    requestWindowStates(window->windowStates());
    requestWindowFlags(window->flags());
}

QWaylandXdgSurface::Toplevel::~Toplevel()
{
    if (m_applied.states & Qt::WindowActive) {
        QWaylandWindow *window = m_xdgSurface->window();
        window->display()->handleWindowDeactivated(window);
    }

    // The protocol spec requires that the decoration object is deleted before xdg_toplevel.
    delete m_decoration;
    m_decoration = nullptr;

    if (isInitialized())
        destroy();
}

void QWaylandXdgSurface::Toplevel::applyConfigure()
{
    if (!(m_applied.states & (Qt::WindowMaximized|Qt::WindowFullScreen)))
        m_normalSize = m_xdgSurface->m_window->window()->frameGeometry().size();

    if ((m_pending.states & Qt::WindowActive) && !(m_applied.states & Qt::WindowActive))
        m_xdgSurface->m_window->display()->handleWindowActivated(m_xdgSurface->m_window);

    if (!(m_pending.states & Qt::WindowActive) && (m_applied.states & Qt::WindowActive))
        m_xdgSurface->m_window->display()->handleWindowDeactivated(m_xdgSurface->m_window);

    // TODO: none of the other plugins send WindowActive either, but is it on purpose?
    Qt::WindowStates statesWithoutActive = m_pending.states & ~Qt::WindowActive;

    m_xdgSurface->m_window->handleWindowStatesChanged(statesWithoutActive);

    if (m_pending.size.isEmpty()) {
        // An empty size in the configure means it's up to the client to choose the size
        bool normalPending = !(m_pending.states & (Qt::WindowMaximized|Qt::WindowFullScreen));
        if (normalPending && !m_normalSize.isEmpty())
            m_xdgSurface->m_window->resizeFromApplyConfigure(m_normalSize);
    } else {
        m_xdgSurface->m_window->resizeFromApplyConfigure(m_pending.size);
    }

    QSize windowGeometrySize = m_xdgSurface->m_window->window()->frameGeometry().size();
    m_xdgSurface->set_window_geometry(0, 0, windowGeometrySize.width(), windowGeometrySize.height());
    m_applied = m_pending;
    qCDebug(lcQpaWayland) << "Applied pending xdg_toplevel configure event:" << m_applied.size << m_applied.states;
}

bool QWaylandXdgSurface::Toplevel::wantsDecorations()
{
    if (m_decoration && m_decoration->pending() == QWaylandXdgToplevelDecorationV1::mode_server_side)
        return false;

    return !(m_pending.states & Qt::WindowFullScreen);
}

void QWaylandXdgSurface::Toplevel::xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states)
{
    m_pending.size = QSize(width, height);

    auto *xdgStates = static_cast<uint32_t *>(states->data);
    size_t numStates = states->size / sizeof(uint32_t);

    m_pending.states = Qt::WindowNoState;

    for (size_t i = 0; i < numStates; i++) {
        switch (xdgStates[i]) {
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            m_pending.states |= Qt::WindowActive;
            break;
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            m_pending.states |= Qt::WindowMaximized;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            m_pending.states |= Qt::WindowFullScreen;
            break;
        default:
            break;
        }
    }
    qCDebug(lcQpaWayland) << "Received xdg_toplevel.configure with" << m_pending.size
                          << "and" << m_pending.states;
}

void QWaylandXdgSurface::Toplevel::xdg_toplevel_close()
{
    m_xdgSurface->m_window->window()->close();
}

void QWaylandXdgSurface::Toplevel::requestWindowFlags(Qt::WindowFlags flags)
{
    if (m_decoration) {
        if (flags & Qt::FramelessWindowHint)
            m_decoration->requestMode(QWaylandXdgToplevelDecorationV1::mode_client_side);
        else
            m_decoration->unsetMode();
    }
}

void QWaylandXdgSurface::Toplevel::requestWindowStates(Qt::WindowStates states)
{
    // Re-send what's different from the applied state
    Qt::WindowStates changedStates = m_applied.states ^ states;

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
        m_xdgSurface->window()->handleWindowStatesChanged(states & ~Qt::WindowMinimized);
    }
}

QWaylandXdgSurface::Popup::Popup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parent,
                                 QtWayland::xdg_positioner *positioner)
    : xdg_popup(xdgSurface->get_popup(parent->object(), positioner->object()))
    , m_xdgSurface(xdgSurface)
    , m_parent(parent)
{
}

QWaylandXdgSurface::Popup::~Popup()
{
    if (isInitialized())
        destroy();

    if (m_grabbing) {
        auto *shell = m_xdgSurface->m_shell;
        Q_ASSERT(shell->m_topmostGrabbingPopup == this);
        shell->m_topmostGrabbingPopup = m_parent->m_popup;
    }
}

void QWaylandXdgSurface::Popup::grab(QWaylandInputDevice *seat, uint serial)
{
    m_xdgSurface->m_shell->m_topmostGrabbingPopup = this;
    xdg_popup::grab(seat->wl_seat(), serial);
    m_grabbing = true;
}

void QWaylandXdgSurface::Popup::xdg_popup_popup_done()
{
    m_xdgSurface->m_window->window()->close();
}

QWaylandXdgSurface::QWaylandXdgSurface(QWaylandXdgShell *shell, ::xdg_surface *surface, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , xdg_surface(surface)
    , m_shell(shell)
    , m_window(window)
{
    QWaylandDisplay *display = window->display();
    Qt::WindowType type = window->window()->type();
    auto *transientParent = window->transientParent();

    if (type == Qt::ToolTip && transientParent) {
        setPopup(transientParent);
    } else if (type == Qt::Popup && transientParent && display->lastInputDevice()) {
        setGrabPopup(transientParent, display->lastInputDevice(), display->lastInputSerial());
    } else {
        setToplevel();
        if (transientParent) {
            auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(transientParent->shellSurface());
            if (parentXdgSurface)
                m_toplevel->set_parent(parentXdgSurface->m_toplevel->object());
        }
    }
}

QWaylandXdgSurface::~QWaylandXdgSurface()
{
    if (m_toplevel) {
        delete m_toplevel;
        m_toplevel = nullptr;
    }
    if (m_popup) {
        delete m_popup;
        m_popup = nullptr;
    }
    destroy();
}

void QWaylandXdgSurface::resize(QWaylandInputDevice *inputDevice, xdg_toplevel_resize_edge edges)
{
    Q_ASSERT(m_toplevel && m_toplevel->isInitialized());
    m_toplevel->resize(inputDevice->wl_seat(), inputDevice->serial(), edges);
}

void QWaylandXdgSurface::resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges)
{
    auto xdgEdges = reinterpret_cast<enum xdg_toplevel_resize_edge const *>(&edges);
    resize(inputDevice, *xdgEdges);
}


bool QWaylandXdgSurface::move(QWaylandInputDevice *inputDevice)
{
    if (m_toplevel && m_toplevel->isInitialized()) {
        m_toplevel->move(inputDevice->wl_seat(), inputDevice->serial());
        return true;
    }
    return false;
}

void QWaylandXdgSurface::setTitle(const QString &title)
{
    if (m_toplevel)
        m_toplevel->set_title(title);
}

void QWaylandXdgSurface::setAppId(const QString &appId)
{
    if (m_toplevel)
        m_toplevel->set_app_id(appId);
}

void QWaylandXdgSurface::setWindowFlags(Qt::WindowFlags flags)
{
    if (m_toplevel)
        m_toplevel->requestWindowFlags(flags);
}

bool QWaylandXdgSurface::isExposed() const
{
    return m_configured || m_pendingConfigureSerial;
}

bool QWaylandXdgSurface::handleExpose(const QRegion &region)
{
    if (!isExposed() && !region.isEmpty()) {
        m_exposeRegion = region;
        return true;
    }
    return false;
}

void QWaylandXdgSurface::applyConfigure()
{
    Q_ASSERT(m_pendingConfigureSerial != 0);

    if (m_toplevel)
        m_toplevel->applyConfigure();

    m_configured = true;
    ack_configure(m_pendingConfigureSerial);

    m_pendingConfigureSerial = 0;
}

bool QWaylandXdgSurface::wantsDecorations() const
{
    return m_toplevel && m_toplevel->wantsDecorations();
}

void QWaylandXdgSurface::requestWindowStates(Qt::WindowStates states)
{
    if (m_toplevel)
        m_toplevel->requestWindowStates(states);
    else
        qCWarning(lcQpaWayland) << "Non-toplevel surfaces can't request window states";
}

void QWaylandXdgSurface::setToplevel()
{
    Q_ASSERT(!m_toplevel && !m_popup);
    m_toplevel = new Toplevel(this);
}

void QWaylandXdgSurface::setPopup(QWaylandWindow *parent)
{
    Q_ASSERT(!m_toplevel && !m_popup);

    auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(parent->shellSurface());

    auto positioner = new QtWayland::xdg_positioner(m_shell->create_positioner());
    // set_popup expects a position relative to the parent
    QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
    transientPos -= parent->geometry().topLeft();
    if (parent->decoration()) {
        transientPos.setX(transientPos.x() + parent->decoration()->margins().left());
        transientPos.setY(transientPos.y() + parent->decoration()->margins().top());
    }
    positioner->set_anchor_rect(transientPos.x(), transientPos.y(), 1, 1);
    positioner->set_anchor(QtWayland::xdg_positioner::anchor_top_left);
    positioner->set_gravity(QtWayland::xdg_positioner::gravity_bottom_right);
    positioner->set_size(m_window->geometry().width(), m_window->geometry().height());
    m_popup = new Popup(this, parentXdgSurface, positioner);
    positioner->destroy();
    delete positioner;
}

void QWaylandXdgSurface::setGrabPopup(QWaylandWindow *parent, QWaylandInputDevice *device, int serial)
{
    auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(parent->shellSurface());
    auto *top = m_shell->m_topmostGrabbingPopup;

    if (top && top->m_xdgSurface != parentXdgSurface) {
        qCWarning(lcQpaWayland) << "setGrabPopup called with a parent," << parentXdgSurface
                                << "which does not match the current topmost grabbing popup,"
                                << top->m_xdgSurface << "According to the xdg-shell protocol, this"
                                << "is not allowed. The wayland QPA plugin is currently handling"
                                << "it by setting the parent to the topmost grabbing popup."
                                << "Note, however, that this may cause positioning errors and"
                                << "popups closing unxpectedly because xdg-shell mandate that child"
                                << "popups close before parents";
        parent = top->m_xdgSurface->m_window;
    }
    setPopup(parent);
    m_popup->grab(device, serial);
}

void QWaylandXdgSurface::xdg_surface_configure(uint32_t serial)
{
    m_pendingConfigureSerial = serial;
    if (!m_configured) {
        // We have to do the initial applyConfigure() immediately, since that is the expose.
        applyConfigure();
        m_exposeRegion = QRegion(QRect(QPoint(), m_window->geometry().size()));
    } else {
        // Later configures are probably resizes, so we have to queue them up for a time when we
        // are not painting to the window.
        m_window->applyConfigureWhenPossible();
    }

    if (!m_exposeRegion.isEmpty()) {
        m_window->handleExpose(m_exposeRegion);
        m_exposeRegion = QRegion();
    }
}

QWaylandXdgShell::QWaylandXdgShell(QWaylandDisplay *display, uint32_t id, uint32_t availableVersion)
    : QtWayland::xdg_wm_base(display->wl_registry(), id, qMin(availableVersion, 1u))
    , m_display(display)
{
    display->addRegistryListener(&QWaylandXdgShell::handleRegistryGlobal, this);
}

QWaylandXdgShell::~QWaylandXdgShell()
{
    m_display->removeListener(&QWaylandXdgShell::handleRegistryGlobal, this);
    destroy();
}

QWaylandXdgSurface *QWaylandXdgShell::getXdgSurface(QWaylandWindow *window)
{
    return new QWaylandXdgSurface(this, get_xdg_surface(window->object()), window);
}

void QWaylandXdgShell::xdg_wm_base_ping(uint32_t serial)
{
    pong(serial);
}

void QWaylandXdgShell::handleRegistryGlobal(void *data, wl_registry *registry, uint id,
                                            const QString &interface, uint version)
{
    QWaylandXdgShell *xdgShell = static_cast<QWaylandXdgShell *>(data);
    if (interface == QLatin1String(QWaylandXdgDecorationManagerV1::interface()->name))
        xdgShell->m_xdgDecorationManager.reset(new QWaylandXdgDecorationManagerV1(registry, id, version));
}

}

QT_END_NAMESPACE
