/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "mocksurface.h"
#include "mockoutput.h"
#include "mockcompositor.h"
#include "mockwlshell.h"

#include <QDebug>

namespace Impl {

void Compositor::sendSurfaceEnter(void *data, const QList<QVariant> &parameters)
{
    Q_UNUSED(data);
    Surface *surface = resolveSurface(parameters.at(0));
    Output *output = resolveOutput(parameters.at(1));
    Q_ASSERT(surface && surface->resource());
    Q_ASSERT(output);
    auto outputResources = output->resourceMap().values(surface->resource()->client());
    Q_ASSERT(!outputResources.isEmpty());

    for (auto outputResource : outputResources)
        surface->send_enter(outputResource->handle);
}

void Compositor::sendSurfaceLeave(void *data, const QList<QVariant> &parameters)
{
    Q_UNUSED(data);
    Surface *surface = resolveSurface(parameters.at(0));
    Output *output = resolveOutput(parameters.at(1));
    Q_ASSERT(surface && surface->resource());
    Q_ASSERT(output);
    auto outputResources = output->resourceMap().values(surface->resource()->client());
    Q_ASSERT(!outputResources.isEmpty());

    for (auto outputResource : outputResources)
        surface->send_leave(outputResource->handle);
}

void Compositor::sendShellSurfaceConfigure(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    Surface *surface = resolveSurface(parameters.at(0));
    QSize size = parameters.at(1).toSize();
    Q_ASSERT(size.isValid());
    if (auto toplevel = surface->xdgToplevelV6()) {
        QVector<uint> states = { ZXDG_TOPLEVEL_V6_STATE_ACTIVATED };
        auto statesBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(states.data()),
                                                   states.size() * static_cast<int>(sizeof(uint)));
        toplevel->send_configure(size.width(), size.height(), statesBytes);
        toplevel->xdgSurface()->sendConfigure(compositor->nextSerial());
    } else if (auto wlShellSurface = surface->wlShellSurface()) {
        const uint edges = 0;
        wlShellSurface->send_configure(edges, size.width(), size.height());
    } else {
        qWarning() << "The mocking framework doesn't know how to send a configure event for this surface";
    }
}

Surface::Surface(wl_client *client, uint32_t id, int v, Compositor *compositor)
    : QtWaylandServer::wl_surface(client, id, v)
    , m_compositor(compositor)
    , m_mockSurface(new MockSurface(this))
{
}

Surface::~Surface()
{
    m_mockSurface->m_surface = 0;
}

void Surface::map()
{
    m_mapped = true;
}

bool Surface::isMapped() const
{
    return m_mapped;
}

Surface *Surface::fromResource(struct ::wl_resource *resource)
{
    if (auto *r = Resource::fromResource(resource))
        return static_cast<Surface *>(r->surface_object);
    return nullptr;
}

void Surface::surface_destroy_resource(Resource *)
{
    compositor()->removeSurface(this);
    delete this;
}

void Surface::surface_destroy(Resource *resource)
{
    if (m_wlShellSurface) // on wl-shell the shell surface is automatically destroyed with the surface
        wl_resource_destroy(m_wlShellSurface->resource()->handle);
    Q_ASSERT(!m_wlShellSurface);
    Q_ASSERT(!m_xdgSurfaceV6);
    wl_resource_destroy(resource->handle);
}

void Surface::surface_attach(Resource *resource, struct wl_resource *buffer, int x, int y)
{
    if (m_xdgSurfaceV6) {
        // It's a protocol error to attach a buffer to an xdgSurface that's not configured
        Q_ASSERT(xdgSurfaceV6()->configureSent());
    }

    Q_UNUSED(resource);
    Q_UNUSED(x);
    Q_UNUSED(y);
    m_buffer = buffer;

    if (!buffer)
        m_mockSurface->image = QImage();
}

void Surface::surface_damage(Resource *resource,
                             int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(resource);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void Surface::surface_frame(Resource *resource,
                            uint32_t callback)
{
    wl_resource *frameCallback = wl_resource_create(resource->client(), &wl_callback_interface, 1, callback);
    m_frameCallbackList << frameCallback;
}

void Surface::surface_commit(Resource *resource)
{
    Q_UNUSED(resource);

    if (m_buffer) {
        struct ::wl_shm_buffer *shm_buffer = wl_shm_buffer_get(m_buffer);
        if (shm_buffer) {
            int stride = wl_shm_buffer_get_stride(shm_buffer);
            uint format = wl_shm_buffer_get_format(shm_buffer);
            Q_UNUSED(format);
            void *data = wl_shm_buffer_get_data(shm_buffer);
            const uchar *char_data = static_cast<const uchar *>(data);
            QImage img(char_data, wl_shm_buffer_get_width(shm_buffer), wl_shm_buffer_get_height(shm_buffer), stride, QImage::Format_ARGB32_Premultiplied);
            m_mockSurface->image = img;
        }
    }

    foreach (wl_resource *frameCallback, m_frameCallbackList) {
        wl_callback_send_done(frameCallback, m_compositor->time());
        wl_resource_destroy(frameCallback);
    }
    m_frameCallbackList.clear();
}

}
MockSurface::MockSurface(Impl::Surface *surface)
    : m_surface(surface)
{
}
