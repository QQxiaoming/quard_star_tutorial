/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef GL_OZONE_EGL_QT
#define GL_OZONE_EGL_QT

#if defined(USE_OZONE)

#include "ui/ozone/common/gl_ozone_egl.h"

namespace ui {

class GLOzoneEGLQt : public GLOzoneEGL {
public:
    bool InitializeGLOneOffPlatform() override;
    bool InitializeExtensionSettingsOneOffPlatform() override;
    scoped_refptr<gl::GLSurface> CreateViewGLSurface(
            gfx::AcceleratedWidget window) override;
    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(
            const gfx::Size& size) override;

protected:
    // Returns native platform display handle. This is used to obtain the EGL
    // display connection for the native display.
    intptr_t GetNativeDisplay() override;

    // Sets up GL bindings for the native surface.
    bool LoadGLES2Bindings(gl::GLImplementation implementation) override;
};

} // namespace ui

#endif // defined(USE_OZONE)

#endif // GL_OZONE_EGL_QT
