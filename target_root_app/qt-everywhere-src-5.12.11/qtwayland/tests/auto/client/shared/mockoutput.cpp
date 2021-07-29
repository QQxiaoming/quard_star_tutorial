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

#include "mockcompositor.h"
#include "mockoutput.h"

#include <QDebug>

namespace Impl {

void Compositor::sendAddOutput(void *data, const QList<QVariant> &parameters) {
    Q_UNUSED(parameters);
    Compositor *compositor = static_cast<Compositor *>(data);
    auto output = new Output(compositor->m_display, QSize(1920, 1200), QPoint(0, 0));
    compositor->m_outputs.append(output);

    // Wait for the client to bind to the output
    while (output->resourceMap().isEmpty())
        compositor->dispatchEvents();
}

void Compositor::sendRemoveOutput(void *data, const QList<QVariant> &parameters) {
    Compositor *compositor = static_cast<Compositor *>(data);
    Q_ASSERT(compositor);
    Output *output = resolveOutput(parameters.first());
    Q_ASSERT(output);
    bool wasRemoved = compositor->m_outputs.removeOne(output);
    Q_ASSERT(wasRemoved);
    delete output;
}

void Compositor::sendOutputGeometry(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    Q_ASSERT(compositor);
    Output *output = resolveOutput(parameters.first());
    Q_ASSERT(output);
    QRect geometry = parameters.at(1).toRect();
    output->sendGeometryAndMode(geometry);
}

void Compositor::setOutputMode(void *data, const QList<QVariant> &parameters)
{
    Compositor *compositor = static_cast<Compositor *>(data);
    QSize size = parameters.first().toSize();
    Output *output = compositor->m_outputs.first();
    Q_ASSERT(output);
    output->setCurrentMode(size);
}

Output::Output(wl_display *display, const QSize &resolution, const QPoint &position)
    : wl_output(display, 2)
    , m_size(resolution)
    , m_position(position)
    , m_physicalSize(520, 320)
    , m_mockOutput(new MockOutput(this))
{
}

void Output::setCurrentMode(const QSize &size)
{
    m_size = size;
    for (Resource *resource : resourceMap()) {
        sendCurrentMode(resource);
        send_done(resource->handle);
    }
}

void Output::sendGeometryAndMode(const QRect &geometry)
{
    m_size = geometry.size();
    m_position = geometry.topLeft();
    for (Resource *resource : resourceMap()) {
        sendGeometry(resource);
        sendCurrentMode(resource);
        send_done(resource->handle);
    }
}

void Output::output_bind_resource(QtWaylandServer::wl_output::Resource *resource)
{
    sendGeometry(resource);
    sendCurrentMode(resource);
    send_done(resource->handle);
}

void Output::sendGeometry(Resource *resource)
{
    const int subPixel = 0;
    const int transform = 0;

    send_geometry(resource->handle,
                  m_position.x(), m_position.y(),
                  m_physicalSize.width(), m_physicalSize.height(),
                  subPixel, "", "", transform );
}

void Output::sendCurrentMode(Resource *resource)
{
    send_mode(resource->handle,
              WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED,
              m_size.width(), m_size.height(), 60000);
}

} // Impl

MockOutput::MockOutput(Impl::Output *output)
    : m_output(output)
{
}
