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

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl_context_qt.h"
#include "ozone/gl_surface_egl_qt.h"

#include "ui/gl/gl_surface_egl.h"
#if !defined(OS_MACOSX)
#include "ui/gl/egl_util.h"
#include "ui/gl/init/gl_factory.h"

// From ANGLE's egl/eglext.h.
#ifndef EGL_ANGLE_surface_d3d_texture_2d_share_handle
#define EGL_ANGLE_surface_d3d_texture_2d_share_handle 1
#define EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE 0x3200
#endif

using ui::GetLastEGLErrorString;

namespace gl{

bool GLSurfaceEGLQt::g_egl_surfaceless_context_supported = false;
bool GLSurfaceEGLQt::s_initialized = false;

GLSurfaceEGLQt::~GLSurfaceEGLQt()
{
    Destroy();
}

bool GLSurfaceEGLQt::InitializeOneOff()
{
    if (s_initialized)
        return true;

    g_display = GLContextHelper::getEGLDisplay();
    if (!g_display) {
        LOG(ERROR) << "GLContextHelper::getEGLDisplay() failed.";
        return false;
    }

    g_config = GLContextHelper::getEGLConfig();
    if (!g_config) {
        LOG(ERROR) << "GLContextHelper::getEGLConfig() failed.";
        return false;
    }

    if (!eglInitialize(g_display, NULL, NULL)) {
        LOG(ERROR) << "eglInitialize failed with error " << GetLastEGLErrorString();
        return false;
    }

    g_extensions = eglQueryString(g_display, EGL_EXTENSIONS);
    g_egl_surfaceless_context_supported = ExtensionsContain(g_extensions, "EGL_KHR_surfaceless_context");
    if (g_egl_surfaceless_context_supported) {
        scoped_refptr<GLSurface> surface = new GLSurfacelessQtEGL(gfx::Size(1, 1));
        gl::GLContextAttribs attribs;
        scoped_refptr<GLContext> context = init::CreateGLContext(
            NULL, surface.get(), attribs);

        if (!context->MakeCurrent(surface.get()))
            g_egl_surfaceless_context_supported = false;

        // Ensure context supports GL_OES_surfaceless_context.
        if (g_egl_surfaceless_context_supported) {
            g_egl_surfaceless_context_supported = context->HasExtension(
                "GL_OES_surfaceless_context");
            context->ReleaseCurrent(surface.get());
        }
    }
    s_initialized = true;
    return true;
}

bool GLSurfaceEGLQt::InitializeExtensionSettingsOneOff()
{
    return s_initialized;
}

bool GLSurfaceEGL::InitializeExtensionSettingsOneOff()
{
    return GLSurfaceEGLQt::InitializeExtensionSettingsOneOff();
}

EGLDisplay GLSurfaceEGL::GetHardwareDisplay()
{
    return static_cast<EGLDisplay>(GLSurfaceQt::g_display);
}

bool GLSurfaceEGL::IsCreateContextRobustnessSupported()
{
    return false;
}

bool GLSurfaceEGL::IsCreateContextBindGeneratesResourceSupported()
{
    return false;
}

bool GLSurfaceEGL::IsCreateContextWebGLCompatabilitySupported()
{
    return false;
}
bool GLSurfaceEGL::IsEGLSurfacelessContextSupported()
{
    return GLSurfaceEGLQt::g_egl_surfaceless_context_supported;
}
bool GLSurfaceEGL::IsEGLContextPrioritySupported()
{
    return false;
}

bool GLSurfaceEGL::IsRobustResourceInitSupported()
{
    return false;
}

bool GLSurfaceEGL::IsDisplayTextureShareGroupSupported()
{
    return false;
}

bool GLSurfaceEGL::IsCreateContextClientArraysSupported()
{
    return false;
}

void GLSurfaceEGL::ShutdownOneOff()
{
}

const char* GLSurfaceEGL::GetEGLExtensions()
{
    return GLSurfaceQt::g_extensions;
}

bool GLSurfaceEGL::HasEGLExtension(const char* name)
{
    return ExtensionsContain(GetEGLExtensions(), name);
}

bool GLSurfaceEGL::InitializeOneOff(EGLNativeDisplayType /*native_display*/)
{
    return GLSurfaceEGLQt::InitializeOneOff();
}

bool GLSurfaceEGL::IsAndroidNativeFenceSyncSupported()
{
     return false;
}

GLSurfaceEGLQt::GLSurfaceEGLQt(const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(0)
{
}

bool GLSurfaceEGLQt::Initialize(GLSurfaceFormat format)
{
    Q_ASSERT(!m_surfaceBuffer);
    m_format = format;

    EGLDisplay display = g_display;
    if (!display) {
        LOG(ERROR) << "Trying to create surface with invalid display.";
        return false;
    }

    const EGLint pbuffer_attributes[] = {
        EGL_WIDTH, m_size.width(),
        EGL_HEIGHT, m_size.height(),
        EGL_LARGEST_PBUFFER, EGL_FALSE,
        EGL_NONE
    };

    m_surfaceBuffer = eglCreatePbufferSurface(display,
                                        g_config,
                                        pbuffer_attributes);
    if (!m_surfaceBuffer) {
        VLOG(1) << "eglCreatePbufferSurface failed with error " << GetLastEGLErrorString();
        Destroy();
        return false;
    }

    return true;
}

void GLSurfaceEGLQt::Destroy()
{
    if (m_surfaceBuffer) {
        if (!eglDestroySurface(g_display, m_surfaceBuffer))
            LOG(ERROR) << "eglDestroySurface failed with error " << GetLastEGLErrorString();

        m_surfaceBuffer = 0;
    }
}

bool GLSurfaceEGLQt::Resize(const gfx::Size& size, float scale_factor,
                             ColorSpace color_space, bool has_alpha)
{
    if (size == m_size)
        return true;

    GLContext *currentContext = GLContext::GetCurrent();
    bool wasCurrent = currentContext && currentContext->IsCurrent(this);
    if (wasCurrent)
        currentContext->ReleaseCurrent(this);

    Destroy();

    m_size = size;

    if (!Initialize(GetFormat())) {
        LOG(ERROR) << "Failed to resize pbuffer.";
        return false;
    }

    if (wasCurrent)
        return currentContext->MakeCurrent(this);

    return true;
}

void* GLSurfaceEGLQt::GetHandle()
{
    return reinterpret_cast<void*>(m_surfaceBuffer);
}

GLSurfacelessQtEGL::GLSurfacelessQtEGL(const gfx::Size& size)
    : GLSurfaceQt(size)
{
}

bool GLSurfacelessQtEGL::Initialize(GLSurfaceFormat format)
{
    m_format = format;
    return true;
}

void GLSurfacelessQtEGL::Destroy()
{
}

bool GLSurfacelessQtEGL::IsSurfaceless() const
{
    return true;
}

bool GLSurfacelessQtEGL::Resize(const gfx::Size& size, float scale_factor,
                                ColorSpace color_space,  bool has_alpha)
{
    m_size = size;
    return true;
}

EGLSurface GLSurfacelessQtEGL::GetHandle()
{
    return EGL_NO_SURFACE;
}

void* GLSurfacelessQtEGL::GetShareHandle()
{
    return NULL;
}

}  // namespace gl
#endif // !defined(OS_MACOSX)

namespace gl {
std::string DriverEGL::GetPlatformExtensions()
{
    EGLDisplay display = GLContextHelper::getEGLDisplay();
    if (display == EGL_NO_DISPLAY)
        return "";

    DCHECK(g_driver_egl.fn.eglQueryStringFn);
    const char* str = g_driver_egl.fn.eglQueryStringFn(display, EGL_EXTENSIONS);
    return str ? std::string(str) : "";
}
} // namespace gl
