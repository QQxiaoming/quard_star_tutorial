/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qwaylandxdgshellv6.h"
#include "qwaylandxdgshellv6_p.h"

#if QT_CONFIG(wayland_compositor_quick)
#include "qwaylandxdgshellv6integration_p.h"
#endif

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandSurfaceRole>
#include <QtWaylandCompositor/QWaylandResource>

#include <QtCore/QObject>

#include <algorithm>

QT_BEGIN_NAMESPACE

QWaylandXdgShellV6Private::QWaylandXdgShellV6Private()
{
}

void QWaylandXdgShellV6Private::ping(QtWaylandServer::zxdg_shell_v6::Resource *resource, uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(resource->handle, serial);
}

void QWaylandXdgShellV6Private::registerXdgSurface(QWaylandXdgSurfaceV6 *xdgSurface)
{
    m_xdgSurfaces.insert(xdgSurface->surface()->client()->client(), xdgSurface);
}

void QWaylandXdgShellV6Private::unregisterXdgSurface(QWaylandXdgSurfaceV6 *xdgSurface)
{
    auto xdgSurfacePrivate = QWaylandXdgSurfaceV6Private::get(xdgSurface);
    if (!m_xdgSurfaces.remove(xdgSurfacePrivate->resource()->client(), xdgSurface))
        qWarning("%s Unexpected state. Can't find registered xdg surface\n", Q_FUNC_INFO);
}

Qt::Edges QWaylandXdgShellV6Private::convertToEdges(uint xdgEdges)
{
    return Qt::Edges(((xdgEdges & 0b1100) >> 1) | ((xdgEdges & 0b0010) << 2) | (xdgEdges & 0b0001));
}

QWaylandXdgSurfaceV6 *QWaylandXdgShellV6Private::xdgSurfaceFromSurface(QWaylandSurface *surface)
{
    for (QWaylandXdgSurfaceV6 *xdgSurface : qAsConst(m_xdgSurfaces)) {
        if (surface == xdgSurface->surface())
            return xdgSurface;
    }
    return nullptr;
}

void QWaylandXdgShellV6Private::zxdg_shell_v6_destroy(Resource *resource)
{
    if (!m_xdgSurfaces.values(resource->client()).empty())
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_DEFUNCT_SURFACES,
                               "xdg_shell was destroyed before children");

    wl_resource_destroy(resource->handle);
}

void QWaylandXdgShellV6Private::zxdg_shell_v6_create_positioner(QtWaylandServer::zxdg_shell_v6::Resource *resource, uint32_t id)
{
    QWaylandResource positionerResource(wl_resource_create(resource->client(), &zxdg_positioner_v6_interface,
                                                           wl_resource_get_version(resource->handle), id));

    new QWaylandXdgPositionerV6(positionerResource);
}

void QWaylandXdgShellV6Private::zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    Q_Q(QWaylandXdgShellV6);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    if (surface->role() != nullptr) {
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_ROLE,
                               "wl_surface@%d, already has role %s\n",
                               wl_resource_get_id(surface->resource()),
                               surface->role()->name().constData());
        return;
    }

    if (surface->hasContent()) {
        //TODO: According to the spec, this is a client error, but there's no appropriate error code
        qWarning() << "get_xdg_surface requested on a zxdg_surface_v6 with content";
    }

    QWaylandResource xdgSurfaceResource(wl_resource_create(resource->client(), &zxdg_surface_v6_interface,
                                                           wl_resource_get_version(resource->handle), id));

    QWaylandXdgSurfaceV6 *xdgSurface = new QWaylandXdgSurfaceV6(q, surface, xdgSurfaceResource);

    registerXdgSurface(xdgSurface);
    emit q->xdgSurfaceCreated(xdgSurface);
}

void QWaylandXdgShellV6Private::zxdg_shell_v6_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgShellV6);
    if (m_pings.remove(serial))
        emit q->pong(serial);
    else
        qWarning("Received an unexpected pong!");
}

/*!
 * \qmltype XdgShellV6
 * \inqmlmodule QtWayland.Compositor
 * \since 5.10
 * \brief Provides an extension for desktop-style user interfaces.
 *
 * The XdgShellV6 extension provides a way to associate a XdgToplevelV6 or XdgPopupV6
 * with a regular Wayland surface. Using the XdgToplevelV6 interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * XdgShellV6 corresponds to the Wayland interface, \c zxdg_shell_v6.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the XdgShellV6 component and add it to the list of extensions
 * supported by the compositor:
 * \code
 * import QtWayland.Compositor 1.1
 *
 * WaylandCompositor {
 *     XdgShellV6 {
 *         // ...
 *     }
 * }
 * \endcode
 */

/*!
 * \class QWaylandXdgShellV6
 * \inmodule QtWaylandCompositor
 * \since 5.10
 * \brief The QWaylandXdgShellV6 class is an extension for desktop-style user interfaces.
 *
 * The QWaylandXdgShellV6 extension provides a way to associate a QWaylandXdgToplevelV6 or
 * QWaylandXdgPopupV6 with a regular Wayland surface. Using the QWaylandXdgToplevelV6 interface,
 * the client can request that the surface is resized, moved, and so on.
 *
 * QWaylandXdgShellV6 corresponds to the Wayland interface, \c zxdg_shell_v6.
 */

/*!
 * Constructs a QWaylandXdgShellV6 object.
 */
QWaylandXdgShellV6::QWaylandXdgShellV6()
    : QWaylandShellTemplate<QWaylandXdgShellV6>(*new QWaylandXdgShellV6Private())
{
}

/*!
 * Constructs a QWaylandXdgShellV6 object for the provided \a compositor.
 */
QWaylandXdgShellV6::QWaylandXdgShellV6(QWaylandCompositor *compositor)
    : QWaylandShellTemplate<QWaylandXdgShellV6>(compositor, *new QWaylandXdgShellV6Private())
{
}

/*!
 * Initializes the shell extension.
 */
void QWaylandXdgShellV6::initialize()
{
    Q_D(QWaylandXdgShellV6);
    QWaylandShellTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandXdgShellV6";
        return;
    }
    d->init(compositor->display(), 1);

    handleSeatChanged(compositor->defaultSeat(), nullptr);

    connect(compositor, &QWaylandCompositor::defaultSeatChanged,
            this, &QWaylandXdgShellV6::handleSeatChanged);
}

/*!
 * Returns the Wayland interface for the QWaylandXdgShellV6.
 */
const struct wl_interface *QWaylandXdgShellV6::interface()
{
    return QWaylandXdgShellV6Private::interface();
}

QByteArray QWaylandXdgShellV6::interfaceName()
{
    return QWaylandXdgShellV6Private::interfaceName();
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgShellV6::ping(WaylandClient client)
 *
 * Sends a ping event to \a client. If the client replies to the event the
 * \l pong signal will be emitted.
 */

/*!
 * Sends a ping event to \a client. If the client replies to the event the
 * \l pong signal will be emitted.
 */
uint QWaylandXdgShellV6::ping(QWaylandClient *client)
{
    Q_D(QWaylandXdgShellV6);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    Q_ASSERT(compositor);

    uint32_t serial = compositor->nextSerial();

    QWaylandXdgShellV6Private::Resource *clientResource = d->resourceMap().value(client->client(), nullptr);
    Q_ASSERT(clientResource);

    d->ping(clientResource, serial);
    return serial;
}

void QWaylandXdgShellV6::handleSeatChanged(QWaylandSeat *newSeat, QWaylandSeat *oldSeat)
{
    if (oldSeat != nullptr) {
        disconnect(oldSeat, &QWaylandSeat::keyboardFocusChanged,
                   this, &QWaylandXdgShellV6::handleFocusChanged);
    }

    if (newSeat != nullptr) {
        connect(newSeat, &QWaylandSeat::keyboardFocusChanged,
                this, &QWaylandXdgShellV6::handleFocusChanged);
    }
}

void QWaylandXdgShellV6::handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_D(QWaylandXdgShellV6);

    QWaylandXdgSurfaceV6 *newXdgSurface = d->xdgSurfaceFromSurface(newSurface);
    QWaylandXdgSurfaceV6 *oldXdgSurface = d->xdgSurfaceFromSurface(oldSurface);

    if (newXdgSurface)
        QWaylandXdgSurfaceV6Private::get(newXdgSurface)->handleFocusReceived();

    if (oldXdgSurface)
        QWaylandXdgSurfaceV6Private::get(oldXdgSurface)->handleFocusLost();
}

QWaylandXdgSurfaceV6Private::QWaylandXdgSurfaceV6Private()
{
}

void QWaylandXdgSurfaceV6Private::setWindowType(Qt::WindowType windowType)
{
    if (m_windowType == windowType)
        return;

    m_windowType = windowType;

    Q_Q(QWaylandXdgSurfaceV6);
    emit q->windowTypeChanged();
}

void QWaylandXdgSurfaceV6Private::handleFocusLost()
{
    if (m_toplevel)
        QWaylandXdgToplevelV6Private::get(m_toplevel)->handleFocusLost();
}

void QWaylandXdgSurfaceV6Private::handleFocusReceived()
{
    if (m_toplevel)
        QWaylandXdgToplevelV6Private::get(m_toplevel)->handleFocusReceived();
}

QRect QWaylandXdgSurfaceV6Private::calculateFallbackWindowGeometry() const
{
    // TODO: The unset window geometry should include subsurfaces as well, so this solution
    // won't work too well on those kinds of clients.
    return QRect(QPoint(0, 0), m_surface->size() / m_surface->bufferScale());
}

void QWaylandXdgSurfaceV6Private::updateFallbackWindowGeometry()
{
    Q_Q(QWaylandXdgSurfaceV6);
    if (!m_unsetWindowGeometry)
        return;

    const QRect unsetGeometry = calculateFallbackWindowGeometry();
    if (unsetGeometry == m_windowGeometry)
        return;

    m_windowGeometry = unsetGeometry;
    emit q->windowGeometryChanged();
}

void QWaylandXdgSurfaceV6Private::zxdg_surface_v6_destroy_resource(QtWaylandServer::zxdg_surface_v6::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurfaceV6);
    QWaylandXdgShellV6Private::get(m_xdgShell)->unregisterXdgSurface(q);
    delete q;
}

void QWaylandXdgSurfaceV6Private::zxdg_surface_v6_destroy(QtWaylandServer::zxdg_surface_v6::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgSurfaceV6Private::zxdg_surface_v6_get_toplevel(QtWaylandServer::zxdg_surface_v6::Resource *resource, uint32_t id)
{
    Q_Q(QWaylandXdgSurfaceV6);

    if (m_toplevel || m_popup) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_ALREADY_CONSTRUCTED,
                               "zxdg_surface_v6 already has a role object");
        return;
    }

    if (!m_surface->setRole(QWaylandXdgToplevelV6::role(), resource->handle, ZXDG_SHELL_V6_ERROR_ROLE))
        return;

    QWaylandResource topLevelResource(wl_resource_create(resource->client(), &zxdg_toplevel_v6_interface,
                                                         wl_resource_get_version(resource->handle), id));

    m_toplevel = new QWaylandXdgToplevelV6(q, topLevelResource);
    emit q->toplevelCreated();
    emit m_xdgShell->toplevelCreated(m_toplevel, q);
}

void QWaylandXdgSurfaceV6Private::zxdg_surface_v6_get_popup(QtWaylandServer::zxdg_surface_v6::Resource *resource, uint32_t id, wl_resource *parentResource, wl_resource *positionerResource)
{
    Q_Q(QWaylandXdgSurfaceV6);

    if (m_toplevel || m_popup) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_ALREADY_CONSTRUCTED,
                               "zxdg_surface_v6 already has a role object");
        return;
    }

    QWaylandXdgSurfaceV6 *parent = QWaylandXdgSurfaceV6::fromResource(parentResource);
    if (!parent) {
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_INVALID_POPUP_PARENT,
                               "zxdg_surface_v6.get_popup with invalid popup parent");
        return;
    }

    QWaylandXdgPositionerV6 *positioner = QWaylandXdgPositionerV6::fromResource(positionerResource);
    if (!positioner) {
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_INVALID_POSITIONER,
                               "zxdg_surface_v6.get_popup without positioner");
        return;
    }
    if (!positioner->m_data.isComplete()) {
        QWaylandXdgPositionerV6Data p = positioner->m_data;
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_INVALID_POSITIONER,
                               "zxdg_surface_v6.get_popup with invalid positioner (size: %dx%d, anchorRect: %dx%d)",
                               p.size.width(), p.size.height(), p.anchorRect.width(), p.anchorRect.height());
        return;
    }

    if (!m_surface->setRole(QWaylandXdgPopupV6::role(), resource->handle, ZXDG_SHELL_V6_ERROR_ROLE))
        return;

    QWaylandResource popupResource(wl_resource_create(resource->client(), &zxdg_popup_v6_interface,
                                                      wl_resource_get_version(resource->handle), id));

    m_popup = new QWaylandXdgPopupV6(q, parent, positioner, popupResource);
    emit q->popupCreated();
    emit m_xdgShell->popupCreated(m_popup, q);
}

void QWaylandXdgSurfaceV6Private::zxdg_surface_v6_ack_configure(QtWaylandServer::zxdg_surface_v6::Resource *resource, uint32_t serial)
{
    if (m_toplevel) {
        QWaylandXdgToplevelV6Private::get(m_toplevel)->handleAckConfigure(serial);
    } else if (m_popup) {
        QWaylandXdgPopupV6Private::get(m_popup)->handleAckConfigure(serial);
    } else {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_NOT_CONSTRUCTED,
                               "ack_configure requested on an unconstructed zxdg_surface_v6");
    }
}

void QWaylandXdgSurfaceV6Private::zxdg_surface_v6_set_window_geometry(QtWaylandServer::zxdg_surface_v6::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_Q(QWaylandXdgSurfaceV6);

    if (!q->surface()->role()) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_NOT_CONSTRUCTED,
                               "set_window_geometry requested on an unconstructed zxdg_surface_v6");
        return;
    }

    if (width <= 0 || height <= 0) {
        // The protocol spec says "setting an invalid size will raise an error". But doesn't tell
        // which error to raise, and there's no fitting error in the zxdg_surface_v6_error enum.
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
 * \qmltype XdgSurfaceV6
 * \inqmlmodule QtWayland.Compositor
 * \since 5.10
 * \brief XdgSurfaceV6 provides desktop-style compositor-specific features to an xdg surface.
 *
 * This type is part of the \l{XdgShellV6} extension and provides a way to
 * extend the functionality of an existing \l{WaylandSurface} with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface \c zxdg_surface_v6.
 */

/*!
 * \class QWaylandXdgSurfaceV6
 * \inmodule QtWaylandCompositor
 * \since 5.10
 * \brief The QWaylandXdgSurfaceV6 class provides desktop-style compositor-specific features to an xdg surface.
 *
 * This class is part of the QWaylandXdgShellV6 extension and provides a way to
 * extend the functionality of an existing QWaylandSurface with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface \c zxdg_surface_v6.
 */

/*!
 * Constructs a QWaylandXdgSurfaceV6.
 */
QWaylandXdgSurfaceV6::QWaylandXdgSurfaceV6()
    : QWaylandShellSurfaceTemplate<QWaylandXdgSurfaceV6>(*new QWaylandXdgSurfaceV6Private)
{
}

/*!
 * Constructs a QWaylandXdgSurfaceV6 for \a surface and initializes it with the
 * given \a xdgShell, \a surface, and resource \a res.
 */
QWaylandXdgSurfaceV6::QWaylandXdgSurfaceV6(QWaylandXdgShellV6 *xdgShell, QWaylandSurface *surface, const QWaylandResource &res)
    : QWaylandShellSurfaceTemplate<QWaylandXdgSurfaceV6>(*new QWaylandXdgSurfaceV6Private)
{
    initialize(xdgShell, surface, res);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgSurfaceV6::initialize(object xdgShell, object surface, object client, int id)
 *
 * Initializes the XdgSurface, associating it with the given \a xdgShell, \a surface,
 * \a client, and \a id.
 */

/*!
 * Initializes the QWaylandXdgSurfaceV6, associating it with the given \a xdgShell, \a surface
 * and \a resource.
 */
void QWaylandXdgSurfaceV6::initialize(QWaylandXdgShellV6 *xdgShell, QWaylandSurface *surface, const QWaylandResource &resource)
{
    Q_D(QWaylandXdgSurfaceV6);
    d->m_xdgShell = xdgShell;
    d->m_surface = surface;
    d->init(resource.resource());
    setExtensionContainer(surface);
    d->m_windowGeometry = d->calculateFallbackWindowGeometry();
    connect(surface, &QWaylandSurface::sizeChanged, this, &QWaylandXdgSurfaceV6::handleSurfaceSizeChanged);
    connect(surface, &QWaylandSurface::bufferScaleChanged, this, &QWaylandXdgSurfaceV6::handleBufferScaleChanged);
    emit shellChanged();
    emit surfaceChanged();
    QWaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty enum QtWaylandCompositor::XdgSurfaceV6::windowType
 *
 * This property holds the window type of the XdgSurfaceV6.
 */
Qt::WindowType QWaylandXdgSurfaceV6::windowType() const
{
    Q_D(const QWaylandXdgSurfaceV6);
    return d->m_windowType;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgSurfaceV6::windowGeometry
 *
 * This property holds the window geometry of the QWaylandXdgSurfaceV6. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */

/*!
 * \property QWaylandXdgSurfaceV6::windowGeometry
 *
 * This property holds the window geometry of the QWaylandXdgSurfaceV6. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */
QRect QWaylandXdgSurfaceV6::windowGeometry() const
{
    Q_D(const QWaylandXdgSurfaceV6);
    return d->m_windowGeometry;
}

/*!
 * \internal
 */
void QWaylandXdgSurfaceV6::initialize()
{
    QWaylandCompositorExtension::initialize();
}

void QWaylandXdgSurfaceV6::handleSurfaceSizeChanged()
{
    Q_D(QWaylandXdgSurfaceV6);
    d->updateFallbackWindowGeometry();
}

void QWaylandXdgSurfaceV6::handleBufferScaleChanged()
{
    Q_D(QWaylandXdgSurfaceV6);
    d->updateFallbackWindowGeometry();
}

/*!
 * \qmlproperty XdgShellV6 QtWaylandCompositor::XdgSurfaceV6::shell
 *
 * This property holds the shell associated with this XdgSurface.
 */

/*!
 * \property QWaylandXdgSurfaceV6::shell
 *
 * This property holds the shell associated with this QWaylandXdgSurfaceV6.
 */
QWaylandXdgShellV6 *QWaylandXdgSurfaceV6::shell() const
{
    Q_D(const QWaylandXdgSurfaceV6);
    return d->m_xdgShell;
}

/*!
 * \qmlproperty WaylandSurface QtWaylandCompositor::XdgSurfaceV6::surface
 *
 * This property holds the surface associated with this XdgSurfaceV6.
 */

/*!
 * \property QWaylandXdgSurfaceV6::surface
 *
 * This property holds the surface associated with this QWaylandXdgSurfaceV6.
 */
QWaylandSurface *QWaylandXdgSurfaceV6::surface() const
{
    Q_D(const QWaylandXdgSurfaceV6);
    return d->m_surface;
}

/*!
 * \qmlproperty XdgToplevelV6 QtWaylandCompositor::XdgSurfaceV6::toplevel
 *
 * This property holds the properties and methods that are specific to the
 * toplevel XdgSurfaceV6.
 *
 * \sa popup, XdgShellV6::toplevelCreated
 */

/*!
 * \property QWaylandXdgSurfaceV6::toplevel
 *
 * This property holds the properties and methods that are specific to the
 * toplevel QWaylandXdgSurfaceV6.
 *
 * \sa QWaylandXdgSurfaceV6::popup, QWaylandXdgShellV6::toplevelCreated
 */
QWaylandXdgToplevelV6 *QWaylandXdgSurfaceV6::toplevel() const
{
    Q_D(const QWaylandXdgSurfaceV6);
    return d->m_toplevel;
}

/*!
 * \qmlproperty XdgPopupV6 QtWaylandCompositor::XdgSurfaceV6::popup
 *
 * This property holds the properties and methods that are specific to the
 * popup XdgSurfaceV6.
 *
 * \sa toplevel, XdgShellV6::popupCreated
 */

/*!
 * \property QWaylandXdgSurfaceV6::popup
 *
 * This property holds the properties and methods that are specific to the
 * popup QWaylandXdgSurfaceV6.
 *
 * \sa QWaylandXdgSurfaceV6::toplevel, QWaylandXdgShellV6::popupCreated
 */
QWaylandXdgPopupV6 *QWaylandXdgSurfaceV6::popup() const
{
    Q_D(const QWaylandXdgSurfaceV6);
    return d->m_popup;
}

/*!
 * Returns the Wayland interface for the QWaylandXdgSurfaceV6.
 */
const wl_interface *QWaylandXdgSurfaceV6::interface()
{
    return QWaylandXdgSurfaceV6Private::interface();
}

/*!
 * \internal
 */
QByteArray QWaylandXdgSurfaceV6::interfaceName()
{
    return QWaylandXdgSurfaceV6Private::interfaceName();
}

/*!
 * Returns the QWaylandXdgSurfaceV6 corresponding to the \a resource.
 */
QWaylandXdgSurfaceV6 *QWaylandXdgSurfaceV6::fromResource(wl_resource *resource)
{
    auto xsResource = QWaylandXdgSurfaceV6Private::Resource::fromResource(resource);
    if (!xsResource)
        return nullptr;
    return static_cast<QWaylandXdgSurfaceV6Private *>(xsResource->zxdg_surface_v6_object)->q_func();
}

#if QT_CONFIG(wayland_compositor_quick)
QWaylandQuickShellIntegration *QWaylandXdgSurfaceV6::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    Q_D(const QWaylandXdgSurfaceV6);

    if (d->m_toplevel)
        return new QtWayland::XdgToplevelV6Integration(item);

    if (d->m_popup)
        return new QtWayland::XdgPopupV6Integration(item);

    return nullptr;
}
#endif

/*!
 * \qmltype XdgToplevelV6
 * \inqmlmodule QtWayland.Compositor
 * \since 5.10
 * \brief XdgToplevelV6 represents the toplevel window specific parts of an xdg surface.
 *
 * This type is part of the \l{XdgShellV6} extension and provides a way to
 * extend the functionality of an XdgSurfaceV6 with features
 * specific to desktop-style windows.
 *
 * It corresponds to the Wayland interface \c zxdg_toplevel_v6.
 */

/*!
 * \class QWaylandXdgToplevelV6
 * \inmodule QtWaylandCompositor
 * \since 5.10
 * \brief The QWaylandXdgToplevelV6 class represents the toplevel window specific parts of an xdg surface.
 *
 * This class is part of the QWaylandXdgShellV6 extension and provides a way to
 * extend the functionality of an QWaylandXdgSurfaceV6 with features
 * specific to desktop-style windows.
 *
 * It corresponds to the Wayland interface \c zxdg_toplevel_v6.
 */

/*!
 * Constructs a QWaylandXdgToplevelV6 for the given \a xdgSurface and \a resource.
 */
QWaylandXdgToplevelV6::QWaylandXdgToplevelV6(QWaylandXdgSurfaceV6 *xdgSurface, QWaylandResource &resource)
    : QObject(*new QWaylandXdgToplevelV6Private(xdgSurface, resource))
{
    QVector<QWaylandXdgToplevelV6::State> states;
    sendConfigure({0, 0}, states);
}

/*!
 * \qmlproperty XdgToplevelV6 QtWaylandCompositor::XdgToplevelV6::parentToplevel
 *
 * This property holds the XdgToplevelV6 parent of this XdgToplevelV6.
 */

/*!
 * \property QWaylandXdgToplevelV6::parentToplevel
 *
 * This property holds the XdgToplevelV6 parent of this XdgToplevelV6.
 *
 */
QWaylandXdgToplevelV6 *QWaylandXdgToplevelV6::parentToplevel() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_parentToplevel;
}

/*!
 * \qmlproperty string QtWaylandCompositor::XdgToplevelV6::title
 *
 * This property holds the title of the XdgToplevelV6.
 */

/*!
 * \property QWaylandXdgToplevelV6::title
 *
 * This property holds the title of the QWaylandXdgToplevelV6.
 */
QString QWaylandXdgToplevelV6::title() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_title;
}

/*!
 * \qmlproperty string QtWaylandCompositor::XdgToplevelV6::appId
 *
 * This property holds the app id of the XdgToplevelV6.
 */

/*!
 * \property QWaylandXdgToplevelV6::appId
 *
 * This property holds the app id of the QWaylandXdgToplevelV6.
 */
QString QWaylandXdgToplevelV6::appId() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_appId;
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgToplevelV6::maxSize
 *
 * This property holds the maximum size of the XdgToplevelV6 as requested by the client.
 *
 * The compositor is free to ignore this value and request a larger size.
 */

/*!
 * \property QWaylandXdgToplevelV6::maxSize
 *
 * This property holds the maximum size of the QWaylandXdgToplevelV6.
 *
 * The compositor is free to ignore this value and request a larger size.
 */
QSize QWaylandXdgToplevelV6::maxSize() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_maxSize;
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgToplevelV6::minSize
 *
 * This property holds the minimum size of the XdgToplevelV6 as requested by the client.
 *
 * The compositor is free to ignore this value and request a smaller size.
 */

/*!
 * \property QWaylandXdgToplevelV6::minSize
 *
 * This property holds the minimum size of the QWaylandXdgToplevelV6.
 *
 * The compositor is free to ignore this value and request a smaller size.
 */
QSize QWaylandXdgToplevelV6::minSize() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_minSize;
}

/*!
 * \property QWaylandXdgToplevelV6::states
 *
 * This property holds the last states the client acknowledged for this QWaylandToplevelV6.
 */
QVector<QWaylandXdgToplevelV6::State> QWaylandXdgToplevelV6::states() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_lastAckedConfigure.states;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevelV6::maximized
 *
 * This property holds whether the client has acknowledged that it should be maximized.
 */

/*!
 * \property QWaylandXdgToplevelV6::maximized
 *
 * This property holds whether the client has acknowledged that it should be maximized.
 */
bool QWaylandXdgToplevelV6::maximized() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevelV6::State::MaximizedState);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevelV6::fullscreen
 *
 * This property holds whether the client has acknowledged that it should be fullscreen.
 */

/*!
 * \property QWaylandXdgToplevelV6::fullscreen
 *
 * This property holds whether the client has acknowledged that it should be fullscreen.
 */
bool QWaylandXdgToplevelV6::fullscreen() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevelV6::State::FullscreenState);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevelV6::resizing
 *
 * This property holds whether the client has acknowledged that it is being resized.
 */

/*!
 * \property QWaylandXdgToplevelV6::resizing
 *
 * This property holds whether the client has acknowledged that it is being resized.
 */
bool QWaylandXdgToplevelV6::resizing() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevelV6::State::ResizingState);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::XdgToplevelV6::activated
 *
 * This property holds whether toplevel is drawing itself as having input focus.
 */

/*!
 * \property QWaylandXdgToplevelV6::activated
 *
 * This property holds whether toplevel is drawing itself as having input focus.
 */
bool QWaylandXdgToplevelV6::activated() const
{
    Q_D(const QWaylandXdgToplevelV6);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgToplevelV6::State::ActivatedState);
}

/*!
 * \qmlmethod size QtWaylandCompositor::XdgToplevelV6::sizeForResize(size size, point delta, uint edges)
 *
 * Convenience for computing the new size given the current \a size, a \a delta, and
 * the \a edges active in the drag.
 */

/*!
 * Convenience for computing the new size given the current \a size, a \a delta, and
 * the \a edges active in the drag.
 */
QSize QWaylandXdgToplevelV6::sizeForResize(const QSizeF &size, const QPointF &delta, Qt::Edges edges) const
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
 * Known \a states are enumerated in QWaylandXdgToplevelV6::State.
 */
uint QWaylandXdgToplevelV6::sendConfigure(const QSize &size, const QVector<QWaylandXdgToplevelV6::State> &states)
{
    if (!size.isValid()) {
        qWarning() << "Can't configure zxdg_toplevel_v6 with an invalid size" << size;
        return 0;
    }
    Q_D(QWaylandXdgToplevelV6);
    auto statesBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(states.data()),
                                               states.size() * static_cast<int>(sizeof(State)));
    uint32_t serial = d->m_xdgSurface->surface()->compositor()->nextSerial();
    d->m_pendingConfigures.append(QWaylandXdgToplevelV6Private::ConfigureEvent{states, size, serial});
    d->send_configure(size.width(), size.height(), statesBytes);
    QWaylandXdgSurfaceV6Private::get(d->m_xdgSurface)->send_configure(serial);
    return serial;
}

/*!
 * \qmlmethod int QtWaylandCompositor::XdgToplevelV6::sendConfigure(size size, list<int> states)
 *
 * Sends a configure event to the client. \a size contains the pixel size of the surface.
 * A size of zero means the client is free to decide the size.
 * Known \a states are enumerated in XdgToplevelV6::State.
 */
uint QWaylandXdgToplevelV6::sendConfigure(const QSize &size, const QVector<int> &states)
{
    QVector<State> s;
    for (auto state : states)
        s << State(state);
    return sendConfigure(size, s);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevelV6::sendClose()
 *
 * Sends a close event to the client. The client may choose to ignore the event.
 */

/*!
 * Sends a close event to the client. The client may choose to ignore the event.
 */
void QWaylandXdgToplevelV6::sendClose()
{
    Q_D(QWaylandXdgToplevelV6);
    d->send_close();
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevelV6::sendMaximized(size size)
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
uint QWaylandXdgToplevelV6::sendMaximized(const QSize &size)
{
    Q_D(QWaylandXdgToplevelV6);
    QWaylandXdgToplevelV6Private::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgToplevelV6::State::MaximizedState))
        conf.states.append(QWaylandXdgToplevelV6::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::ResizingState);

    return sendConfigure(size, conf.states);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevelV6::sendUnmaximized(size size)
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
uint QWaylandXdgToplevelV6::sendUnmaximized(const QSize &size)
{
    Q_D(QWaylandXdgToplevelV6);
    QWaylandXdgToplevelV6Private::ConfigureEvent conf = d->lastSentConfigure();

    conf.states.removeOne(QWaylandXdgToplevelV6::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::ResizingState);

    return sendConfigure(size, conf.states);

}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevelV6::sendFullscreen(size size)
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
uint QWaylandXdgToplevelV6::sendFullscreen(const QSize &size)
{
    Q_D(QWaylandXdgToplevelV6);
    QWaylandXdgToplevelV6Private::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgToplevelV6::State::FullscreenState))
        conf.states.append(QWaylandXdgToplevelV6::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::ResizingState);

    return sendConfigure(size, conf.states);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgToplevelV6::sendResizing(size maxSize)
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
uint QWaylandXdgToplevelV6::sendResizing(const QSize &maxSize)
{
    Q_D(QWaylandXdgToplevelV6);
    QWaylandXdgToplevelV6Private::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgToplevelV6::State::ResizingState))
        conf.states.append(QWaylandXdgToplevelV6::State::ResizingState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgToplevelV6::State::FullscreenState);

    return sendConfigure(maxSize, conf.states);
}

/*!
 * Returns the surface role for the QWaylandToplevelV6.
 */
QWaylandSurfaceRole *QWaylandXdgToplevelV6::role()
{
    return &QWaylandXdgToplevelV6Private::s_role;
}


/*!
 * \qmlsignal QtWaylandCompositor::XdgShellV6::xdgSurfaceCreated(XdgSurfaceV6 xdgSurface)
 *
 * This signal is emitted when the client has created a \c zxdg_surface_v6.
 * Note that \a xdgSurface is not mapped, i.e. according to the \c xdg-shell
 * protocol it should not be displayed, until it has received a role object.
 *
 * \sa toplevelCreated(), popupCreated()
 */

/*!
 * \fn void QWaylandXdgShellV6::xdgSurfaceCreated(QWaylandXdgSurfaceV6 *xdgSurface)
 *
 * This signal is emitted when the client has created a \c zxdg_surface_v6.
 * Note that \a xdgSurface is not mapped, i.e. according to the \c xdg-shell
 * protocol it should not be displayed, until it has received a role object.
 *
 * \sa toplevelCreated(), popupCreated()
 */

/*!
 * \qmlsignal QtWaylandCompositor::XdgShellV6::toplevelCreated(XdgToplevelV6 toplevel, XdgSurfaceV6 xdgSurface)
 *
 * This signal is emitted when the client has created a \c zxdg_toplevel_v6.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a toplevel in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurfaceV6 \a toplevel is the role object for.
 */

/*!
 * \fn void QWaylandXdgShellV6::toplevelCreated(QWaylandXdgToplevelV6 *toplevel, QWaylandXdgSurfaceV6 *xdgSurface)
 *
 * This signal is emitted when the client has created a \c zxdg_toplevel_v6.
 * A common use case is to let the handler of this signal instantiate a QWaylandShellSurfaceItem or
 * QWaylandQuickItem for displaying \a toplevel in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurfaceV6 \a toplevel is the role object for.
 */

/*!
 * \qmlsignal QtWaylandCompositor::XdgShellV6::popupCreated(XdgPopupV6 popup, XdgSurfaceV6 xdgSurface)
 *
 * This signal is emitted when the client has created a \c zxdg_popup_v6.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a popup in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurfaceV6 \a popup is the role object for.
 */

/*!
 * \fn void QWaylandXdgShellV6::popupCreated(QWaylandXdgPopupV6 *popup, QWaylandXdgSurfaceV6 *xdgSurface)
 *
 * This signal is emitted when the client has created a \c zxdg_popup_v6.
 * A common use case is to let the handler of this signal instantiate a QWaylandShellSurfaceItem or
 * QWaylandQuickItem for displaying \a popup in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurfaceV6 \a popup is the role object for.
 */

/*!
 * \qmlsignal QtWaylandCompositor::XdgShellV6::pong(int serial)
 *
 * This signal is emitted when the client has responded to a ping event with serial, \a serial.
 *
 * \sa ping()
 */

/*!
 * \fn void QWaylandXdgShellV6::pong(uint serial)
 *
 * This signal is emitted when the client has responded to a ping event with serial, \a serial.
 *
 * \sa QWaylandXdgShellV6::ping()
 */

QList<int> QWaylandXdgToplevelV6::statesAsInts() const
{
   QList<int> list;
   Q_FOREACH (uint state, states()) {
       list << static_cast<int>(state);
   }
   return list;
}

QWaylandSurfaceRole QWaylandXdgToplevelV6Private::s_role("zxdg_toplevel_v6");

QWaylandXdgToplevelV6Private::QWaylandXdgToplevelV6Private(QWaylandXdgSurfaceV6 *xdgSurface, const QWaylandResource &resource)
    : m_xdgSurface(xdgSurface)
{
    init(resource.resource());
}

void QWaylandXdgToplevelV6Private::handleAckConfigure(uint serial)
{
    Q_Q(QWaylandXdgToplevelV6);
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

void QWaylandXdgToplevelV6Private::handleFocusLost()
{
    Q_Q(QWaylandXdgToplevelV6);
    QWaylandXdgToplevelV6Private::ConfigureEvent current = lastSentConfigure();
    current.states.removeOne(QWaylandXdgToplevelV6::State::ActivatedState);
    q->sendConfigure(current.size, current.states);
}

void QWaylandXdgToplevelV6Private::handleFocusReceived()
{
    Q_Q(QWaylandXdgToplevelV6);
    QWaylandXdgToplevelV6Private::ConfigureEvent current = lastSentConfigure();
    if (!current.states.contains(QWaylandXdgToplevelV6::State::ActivatedState)) {
        current.states.push_back(QWaylandXdgToplevelV6::State::ActivatedState);
        q->sendConfigure(current.size, current.states);
    }
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_destroy_resource(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevelV6);
    delete q;
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_destroy(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    wl_resource_destroy(resource->handle);
    //TODO: Should the xdg surface be desroyed as well? Or is it allowed to recreate a new toplevel for it?
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_parent(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, wl_resource *parent)
{
    Q_UNUSED(resource);
    QWaylandXdgToplevelV6 *parentToplevel = nullptr;
    if (parent) {
        parentToplevel = static_cast<QWaylandXdgToplevelV6Private *>(
                    QWaylandXdgToplevelV6Private::Resource::fromResource(parent)->zxdg_toplevel_v6_object)->q_func();
    }

    Q_Q(QWaylandXdgToplevelV6);

    if (m_parentToplevel != parentToplevel) {
        m_parentToplevel = parentToplevel;
        emit q->parentToplevelChanged();
    }

    if (m_parentToplevel && m_xdgSurface->windowType() != Qt::WindowType::SubWindow) {
        // There's a parent now, which means the surface is transient
        QWaylandXdgSurfaceV6Private::get(m_xdgSurface)->setWindowType(Qt::WindowType::SubWindow);
    } else if (!m_parentToplevel && m_xdgSurface->windowType() != Qt::WindowType::Window) {
        // When the surface has no parent it is toplevel
        QWaylandXdgSurfaceV6Private::get(m_xdgSurface)->setWindowType(Qt::WindowType::Window);
    }
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_title(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, const QString &title)
{
    Q_UNUSED(resource);
    if (title == m_title)
        return;
    Q_Q(QWaylandXdgToplevelV6);
    m_title = title;
    emit q->titleChanged();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_app_id(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, const QString &app_id)
{
    Q_UNUSED(resource);
    if (app_id == m_appId)
        return;
    Q_Q(QWaylandXdgToplevelV6);
    m_appId = app_id;
    emit q->appIdChanged();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_show_window_menu(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, wl_resource *seatResource, uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    QPoint position(x, y);
    auto seat = QWaylandSeat::fromSeatResource(seatResource);
    Q_Q(QWaylandXdgToplevelV6);
    emit q->showWindowMenu(seat, position);
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_move(Resource *resource, wl_resource *seatResource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandXdgToplevelV6);
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    emit q->startMove(seat);
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_resize(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, wl_resource *seatResource, uint32_t serial, uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandXdgToplevelV6);
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    emit q->startResize(seat, QWaylandXdgShellV6Private::convertToEdges(edges));
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_max_size(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    QSize maxSize(width, height);
    if (width == 0 && height == 0)
        maxSize = QSize(); // Wayland size of zero means unspecified which best translates to invalid

    if (m_maxSize == maxSize)
        return;

    if (width < 0 || height < 0) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a zxdg_toplevel_v6.set_max_size request with a negative size";
        return;
    }

    if (m_minSize.isValid() && maxSize.isValid() &&
            (maxSize.width() < m_minSize.width() || maxSize.height() < m_minSize.height())) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a zxdg_toplevel_v6.set_max_size request with a size smaller than the minimium size";
        return;
    }

    m_maxSize = maxSize;

    Q_Q(QWaylandXdgToplevelV6);
    emit q->maxSizeChanged();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_min_size(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    QSize minSize(width, height);
    if (width == 0 && height == 0)
        minSize = QSize(); // Wayland size of zero means unspecified

    if (m_minSize == minSize)
        return;

    if (width < 0 || height < 0) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a zxdg_toplevel_v6.set_min_size request with a negative size";
        return;
    }

    if (m_maxSize.isValid() && minSize.isValid() &&
            (minSize.width() > m_maxSize.width() || minSize.height() > m_maxSize.height())) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a zxdg_toplevel_v6.set_min_size request with a size larger than the maximum size";
        return;
    }

    m_minSize = minSize;

    Q_Q(QWaylandXdgToplevelV6);
    emit q->minSizeChanged();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_maximized(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevelV6);
    emit q->setMaximized();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_unset_maximized(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevelV6);
    emit q->unsetMaximized();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_fullscreen(QtWaylandServer::zxdg_toplevel_v6::Resource *resource, wl_resource *output_res)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevelV6);
    QWaylandOutput *output = output_res ? QWaylandOutput::fromResource(output_res) : nullptr;
    emit q->setFullscreen(output);
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_unset_fullscreen(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevelV6);
    emit q->unsetFullscreen();
}

void QWaylandXdgToplevelV6Private::zxdg_toplevel_v6_set_minimized(QtWaylandServer::zxdg_toplevel_v6::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgToplevelV6);
    emit q->setMinimized();
}

/*!
 * \qmltype XdgPopupV6
 * \inqmlmodule QtWayland.Compositor
 * \since 5.10
 * \brief XdgPopupV6 represents the popup specific parts of and xdg surface.
 *
 * This type is part of the \l{XdgShellV6} extension and provides a way to extend
 * extend the functionality of an \l{XdgSurfaceV6} with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface \c zxdg_popup_v6.
 */

/*!
 * \class QWaylandXdgPopupV6
 * \inmodule QtWaylandCompositor
 * \since 5.10
 * \brief The QWaylandXdgPopupV6 class represents the popup specific parts of an xdg surface.
 *
 * This class is part of the QWaylandXdgShellV6 extension and provides a way to
 * extend the functionality of a QWaylandXdgSurfaceV6 with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface \c zxdg_popup_v6.
 */

/*!
 * Constructs a QWaylandXdgPopupV6.
 */
QWaylandXdgPopupV6::QWaylandXdgPopupV6(QWaylandXdgSurfaceV6 *xdgSurface, QWaylandXdgSurfaceV6 *parentXdgSurface,
                                       QWaylandXdgPositionerV6 *positioner, QWaylandResource &resource)
    : QObject(*new QWaylandXdgPopupV6Private(xdgSurface, parentXdgSurface, positioner, resource))
{
}

/*!
 * \qmlproperty XdgSurfaceV6 QtWaylandCompositor::XdgPopupV6::xdgSurface
 *
 * This property holds the XdgSurfaceV6 associated with this XdgPopupV6.
 */

/*!
 * \property QWaylandXdgPopupV6::xdgSurface
 *
 * This property holds the QWaylandXdgSurfaceV6 associated with this QWaylandXdgPopupV6.
 */
QWaylandXdgSurfaceV6 *QWaylandXdgPopupV6::xdgSurface() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_xdgSurface;
}

/*!
 * \qmlproperty XdgSurfaceV6 QtWaylandCompositor::XdgPopupV6::parentXdgSurface
 *
 * This property holds the XdgSurfaceV6 associated with the parent of this XdgPopupV6.
 */

/*!
 * \property QWaylandXdgPopupV6::parentXdgSurface
 *
 * This property holds the QWaylandXdgSurfaceV6 associated with the parent of this
 * QWaylandXdgPopupV6.
 */
QWaylandXdgSurfaceV6 *QWaylandXdgPopupV6::parentXdgSurface() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_parentXdgSurface;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgPopupV6::configuredGeometry
 *
 * The window geometry the popup received in the configure event. Relative to the
 * upper left corner of the parent surface.
 */

/*!
 * \property QWaylandXdgPopupV6::configuredGeometry
 *
 * The window geometry the popup received in the configure event. Relative to the
 * upper left corner of the parent surface.
 */
QRect QWaylandXdgPopupV6::configuredGeometry() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_geometry;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgPopupV6::anchorRect
 *
 * The anchor rectangle relative to the parent window geometry that the child
 * surface should be placed relative to.
 */

/*!
 * \property QWaylandXdgPopupV6::anchorRect
 *
 * Returns the anchor rectangle relative to the parent window geometry that the child
 * surface should be placed relative to.
 */
QRect QWaylandXdgPopupV6::anchorRect() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_positionerData.anchorRect;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopupV6::anchorEdges
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
 * \property QWaylandXdgPopupV6::anchorEdges
 *
 * Returns the set of edges on the anchor rect that the child surface should be placed
 * relative to. If no edges are specified in a direction, the anchor point should be
 * centered between the edges.
 */
Qt::Edges QWaylandXdgPopupV6::anchorEdges() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_positionerData.anchorEdges;
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgPopupV6::gravityEdges
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
 * \property QWaylandXdgPopupV6::gravityEdges
 *
 * Specifies in what direction the surface should be positioned, relative to the anchor
 * point.
 */
Qt::Edges QWaylandXdgPopupV6::gravityEdges() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_positionerData.gravityEdges;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopupV6::slideConstraints
 *
 * This property holds the orientations in which the child should slide to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property QWaylandXdgPopupV6::slideConstraints
 *
 * This property holds the orientations in which the child should slide to fit within the screen.
 */
Qt::Orientations QWaylandXdgPopupV6::slideConstraints() const
{
    Q_D(const QWaylandXdgPopupV6);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = 0;

    if (flags & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_SLIDE_X)
        constraints |= Qt::Horizontal;
    if (flags & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_SLIDE_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopupV6::flipConstraints
 *
 * This property holds the orientations in which the child should flip to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property QWaylandXdgPopupV6::flipConstraints
 *
 * This property holds the orientations in which the child should flip to fit within the screen.
 */
Qt::Orientations QWaylandXdgPopupV6::flipConstraints() const
{
    Q_D(const QWaylandXdgPopupV6);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = 0;

    if (flags & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_FLIP_X)
        constraints |= Qt::Horizontal;
    if (flags & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_FLIP_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty enumeration QtWaylandCompositor::XdgPopupV6::resizeConstraints
 *
 * This property holds the orientations in which the child should resize to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property QWaylandXdgPopupV6::resizeConstraints
 *
 * This property holds the orientations in which the child should resize to fit within the screen.
 */
Qt::Orientations QWaylandXdgPopupV6::resizeConstraints() const
{
    Q_D(const QWaylandXdgPopupV6);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = 0;

    if (flags & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_RESIZE_X)
        constraints |= Qt::Horizontal;
    if (flags & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_RESIZE_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty point QtWaylandCompositor::XdgPopupV6::offset
 *
 * The position relative to the position of the anchor on the anchor rectangle and
 * the anchor on the surface.
 */

/*!
 * \property QWaylandXdgPopupV6::offset
 *
 * Returns the surface position relative to the position of the anchor on the anchor
 * rectangle and the anchor on the surface.
 */
QPoint QWaylandXdgPopupV6::offset() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_positionerData.offset;
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgPopupV6::positionerSize
 *
 * The size requested for the window geometry by the positioner object.
 */

/*!
 * \property QWaylandXdgPopupV6::positionerSize
 *
 * Returns the size requested for the window geometry by the positioner object.
 */
QSize QWaylandXdgPopupV6::positionerSize() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_positionerData.size;
}

/*!
 * \qmlproperty point QtWaylandCompositor::XdgPopupV6::unconstrainedPosition
 *
 * The position of the surface relative to the parent window geometry if the surface
 * is not constrained. I.e. when not moved to fit inside the screen or similar.
 */

/*!
 * \property QWaylandXdgPopupV6::unconstrainedPosition
 *
 * The position of the surface relative to the parent window geometry if the surface
 * is not constrained. I.e. when not moved to fit inside the screen or similar.
 */
QPoint QWaylandXdgPopupV6::unconstrainedPosition() const
{
    Q_D(const QWaylandXdgPopupV6);
    return d->m_positionerData.unconstrainedPosition();
}

/*!
 * \qmlmethod int QtWaylandCompositor::XdgPopupV6::sendConfigure(rect geometry)
 *
 * Sends a configure event to the client. \a geometry contains the window geometry
 * relative to the upper left corner of the window geometry of the parent surface.
 *
 * This implicitly sends a configure event to the corresponding XdgSurfaceV6 as well.
 */

/*!
 * Sends a configure event to the client. \a geometry contains the window geometry
 * relative to the upper left corner of the window geometry of the parent surface.
 *
 * This implicitly sends a configure event to the corresponding QWaylandXdgSurfaceV6
 * as well.
 */
uint QWaylandXdgPopupV6::sendConfigure(const QRect &geometry)
{
    Q_D(QWaylandXdgPopupV6);
    return d->sendConfigure(geometry);
}

/*!
 * Returns the surface role for the QWaylandPopupV6.
 */
QWaylandSurfaceRole *QWaylandXdgPopupV6::role()
{
    return &QWaylandXdgPopupV6Private::s_role;
}

QWaylandXdgPopupV6Private::QWaylandXdgPopupV6Private(QWaylandXdgSurfaceV6 *xdgSurface, QWaylandXdgSurfaceV6 *parentXdgSurface,
                                                     QWaylandXdgPositionerV6 *positioner, const QWaylandResource &resource)
    : m_xdgSurface(xdgSurface)
    , m_parentXdgSurface(parentXdgSurface)
{
    init(resource.resource());
    m_positionerData = positioner->m_data;

    if (!m_positionerData.isComplete())
        qWarning() << "Trying to create xdg popup with incomplete positioner";

    QWaylandXdgSurfaceV6Private::get(m_xdgSurface)->setWindowType(Qt::WindowType::Popup);

    //TODO: positioner rect may not extend parent's window geometry, enforce this?
    //TODO: Need an API for sending a different initial configure
    sendConfigure(QRect(m_positionerData.unconstrainedPosition(), m_positionerData.size));
}

void QWaylandXdgPopupV6Private::handleAckConfigure(uint serial)
{
    Q_Q(QWaylandXdgPopupV6);
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

uint QWaylandXdgPopupV6Private::sendConfigure(const QRect &geometry)
{
    uint32_t serial = m_xdgSurface->surface()->compositor()->nextSerial();
    m_pendingConfigures.append(QWaylandXdgPopupV6Private::ConfigureEvent{geometry, serial});
    send_configure(geometry.x(), geometry.y(), geometry.width(), geometry.height());
    QWaylandXdgSurfaceV6Private::get(m_xdgSurface)->send_configure(serial);
    return serial;
}

void QWaylandXdgPopupV6Private::zxdg_popup_v6_destroy(QtWaylandServer::zxdg_popup_v6::Resource *resource)
{
    Q_UNUSED(resource);
    qWarning() << Q_FUNC_INFO << "Not implemented"; //TODO
}

void QWaylandXdgPopupV6Private::zxdg_popup_v6_grab(QtWaylandServer::zxdg_popup_v6::Resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_UNUSED(seat);
    qWarning() << Q_FUNC_INFO << "Not implemented"; //TODO
    //switch keyboard focus
    //eventually send configure with activated.
}

QWaylandSurfaceRole QWaylandXdgPopupV6Private::s_role("zxdg_popup_v6");

QWaylandXdgPositionerV6Data::QWaylandXdgPositionerV6Data()
    : offset(0, 0)
{}

bool QWaylandXdgPositionerV6Data::isComplete() const
{
    return size.width() > 0 && size.height() > 0 && anchorRect.size().width() > 0 && anchorRect.size().height() > 0;
}

QPoint QWaylandXdgPositionerV6Data::anchorPoint() const
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

QPoint QWaylandXdgPositionerV6Data::unconstrainedPosition() const
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

QWaylandXdgPositionerV6::QWaylandXdgPositionerV6(const QWaylandResource &resource)
{
    init(resource.resource());
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_destroy_resource(QtWaylandServer::zxdg_positioner_v6::Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_destroy(QtWaylandServer::zxdg_positioner_v6::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_set_size(QtWaylandServer::zxdg_positioner_v6::Resource *resource, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "zxdg_positioner_v6.set_size requested with non-positive dimensions");
        return;
    }

    QSize size(width, height);
    m_data.size = size;
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_set_anchor_rect(QtWaylandServer::zxdg_positioner_v6::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "zxdg_positioner_v6.set_anchor_rect requested with non-positive dimensions");
        return;
    }

    QRect anchorRect(x, y, width, height);
    m_data.anchorRect = anchorRect;
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_set_anchor(QtWaylandServer::zxdg_positioner_v6::Resource *resource, uint32_t anchor)
{
    Qt::Edges anchorEdges = QWaylandXdgShellV6Private::convertToEdges(anchor);

    if ((anchorEdges & Qt::BottomEdge && anchorEdges & Qt::TopEdge) ||
            (anchorEdges & Qt::LeftEdge && anchorEdges & Qt::RightEdge)) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "zxdg_positioner_v6.set_anchor requested with parallel edges");
        return;
    }

    m_data.anchorEdges = anchorEdges;
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_set_gravity(QtWaylandServer::zxdg_positioner_v6::Resource *resource, uint32_t gravity)
{
    Qt::Edges gravityEdges = QWaylandXdgShellV6Private::convertToEdges(gravity);

    if ((gravityEdges & Qt::BottomEdge && gravityEdges & Qt::TopEdge) ||
            (gravityEdges & Qt::LeftEdge && gravityEdges & Qt::RightEdge)) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "zxdg_positioner_v6.set_gravity requested with parallel edges");
        return;
    }

    m_data.gravityEdges = gravityEdges;
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_set_constraint_adjustment(QtWaylandServer::zxdg_positioner_v6::Resource *resource, uint32_t constraint_adjustment)
{
    Q_UNUSED(resource);
    m_data.constraintAdjustments = constraint_adjustment;
}

void QWaylandXdgPositionerV6::zxdg_positioner_v6_set_offset(QtWaylandServer::zxdg_positioner_v6::Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    m_data.offset = QPoint(x, y);
}

QWaylandXdgPositionerV6 *QWaylandXdgPositionerV6::fromResource(wl_resource *resource)
{
    if (auto *r = Resource::fromResource(resource))
        return static_cast<QWaylandXdgPositionerV6 *>(r->zxdg_positioner_v6_object);
    return nullptr;
}

QT_END_NAMESPACE
