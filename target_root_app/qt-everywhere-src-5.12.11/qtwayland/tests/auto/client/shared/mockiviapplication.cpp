/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "mockiviapplication.h"
#include "mocksurface.h"
#include "mockcompositor.h"

namespace Impl {

void Compositor::sendIviSurfaceConfigure(void *data, const QList<QVariant> &parameters)
{
    Q_UNUSED(data);
    IviSurface *iviSurface = resolveIviSurface(parameters.at(0));
    Q_ASSERT(iviSurface && iviSurface->resource());
    QSize size = parameters.at(1).toSize();
    Q_ASSERT(!size.isEmpty());
    iviSurface->send_configure(size.width(), size.height());
}

IviSurface::IviSurface(IviApplication *iviApplication, Surface *surface, uint iviId, wl_client *client, uint32_t id)
    : QtWaylandServer::ivi_surface(client, id, 1)
    , m_surface(surface)
    , m_iviApplication(iviApplication)
    , m_iviId(iviId)
    , m_mockIviSurface(new MockIviSurface(this))
{
    iviApplication->addIviSurface(this);
    surface->map();
}

IviSurface::~IviSurface()
{
    m_iviApplication->removeIviSurface(this);
    m_mockIviSurface->m_iviSurface = nullptr;
}

void IviSurface::ivi_surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void IviApplication::ivi_application_surface_create(Resource *resource, uint32_t ivi_id, ::wl_resource *surface, uint32_t id)
{
    new IviSurface(this, Surface::fromResource(surface), ivi_id, resource->client(), id);
}

} // namespace Impl
