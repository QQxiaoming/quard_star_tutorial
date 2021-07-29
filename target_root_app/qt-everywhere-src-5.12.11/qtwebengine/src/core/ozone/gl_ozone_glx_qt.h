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

#ifndef UI_OZONE_GLX_QT_H_
#define UI_OZONE_GLX_QT_H

#include "base/macros.h"
#include "ui/gl/gl_implementation.h"
#include "ui/ozone/public/gl_ozone.h"

namespace ui {

class GLOzoneGLXQt : public GLOzone {

public:
    GLOzoneGLXQt() {}
    ~GLOzoneGLXQt() override {}

    bool InitializeGLOneOffPlatform() override;
    bool InitializeStaticGLBindings(gl::GLImplementation implementation) override;
    void InitializeDebugGLBindings() override;
    bool InitializeExtensionSettingsOneOffPlatform() override;
    void ShutdownGL() override;
    void SetDisabledExtensionsPlatform(
        const std::string& disabled_extensions) override;
    bool GetGLWindowSystemBindingInfo(
            gl::GLWindowSystemBindingInfo* info) override;

    scoped_refptr<gl::GLContext> CreateGLContext(
            gl::GLShareGroup* share_group,
            gl::GLSurface* compatible_surface,
            const gl::GLContextAttribs& attribs) override;

    scoped_refptr<gl::GLSurface> CreateViewGLSurface(
            gfx::AcceleratedWidget window) override;

    scoped_refptr<gl::GLSurface> CreateSurfacelessViewGLSurface(
            gfx::AcceleratedWidget window) override;

    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(
            const gfx::Size& size) override;

private:
    DISALLOW_COPY_AND_ASSIGN(GLOzoneGLXQt);
};

}  // namespace ui

#endif  // UI_OZONE_GLX_QT_H
