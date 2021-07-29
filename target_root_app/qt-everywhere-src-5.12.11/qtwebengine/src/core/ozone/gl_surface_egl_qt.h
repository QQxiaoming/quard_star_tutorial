/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#ifndef GL_SURFACE_EGL_QT_H_
#define GL_SURFACE_EGL_QT_H_

#include "gl_surface_qt.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace gl {

class GLSurfaceEGLQt: public GLSurfaceQt {
public:
    explicit GLSurfaceEGLQt(const gfx::Size& size);

    static bool InitializeOneOff();
    static bool InitializeExtensionSettingsOneOff();

    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    void* GetHandle() override;
    bool Resize(const gfx::Size& size, float scale_factor,
                ColorSpace color_space, bool has_alpha) override;

protected:
    ~GLSurfaceEGLQt();

public:
   static bool g_egl_surfaceless_context_supported;

private:
   EGLSurface m_surfaceBuffer;
   static bool s_initialized;
   DISALLOW_COPY_AND_ASSIGN(GLSurfaceEGLQt);
};

// The following comment is cited from chromium/ui/gl/gl_surface_egl.cc:
// SurfacelessEGL is used as Offscreen surface when platform supports
// KHR_surfaceless_context and GL_OES_surfaceless_context. This would avoid the
// need to create a dummy EGLsurface in case we render to client API targets.

class GLSurfacelessQtEGL : public GLSurfaceQt {
public:
    explicit GLSurfacelessQtEGL(const gfx::Size& size);

public:
    bool Initialize(GLSurfaceFormat format) override;
    void Destroy() override;
    bool IsSurfaceless() const override;
    bool Resize(const gfx::Size& size, float scale_factor,
                ColorSpace color_space, bool has_alpha) override;
    EGLSurface GetHandle() override;
    void* GetShareHandle() override;

private:
    DISALLOW_COPY_AND_ASSIGN(GLSurfacelessQtEGL);
};
}

#endif
