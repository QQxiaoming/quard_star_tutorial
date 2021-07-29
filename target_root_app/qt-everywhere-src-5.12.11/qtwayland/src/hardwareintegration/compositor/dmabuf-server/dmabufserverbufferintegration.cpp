/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "dmabufserverbufferintegration.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLTexture>

#include <drm_fourcc.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE

DmaBufServerBuffer::DmaBufServerBuffer(DmaBufServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format)
    : QtWayland::ServerBuffer(qimage.size(),format)
    , m_integration(integration)
{
    m_format = format;

    EGLContext eglContext = eglGetCurrentContext();

    m_texture = new QOpenGLTexture(qimage);

    m_image  = m_integration->eglCreateImageKHR(eglContext, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(unsigned long)m_texture->textureId(), nullptr);

    qCDebug(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer created egl image" << m_image;

    int err = eglGetError();
    if (err != EGL_SUCCESS || m_image == EGL_NO_IMAGE_KHR)
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer error creating EGL image" << hex << err;

    // TODO: formats with more than one plane

    int num_planes = 1;

    if (!m_integration->eglExportDMABUFImageQueryMESA(m_image, &m_fourcc_format, &num_planes, nullptr)) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer: Failed to query egl image";
        qCDebug(qLcWaylandCompositorHardwareIntegration) << "error" << hex << eglGetError();
    } else {
        qCDebug(qLcWaylandCompositorHardwareIntegration) << "num_planes" << num_planes << "fourcc_format" << m_fourcc_format;
        if (num_planes != 1) {
            qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer: multi-plane formats not supported";
            delete m_texture;
            m_texture = nullptr;
            m_integration->eglDestroyImageKHR(m_image);
            m_image = EGL_NO_IMAGE_KHR;
            return;
        }
    }

    if (!m_integration->eglExportDMABUFImageMESA(m_image, &m_fd, &m_stride, &m_offset)) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer: Failed to export egl image. Error code" << hex << eglGetError();
    } else {
        qCDebug(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer exported egl image: fd" << m_fd << "stride" << m_stride << "offset" << m_offset;
        m_texture->release();
    }
}

DmaBufServerBuffer::~DmaBufServerBuffer()
{
    delete m_texture;

    int err;
    m_integration->eglDestroyImageKHR(m_image);
    if ((err = eglGetError()) != EGL_SUCCESS)
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "~DmaBufServerBuffer: eglDestroyImageKHR error" << hex << err;

    err = ::close(m_fd);
    if (err)
        perror("~DmaBufServerBuffer:: error closing fd");

}

struct ::wl_resource *DmaBufServerBuffer::resourceForClient(struct ::wl_client *client)
{
    auto *bufferResource = resourceMap().value(client);
    if (!bufferResource) {
        auto integrationResource = m_integration->resourceMap().value(client);
        if (!integrationResource) {
            qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer::resourceForClient: Trying to get resource for ServerBuffer. But client is not bound to the qt_dmabuf_server_buffer interface";
            return nullptr;
        }
        struct ::wl_resource *dmabuf_integration_resource = integrationResource->handle;

        Resource *resource = add(client, 1);
        m_integration->send_server_buffer_created(dmabuf_integration_resource, resource->handle, m_fd, m_size.width(), m_size.height(), m_stride, m_offset, m_fourcc_format);
        return resource->handle;
    }
    return bufferResource->handle;
}


QOpenGLTexture *DmaBufServerBuffer::toOpenGlTexture()
{
    if (!m_texture) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBuffer::toOpenGlTexture: no texture defined";
    }
    return m_texture;
}

bool DmaBufServerBuffer::bufferInUse()
{
    return resourceMap().count() > 0;
}

DmaBufServerBufferIntegration::DmaBufServerBufferIntegration()
{
}

DmaBufServerBufferIntegration::~DmaBufServerBufferIntegration()
{
}

void DmaBufServerBufferIntegration::initializeHardware(QWaylandCompositor *compositor)
{
    Q_ASSERT(QGuiApplication::platformNativeInterface());

    m_egl_display = static_cast<EGLDisplay>(QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("egldisplay"));
    if (!m_egl_display) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Cannot initialize dmabuf server buffer integration. Missing egl display from platform plugin";
        return;
    }

    const char *extensionString = eglQueryString(m_egl_display, EGL_EXTENSIONS);
    if (!extensionString || !strstr(extensionString, "EGL_KHR_image")) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize dmabuf server buffer integration. There is no EGL_KHR_image extension.";
        return;
    }

    m_egl_create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    m_egl_destroy_image = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!m_egl_create_image || !m_egl_destroy_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize dmabuf server buffer integration. Could not resolve eglCreateImageKHR or eglDestroyImageKHR";
        return;
    }

    m_gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!m_gl_egl_image_target_texture_2d) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize dmabuf server buffer integration. Could not find glEGLImageTargetTexture2DOES.";
        return;
    }

    m_egl_export_dmabuf_image_query = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC>(eglGetProcAddress("eglExportDMABUFImageQueryMESA"));
    if (!m_egl_export_dmabuf_image_query) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize dmabuf server buffer integration. Could not find eglExportDMABUFImageQueryMESA.";
        return;
    }

    m_egl_export_dmabuf_image = reinterpret_cast<PFNEGLEXPORTDMABUFIMAGEMESAPROC>(eglGetProcAddress("eglExportDMABUFImageMESA"));
    if (!m_egl_export_dmabuf_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize dmabuf server buffer integration. Could not find eglExportDMABUFImageMESA.";
        return;
    }

    QtWaylandServer::qt_dmabuf_server_buffer::init(compositor->display(), 1);
}

bool DmaBufServerBufferIntegration::supportsFormat(QtWayland::ServerBuffer::Format format) const
{
    // TODO: 8-bit format support
    switch (format) {
    case QtWayland::ServerBuffer::RGBA32:
        return true;
    case QtWayland::ServerBuffer::A8:
        return false;
    default:
        return false;
    }
}

QtWayland::ServerBuffer *DmaBufServerBufferIntegration::createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format)
{
    return new DmaBufServerBuffer(this, qimage, format);
}

void DmaBufServerBuffer::server_buffer_release(Resource *resource)
{
    qCDebug(qLcWaylandCompositorHardwareIntegration) << "server_buffer RELEASE resource" << resource->handle << wl_resource_get_id(resource->handle) << "for client" << resource->client();
    wl_resource_destroy(resource->handle);
}

QT_END_NAMESPACE
