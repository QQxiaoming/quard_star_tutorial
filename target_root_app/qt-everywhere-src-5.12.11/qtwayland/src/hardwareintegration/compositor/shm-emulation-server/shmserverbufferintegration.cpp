/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#include "shmserverbufferintegration.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLTexture>
#include <QtCore/QSharedMemory>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

ShmServerBuffer::ShmServerBuffer(ShmServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format)
    : QtWayland::ServerBuffer(qimage.size(),format)
    , m_integration(integration)
    , m_width(qimage.width())
    , m_height(qimage.height())
    , m_bpl(qimage.bytesPerLine())
{
    m_format = format;
    switch (m_format) {
        case RGBA32:
            m_shm_format = QtWaylandServer::qt_shm_emulation_server_buffer::format_RGBA32;
            break;
        case A8:
            m_shm_format = QtWaylandServer::qt_shm_emulation_server_buffer::format_A8;
            break;
        default:
            qWarning("ShmServerBuffer: unsupported format");
            m_shm_format = QtWaylandServer::qt_shm_emulation_server_buffer::format_RGBA32;
            break;
    }

    QString key = "qt_shm_emulation_" + QString::number(qimage.cacheKey());
    m_shm = new QSharedMemory(key);
    qsizetype shm_size = qimage.sizeInBytes();
    bool ok = m_shm->create(shm_size) && m_shm->lock();
    if (ok) {
        memcpy(m_shm->data(), qimage.constBits(), shm_size);
        m_shm->unlock();
    } else {
        qWarning() << "Could not create shared memory" << key << shm_size;
    }
}

ShmServerBuffer::~ShmServerBuffer()
{
    delete m_shm;
}

struct ::wl_resource *ShmServerBuffer::resourceForClient(struct ::wl_client *client)
{
    auto *bufferResource = resourceMap().value(client);
    if (!bufferResource) {
        auto integrationResource = m_integration->resourceMap().value(client);
        if (!integrationResource) {
            qWarning("ShmServerBuffer::resourceForClient: Trying to get resource for ServerBuffer. But client is not bound to the shm_emulation interface");
            return nullptr;
        }
        struct ::wl_resource *shm_integration_resource = integrationResource->handle;
        Resource *resource = add(client, 1);
        m_integration->send_server_buffer_created(shm_integration_resource, resource->handle, m_shm->key(), m_width, m_height, m_bpl, m_shm_format);
        return resource->handle;
    }
    return bufferResource->handle;
}

bool ShmServerBuffer::bufferInUse()
{
    return resourceMap().count() > 0;
}

QOpenGLTexture *ShmServerBuffer::toOpenGlTexture()
{
    if (!m_texture) {
        qWarning("ShmServerBuffer::toOpenGlTexture: no texture defined");
    }
    return m_texture;
}

ShmServerBufferIntegration::ShmServerBufferIntegration()
{
}

ShmServerBufferIntegration::~ShmServerBufferIntegration()
{
}

void ShmServerBufferIntegration::initializeHardware(QWaylandCompositor *compositor)
{
    Q_ASSERT(QGuiApplication::platformNativeInterface());

    QtWaylandServer::qt_shm_emulation_server_buffer::init(compositor->display(), 1);
}

bool ShmServerBufferIntegration::supportsFormat(QtWayland::ServerBuffer::Format format) const
{
    switch (format) {
        case QtWayland::ServerBuffer::RGBA32:
            return true;
        case QtWayland::ServerBuffer::A8:
            return true;
        default:
            return false;
    }
}

QtWayland::ServerBuffer *ShmServerBufferIntegration::createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format)
{
    return new ShmServerBuffer(this, qimage, format);
}

QT_END_NAMESPACE
