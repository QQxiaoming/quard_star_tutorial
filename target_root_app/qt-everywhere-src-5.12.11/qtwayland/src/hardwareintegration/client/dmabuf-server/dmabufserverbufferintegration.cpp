/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QDebug>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLTexture>

#include <EGL/egl.h>
#include <EGL/eglext.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

DmaBufServerBuffer::DmaBufServerBuffer(DmaBufServerBufferIntegration *integration
                                       , struct ::qt_server_buffer *id
                                       , int32_t fd
                                       , int32_t width
                                       , int32_t height
                                       , int32_t stride
                                       , int32_t offset
                                       , int32_t fourcc_format)
    : m_integration(integration)
    , m_server_buffer(id)
{
    m_size = QSize(width, height);


    EGLint import_attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_LINUX_DRM_FOURCC_EXT, fourcc_format,
        EGL_DMA_BUF_PLANE0_FD_EXT, fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
        EGL_NONE
    };

    m_image = integration->eglCreateImageKHR(
        EGL_NO_CONTEXT,
        EGL_LINUX_DMA_BUF_EXT,
        (EGLClientBuffer)nullptr,
        import_attribs);

    int err = eglGetError();
    qCDebug(lcQpaWayland) << "imported egl image" << m_image;
    if (m_image == EGL_NO_IMAGE_KHR || err != EGL_SUCCESS)
       qCWarning(lcQpaWayland) << "DmaBufServerBuffer error importing image. EGL error code" << hex << err;

    qt_server_buffer_set_user_data(id, this);

}

DmaBufServerBuffer::~DmaBufServerBuffer()
{
    int err = m_integration->eglDestroyImageKHR(m_image);
    if (err != EGL_SUCCESS)
        qCWarning(lcQpaWayland) << "~DmaBufServerBuffer error destroying image" << m_image << "error code " << hex << err;
    qt_server_buffer_release(m_server_buffer);
    qt_server_buffer_destroy(m_server_buffer);
}

QOpenGLTexture *DmaBufServerBuffer::toOpenGlTexture()
{
    if (!QOpenGLContext::currentContext())
        qCWarning(lcQpaWayland) <<"DmaBufServerBuffer: creating texture with no current context";

    if (!m_texture) {
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        m_texture->create();
    }

    m_texture->bind();
    m_integration->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_texture->release();
    m_texture->setSize(m_size.width(), m_size.height());
    return m_texture;
}

void DmaBufServerBufferIntegration::initializeEgl()
{
    if (m_egl_initialized)
        return;
    m_egl_initialized = true;

    m_egl_display = eglGetDisplay((EGLNativeDisplayType) m_display->wl_display());
    if (m_egl_display == EGL_NO_DISPLAY) {
        qCWarning(lcQpaWayland) << "Failed to initialize drm egl server buffer integration. Could not get egl display from wl_display.";
        return;
    }

    const char *extensionString = eglQueryString(m_egl_display, EGL_EXTENSIONS);


    if (!extensionString || !strstr(extensionString, "EGL_KHR_image")) {
        qCWarning(lcQpaWayland) << "Failed to initialize dmabuf server buffer integration. There is no EGL_KHR_image extension.\n";
        return;
    }
    m_egl_create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    m_egl_destroy_image = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!m_egl_create_image || !m_egl_destroy_image) {
        qCWarning(lcQpaWayland) << "Failed to initialize dmabuf server buffer integration. Could not resolve eglCreateImageKHR or eglDestroyImageKHR";
        return;
    }

    m_gl_egl_image_target_texture = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!m_gl_egl_image_target_texture) {
        qCWarning(lcQpaWayland) << "Failed to initialize dmabuf server buffer integration. Could not resolve glEGLImageTargetTexture2DOES";
        return;
    }
}

void DmaBufServerBufferIntegration::initialize(QWaylandDisplay *display)
{
    m_display = display;
    display->addRegistryListener(&wlDisplayHandleGlobal, this);
}

QWaylandServerBuffer *DmaBufServerBufferIntegration::serverBuffer(struct qt_server_buffer *buffer)
{
    return static_cast<QWaylandServerBuffer *>(qt_server_buffer_get_user_data(buffer));
}

void DmaBufServerBufferIntegration::wlDisplayHandleGlobal(void *data, ::wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    Q_UNUSED(version);
    if (interface == QStringLiteral("qt_dmabuf_server_buffer")) {
        auto *integration = static_cast<DmaBufServerBufferIntegration *>(data);
        integration->QtWayland::qt_dmabuf_server_buffer::init(registry, id, 1);
    }
}

void DmaBufServerBufferIntegration::dmabuf_server_buffer_server_buffer_created(struct ::qt_server_buffer *id
                                                                               , int32_t name
                                                                               , int32_t width
                                                                               , int32_t height
                                                                               , int32_t stride
                                                                               , int32_t offset
                                                                               , int32_t format)
{
    (void) new DmaBufServerBuffer(this, id, name, width, height, stride, offset, format);
}

}

QT_END_NAMESPACE
