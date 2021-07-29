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

#ifndef DMABUFSERVERBUFFERINTEGRATION_H
#define DMABUFSERVERBUFFERINTEGRATION_H

#include <QtWaylandClient/private/qwayland-wayland.h>
#include "qwayland-qt-dmabuf-server-buffer.h"
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>

#include "dmabufserverbufferintegration.h"
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtCore/QTextStream>

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

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class DmaBufServerBufferIntegration;

class DmaBufServerBuffer : public QWaylandServerBuffer
{
public:
    DmaBufServerBuffer(DmaBufServerBufferIntegration *integration, struct ::qt_server_buffer *id, int32_t fd,
                       int32_t width, int32_t height, int32_t stride, int32_t offset, int32_t fourcc_format);
    ~DmaBufServerBuffer() override;
    QOpenGLTexture* toOpenGlTexture() override;
private:
    DmaBufServerBufferIntegration *m_integration = nullptr;
    EGLImageKHR m_image = EGL_NO_IMAGE_KHR;
    QOpenGLTexture *m_texture = nullptr;
    struct ::qt_server_buffer *m_server_buffer = nullptr;
};

class DmaBufServerBufferIntegration
    : public QWaylandServerBufferIntegration
    , public QtWayland::qt_dmabuf_server_buffer
{
public:
    void initialize(QWaylandDisplay *display) override;

    QWaylandServerBuffer *serverBuffer(struct qt_server_buffer *buffer) override;

    inline EGLImageKHR eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    inline EGLBoolean eglDestroyImageKHR(EGLImageKHR image);
    inline void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image);
protected:
    void dmabuf_server_buffer_server_buffer_created(struct ::qt_server_buffer *id, int32_t fd,
                                                    int32_t width, int32_t height, int32_t stride,
                                                    int32_t offset, int32_t fourcc_format) override;

private:
    static void wlDisplayHandleGlobal(void *data, struct ::wl_registry *registry, uint32_t id,
                                      const QString &interface, uint32_t version);
    void initializeEgl();

    PFNEGLCREATEIMAGEKHRPROC m_egl_create_image = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC m_egl_destroy_image = nullptr;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_gl_egl_image_target_texture = nullptr;
    QWaylandDisplay *m_display = nullptr;
    EGLDisplay m_egl_display = EGL_NO_DISPLAY;
    bool m_egl_initialized = false;
};

EGLImageKHR DmaBufServerBufferIntegration::eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    if (!m_egl_initialized)
        initializeEgl();
    if (!m_egl_create_image) {
        qCWarning(lcQpaWayland) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglCreateImageKHR";
        return EGL_NO_IMAGE_KHR;
    }
    return m_egl_create_image(m_egl_display, ctx, target, buffer, attrib_list);
}

EGLBoolean DmaBufServerBufferIntegration::eglDestroyImageKHR(EGLImageKHR image)
{
    if (!m_egl_destroy_image) {
        qCWarning(lcQpaWayland) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglDestroyImageKHR";
        return false;
    }
    return m_egl_destroy_image(m_egl_display, image);
}

void DmaBufServerBufferIntegration::glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    if (!m_gl_egl_image_target_texture) {
        qCWarning(lcQpaWayland) << "DmaBufServerBufferIntegration: Trying to use unresolved function glEGLImageTargetTexture2DOES";
        return;
    }
    m_gl_egl_image_target_texture(target, image);
}

}

QT_END_NAMESPACE

#endif
