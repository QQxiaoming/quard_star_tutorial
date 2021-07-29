/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "sharebufferextension.h"

#include <QWaylandSurface>

#include <QDebug>

#include <QQuickWindow>

#include <QPainter>
#include <QPen>

ShareBufferExtension::ShareBufferExtension(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(compositor)
{
}

void ShareBufferExtension::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);
}

QtWayland::ServerBuffer *ShareBufferExtension::addImage(const QImage &img)
{
    if (!m_server_buffer_integration) {
        QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());

        m_server_buffer_integration = QWaylandCompositorPrivate::get(compositor)->serverBufferIntegration();
        if (!m_server_buffer_integration) {
            qWarning("Could not find a server buffer integration");
            return nullptr;
        }
    }

    QImage image = img.convertToFormat(QImage::Format_RGBA8888);

    auto *buffer = m_server_buffer_integration->createServerBufferFromImage(image, QtWayland::ServerBuffer::RGBA32);

    m_server_buffers.append(buffer);
    return buffer;
}

void ShareBufferExtension::createServerBuffers()
{
    QImage image(100,100,QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0x55,0x0,0x55,0x01));
    {
        QPainter p(&image);
        QPen pen = p.pen();
        pen.setWidthF(3);
        pen.setColor(Qt::red);
        p.setPen(pen);
        p.drawLine(0,0,100,100);
        pen.setColor(Qt::green);
        p.setPen(pen);
        p.drawLine(100,0,0,100);
        pen.setColor(Qt::blue);
        p.setPen(pen);
        p.drawLine(25,15,75,15);
    }

    addImage(image);

    QImage image2(":/images/Siberischer_tiger_de_edit02.jpg");
    addImage(image2);

    m_server_buffers_created = true;
}


void ShareBufferExtension::share_buffer_bind_resource(Resource *resource)
{
    if (!m_server_buffers_created)
        createServerBuffers();

    for (auto *buffer : qAsConst(m_server_buffers)) {
        qDebug() << "sending" << buffer << "to client";
        struct ::wl_client *client = wl_resource_get_client(resource->handle);
        struct ::wl_resource *buffer_resource = buffer->resourceForClient(client);
        send_cross_buffer(resource->handle, buffer_resource);
    }
}
