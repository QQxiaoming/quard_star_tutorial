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

#ifndef DMABUFSERVERBUFFERINTEGRATION_H
#define DMABUFSERVERBUFFERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-qt-dmabuf-server-buffer.h"

#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/private/qwayland-server-server-buffer-extension.h>

#include <QtCore/QDebug>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_KHR_image
typedef void *EGLImageKHR;
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
#endif

#ifndef EGL_MESA_image_dma_buf_export
typedef EGLBoolean (EGLAPIENTRYP PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC) (EGLDisplay dpy, EGLImageKHR image, int *fourcc, int *num_planes, EGLuint64KHR *modifiers);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLEXPORTDMABUFIMAGEMESAPROC) (EGLDisplay dpy, EGLImageKHR image, int *fds, EGLint *strides, EGLint *offsets);
#endif

QT_BEGIN_NAMESPACE

class DmaBufServerBufferIntegration;
class QImage;

class DmaBufServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_server_buffer
{
public:
    DmaBufServerBuffer(DmaBufServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);
    ~DmaBufServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    QOpenGLTexture *toOpenGlTexture() override;
    bool bufferInUse() override;

protected:
    void server_buffer_release(Resource *resource) override;

private:
    DmaBufServerBufferIntegration *m_integration = nullptr;

    EGLImageKHR m_image;

    int32_t m_offset;
    int32_t m_stride;
    QOpenGLTexture *m_texture = nullptr;
    int m_fourcc_format;
    int m_fd;
};

class DmaBufServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::qt_dmabuf_server_buffer
{
public:
    DmaBufServerBufferIntegration();
    ~DmaBufServerBufferIntegration() override;

    void initializeHardware(QWaylandCompositor *) override;

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;

    EGLDisplay display() const { return m_egl_display; }

    inline EGLImageKHR eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    inline EGLBoolean eglDestroyImageKHR(EGLImageKHR image);

    inline EGLBoolean eglExportDMABUFImageQueryMESA(EGLImageKHR image,
                                                    int *fourcc,
                                                    int *num_planes,
                                                    EGLuint64KHR *modifiers);

    inline EGLBoolean eglExportDMABUFImageMESA(EGLImageKHR image,
                                               int *fds,
                                               EGLint *strides,
                                               EGLint *offsets);

    inline void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image);

private:
    EGLDisplay m_egl_display;

    PFNEGLEXPORTDMABUFIMAGEMESAPROC m_egl_export_dmabuf_image = nullptr;
    PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC m_egl_export_dmabuf_image_query = nullptr;

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_gl_egl_image_target_texture_2d = nullptr;
    PFNEGLCREATEIMAGEKHRPROC m_egl_create_image = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC m_egl_destroy_image = nullptr;
};

EGLImageKHR DmaBufServerBufferIntegration::eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    if (!m_egl_create_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglCreateImageKHR";
        return EGL_NO_IMAGE_KHR;
    }
    return m_egl_create_image(m_egl_display, ctx, target, buffer, attrib_list);
}

EGLBoolean DmaBufServerBufferIntegration::eglDestroyImageKHR(EGLImageKHR image)
{
    if (!m_egl_destroy_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglDestroyImageKHR";
        return false;
    }
    return m_egl_destroy_image(m_egl_display, image);
}

EGLBoolean DmaBufServerBufferIntegration::eglExportDMABUFImageQueryMESA(EGLImageKHR image,
                                                                        int *fourcc,
                                                                        int *num_planes,
                                                                        EGLuint64KHR *modifiers)
{
    if (m_egl_export_dmabuf_image_query)
        return m_egl_export_dmabuf_image_query(m_egl_display, image, fourcc, num_planes, modifiers);
    else
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglExportDMABUFImageQueryMESA";
    return false;
}

EGLBoolean DmaBufServerBufferIntegration::eglExportDMABUFImageMESA(EGLImageKHR image,
                                                                   int *fds,
                                                                   EGLint *strides,
                                                                   EGLint *offsets)
{
    if (m_egl_export_dmabuf_image)
        return m_egl_export_dmabuf_image(m_egl_display, image, fds, strides, offsets);
    else
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglExportDMABUFImageMESA";
    return false;
}

void DmaBufServerBufferIntegration::glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    if (m_gl_egl_image_target_texture_2d)
        return m_gl_egl_image_target_texture_2d(target, image);
    else
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function glEGLImageTargetTexture2DOES";
}
QT_END_NAMESPACE

#endif
