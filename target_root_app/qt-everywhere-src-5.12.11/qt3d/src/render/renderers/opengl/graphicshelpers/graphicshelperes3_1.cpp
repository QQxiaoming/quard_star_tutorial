/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "graphicshelperes3_1_p.h"
#include <private/qgraphicsutils_p.h>
#include <QOpenGLExtraFunctions>

QT_BEGIN_NAMESPACE

// ES 3.1+
#ifndef GL_SAMPLER_2D_MULTISAMPLE
#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
#endif
#ifndef GL_INT_SAMPLER_2D_MULTISAMPLE
#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#endif
#ifndef GL_ACTIVE_RESOURCES
#define GL_ACTIVE_RESOURCES 0x92F5
#endif
#ifndef GL_BUFFER_BINDING
#define GL_BUFFER_BINDING 0x9302
#endif
#ifndef GL_BUFFER_DATA_SIZE
#define GL_BUFFER_DATA_SIZE 0x9303
#endif
#ifndef GL_NUM_ACTIVE_VARIABLES
#define GL_NUM_ACTIVE_VARIABLES 0x9304
#endif
#ifndef GL_SHADER_STORAGE_BLOCK
#define GL_SHADER_STORAGE_BLOCK 0x92E6
#endif
#ifndef GL_ALL_BARRIER_BITS
#define GL_ALL_BARRIER_BITS 0xFFFFFFFF
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#endif
#ifndef GL_ELEMENT_ARRAY_BARRIER_BIT
#define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#endif
#ifndef GL_UNIFORM_BARRIER_BIT
#define GL_UNIFORM_BARRIER_BIT 0x00000004
#endif
#ifndef GL_TEXTURE_FETCH_BARRIER_BIT
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#endif
#ifndef GL_COMMAND_BARRIER_BIT
#define GL_COMMAND_BARRIER_BIT 0x00000040
#endif
#ifndef GL_PIXEL_BUFFER_BARRIER_BIT
#define GL_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#endif
#ifndef GL_TEXTURE_UPDATE_BARRIER_BIT
#define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#endif
#ifndef GL_BUFFER_UPDATE_BARRIER_BIT
#define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#endif
#ifndef GL_FRAMEBUFFER_BARRIER_BIT
#define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BARRIER_BIT
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#endif
#ifndef GL_ATOMIC_COUNTER_BARRIER_BIT
#define GL_ATOMIC_COUNTER_BARRIER_BIT 0x00001000
#endif
#ifndef GL_SHADER_STORAGE_BARRIER_BIT
#define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#endif


namespace Qt3DRender {
namespace Render {

namespace {

GLbitfield memoryBarrierGLBitfield(QMemoryBarrier::Operations barriers)
{
    GLbitfield bits = 0;

    if (barriers.testFlag(QMemoryBarrier::All)) {
        bits |= GL_ALL_BARRIER_BITS;
        return bits;
    }

    if (barriers.testFlag(QMemoryBarrier::VertexAttributeArray))
        bits |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::ElementArray))
        bits |= GL_ELEMENT_ARRAY_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::Uniform))
        bits |= GL_UNIFORM_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::TextureFetch))
        bits |= GL_TEXTURE_FETCH_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::ShaderImageAccess))
        bits |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::Command))
        bits |= GL_COMMAND_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::PixelBuffer))
        bits |= GL_PIXEL_BUFFER_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::TextureUpdate))
        bits |= GL_TEXTURE_UPDATE_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::BufferUpdate))
        bits |= GL_BUFFER_UPDATE_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::FrameBuffer))
        bits |= GL_FRAMEBUFFER_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::TransformFeedback))
        bits |= GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::AtomicCounter))
        bits |= GL_ATOMIC_COUNTER_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::ShaderStorage))
        bits |= GL_SHADER_STORAGE_BARRIER_BIT;
    if (barriers.testFlag(QMemoryBarrier::QueryBuffer))
        qWarning() << "QueryBuffer barrier not supported by ES 3.1";

    return bits;
}

} // anonymous


GraphicsHelperES3_1::GraphicsHelperES3_1()
{
}

GraphicsHelperES3_1::~GraphicsHelperES3_1()
{
}

bool GraphicsHelperES3_1::supportsFeature(GraphicsHelperInterface::Feature feature) const
{
    switch (feature) {
    case GraphicsHelperInterface::Compute:
    case GraphicsHelperInterface::ShaderStorageObject:
    case GraphicsHelperInterface::IndirectDrawing:
        return true;
    default:
        break;
    }
    return GraphicsHelperES3::supportsFeature(feature);
}

void GraphicsHelperES3_1::dispatchCompute(GLuint wx, GLuint wy, GLuint wz)
{
    m_extraFuncs->glDispatchCompute(wx, wy, wz);
}

void GraphicsHelperES3_1::memoryBarrier(QMemoryBarrier::Operations barriers)
{
    m_extraFuncs->glMemoryBarrier(memoryBarrierGLBitfield(barriers));
}

void GraphicsHelperES3_1::drawArraysIndirect(GLenum mode, void *indirect)
{
    m_extraFuncs->glDrawArraysIndirect(mode, indirect);
}

void GraphicsHelperES3_1::drawElementsIndirect(GLenum mode, GLenum type, void *indirect)
{
    m_extraFuncs->glDrawElementsIndirect(mode, type, indirect);
}

void GraphicsHelperES3_1::bindShaderStorageBlock(GLuint , GLuint , GLuint )
{
    // ES 3.1 has no API for that, bindings have to be specified directly in the shader
    // with layout(std430, binding = 3)
    qWarning() << "ES 3.1 has no bindShaderStorageBlock API, it uses binding declaration from the shader storage block";
}

QVector<ShaderStorageBlock> GraphicsHelperES3_1::programShaderStorageBlocks(GLuint programId)
{
    QVector<ShaderStorageBlock> blocks;
    GLint nbrActiveShaderStorageBlocks = 0;
    m_extraFuncs->glGetProgramInterfaceiv(programId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &nbrActiveShaderStorageBlocks);
    blocks.reserve(nbrActiveShaderStorageBlocks);
    for (GLint i = 0; i < nbrActiveShaderStorageBlocks; ++i) {
        QByteArray storageBlockName(256, '\0');
        GLsizei length = 0;
        ShaderStorageBlock storageBlock;
        m_extraFuncs->glGetProgramResourceName(programId, GL_SHADER_STORAGE_BLOCK, i, 256, &length, storageBlockName.data());
        storageBlock.m_index = i;
        storageBlock.m_name = QString::fromUtf8(storageBlockName.left(length));
        GLenum prop = GL_BUFFER_BINDING;
        m_extraFuncs->glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, i, 1, &prop, 4, NULL, &storageBlock.m_binding);
        prop = GL_BUFFER_DATA_SIZE;
        m_extraFuncs->glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, i, 1, &prop, 4, NULL, &storageBlock.m_size);
        prop = GL_NUM_ACTIVE_VARIABLES;
        m_extraFuncs->glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, i, 1, &prop, 4, NULL, &storageBlock.m_activeVariablesCount);
        blocks.push_back(storageBlock);
    }
    return blocks;
}

UniformType GraphicsHelperES3_1::uniformTypeFromGLType(GLenum glType)
{
    switch (glType) {
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        return UniformType::Sampler;

    default:
       return GraphicsHelperES3::uniformTypeFromGLType(glType);
    }
}

uint GraphicsHelperES3_1::uniformByteSize(const ShaderUniform &description)
{
    uint rawByteSize = 0;

    switch (description.m_type) {
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        rawByteSize = 4;
        break;

    default:
        rawByteSize = GraphicsHelperES3::uniformByteSize(description);
        break;
    }

    return rawByteSize;
}

void GraphicsHelperES3_1::buildUniformBuffer(const QVariant &v, const ShaderUniform &description, QByteArray &buffer)
{
    char *bufferData = buffer.data();

    switch (description.m_type) {
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    {
        Q_ASSERT(description.m_size == 1);
        int value = v.toInt();
        QGraphicsUtils::fillDataArray<GLint>(bufferData, &value, description, 1);
        break;
    }

    default:
        GraphicsHelperES3::buildUniformBuffer(v, description, buffer);
        break;
    }
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
