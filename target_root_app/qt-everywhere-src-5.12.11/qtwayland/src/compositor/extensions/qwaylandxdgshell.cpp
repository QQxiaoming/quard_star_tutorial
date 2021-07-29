/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandxdgshell.h"
#include "qwaylandxdgshell_p.h"

#if QT_CONFIG(wayland_compositor_quick)
#include "qwaylandxdgshellintegration_p.h"
#endif

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandSurfaceRole>
#include <QtWaylandCompositor/QWaylandResource>

#include <QtCore/QObject>

#include <algorithm>

QT_BEGIN_NAMESPACE

QWaylandXdgShellPrivate::QWaylandXdgShellPrivate()
{
}

void QWaylandXdgShellPrivate::ping(QtWaylandServer::xdg_wm_base::Resource *resource, uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(resource->handle, serial);
}

void QWaylandXdgShellPrivate::registerXdgSurface(QWaylandXdgSurface *xdgSurface)
{
    m_xdgSurfaces.insert(xdgSurface->surface()->client()->client(), xdgSurface);
}

void QWaylandXdgShellPrivate::unregisterXdgSurface(QWaylandXdgSurface *xdgSurface)
{
    auto xdgSurfacePrivate = QWaylandXdgSurfacePrivate::get(xdgSurface);
    if (!m_xdgSurfaces.remove(xdgSurfacePrivate->resource()->client(), xdgSurface))
        qWarning("%s Unexpected state. Can't find registered xdg surface\n", Q_FUNC_INFO);
}

QWaylandXdgSurface *QWaylandXdgShellPrivate::xdgSurfaceFromSurface(QWaylandSurface *surface)
{
    for (QWaylandXdgSurface *xdgSurface : qAsConst(m_xdgSurfaces)) {
        if (surface == xdgSurface->surface())
            return xdgSurface;
    }
    return nullptr;
}

void QWaylandXdgShellPrivate::xdg_wm_base_destroy(Resource *resource)
{
    if (!m_xdgSurfaces.values(resource->client()).empty())
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES,
                               "xdg_shell was destroyed before children");

    wl_resource_destroy(resource->handle);
}

void QWaylandXdgShellPrivate::xdg_wm_base_create_positioner(QtWaylandServer::xdg_wm_base::Resource *resource, uint32_t id)
{
    QWaylandResource positionerResource(wl_resource_create(resource->client(), &xdg_positioner_interface,
                                                           wl_resource_get_version(resource->handle), id));

    new QWaylandXdgPositioner(positionerResource);
}

void QWaylandXdgShellPrivate::xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    Q_Q(QWaylandXdgShell);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    if (surface->role() != nullptr) {
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_ROLE,
                               "wl_surface@%d, already has role %s\n",
                               wl_resource_get_id(surface->resource()),
                               surface->role()->name().constData());
        return;
    }

    if (surface->hasContent()) {
        //TODO: According to the spec, this is a client error, but there's no appropriate error code
        qWarning() << "get_xdg_surface requested on a xdg_surface with content";
    }

    QWaylandResource xdgSurfaceResource(wl_resource_create(resource->client(), &xdg_surface_interface,
                                                           wl_resource_get_version(resource->handle), id));

    QWaylandXdgSurface *xdgSurface = new QWaylandXdgSurface(q, surface, xdgSurfaceResource);

    registerXdgSurface(xdgSurface);
    emit q->xdgSurfaceCreated(xdgSurface);
}

void QWaylandXdgShellPrivate::xdg_wm_base_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgShell);
    if (m_pings.remove(serial))
        emit q->pong(serial);
    else
        qWarning("Received an unexpected pong!");
}

/*!
 * \qmltype XdgShell
 * \inqmlmodule QtWayland.Compositor
 * \since 5.12
 * \brief Provides an extension for desktop-style user interfaces.
 *
 * The XdgShell extension provides a way to associate a XdgToplevel or XdgPopup
 * with a regular Wayland surface. Using the XdgToplevel interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * XdgShell corresponds to the Wayland interface, \c xdg_shell.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the XdgShell component and add it to the list of extensions
 * supported by the compositor:
 * \code
 * import QtWayland.Compositor 1.3
 *
 * WaylandCompositor {
 *     XdgShell {
 *         // ...
 *     }
 * }
 * \endcode
 */

/*!
 * \class QWaylandXdgShell
 * \inmodule QtWaylandCompositor
 * \since 5.12
 * \brief The QWaylandXdgShell class is an extension for desktop-style user interfaces.
 *
 * The QWaylandXdgShell extension provides a way to associate a QWaylandXdgToplevel or
 * QWaylandXdgPopup with a regular Wayland surface. Using the QWaylandXdgToplevel interface,
 * the client can request that the surface is resized, moved, and so on.
 *
 * QWaylandXdgShell corresponds to the Wayland interface, \c xdg_shell.
 */

/*!
 * Constructs a QWaylandXdgShell object.
 */
QWaylandXdgShell::QWaylandXdgShell()
    : QWaylandShellTemplate<QWaylandXdgShell>(*new QWaylandXdgShellPrivate())
{
}

/*!
 * Constructs a QWaylandXdgShell object for the provided \a compositor.
 */
QWaylandXdgShell::QWaylandXdgShell(QWaylandCompositor *compositor)
    : QWaylandShellTemplate<QWaylandXdgShell>(compositor, *new QWaylandXdgShellPrivate())
{
}

/*!
 * Initializes the shell extension.
 */
void QWaylandXdgShell::initialize()
{
    Q_D(QWaylandXdgShell);
    QWaylandShellTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandXdgShell";
        return;
    }
    d->init(compositor->display(), 1);

    handleSeatChanged(compositor->defaultSeat(), nullptr);

    connect(compositor, &QWaylandCompositor::defaultSeatChanged,
            this, &QWaylandXdgShell::handleSeatChanged);
}

/*!
 * Returns the Wayland interface for the QWaylandXdgShell.
 */
const struct wl_interface *QWaylandXdgShell::interface()
{
    return QWaylandXdgShellPrivate::interface();
}

QByteArray QWaylandXdgShell::interfaceName()
{
    return QWaylandXdgShellPrivate::interfaceName();
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgShell::ping(WaylandClient client)
 *
 * Sends a ping event to \a client. If the client replies to the event the
 * \l pong signal will be emitted.
 */

/*!
 * Sends a ping event to \a client. If the client replies to the event the
 * \l pong signal will be emitted.
 */
uint QWaylandXdgShell::ping(QWaylandClient *client)
{
    Q_D(QWaylandXdgShell);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    Q_ASSERT(compositor);

    uint32_t serial = compositor->nextSerial();

    QWaylandXdgShellPrivate::Resource *clientResource = d->resourceMap().value(client->client(), nullptr);
    Q_ASSERT(clientResource);

    d->ping(clientResource, serial);
    return serial;
}

void QWaylandXdgShell::handleSeatChanged(QWaylandSeat *newSeat, QWaylandSeat *oldSeat)
{
    if (oldSeat != nullptr) {
        disconnect(oldSeat, &QWaylandSeat::keyboardFocusChanged,
                   this, &QWaylandXdgShell::handleFocusChanged);
    }

    if (newSeat != nullptr) {
        connect(newSeat, &QWaylandSeat::keyboardFocusChanged,
                this, &QWaylandXdgShell::handleFocusChanged);
    }
}

void QWaylandXdgShell::handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_D(QWaylandXdgShell);

    QWaylandXdgSurface *newXdgSurface = d->xdgSurfaceFromSurface(newSurface);
    QWaylandXdgSurface *oldXdgSurface = d->xdgSurfaceFromSurface(oldSurface);

    if (newXdgSurface)
        QWaylandXdgSurfacePrivate::get(newXdgSurface)->handleFocusReceived();

    if (oldXdgSurface)
        QWaylandXdgSurfacePrivate::get(oldXdgSurface)->handleFocusLost();
}

QWaylandXdgSurfacePrivate::QWaylandXdgSurfacePrivate()
{
}

void QWaylandXdgSurfacePrivate::setWindowType(Qt::WindowType windowType)
{
    if (m_windowType == windowType)
        return;

    m_windowType = windowType;

    Q_Q(QWaylandXdgSurface);
    emit q->windowTypeChanged();
}

void QWaylandXdgSurfacePrivate::handleFocusLost()
{
    if (m_toplevel)
        QWaylandXdgToplevelPrivate::get(m_toplevel)->handleFocusLost();
}

void QWaylandXdgSurfacePrivate::handleFocusReceived()
{
    if (m_toplevel)
        QWaylandXdgToplevelPrivate::get(m_toplevel)->handleFocusReceived();
}

QRect QWaylandXdgSurfacePrivate::calculateFallbackWindowGeometry() const
{
    // TODO: The unset window geometry should include subsurfaces as well, so this solution
    // won't work too well on those kinds of clients.
    return QRect(QPoint(0, 0), m_surface->size() / m_surface->bufferScale());
}

void QWaylandXdgSurfacePrivate::updateFallbackWindowGeometry()
{
    Q_Q(QWaylandXdgSurface);
    if (!m_unsetWindowGeometry)
        return;

    const QRect unsetGeometry = calculateFallbackWindowGeometry();
    if (unsetGeometry == m_windowGeometry)
        return;

    m_windowGeometry = unsetGeometry;
    emit q->windowGeometryChanged();
}

void QWaylandXdgSurfacePrivate::xdg_surface_destroy_resource(QtWaylandServer::xdg_surface::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    QWaylandXdgShellPrivate::get(m_xdgShell)->unregisterXdgSurface(q);
    delete q;
}

void QWaylandXdgSurfacePrivate::xdg_surface_destroy(QtWaylandServer::xdg_surface::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgSurfacePrivate::xdg_surface_get_toplevel(QtWaylandServer::xdg_surface::Resource *resource, uint32_t id)
{
    Q_Q(QWaylandXdgSurface);

    if (m_toplevel || m_popup) {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
                               "xdg_surface already has a role object");
        return;
    }

    if (!m_surface->setRole(QWaylandXdgToplevel::role(), resource->handle, XDG_WM_BASE_ERROR_ROLE))
        return;

    QWaylandResource topLevelResource(wl_resource_create(resource->client(), &xdg_toplevel_interface,
                                                         wl_resource_get_version(resource->handle), id));

    m_toplevel = new QWaylandXdgToplevel(q, topLevelResource);
    emit q->toplevelCreated();
    emit m_xdgShell->toplevelCreated(m_toplevel, q);
}

void QWaylandXdgSurfacePrivate::xdg_surface_get_popup(QtWaylandServer::xdg_surface::Resource *resource, uint32_t id, wl_resource *parentResource, wl_resource *positionerResource)
{
    Q_Q(QWaylandXdgSurface);

    if (m_toplevel || m_popup) {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
                               "xdg_surface already has a role object");
        return;
    }

    QWaylandXdgSurface *parent = QWaylandXdgSurface::fromResource(parentResource);
    if (!parent) {
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "xdg_surface.get_popup with invalid popup parent");
        return;
    }

    QWaylandXdgPositioner *positioner = QWaylandXdgPositioner::fromResource(positionerResource);
    if (!positioner) {
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                               "xdg_surface.get_popup without positioner");
        return;
    }
    if (!positioner->m_data.isComplete()) {
        QWaylandXdgPositionerData p = positioner->m_data;
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                               "xdg_surface.get_popup with invalid positioner (size: %dx%d, anchorRect: %dx%d)",
                               p.size.width(), p.size.height(), p.anchorRect.width(), p.anchorRect.height());
        return;
    }

    if (!m_surface->setRole(QWaylandXdgPopup::role(), resource->handle, XDG_WM_BASE_ERROR_ROLE))
        return;

    QWaylandResource popupResource(wl_resource_create(resource->client(), &xdg_popup_interface,
                                                      wl_resource_get_version(resource->handle), id));

    m_popup = new QWaylandXdgPopup(q, parent, positioner, popupResource);
    emit q->popupCreated();
    emit m_xdgShell->popupCreated(m_popup, q);
}

void QWaylandXdgSurfacePrivate::xdg_surface_ack_configure(QtWaylandServer::xdg_surface::Resource *resource, uint32_t serial)
{
    if (m_toplevel) {
        QWaylandXdgToplevelPrivate::get(m_toplevel)->handleAckConfigure(serial);
    } else if (m_popup) {
        QWaylandXdgPopupPrivate::get(m_popup)->handleAckConfigure(serial);
    } else {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                               "ack_configure requested on an unconstructed xdg_surface");
    }
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_window_geometry(QtWaylandServer::xdg_surface::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_Q(QWaylandXdgSurface);

    if (!q->surface()->role()) {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                               "set_window_geometry requested on an unconstructed xdg_surface");
        return;
    }

    if (width <= 0 || height <= 0) {
        // The protocol spec says "setting an invalid size will raise an error". But doesn't tell
        // which error to raise, and there's no fitting error in the xdg_surface_error enum.
        // So until this is fixed, just output a warning and return.
        qWarning() << "Invalid (non-positive) dimensions received in set_window_geometry";
        return;
    }

    m_unsetWindowGeometry = false;

    QRect geometry(x, y, width, height);

    if (m_windowGeometry == geometry)
        return;

    m_windowGeometry = geometry;
    emit q->windowGeometryChanged();
}

/*!
 * \qmltype XdgSurface
 * \inqmlmodule QtWayland.Compositor
 * \since 5.12
 * \brief XdgSurface provides desktop-style compositor-specific features to an xdg surface.
 *
 * This type is part of the \l{XdgShell} extension and provides a way to
 * extend the functionality of an existing \l{WaylandSurface} with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface \c xdg_surface.
 */

/*!
 * \class QWaylandXdgSurface
 * \inmodule QtWaylandCompositor
 * \since 5.12
 * \brief The QWaylandXdgSurface class provides desktop-style compositor-specific features to an xdg surface.
 *
 * This class is part of the QWaylandXdgShell extension and provides a way to
 * extend the functionality of an existing QWaylandSurface with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface \c xdg_surface.
 */

/*!
 * Constructs a QWaylandXdgSurface.
 */
QWaylandXdgSurface::QWaylandXdgSurface()
    : QWaylandShellSurfaceTemplate<QWaylandXdgSurface>(*new QWaylandXdgSurfacePrivate)
{
}

/*!
 * Constructs a QWaylandXdgSurface for \a surface and initializes it with the
 * given \a xdgShell, \a surface, and resource \a res.
 */
QWaylandXdgSurface::QWaylandXdgSurface(QWaylandXdgShell *xdgShell, QWaylandSurface *surface, const QWaylandResource &res)
    : QWaylandShellSurfaceTemplate<QWaylandXdgSurface>(*new QWaylandXdgSurfacePrivate)
{
    initialize(xdgShell, surface, res);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgSurface::initialize(object xdgShell, object surface, object client, int id)
 *
 * Initializes the XdgSurface, associating it with the given \a xdgShell, \a surface,
 * \a client, and \a id.
 */

/*!
 * Initializes the QWaylandXdgSurface, associating it with the given \a xdgShell, \a surface
 * and \a resource.
 */
void QWaylandXdgSurface::initialize(QWaylandXdgShell *xdgShell, QWaylandSurface *surface, const QWaylandResource &resource)
{
    Q_D(QWaylandXdgSurface);
    d->m_xdgShell = xdgShell;
    d->m_surface = surface;
    d->init(resource.resource());
    setExtensionContainer(surface);
    d->m_windowGeometry = d->calculateFallbackWindowGeometry();
    connect(surface, &QWaylandSurface::sizeChanged, this, &QWaylandXdgSurface::handleSurfaceSizeChanged);
    connect(surface, &QWaylandSurface::bufferScaleChanged, this, &QWaylandXdgSurface::handleBufferScaleChanged);
    emit shellChanged();
    emit surfaceChanged();
    QWaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty enum QtWaylandCompositor::XdgSurface::windowType
 *
 * This property holds the window type of the XdgSurface.
 */
Qt::WindowType QWaylandXdgSurface::windowType() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_windowType;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgSurface::windowGeometry
 *
 * This property holds the window geometry of the QWaylandXdgSurface. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */

/*!
 * \property QWaylandXdgSurface::windowGeometry
 *
 * This property holds the window geometry of the QWaylandXdgSurface. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */
QRect QWaylandXdgSurface::windowGeometry() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_windowGeometry;
}

/*!
 * \internal
 */
void QWaylandXdgSurface::initialize()
{
    QWaylandCompositorExtension::initialize();
}

void QWaylandXdgSurface::handleSurfaceSizeChanged()
{
    Q_D(QWaylandXdgSurface);
    d->updateFallbackWindowGeometry();
}

void QWaylandXdgSurface::handleBufferScaleChanged()
{
    Q_D(QWaylandXdgSurface);
    d->updateFallbackWindowGeometry();
}

/*!
 * \qmlproperty XdgShell QtWaylandCompositor::XdgSurface::shell
 *
 * This property holds the shell associated with this XdgSurface.
 */

/*!
 * \property QWaylandXdgSurface::shell
 *
 * This property holds the shell associated with this QWaylandXdgSurface.
 */
QWaylandXdgShell *QWaylandXdgSurface::shell() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_xdgShell;
}

/*!
 * \qmlproperty WaylandSurface QtWaylandCompositor::XdgSurface::surface
 *
 * This property holds the surface associated with this XdgSurface.
 */

/*!
 * \property QWaylandXdgSurface::surface
 *
 * This property holds the surface associated with this QWaylandXdgSurface.
 */
QWaylandSurface *QWaylandXdgSurface::surface() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_surface;
}

/*!
 * \qmlproperty XdgToplevel QtWaylandCompositor::XdgSurface::toplevel
 *
 * This property holds the properties and methods that are specific to the
 * toplevel XdgSurface.
 *
 * \sa popup, XdgShell::toplevelCreated
 */

/*!
 * \property QWaylandXdgSurface::toplevel
 *
 * This property holds the properties and methods that are specific to the
 * toplevel QWaylandXdgSurface.
 *
 * \sa QWaylandXdgSurface::popup, QWaylandXdgShell::toplevelCreated
 */
QWaylandXdgToplevel *QWaylandXdgSurface::toplevel() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_toplevel;
}

/*!
 * \qmlproperty XdgPopup QtWaylandCompositor::XdgSurface::popup
 *
 * This property holds the properties and methods that are specific to the
 * popup XdgSurface.
 *
 * \sa toplevel, XdgShell::popupCreated
 */

/*!
 * \property QWaylandXdgSurface::popup
 *
 * This property holds the properties and methods that are specific to the
 * popup QWaylandXdgSurface.
 *
 * \sa QWaylandXdgSurface::toplevel, QWaylandXdgShell::popupCreated
 */
QWaylandXdgPopup *QWaylandXdgSurface::popup() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_popup;
}

/*!
 * Returns the Wayland interface for the QWaylandXdgSurface.
 */
const wl_interface *QWaylandXdgSurface::interface()
{
    return QWaylandXdgSurfacePrivate::interface();
}

/*!
 * \internal
 */
QByteArray QWaylandXdgSurface::interfaceName()
{
    return QWaylandXdgSurfacePrivate::interfaceName();
}

/*!
 * Returns the QWaylandXdgSurface corresponding to the \a resource.
 */
QWaylandXdgSurface *QWaylandXdgSurface::fromResource(wl_resource *resource)
{
    auto xsResource = QWaylandXdgSurfacePrivate::Resource::fromResource(resource);
    if (!xsResource)
        return nullptr;
    return static_cast<QWaylandXdgSurfacePrivate *>(xsResource->xdg_surface_object)->q_func();
}

#if QT_CONFIG(wayland_compositor_quick)
QWaylandQuickShellIntegration *QWaylandXdgSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    Q_D(const QWaylandXdgSurface);

    if (d->m_toplevel)
        return new QtWayland::XdgToplevelIntegration(item);

    if (d->m_popup)
        return new QtWayland::XdgPopupIntegration(item);

    return nullptr;
}
#endif

/*!
 * \qmltype XdgToplevel
 * \inqmlmodule QtWayland.Compositor
 * \since 5.12
 * \brief XdgToplevel represents the toplevel window specific parts of an xdg surface.
 *
 * This type is part of the \l{XdgShell} extension and provides a way to
 * extend the functionality of an XdgSurface with features
 * specific to desktop-style windows.
 *
 * It corresponds to the Wayland interface \c xdg_toplevel.
 */

/*!
 * \class QWaylandXdgToplevel
 * \inmodule QtWaylandCompositor
 * \since 5.12
 * \brief The QWaylandXdgToplevel class represents the toplevel window specific parts of an xdg surface.
 *
 * This class is part of the QWaylandXdgShell extension and provides a way to
 * extend the functionality of an QWaylandXdgSurface with features
 * specific to desktop-style windows.
 *
 * It corresponds to the Wayland interface \c xdg_toplevel.
 */

/*!
 * Constructs a QWaylandXdgToplevel for the given \a xdgSurface and \a resource.
 */
QWaylandXdgToplevel::QWaylandXdgToplevel(QWaylandXdgSurface *xdgSurface, QWaylandResource &resource)
    : QObject(*new QWaylandXdgToplevelPrivate(xdgSurface, resource))
{
    QVector<QWaylandXdgToplevel::State> states;
    sendConfigure({0, 0}, states);
}

QWaylandXdgToplevel::~QWaylandXdgToplevel()
{
    Q_D(QWaylandXdgToplevel);
    // Usually, the decoration is destroyed by the client (according to the protocol),
    // but if the client misbehaves, or is shut down, we need to clean up here.
    if (Q_UNLIKELY(d->m_decoration))
        wl_resource_destroy(d->m_decoration->resource()->handle);
    Q_ASSERT(!d->m_decoration);
}

/*!
 * \qmlproperty XdgSurface QtWaylandCompositor::XdgToplevel::xdgSurface
 *
 * This property holds the XdgSurface for this XdgToplevel.
 */

/*!
 * \property QWaylandXdgToplevel::xdgSurface
 *
 * This property holds the QWaylandXdgSurface for this QWaylandXdgToplevel.
 */
QWaylandXdgSurface *QWaylandXdgToplevel::xdgSurface() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_xdgSurface;
}

/*!
 * \qmlproperty XdgToplevel QtWaylandCompositor::XdgToplevel::parentToplevel
 *
 * This property holds the XdgToplevel parent of this XdgToplevel.
 */

/*!
 * \property QWaylandXdgToplevel::parentToplevel
 *
 * This property holds the XdgToplevel parent of this XdgToplevel.
 *
 */
QWaylandXdgToplevel *QWaylandXdgToplevel::parentToplevel() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_parentToplevel;
}

/*!
 * \qmlproperty string QtWaylandCompositor::XdgToplevel::title
 *
 * This property holds the title of the XdgToplevel.
 */

/*!
 * \property QWaylandXdgToplevel::title
 *
 * This property holds the title of the QWaylandXdgToplevel.
 */
QString QWaylandXdgToplevel::title() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_title;
}

/*!
 * \qmlproperty string QtWaylandCompositor::XdgToplevel::appId
 *
 * This property holds the app id of the XdgToplevel.
 */

/*!
 * \property QWaylandXdgToplevel::appId
 *
 * This property holds the app id of the QWaylandXdgToplevel.
 */
QString QWaylandXdgToplevel::appId() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_appId;
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgToplevel::maxSize
 *
 * This property holds the maximum size of the XdgToplevel as requested by the client.
 *
 * The compositor is free to ignore this value and request a larger size.
 */

/*!
 * \property QWaylandXdgToplevel::maxSize
 *
 * This property holds the maximum size of the QWaylandXdgToplevel.
 *
 * The compositor is free to ignore this value and request a larger size.
 */
QSize QWaylandXdgToplevel::maxSize() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_maxSize;
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgToplevel::minSize
 *
 * This property holds the minimum size of the XdgToplevel as requested by the client.
 *
 * The compositor is free to ignore this value and request a smaller size.
 */

/*!
 * \property QWaylandXdgToplevel::minSize
 *
 * This property holds the minimum size of the QWaylandXdgToplevel.
 *
 * The compositor is free to ignore this value and request a smaller size.
 */
QSize QWaylandXdgToplevel::minSize() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_minSize;
}

/*!
 * \property QWaylandXdgToplevel::states
 *
 * This property holds the last states the client acknowledged for this QWaylandToplevel.
 */
QVector<QWaylandXdgToplevel::State> QWaylandXdgToplevel::states() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_lastAckedConfigure.states;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevel::maximized
 *
 * This property holds whether the client has acknowledged that it should be maximized.
 */

/*!
 * \property QWaylandXdgToplevel::maximized
 *
 * This property holds whether the client has acknowledged that it should be maximized.
 */
bool QWaylandXdgToplevel::maximized() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevel::State::MaximizedState);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevel::fullscreen
 *
 * This property holds whether the client has acknowledged that it should be fullscreen.
 */

/*!
 * \property QWaylandXdgToplevel::fullscreen
 *
 * This property holds whether the client has acknowledged that it should be fullscreen.
 */
bool QWaylandXdgToplevel::fullscreen() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevel::State::FullscreenState);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevel::resizing
 *
 * This property holds whether the client has acknowledged that it is being resized.
 */

/*!
 * \property QWaylandXdgToplevel::resizing
 *
 * This property holds whether the client has acknowledged that it is being resized.
 */
bool QWaylandXdgToplevel::resizing() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevel::State::ResizingState);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevel::activated
 *
 * This property holds whether toplevel is drawing itself as having input focus.
 */

/*!
 * \property QWaylandXdgToplevel::activated
 *
 * This property holds whether toplevel is drawing itself as having input focus.
 */
bool QWaylandXdgToplevel::activated() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevel::State::ActivatedState);
}

/*!
 * \enum QWaylandXdgToplevel::DecorationMode
 *
 * This enum type is used to specify the window decoration mode for toplevel windows.
 *
 * \value ServerSideDecoration The compositor should draw window decorations.
 * \value ClientSideDecoration The client should draw window decorations.
 */

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgToplevel::decorationMode
 *
 * This property holds the current window decoration mode for this toplevel.
 *
 * The possible values are:
 * \value XdgToplevel.ServerSideDecoration The compositor should draw window decorations.
 * \value XdgToplevel.ClientSideDecoration The client should draw window decorations.
 *
 * \sa XdgDecorationManagerV1
 */

/*!
 * \property QWaylandXdgToplevel::decorationMode
 *
 * This property holds the current window decoration mode for this toplevel.
 *
 * \sa QWaylandXdgDecorationManagerV1
 */
QWaylandXdgToplevel::DecorationMode QWaylandXdgToplevel::decorationMode() const
{
    Q_D(const QWaylandXdgToplevel);
    return d->m_decoration ? d->m_decoration->configuredMode() : DecorationMode::ClientSideDecoration;
}

/*!
 * \qmlmethod size QtWaylandCompositor::XdgToplevel::sizeForResize(size size, point delta, uint edges)
 *
 * Convenience for computing the new size given the current \a size, a \a delta, and
 * the \a edges active in the drag.
 */

/*!
 * Convenience for computing the new size given the current \a size, a \a delta, and
 * the \a edges active in the drag.
 */
QSize QWaylandXdgToplevel::sizeForResize(const QSizeF &size, const QPointF &delta, Qt::Edges edges) const
{
    qreal width = size.width();
    qreal height = size.height();
    if (edges & Qt::LeftEdge)
        width -= delta.x();
    else if (edges & Qt::RightEdge)
        width += delta.x();

    if (edges & Qt::TopEdge)
        height -= delta.y();
    else if (edges & Qt::BottomEdge)
        height += delta.y();

    QSize newSize = QSize(width, height)
            .expandedTo(minSize())
            .expandedTo({1, 1}); // We don't want to send a size of (0,0) as that means that the client decides

    if (maxSize().isValid())
        newSize = newSize.boundedTo(maxSize());

    return newSize;
}

/*!
 * Sends a configure event to the client. Parameter \a size contains the pixel size
 * of the surface. A size of zero means the client is free to decide the size.
 * Known \a states are enumerated in QWaylandXdgToplevel::State.
 */
uint QWaylandXdgToplevel::sendConfigure(const QSize &size, const QVector<QWaylandXdgToplevel::State> &states)
{
    if (!size.isValid()) {
        qWarning() << "Can't configure xdg_toplevel with an invalid size" << size;
        return 0;
    }
    Q_D(QWaylandXdgToplevel);
    auto statesBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(states.data()),
                                               states.size() * static_cast<int>(sizeof(State)));
    uint32_t serial = d->m_xdgSurface->surface()->compositor()->nextSerial();
    d->m_pendingConfigures.append(QWaylandXdgToplevelPrivate::ConfigureEvent{states, size, serial});
    d->send_configure(size.width(), size.height(), statesBytes);
    QWaylandXdgSurfacePrivate::get(d->m_xdgSurface)->send_configure(serial);
    return serial;
}

/*!
 * \qmlmethod int QtWaylandCompositor::XdgToplevel::sendConfigure(size size, list<int> states)
 *
 * Sends a configure event to the client. \a size contains the pixel size of the surface.
 * A size of zero means the client is free to decide the size.
 * Known \a states are enumerated in XdgToplevel::State.
 */
uint QWaylandXdgToplevel::sendConfigure(const QSize &size, const QVector<int> &states)
{
    QVector<State> s;
    for (auto state : states)
        s << State(state);
    return sendConfigure(size, s);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevel::sendClose()
 *
 * Sends a close event to the client. The client may choose to ignore the event.
 */

/*!
 * Sends a close event to the client. The client may choose to ignore the event.
 */
void QWaylandXdgToplevel::sendClose()
{
    Q_D(QWaylandXdgToplevel);
    d->send_close();
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevel::sendMaximized(size size)
 *
 * Convenience for sending a configure event with the maximized state set, and
 * fullscreen and resizing removed. The activated state is left in its current state.
 *
 * \a size is the new size of the window.
 */

/*!
 * Convenience for sending a configure event with the maximized state set, and
 * fullscreen and resizing removed. The activated state is left in its current state.
 *
 * \a size is the new size of the window.
 */
uint QWaylandXdgToplevel::sendMaximized(const QSize &size)
{
    Q_D(QWaylandXdgToplevel);
    QWaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgToplevel::State::MaximizedState))
        conf.states.append(QWaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevel::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgToplevel::State::ResizingState);

    return sendConfigure(size, conf.states);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevel::sendUnmaximized(size size)
 *
 * Convenience for sending a configure event with the maximized, fullscreen and
 * resizing states removed, and fullscreen and resizing removed. The activated
 * state is left in its current state.
 *
 * \a size is the new size of the window. If \a size is zero, the client decides the size.
 */

/*!
 * Convenience for sending a configure event with the maximized, fullscreen and
 * resizing states removed, and fullscreen and resizing removed. The activated
 * state is left in its current state.
 *
 * \a size is the new size of the window. If \a size is zero, the client decides the size.
 */
uint QWaylandXdgToplevel::sendUnmaximized(const QSize &size)
{
    Q_D(QWaylandXdgToplevel);
    QWaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    conf.states.removeOne(QWaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevel::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgToplevel::State::ResizingState);

    return sendConfigure(size, conf.states);

}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevel::sendFullscreen(size size)
 *
 * Convenience for sending a configure event with the fullscreen state set, and
 * maximized and resizing removed. The activated state is left in its current state.
 *
 * \sa sendUnmaximized
 *
 * \a size is the new size of the window.
 */

/*!
 * Convenience for sending a configure event with the fullscreen state set, and
 * maximized and resizing removed. The activated state is left in its current state.
 *
 * \sa sendUnmaximized
 *
 * \a size is the new size of the window.
 */
uint QWaylandXdgToplevel::sendFullscreen(const QSize &size)
{
    Q_D(QWaylandXdgToplevel);
    QWaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgToplevel::State::FullscreenState))
        conf.states.append(QWaylandXdgToplevel::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevel::State::ResizingState);

    return sendConfigure(size, conf.states);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevel::sendResizing(size maxSize)
 *
 * Convenience for sending a configure event with the resizing state set, and
 * maximized and fullscreen removed. The activated state is left in its current state.
 *
 * \a maxSize is the new size of the window.
 */

/*!
 * Convenience for sending a configure event with the resizing state set, and
 * maximized and fullscreen removed. The activated state is left in its current state.
 *
 * \a maxSize is the new size of the window.
 */
uint QWaylandXdgToplevel::sendResizing(const QSize &maxSize)
{
    Q_D(QWaylandXdgToplevel);
    QWaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgToplevel::State::ResizingState))
        conf.states.append(QWaylandXdgToplevel::State::ResizingState);
    conf.states.removeOne(QWaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevel::State::FullscreenState);

    return sendConfigure(maxSize, conf.states);
}

/*!
 * Returns the surface role for the QWaylandToplevel.
 */
QWaylandSurfaceRole *QWaylandXdgToplevel::role()
{
    return &QWaylandXdgToplevelPrivate::s_role;
}

/*!
 * Returns the QWaylandXdgToplevel corresponding to the \a resource.
 */
QWaylandXdgToplevel *QWaylandXdgToplevel::fromResource(wl_resource *resource)
{
    if (auto *r = QWaylandXdgToplevelPrivate::Resource::fromResource(resource))
        return static_cast<QWaylandXdgToplevelPrivate *>(r->xdg_toplevel_object)->q_func();
    return nullptr;
}

/*!
 * \qmlsignal QtWaylandCompositor::XdgShell::xdgSurfaceCreated(XdgSurface xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_surface.
 * Note that \a xdgSurface is not mapped, i.e. according to the \c xdg-shell
 * protocol it should not be displayed, until it has received a role object.
 *
 * \sa toplevelCreated(), popupCreated()
 */

/*!
 * \fn void QWaylandXdgShell::xdgSurfaceCreated(QWaylandXdgSurface *xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_surface.
 * Note that \a xdgSurface is not mapped, i.e. according to the \c xdg-shell
 * protocol it should not be displayed, until it has received a role object.
 *
 * \sa toplevelCreated(), popupCreated()
 */

/*!
 * \qmlsignal QtWaylandCompositor::XdgShell::toplevelCreated(XdgToplevel toplevel, XdgSurface xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_toplevel.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a toplevel in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a toplevel is the role object for.
 */

/*!
 * \fn void QWaylandXdgShell::toplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_toplevel.
 * A common use case is to let the handler of this signal instantiate a QWaylandShellSurfaceItem or
 * QWaylandQuickItem for displaying \a toplevel in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a toplevel is the role object for.
 */

/*!
 * \qmlsignal QtWaylandCompositor::XdgShell::popupCreated(XdgPopup popup, XdgSurface xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_popup.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a popup in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a popup is the role object for.
 */

/*!
 * \fn void QWaylandXdgShell::popupCreated(QWaylandXdgPopup *popup, QWaylandXdgSurface *xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_popup.
 * A common use case is to let the handler of this signal instantiate a QWaylandShellSurfaceItem or
 * QWaylandQuickItem for displaying \a popup in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a popup is the role object for.
 */

/*!
 * \qmlsignal QtWaylandCompositor::XdgShell::pong(int serial)
 *
 * This signal is emitted when the client has responded to a ping event with serial, \a serial.
 *
 * \sa ping()
 */

/*!
 * \fn void QWaylandXdgShell::pong(uint serial)
 *
 * This signal is emitted when the client has responded to a ping event with serial, \a serial.
 *
 * \sa QWaylandXdgShell::ping()
 */

QList<int> QWaylandXdgToplevel::statesAsInts() const
{
   QList<int> list;
   Q_FOREACH (uint state, states()) {
       list << static_cast<int>(state);
   }
   return list;
}

QWaylandSurfaceRole QWaylandXdgToplevelPrivate::s_role("xdg_toplevel");

QWaylandXdgToplevelPrivate::QWaylandXdgToplevelPrivate(QWaylandXdgSurface *xdgSurface, const QWaylandResource &resource)
    : m_xdgSurface(xdgSurface)
{
    init(resource.resource());
}

void QWaylandXdgToplevelPrivate::handleAckConfigure(uint serial)
{
    Q_Q(QWaylandXdgToplevel);
    ConfigureEvent config;
    Q_FOREVER {
        if (m_pendingConfigures.empty()) {
            qWarning("Toplevel received an unexpected ack_configure!");
            return;
        }

        // This won't work unless there always is a toplevel.configure for each xdgsurface.configure
        config = m_pendingConfigures.takeFirst();

        if (config.serial == serial)
            break;
    }

    QVector<uint> changedStates;
    std::set_symmetric_difference(
                m_lastAckedConfigure.states.begin(), m_lastAckedConfigure.states.end(),
                config.states.begin(), config.states.end(),
                std::back_inserter(changedStates));

    m_lastAckedConfigure = config;

    for (uint state : changedStates) {
        switch (state) {
        case state_maximized:
            emit q->maximizedChanged();
            break;
        case state_fullscreen:
            emit q->fullscreenChanged();
            break;
        case state_resizing:
            emit q->resizingChanged();
            break;
        case state_activated:
            emit q->activatedChanged();
            break;
        }
    }

    if (!changedStates.empty())
        emit q->statesChanged();
}

void QWaylandXdgToplevelPrivate::handleFocusLost()
{
    Q_Q(QWaylandXdgToplevel);
    QWaylandXdgToplevelPrivate::ConfigureEvent current = lastSentConfigure();
    current.states.removeOne(QWaylandXdgToplevel::State::ActivatedState);
    q->sendConfigure(current.size, current.states);
}

void QWaylandXdgToplevelPrivate::handleFocusReceived()
{
    Q_Q(QWaylandXdgToplevel);
    QWaylandXdgToplevelPrivate::ConfigureEvent current = lastSentConfigure();
    if (!current.states.contains(QWaylandXdgToplevel::State::ActivatedState)) {
        current.states.push_back(QWaylandXdgToplevel::State::ActivatedState);
        q->sendConfigure(current.size, current.states);
    }
}

Qt::Edges QWaylandXdgToplevelPrivate::convertToEdges(resize_edge edge)
{
    return Qt::Edges(((edge & 0b1100) >> 1) | ((edge & 0b0010) << 2) | (edge & 0b0001));
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_destroy_resource(QtWaylandServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevel);
    delete q;
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_destroy(QtWaylandServer::xdg_toplevel::Resource *resource)
{
    if (Q_UNLIKELY(m_decoration))
        qWarning() << "Client error: xdg_toplevel destroyed before its decoration object";

    wl_resource_destroy(resource->handle);
    //TODO: Should the xdg surface be desroyed as well? Or is it allowed to recreate a new toplevel for it?
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_parent(QtWaylandServer::xdg_toplevel::Resource *resource, wl_resource *parent)
{
    Q_UNUSED(resource);
    QWaylandXdgToplevel *parentToplevel = QWaylandXdgToplevel::fromResource(parent);

    Q_Q(QWaylandXdgToplevel);

    if (m_parentToplevel != parentToplevel) {
        m_parentToplevel = parentToplevel;
        emit q->parentToplevelChanged();
    }

    if (m_parentToplevel && m_xdgSurface->windowType() != Qt::WindowType::SubWindow) {
        // There's a parent now, which means the surface is transient
        QWaylandXdgSurfacePrivate::get(m_xdgSurface)->setWindowType(Qt::WindowType::SubWindow);
    } else if (!m_parentToplevel && m_xdgSurface->windowType() != Qt::WindowType::Window) {
        // When the surface has no parent it is toplevel
        QWaylandXdgSurfacePrivate::get(m_xdgSurface)->setWindowType(Qt::WindowType::Window);
    }
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_title(QtWaylandServer::xdg_toplevel::Resource *resource, const QString &title)
{
    Q_UNUSED(resource);
    if (title == m_title)
        return;
    Q_Q(QWaylandXdgToplevel);
    m_title = title;
    emit q->titleChanged();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_app_id(QtWaylandServer::xdg_toplevel::Resource *resource, const QString &app_id)
{
    Q_UNUSED(resource);
    if (app_id == m_appId)
        return;
    Q_Q(QWaylandXdgToplevel);
    m_appId = app_id;
    emit q->appIdChanged();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_show_window_menu(QtWaylandServer::xdg_toplevel::Resource *resource, wl_resource *seatResource, uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    QPoint position(x, y);
    auto seat = QWaylandSeat::fromSeatResource(seatResource);
    Q_Q(QWaylandXdgToplevel);
    emit q->showWindowMenu(seat, position);
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_move(Resource *resource, wl_resource *seatResource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandXdgToplevel);
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    emit q->startMove(seat);
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_resize(QtWaylandServer::xdg_toplevel::Resource *resource, wl_resource *seatResource, uint32_t serial, uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandXdgToplevel);
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    emit q->startResize(seat, convertToEdges(resize_edge(edges)));
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_max_size(QtWaylandServer::xdg_toplevel::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    QSize maxSize(width, height);
    if (width == 0 && height == 0)
        maxSize = QSize(); // Wayland size of zero means unspecified which best translates to invalid

    if (m_maxSize == maxSize)
        return;

    if (width < 0 || height < 0) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_max_size request with a negative size";
        return;
    }

    if (m_minSize.isValid() && maxSize.isValid() &&
            (maxSize.width() < m_minSize.width() || maxSize.height() < m_minSize.height())) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_max_size request with a size smaller than the minimium size";
        return;
    }

    m_maxSize = maxSize;

    Q_Q(QWaylandXdgToplevel);
    emit q->maxSizeChanged();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_min_size(QtWaylandServer::xdg_toplevel::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    QSize minSize(width, height);
    if (width == 0 && height == 0)
        minSize = QSize(); // Wayland size of zero means unspecified

    if (m_minSize == minSize)
        return;

    if (width < 0 || height < 0) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_min_size request with a negative size";
        return;
    }

    if (m_maxSize.isValid() && minSize.isValid() &&
            (minSize.width() > m_maxSize.width() || minSize.height() > m_maxSize.height())) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_min_size request with a size larger than the maximum size";
        return;
    }

    m_minSize = minSize;

    Q_Q(QWaylandXdgToplevel);
    emit q->minSizeChanged();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_maximized(QtWaylandServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevel);
    emit q->setMaximized();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_unset_maximized(QtWaylandServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevel);
    emit q->unsetMaximized();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_fullscreen(QtWaylandServer::xdg_toplevel::Resource *resource, wl_resource *output_res)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevel);
    QWaylandOutput *output = output_res ? QWaylandOutput::fromResource(output_res) : nullptr;
    emit q->setFullscreen(output);
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_unset_fullscreen(QtWaylandServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevel);
    emit q->unsetFullscreen();
}

void QWaylandXdgToplevelPrivate::xdg_toplevel_set_minimized(QtWaylandServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevel);
    emit q->setMinimized();
}

/*!
 * \qmltype XdgPopup
 * \inqmlmodule QtWayland.Compositor
 * \since 5.12
 * \brief XdgPopup represents the popup specific parts of and xdg surface.
 *
 * This type is part of the \l{XdgShell} extension and provides a way to extend
 * extend the functionality of an \l{XdgSurface} with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface \c xdg_popup.
 */

/*!
 * \class QWaylandXdgPopup
 * \inmodule QtWaylandCompositor
 * \since 5.12
 * \brief The QWaylandXdgPopup class represents the popup specific parts of an xdg surface.
 *
 * This class is part of the QWaylandXdgShell extension and provides a way to
 * extend the functionality of a QWaylandXdgSurface with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface \c xdg_popup.
 */

/*!
 * Constructs a QWaylandXdgPopup.
 */
QWaylandXdgPopup::QWaylandXdgPopup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parentXdgSurface,
                                   QWaylandXdgPositioner *positioner, QWaylandResource &resource)
    : QObject(*new QWaylandXdgPopupPrivate(xdgSurface, parentXdgSurface, positioner, resource))
{
}

/*!
 * \qmlproperty XdgSurface QtWaylandCompositor::XdgPopup::xdgSurface
 *
 * This property holds the XdgSurface associated with this XdgPopup.
 */

/*!
 * \property QWaylandXdgPopup::xdgSurface
 *
 * This property holds the QWaylandXdgSurface associated with this QWaylandXdgPopup.
 */
QWaylandXdgSurface *QWaylandXdgPopup::xdgSurface() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_xdgSurface;
}

/*!
 * \qmlproperty XdgSurface QtWaylandCompositor::XdgPopup::parentXdgSurface
 *
 * This property holds the XdgSurface associated with the parent of this XdgPopup.
 */

/*!
 * \property QWaylandXdgPopup::parentXdgSurface
 *
 * This property holds the QWaylandXdgSurface associated with the parent of this
 * QWaylandXdgPopup.
 */
QWaylandXdgSurface *QWaylandXdgPopup::parentXdgSurface() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_parentXdgSurface;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgPopup::configuredGeometry
 *
 * The window geometry the popup received in the configure event. Relative to the
 * upper left corner of the parent surface.
 */

/*!
 * \property QWaylandXdgPopup::configuredGeometry
 *
 * The window geometry the popup received in the configure event. Relative to the
 * upper left corner of the parent surface.
 */
QRect QWaylandXdgPopup::configuredGeometry() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_geometry;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgPopup::anchorRect
 *
 * The anchor rectangle relative to the parent window geometry that the child
 * surface should be placed relative to.
 */

/*!
 * \property QWaylandXdgPopup::anchorRect
 *
 * Returns the anchor rectangle relative to the parent window geometry that the child
 * surface should be placed relative to.
 */
QRect QWaylandXdgPopup::anchorRect() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_positionerData.anchorRect;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopup::anchorEdges
 *
 * This property holds the set of edges on the anchor rect that the child surface should be placed
 * relative to. If no edges are specified in a direction, the anchor point should be
 * centered between the edges.
 *
 * The possible values are:
 * \value Qt.TopEdge The top edge of the rectangle.
 * \value Qt.LeftEdge The left edge of the rectangle.
 * \value Qt.RightEdge The right edge of the rectangle.
 * \value Qt.BottomEdge The bottom edge of the rectangle.
 */

/*!
 * \property QWaylandXdgPopup::anchorEdges
 *
 * Returns the set of edges on the anchor rect that the child surface should be placed
 * relative to. If no edges are specified in a direction, the anchor point should be
 * centered between the edges.
 */
Qt::Edges QWaylandXdgPopup::anchorEdges() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_positionerData.anchorEdges;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgPopup::gravityEdges
 *
 * Specifies in what direction the surface should be positioned, relative to the anchor
 * point.
 *
 * The possible values are:
 * \value Qt.TopEdge The surface should slide towards the top of the screen.
 * \value Qt.LeftEdge The surface should slide towards the left of the screen.
 * \value Qt.RightEdge The surface should slide towards the right of the screen.
 * \value Qt.BottomEdge The surface should slide towards the bottom of the screen.
 */

/*!
 * \property QWaylandXdgPopup::gravityEdges
 *
 * Specifies in what direction the surface should be positioned, relative to the anchor
 * point.
 */
Qt::Edges QWaylandXdgPopup::gravityEdges() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_positionerData.gravityEdges;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopup::slideConstraints
 *
 * This property holds the orientations in which the child should slide to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property QWaylandXdgPopup::slideConstraints
 *
 * This property holds the orientations in which the child should slide to fit within the screen.
 */
Qt::Orientations QWaylandXdgPopup::slideConstraints() const
{
    Q_D(const QWaylandXdgPopup);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = 0;

    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X)
        constraints |= Qt::Horizontal;
    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopup::flipConstraints
 *
 * This property holds the orientations in which the child should flip to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property QWaylandXdgPopup::flipConstraints
 *
 * This property holds the orientations in which the child should flip to fit within the screen.
 */
Qt::Orientations QWaylandXdgPopup::flipConstraints() const
{
    Q_D(const QWaylandXdgPopup);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = 0;

    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X)
        constraints |= Qt::Horizontal;
    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopup::resizeConstraints
 *
 * This property holds the orientations in which the child should resize to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property QWaylandXdgPopup::resizeConstraints
 *
 * This property holds the orientations in which the child should resize to fit within the screen.
 */
Qt::Orientations QWaylandXdgPopup::resizeConstraints() const
{
    Q_D(const QWaylandXdgPopup);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = 0;

    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X)
        constraints |= Qt::Horizontal;
    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty point QtWaylandCompositor::XdgPopup::offset
 *
 * The position relative to the position of the anchor on the anchor rectangle and
 * the anchor on the surface.
 */

/*!
 * \property QWaylandXdgPopup::offset
 *
 * Returns the surface position relative to the position of the anchor on the anchor
 * rectangle and the anchor on the surface.
 */
QPoint QWaylandXdgPopup::offset() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_positionerData.offset;
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgPopup::positionerSize
 *
 * The size requested for the window geometry by the positioner object.
 */

/*!
 * \property QWaylandXdgPopup::positionerSize
 *
 * Returns the size requested for the window geometry by the positioner object.
 */
QSize QWaylandXdgPopup::positionerSize() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_positionerData.size;
}

/*!
 * \qmlproperty point QtWaylandCompositor::XdgPopup::unconstrainedPosition
 *
 * The position of the surface relative to the parent window geometry if the surface
 * is not constrained. I.e. when not moved to fit inside the screen or similar.
 */

/*!
 * \property QWaylandXdgPopup::unconstrainedPosition
 *
 * The position of the surface relative to the parent window geometry if the surface
 * is not constrained. I.e. when not moved to fit inside the screen or similar.
 */
QPoint QWaylandXdgPopup::unconstrainedPosition() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_positionerData.unconstrainedPosition();
}

/*!
 * \qmlmethod int QtWaylandCompositor::XdgPopup::sendConfigure(rect geometry)
 *
 * Sends a configure event to the client. \a geometry contains the window geometry
 * relative to the upper left corner of the window geometry of the parent surface.
 *
 * This implicitly sends a configure event to the corresponding XdgSurface as well.
 */

/*!
 * Sends a configure event to the client. \a geometry contains the window geometry
 * relative to the upper left corner of the window geometry of the parent surface.
 *
 * This implicitly sends a configure event to the corresponding QWaylandXdgSurface
 * as well.
 */
uint QWaylandXdgPopup::sendConfigure(const QRect &geometry)
{
    Q_D(QWaylandXdgPopup);
    return d->sendConfigure(geometry);
}

/*!
 * Returns the surface role for the QWaylandPopup.
 */
QWaylandSurfaceRole *QWaylandXdgPopup::role()
{
    return &QWaylandXdgPopupPrivate::s_role;
}

QWaylandXdgPopupPrivate::QWaylandXdgPopupPrivate(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parentXdgSurface,
                                                 QWaylandXdgPositioner *positioner, const QWaylandResource &resource)
    : m_xdgSurface(xdgSurface)
    , m_parentXdgSurface(parentXdgSurface)
{
    init(resource.resource());
    m_positionerData = positioner->m_data;

    if (!m_positionerData.isComplete())
        qWarning() << "Trying to create xdg popup with incomplete positioner";

    QWaylandXdgSurfacePrivate::get(m_xdgSurface)->setWindowType(Qt::WindowType::Popup);

    //TODO: positioner rect may not extend parent's window geometry, enforce this?
    //TODO: Need an API for sending a different initial configure
    sendConfigure(QRect(m_positionerData.unconstrainedPosition(), m_positionerData.size));
}

void QWaylandXdgPopupPrivate::handleAckConfigure(uint serial)
{
    Q_Q(QWaylandXdgPopup);
    ConfigureEvent config;
    Q_FOREVER {
        if (m_pendingConfigures.empty()) {
            qWarning("Popup received an unexpected ack_configure!");
            return;
        }

        // This won't work unless there always is a popup.configure for each xdgsurface.configure
        config = m_pendingConfigures.takeFirst();

        if (config.serial == serial)
            break;
    }

    if (m_geometry == config.geometry)
        return;

    m_geometry = config.geometry;
    emit q->configuredGeometryChanged();
}

uint QWaylandXdgPopupPrivate::sendConfigure(const QRect &geometry)
{
    uint32_t serial = m_xdgSurface->surface()->compositor()->nextSerial();
    m_pendingConfigures.append(QWaylandXdgPopupPrivate::ConfigureEvent{geometry, serial});
    send_configure(geometry.x(), geometry.y(), geometry.width(), geometry.height());
    QWaylandXdgSurfacePrivate::get(m_xdgSurface)->send_configure(serial);
    return serial;
}

void QWaylandXdgPopupPrivate::xdg_popup_destroy(QtWaylandServer::xdg_popup::Resource *resource)
{
    Q_UNUSED(resource);
    qWarning() << Q_FUNC_INFO << "Not implemented"; //TODO
}

void QWaylandXdgPopupPrivate::xdg_popup_grab(QtWaylandServer::xdg_popup::Resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_UNUSED(seat);
    qWarning() << Q_FUNC_INFO << "Not implemented"; //TODO
    //switch keyboard focus
    //eventually send configure with activated.
}

QWaylandSurfaceRole QWaylandXdgPopupPrivate::s_role("xdg_popup");

QWaylandXdgPositionerData::QWaylandXdgPositionerData()
    : offset(0, 0)
{}

bool QWaylandXdgPositionerData::isComplete() const
{
    return size.width() > 0 && size.height() > 0 && anchorRect.size().width() > 0 && anchorRect.size().height() > 0;
}

QPoint QWaylandXdgPositionerData::anchorPoint() const
{
    int yPosition = 0;
    if (anchorEdges & Qt::TopEdge)
        yPosition = anchorRect.top();
    else if (anchorEdges & Qt::BottomEdge)
        yPosition = anchorRect.bottom() + 1;
    else
        yPosition = anchorRect.top() + anchorRect.height() / 2;

    int xPosition = 0;
    if (anchorEdges & Qt::LeftEdge)
        xPosition = anchorRect.left();
    else if (anchorEdges & Qt::RightEdge)
        xPosition = anchorRect.right() + 1;
    else
        xPosition = anchorRect.left() + anchorRect.width() / 2;

    return QPoint(xPosition, yPosition);
}

QPoint QWaylandXdgPositionerData::unconstrainedPosition() const
{
    int gravityOffsetY = 0;
    if (gravityEdges & Qt::TopEdge)
        gravityOffsetY = -size.height();
    else if (!(gravityEdges & Qt::BottomEdge))
        gravityOffsetY = -size.height() / 2;

    int gravityOffsetX = 0;
    if (gravityEdges & Qt::LeftEdge)
        gravityOffsetX = -size.width();
    else if (!(gravityEdges & Qt::RightEdge))
        gravityOffsetX = -size.width() / 2;

    QPoint gravityOffset(gravityOffsetX, gravityOffsetY);
    return anchorPoint() + gravityOffset + offset;
}

QWaylandXdgPositioner::QWaylandXdgPositioner(const QWaylandResource &resource)
{
    init(resource.resource());
}

void QWaylandXdgPositioner::xdg_positioner_destroy_resource(QtWaylandServer::xdg_positioner::Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandXdgPositioner::xdg_positioner_destroy(QtWaylandServer::xdg_positioner::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgPositioner::xdg_positioner_set_size(QtWaylandServer::xdg_positioner::Resource *resource, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_size requested with non-positive dimensions");
        return;
    }

    QSize size(width, height);
    m_data.size = size;
}

void QWaylandXdgPositioner::xdg_positioner_set_anchor_rect(QtWaylandServer::xdg_positioner::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_anchor_rect requested with non-positive dimensions");
        return;
    }

    QRect anchorRect(x, y, width, height);
    m_data.anchorRect = anchorRect;
}

void QWaylandXdgPositioner::xdg_positioner_set_anchor(QtWaylandServer::xdg_positioner::Resource *resource, uint32_t anchor)
{
    Qt::Edges anchorEdges = convertToEdges(xdg_positioner::anchor(anchor));

    if ((anchorEdges & Qt::BottomEdge && anchorEdges & Qt::TopEdge) ||
            (anchorEdges & Qt::LeftEdge && anchorEdges & Qt::RightEdge)) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_anchor requested with parallel edges");
        return;
    }

    m_data.anchorEdges = anchorEdges;
}

void QWaylandXdgPositioner::xdg_positioner_set_gravity(QtWaylandServer::xdg_positioner::Resource *resource, uint32_t gravity)
{
    Qt::Edges gravityEdges = convertToEdges(xdg_positioner::gravity(gravity));

    if ((gravityEdges & Qt::BottomEdge && gravityEdges & Qt::TopEdge) ||
            (gravityEdges & Qt::LeftEdge && gravityEdges & Qt::RightEdge)) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_gravity requested with parallel edges");
        return;
    }

    m_data.gravityEdges = gravityEdges;
}

void QWaylandXdgPositioner::xdg_positioner_set_constraint_adjustment(QtWaylandServer::xdg_positioner::Resource *resource, uint32_t constraint_adjustment)
{
    Q_UNUSED(resource);
    m_data.constraintAdjustments = constraint_adjustment;
}

void QWaylandXdgPositioner::xdg_positioner_set_offset(QtWaylandServer::xdg_positioner::Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    m_data.offset = QPoint(x, y);
}

QWaylandXdgPositioner *QWaylandXdgPositioner::fromResource(wl_resource *resource)
{
    if (auto *r = Resource::fromResource(resource))
        return static_cast<QWaylandXdgPositioner *>(r->xdg_positioner_object);
    return nullptr;
}

Qt::Edges QWaylandXdgPositioner::convertToEdges(anchor anchor)
{
    switch (anchor) {
    case anchor_none:
        return Qt::Edges();
    case anchor_top:
        return Qt::TopEdge;
    case anchor_bottom:
        return Qt::BottomEdge;
    case anchor_left:
        return Qt::LeftEdge;
    case anchor_right:
        return Qt::RightEdge;
    case anchor_top_left:
        return Qt::TopEdge | Qt::LeftEdge;
    case anchor_bottom_left:
        return Qt::BottomEdge | Qt::LeftEdge;
    case anchor_top_right:
        return Qt::TopEdge | Qt::RightEdge;
    case anchor_bottom_right:
        return Qt::BottomEdge | Qt::RightEdge;
    default:
        qWarning() << "Unknown Wayland xdg edge" << anchor;
        return Qt::Edges();
    }
}

Qt::Edges QWaylandXdgPositioner::convertToEdges(QWaylandXdgPositioner::gravity gravity)
{
    return convertToEdges(anchor(gravity));
}


QT_END_NAMESPACE
