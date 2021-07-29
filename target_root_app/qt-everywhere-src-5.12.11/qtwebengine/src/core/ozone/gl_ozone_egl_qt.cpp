/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#if defined(USE_OZONE)
#include "gl_ozone_egl_qt.h"
#include "gl_context_qt.h"
#include "gl_surface_egl_qt.h"
#include "base/files/file_path.h"
#include "base/native_library.h"
#include "gl_context_qt.h"
#include "gl_ozone_egl_qt.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/gl/init/gl_initializer.h"


#include <EGL/egl.h>
#include <dlfcn.h>

#include <QtGui/qtgui-config.h> // for QT_NO_OPENGL

#ifndef QT_NO_OPENGL
#include <QOpenGLContext>
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE
#endif

namespace ui {

base::NativeLibrary LoadLibrary(const base::FilePath& filename) {
    base::NativeLibraryLoadError error;
    base::NativeLibrary library = base::LoadNativeLibrary(filename, &error);
    if (!library) {
        LOG(ERROR) << "Failed to load " << filename.MaybeAsASCII() << ": " << error.ToString();
        return NULL;
    }
    return library;
}

bool GLOzoneEGLQt::LoadGLES2Bindings(gl::GLImplementation /*implementation*/)
{
    base::NativeLibrary eglgles2Library = dlopen(NULL, RTLD_LAZY);
    if (!eglgles2Library) {
        LOG(ERROR) << "Failed to open EGL/GLES2 context " << dlerror();
        return false;
    }

    gl::GLGetProcAddressProc get_proc_address =
            reinterpret_cast<gl::GLGetProcAddressProc>(
                base::GetFunctionPointerFromNativeLibrary(eglgles2Library,
                                                          "eglGetProcAddress"));
#ifndef QT_NO_OPENGL
    if (!get_proc_address) {
        // QTBUG-63341 most likely libgles2 not linked with libegl -> fallback to qpa
        if (QOpenGLContext *context = qt_gl_global_share_context()) {
            get_proc_address = reinterpret_cast<gl::GLGetProcAddressProc>(
                context->getProcAddress("eglGetProcAddress"));
        }
    }
#endif

    if (!get_proc_address) {
        LOG(ERROR) << "eglGetProcAddress not found.";
        base::UnloadNativeLibrary(eglgles2Library);
        return false;
    }

    gl::SetGLGetProcAddressProc(get_proc_address);
    gl::AddGLNativeLibrary(eglgles2Library);
    return true;
}

bool GLOzoneEGLQt::InitializeGLOneOffPlatform()
{
    if (!gl::GLSurfaceEGLQt::InitializeOneOff()) {
        LOG(ERROR) << "GLOzoneEGLQt::InitializeOneOff failed.";
        return false;
    }
    return true;
}

bool GLOzoneEGLQt::InitializeExtensionSettingsOneOffPlatform()
{
    return gl::GLSurfaceEGLQt::InitializeExtensionSettingsOneOff();
}

scoped_refptr<gl::GLSurface> GLOzoneEGLQt::CreateViewGLSurface(gfx::AcceleratedWidget window)
{
    return nullptr;
}

scoped_refptr<gl::GLSurface> GLOzoneEGLQt::CreateOffscreenGLSurface(const gfx::Size &size)
{
    scoped_refptr<gl::GLSurface> surface = new gl::GLSurfaceEGLQt(size);
    if (surface->Initialize(gl::GLSurfaceFormat()))
        return surface;

    surface = new gl::GLSurfacelessQtEGL(size);
    if (surface->Initialize(gl::GLSurfaceFormat()))
        return surface;

    LOG(WARNING) << "Failed to create offscreen GL surface";
    return nullptr;
}

intptr_t GLOzoneEGLQt::GetNativeDisplay()
{
    static void *display = GLContextHelper::getNativeDisplay();

    if (display)
        return reinterpret_cast<intptr_t>(display);

    return reinterpret_cast<intptr_t>(EGL_DEFAULT_DISPLAY);
}

} // namespace ui

#endif // defined(USE_OZONE)
