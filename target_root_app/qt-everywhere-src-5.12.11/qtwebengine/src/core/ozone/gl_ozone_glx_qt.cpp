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

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QGuiApplication>
#include "gl_ozone_glx_qt.h"
#include "gl_surface_glx_qt.h"
#include "gl_context_qt.h"
#include "ui/gl/gl_context_glx.h"
#include "ui/gl/gl_gl_api_implementation.h"
#include "ui/gl/gl_glx_api_implementation.h"
#include <dlfcn.h>

namespace ui {

bool GLOzoneGLXQt::InitializeGLOneOffPlatform() {
    if (!gl::GLSurfaceGLXQt::InitializeOneOff()) {
        LOG(ERROR) << "GLSurfaceGLXQt::InitializeOneOff failed.";
        return false;
    }
    return true;
}

bool GLOzoneGLXQt::InitializeStaticGLBindings(
        gl::GLImplementation implementation) {

    base::NativeLibrary library = dlopen(NULL, RTLD_LAZY);
    if (!library) {
        LOG(ERROR) << "Failed to open GL context " << dlerror();
        return false;
    }

    gl::GLGetProcAddressProc get_proc_address =
            reinterpret_cast<gl::GLGetProcAddressProc>(
                base::GetFunctionPointerFromNativeLibrary(library,
                                                          "glXGetProcAddress"));
    if (!get_proc_address) {
    // glx handle not loaded, fallback to qpa
        QFunctionPointer address = GLContextHelper::getGlXGetProcAddress();
        get_proc_address = reinterpret_cast<gl::GLGetProcAddressProc>(address);
    }

    if (!get_proc_address) {
        LOG(ERROR) << "glxGetProcAddress not found.";
        base::UnloadNativeLibrary(library);
        return false;
    }

    gl::SetGLGetProcAddressProc(get_proc_address);
    gl::AddGLNativeLibrary(library);
    gl::SetGLImplementation(gl::kGLImplementationDesktopGL);

    gl::InitializeStaticGLBindingsGL();
    gl::InitializeStaticGLBindingsGLX();

    return true;
}

void GLOzoneGLXQt::InitializeDebugGLBindings() {
    gl::InitializeDebugGLBindingsGL();
    gl::InitializeDebugGLBindingsGLX();
}

void GLOzoneGLXQt::SetDisabledExtensionsPlatform(
        const std::string& disabled_extensions) {
    gl::SetDisabledExtensionsGLX(disabled_extensions);
}

void GLOzoneGLXQt::ShutdownGL() {
    gl::ClearBindingsGL();
    gl::ClearBindingsGLX();
}

bool GLOzoneGLXQt::GetGLWindowSystemBindingInfo(
        gl::GLWindowSystemBindingInfo* info) {
    return gl::GetGLWindowSystemBindingInfoGLX(info);
}

scoped_refptr<gl::GLContext> GLOzoneGLXQt::CreateGLContext(
        gl::GLShareGroup* share_group,
        gl::GLSurface* compatible_surface,
        const gl::GLContextAttribs& attribs) {
    return gl::InitializeGLContext(new gl::GLContextGLX(share_group),
                                   compatible_surface, attribs);
}

scoped_refptr<gl::GLSurface> GLOzoneGLXQt::CreateViewGLSurface(
        gfx::AcceleratedWidget window) {
    return nullptr;
}

scoped_refptr<gl::GLSurface> GLOzoneGLXQt::CreateSurfacelessViewGLSurface(
        gfx::AcceleratedWidget window) {
    return nullptr;
}

scoped_refptr<gl::GLSurface> GLOzoneGLXQt::CreateOffscreenGLSurface(
        const gfx::Size& size) {
    scoped_refptr<gl::GLSurface> surface = new gl::GLSurfaceGLXQt(size);
    if (surface->Initialize(gl::GLSurfaceFormat()))
        return surface;
    LOG(WARNING) << "Failed to create offscreen GL surface";
    return nullptr;
}

bool GLOzoneGLXQt::InitializeExtensionSettingsOneOffPlatform()
{
    return gl::GLSurfaceGLXQt::InitializeExtensionSettingsOneOff();
}

}  // namespace ui
