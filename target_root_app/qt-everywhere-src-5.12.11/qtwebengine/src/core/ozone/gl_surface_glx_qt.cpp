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
#include "ozone/gl_surface_glx_qt.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_surface_glx.h"
#include <GL/glx.h>
#include <GL/glxext.h>

namespace gl {

bool GLSurfaceGLXQt::s_initialized = false;

GLSurfaceGLXQt::~GLSurfaceGLXQt()
{
    Destroy();
}

void GLSurfaceGLX::ShutdownOneOff()
{
}

bool GLSurfaceGLX::IsCreateContextSupported()
{
    return ExtensionsContain(GLSurfaceQt::g_extensions, "GLX_ARB_create_context");
}

bool GLSurfaceGLX::IsCreateContextRobustnessSupported()
{
    return false; // ExtensionsContain(g_extensions, "GLX_ARB_create_context_robustness");
}

bool GLSurfaceGLX::IsEXTSwapControlSupported()
{
    return HasGLXExtension("GLX_EXT_swap_control");
}

bool GLSurfaceGLX::IsMESASwapControlSupported()
{
    return HasGLXExtension("GLX_MESA_swap_control");
}

bool GLSurfaceGLX::IsCreateContextProfileSupported()
{
    return false; // ExtensionsContain(g_extensions, "GLX_ARB_create_context_profile");
}

bool GLSurfaceGLX::IsCreateContextES2ProfileSupported()
{
    return ExtensionsContain(GLSurfaceQt::g_extensions, "GLX_ARB_create_context_es2_profile");
}

bool GLSurfaceGLX::IsOMLSyncControlSupported()
{
    return false; // ExtensionsContain(g_extensions, "GLX_OML_sync_control");
}

bool GLSurfaceGLX::HasGLXExtension(const char *name)
{
    return ExtensionsContain(GLSurfaceQt::g_extensions, name);
}

bool GLSurfaceGLX::IsTextureFromPixmapSupported()
{
    return ExtensionsContain(GLSurfaceQt::g_extensions, "GLX_EXT_texture_from_pixmap");
}

const char* GLSurfaceGLX::GetGLXExtensions()
{
    return GLSurfaceQt::g_extensions;
}

bool GLSurfaceGLXQt::InitializeOneOff()
{
    if (s_initialized)
        return true;

    XInitThreads();

    g_display = GLContextHelper::getXDisplay();
    if (!g_display) {
        LOG(ERROR) << "GLContextHelper::getXDisplay() failed.";
        return false;
    }

    g_config = GLContextHelper::getGlXConfig();
    if (!g_config) {
        LOG(ERROR) << "GLContextHelper::getGlxConfig() failed.";
        return false;
    }

    Display* display = static_cast<Display*>(g_display);
    int major, minor;
    if (!glXQueryVersion(display, &major, &minor)) {
        LOG(ERROR) << "glxQueryVersion failed.";
        return false;
    }

    if (major == 1 && minor < 3) {
        LOG(ERROR) << "GLX 1.3 or later is required.";
        return false;
    }

    s_initialized = true;
    return true;
}


bool GLSurfaceGLXQt::InitializeExtensionSettingsOneOff()
{
    if (!s_initialized)
        return false;

    Display* display = static_cast<Display*>(g_display);
    GLSurfaceQt::g_extensions = glXQueryExtensionsString(display, 0);
    g_driver_glx.InitializeExtensionBindings(g_extensions);
    return true;
}

bool GLSurfaceGLX::InitializeExtensionSettingsOneOff()
{
    return GLSurfaceGLXQt::InitializeExtensionSettingsOneOff();
}

bool GLSurfaceGLXQt::Initialize(GLSurfaceFormat format)
{
    Q_ASSERT(!m_surfaceBuffer);

    Display* display = static_cast<Display*>(g_display);
    const int pbuffer_attributes[] = {
        GLX_PBUFFER_WIDTH, m_size.width(),
        GLX_PBUFFER_HEIGHT, m_size.height(),
        GLX_LARGEST_PBUFFER, x11::False,
        GLX_PRESERVED_CONTENTS, x11::False,
        x11::None // MEMO doc: ...must be terminated with None or NULL
    };

    m_surfaceBuffer = glXCreatePbuffer(display, static_cast<GLXFBConfig>(g_config), pbuffer_attributes);
    m_format = format;

    if (!m_surfaceBuffer) {
        Destroy();
        LOG(ERROR) << "glXCreatePbuffer failed.";
        return false;
    }
    return true;
}

void GLSurfaceGLXQt::Destroy()
{
    if (m_surfaceBuffer) {
        glXDestroyPbuffer(static_cast<Display*>(g_display), m_surfaceBuffer);
        m_surfaceBuffer = 0;
    }
}

GLSurfaceGLXQt::GLSurfaceGLXQt(const gfx::Size& size)
    : GLSurfaceQt(size),
      m_surfaceBuffer(0)
{
}

void* GLSurfaceGLXQt::GetHandle()
{
    return reinterpret_cast<void*>(m_surfaceBuffer);
}

} //namespace gl

