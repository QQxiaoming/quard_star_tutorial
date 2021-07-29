/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "graphicshelperes3_2_p.h"
#include <QOpenGLExtraFunctions>
#include <Qt3DRender/qrendertargetoutput.h>
#include <private/attachmentpack_p.h>

QT_BEGIN_NAMESPACE

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

#ifndef GL_PATCH_VERTICES
#define GL_PATCH_VERTICES 36466
#endif

namespace Qt3DRender {
namespace Render {

GraphicsHelperES3_2::GraphicsHelperES3_2()
{
}

GraphicsHelperES3_2::~GraphicsHelperES3_2()
{
}

bool GraphicsHelperES3_2::supportsFeature(GraphicsHelperInterface::Feature feature) const
{
    switch (feature) {
    case GraphicsHelperInterface::Tessellation:
        return true;
    default:
        break;
    }
    return GraphicsHelperES3_1::supportsFeature(feature);
}

bool GraphicsHelperES3_2::frameBufferNeedsRenderBuffer(const Attachment &attachment)
{
    Q_UNUSED(attachment);
    // This is first ES version where we have glFramebufferTexture, so
    // attaching a D24S8 texture to the combined depth-stencil attachment point
    // should work.
    return false;
}

void GraphicsHelperES3_2::bindFrameBufferAttachment(QOpenGLTexture *texture, const Attachment &attachment)
{
    GLenum attr = GL_COLOR_ATTACHMENT0;

    if (attachment.m_point <= QRenderTargetOutput::Color15)
        attr = GL_COLOR_ATTACHMENT0 + attachment.m_point;
    else if (attachment.m_point == QRenderTargetOutput::Depth)
        attr = GL_DEPTH_ATTACHMENT;
    else if (attachment.m_point == QRenderTargetOutput::Stencil)
        attr = GL_STENCIL_ATTACHMENT;
    else if (attachment.m_point == QRenderTargetOutput::DepthStencil)
        attr = GL_DEPTH_STENCIL_ATTACHMENT;
    else
        qCritical() << "Unsupported FBO attachment OpenGL ES 3.2";

    const QOpenGLTexture::Target target = texture->target();

    texture->bind();
    if (target == QOpenGLTexture::TargetCubeMap && attachment.m_face != QAbstractTexture::AllFaces)
        m_funcs->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attr, attachment.m_face, texture->textureId(), attachment.m_mipLevel);
    else
        m_extraFuncs->glFramebufferTexture(GL_DRAW_FRAMEBUFFER, attr, texture->textureId(), attachment.m_mipLevel);
    texture->release();
}

void GraphicsHelperES3_2::setVerticesPerPatch(GLint verticesPerPatch)
{
    m_extraFuncs->glPatchParameteri(GL_PATCH_VERTICES, verticesPerPatch);
}

void GraphicsHelperES3_2::drawElementsInstancedBaseVertexBaseInstance(GLenum primitiveType, GLsizei primitiveCount, GLint indexType, void *indices, GLsizei instances, GLint baseVertex, GLint baseInstance)
{
    if (baseInstance != 0)
        qWarning() << "glDrawElementsInstancedBaseVertexBaseInstance is not supported with OpenGL ES 2";

    m_extraFuncs->glDrawElementsInstancedBaseVertex(primitiveType,
                                                    primitiveCount,
                                                    indexType,
                                                    indices,
                                                    instances,
                                                    baseVertex);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
