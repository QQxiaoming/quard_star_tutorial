/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: https://www.qt.io/licensing/
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

#include "graphicscontext_p.h"

#include <Qt3DRender/qgraphicsapifilter.h>
#include <Qt3DRender/qparameter.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/shader_p.h>
#include <Qt3DRender/private/material_p.h>
#include <Qt3DRender/private/gltexture_p.h>
#include <Qt3DRender/private/buffer_p.h>
#include <Qt3DRender/private/attribute_p.h>
#include <Qt3DRender/private/rendercommand_p.h>
#include <Qt3DRender/private/renderstateset_p.h>
#include <Qt3DRender/private/rendertarget_p.h>
#include <Qt3DRender/private/graphicshelperinterface_p.h>
#include <Qt3DRender/private/renderer_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/buffermanager_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/gltexturemanager_p.h>
#include <Qt3DRender/private/attachmentpack_p.h>
#include <Qt3DRender/private/qbuffer_p.h>
#include <Qt3DRender/private/renderbuffer_p.h>
#include <QOpenGLShaderProgram>

#if !defined(QT_OPENGL_ES_2)
#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_4_3_Core>
#include <Qt3DRender/private/graphicshelpergl2_p.h>
#include <Qt3DRender/private/graphicshelpergl3_2_p.h>
#include <Qt3DRender/private/graphicshelpergl3_3_p.h>
#include <Qt3DRender/private/graphicshelpergl4_p.h>
#endif
#include <Qt3DRender/private/graphicshelperes2_p.h>
#include <Qt3DRender/private/graphicshelperes3_p.h>
#include <Qt3DRender/private/graphicshelperes3_1_p.h>
#include <Qt3DRender/private/graphicshelperes3_2_p.h>

#include <QSurface>
#include <QWindow>
#include <QOpenGLTexture>
#include <QOpenGLDebugLogger>

QT_BEGIN_NAMESPACE

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

namespace {

QOpenGLShader::ShaderType shaderType(Qt3DRender::QShaderProgram::ShaderType type)
{
    switch (type) {
    case Qt3DRender::QShaderProgram::Vertex: return QOpenGLShader::Vertex;
    case Qt3DRender::QShaderProgram::TessellationControl: return QOpenGLShader::TessellationControl;
    case Qt3DRender::QShaderProgram::TessellationEvaluation: return QOpenGLShader::TessellationEvaluation;
    case Qt3DRender::QShaderProgram::Geometry: return QOpenGLShader::Geometry;
    case Qt3DRender::QShaderProgram::Fragment: return QOpenGLShader::Fragment;
    case Qt3DRender::QShaderProgram::Compute: return QOpenGLShader::Compute;
    default: Q_UNREACHABLE();
    }
}

} // anonymous namespace

namespace Qt3DRender {
namespace Render {

namespace {

void logOpenGLDebugMessage(const QOpenGLDebugMessage &debugMessage)
{
    qDebug() << "OpenGL debug message:" << debugMessage;
}

} // anonymous

GraphicsContext::GraphicsContext()
    : m_initialized(false)
    , m_supportsVAO(false)
    , m_maxTextureUnits(0)
    , m_defaultFBO(0)
    , m_gl(nullptr)
    , m_glHelper(nullptr)
    , m_shaderCache(nullptr)
    , m_debugLogger(nullptr)
    , m_currentVAO(nullptr)
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::setOpenGLContext(QOpenGLContext* ctx)
{
    Q_ASSERT(ctx);
    m_gl = ctx;
}

void GraphicsContext::initialize()
{
    m_initialized = true;

    Q_ASSERT(m_gl && m_shaderCache);

    m_gl->functions()->glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits);
    qCDebug(Backend) << "context supports" << m_maxTextureUnits << "texture units";

    if (m_gl->format().majorVersion() >= 3) {
        m_supportsVAO = true;
    } else {
        QSet<QByteArray> extensions = m_gl->extensions();
        m_supportsVAO = extensions.contains(QByteArrayLiteral("GL_OES_vertex_array_object"))
                || extensions.contains(QByteArrayLiteral("GL_ARB_vertex_array_object"))
                || extensions.contains(QByteArrayLiteral("GL_APPLE_vertex_array_object"));
    }

    m_defaultFBO = m_gl->defaultFramebufferObject();
    qCDebug(Backend) << "VAO support = " << m_supportsVAO;
}

void GraphicsContext::clearBackBuffer(QClearBuffers::BufferTypeFlags buffers)
{
    if (buffers != QClearBuffers::None) {
        GLbitfield mask = 0;

        if (buffers & QClearBuffers::ColorBuffer)
            mask |= GL_COLOR_BUFFER_BIT;
        if (buffers & QClearBuffers::DepthBuffer)
            mask |= GL_DEPTH_BUFFER_BIT;
        if (buffers & QClearBuffers::StencilBuffer)
            mask |= GL_STENCIL_BUFFER_BIT;

        m_gl->functions()->glClear(mask);
    }
}

bool GraphicsContext::hasValidGLHelper() const
{
    return m_glHelper != nullptr;
}

bool GraphicsContext::isInitialized() const
{
    return m_initialized;
}

bool GraphicsContext::makeCurrent(QSurface *surface)
{
    Q_ASSERT(m_gl);
    if (!m_gl->makeCurrent(surface)) {
        qCWarning(Backend) << Q_FUNC_INFO << "makeCurrent failed";
        return false;
    }

    initializeHelpers(surface);

    return true;
}

void GraphicsContext::initializeHelpers(QSurface *surface)
{
    // Set the correct GL Helper depending on the surface
    // If no helper exists, create one
    m_glHelper = m_glHelpers.value(surface);
    if (!m_glHelper) {
        m_glHelper = resolveHighestOpenGLFunctions();
        m_glHelpers.insert(surface, m_glHelper);
    }
}

void GraphicsContext::doneCurrent()
{
    Q_ASSERT(m_gl);
    m_gl->doneCurrent();
    m_glHelper = nullptr;
}

// Called by GL Command Thread
QOpenGLShaderProgram *GraphicsContext::createShaderProgram(Shader *shaderNode)
{
    QScopedPointer<QOpenGLShaderProgram> shaderProgram(new QOpenGLShaderProgram);

    // Compile shaders
    const auto shaderCode = shaderNode->shaderCode();
    QString logs;
    for (int i = QShaderProgram::Vertex; i <= QShaderProgram::Compute; ++i) {
        const QShaderProgram::ShaderType type = static_cast<QShaderProgram::ShaderType>(i);
        if (!shaderCode.at(i).isEmpty()) {
            // Note: logs only return the error but not all the shader code
            // we could append it
            if (!shaderProgram->addCacheableShaderFromSourceCode(shaderType(type), shaderCode.at(i)))
                logs += shaderProgram->log();
        }
    }

    // Call glBindFragDataLocation and link the program
    // Since we are sharing shaders in the backend, we assume that if using custom
    // fragOutputs, they should all be the same for a given shader
    bindFragOutputs(shaderProgram->programId(), shaderNode->fragOutputs());

    const bool linkSucceeded = shaderProgram->link();
    logs += shaderProgram->log();
    shaderNode->setLog(logs);
    shaderNode->setStatus(linkSucceeded ? QShaderProgram::Ready : QShaderProgram::Error);

    if (!linkSucceeded)
        return nullptr;

    // take from scoped-pointer so it doesn't get deleted
    return shaderProgram.take();
}

// Called by GL Command Thread (can't use global glHelpers)
// That assumes that the shaderProgram in Shader stays the same
void GraphicsContext::introspectShaderInterface(Shader *shader, QOpenGLShaderProgram *shaderProgram)
{
    GraphicsHelperInterface *glHelper = resolveHighestOpenGLFunctions();
    shader->initializeUniforms(glHelper->programUniformsAndLocations(shaderProgram->programId()));
    shader->initializeAttributes(glHelper->programAttributesAndLocations(shaderProgram->programId()));
    if (m_glHelper->supportsFeature(GraphicsHelperInterface::UniformBufferObject))
        shader->initializeUniformBlocks(glHelper->programUniformBlocks(shaderProgram->programId()));
    if (m_glHelper->supportsFeature(GraphicsHelperInterface::ShaderStorageObject))
        shader->initializeShaderStorageBlocks(glHelper->programShaderStorageBlocks(shaderProgram->programId()));
}


// Called by GL Command Thread
void GraphicsContext::loadShader(Shader *shader, ShaderManager *manager)
{
    bool wasPresent = false;
    QOpenGLShaderProgram *shaderProgram = m_shaderCache->getShaderProgramAndAddRef(shader->dna(), shader->peerId(), &wasPresent);
    if (!shaderProgram && !wasPresent) {
        // No matching QOpenGLShader in the cache so create one
        shaderProgram = createShaderProgram(shader);

        // Store in cache (even when failed and shaderProgram is null)
        m_shaderCache->insert(shader->dna(), shader->peerId(), shaderProgram);
    }

    // Ensure the Shader node knows about the program interface
    if (Q_LIKELY(shaderProgram != nullptr) && !shader->isLoaded()) {

        // Find an already loaded shader that shares the same QOpenGLShaderProgram
        Shader *refShader = nullptr;
        const QVector<Qt3DCore::QNodeId> sharedShaderIds = m_shaderCache->shaderIdsForProgram(shader->dna());
        for (const Qt3DCore::QNodeId sharedShaderId : sharedShaderIds) {
            Shader *sharedShader = manager->lookupResource(sharedShaderId);
            // Note: no need to check if shader->peerId != sharedShader->peerId
            // as we are sure that this code path is executed only if !shared->isLoaded
            if (sharedShader->isLoaded()) {
                refShader = sharedShader;
                break;
            }
        }

        // We only introspect once per actual OpenGL shader program
        // rather than once per ShaderNode.
        if (refShader != nullptr)
            shader->initializeFromReference(*refShader);
        else // Introspect and set up interface description on Shader backend node
            introspectShaderInterface(shader, shaderProgram);

        shader->setGraphicsContext(this);
        shader->setLoaded(true);
        shader->markDirty(AbstractRenderer::AllDirty);
    }
}

void GraphicsContext::removeShaderProgramReference(Shader *shaderNode)
{
    m_shaderCache->removeRef(shaderNode->dna(), shaderNode->peerId());
}

void GraphicsContext::activateDrawBuffers(const AttachmentPack &attachments)
{
    const QVector<int> activeDrawBuffers = attachments.getGlDrawBuffers();

    if (m_glHelper->checkFrameBufferComplete()) {
        if (activeDrawBuffers.size() > 1) {// We need MRT
            if (m_glHelper->supportsFeature(GraphicsHelperInterface::MRT)) {
                // Set up MRT, glDrawBuffers...
                m_glHelper->drawBuffers(activeDrawBuffers.size(), activeDrawBuffers.data());
            }
        }
    } else {
        qWarning() << "FBO incomplete";
    }
}

/*!
 * \internal
 * Finds the highest supported opengl version and internally use the most optimized
 * helper for a given version.
 */
GraphicsHelperInterface *GraphicsContext::resolveHighestOpenGLFunctions()
{
    Q_ASSERT(m_gl);
    GraphicsHelperInterface *glHelper = nullptr;

    if (m_gl->isOpenGLES()) {
        if (m_gl->format().majorVersion() >= 3) {
            if (m_gl->format().minorVersion() >= 2) {
                glHelper = new GraphicsHelperES3_2;
                qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL ES 3.2 Helper";
            } else if (m_gl->format().minorVersion() >= 1) {
                glHelper = new GraphicsHelperES3_1;
                qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL ES 3.1 Helper";
            } else {
                glHelper = new GraphicsHelperES3();
                qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL ES 3.0 Helper";
            }
        } else {
            glHelper = new GraphicsHelperES2();
            qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL ES2 Helper";
        }
        glHelper->initializeHelper(m_gl, nullptr);
    }
#ifndef QT_OPENGL_ES_2
    else {
        QAbstractOpenGLFunctions *glFunctions = nullptr;
        if ((glFunctions = m_gl->versionFunctions<QOpenGLFunctions_4_3_Core>()) != nullptr) {
            qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL 4.3";
            glHelper = new GraphicsHelperGL4();
        } else if ((glFunctions = m_gl->versionFunctions<QOpenGLFunctions_3_3_Core>()) != nullptr) {
            qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL 3.3";
            glHelper = new GraphicsHelperGL3_3();
        } else if ((glFunctions = m_gl->versionFunctions<QOpenGLFunctions_3_2_Core>()) != nullptr) {
            qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL 3.2";
            glHelper = new GraphicsHelperGL3_2();
        } else if ((glFunctions = m_gl->versionFunctions<QOpenGLFunctions_2_0>()) != nullptr) {
            qCDebug(Backend) << Q_FUNC_INFO << " Building OpenGL 2 Helper";
            glHelper = new GraphicsHelperGL2();
        }
        Q_ASSERT_X(glHelper, "GraphicsContext::resolveHighestOpenGLFunctions", "unable to create valid helper for available OpenGL version");
        glHelper->initializeHelper(m_gl, glFunctions);
    }
#endif

    // Note: at this point we are certain the context (m_gl) is current with a surface
    const QByteArray debugLoggingMode = qgetenv("QT3DRENDER_DEBUG_LOGGING");
    const bool enableDebugLogging = !debugLoggingMode.isEmpty();

    if (enableDebugLogging && !m_debugLogger) {
        if (m_gl->hasExtension("GL_KHR_debug")) {
            qCDebug(Backend) << "Qt3D: Enabling OpenGL debug logging";
            m_debugLogger.reset(new QOpenGLDebugLogger);
            if (m_debugLogger->initialize()) {
                QObject::connect(m_debugLogger.data(), &QOpenGLDebugLogger::messageLogged, &logOpenGLDebugMessage);
                const QString mode = QString::fromLocal8Bit(debugLoggingMode);
                m_debugLogger->startLogging(mode.startsWith(QLatin1String("sync"), Qt::CaseInsensitive)
                                            ? QOpenGLDebugLogger::SynchronousLogging
                                            : QOpenGLDebugLogger::AsynchronousLogging);

                const auto msgs = m_debugLogger->loggedMessages();
                for (const QOpenGLDebugMessage &msg : msgs)
                    logOpenGLDebugMessage(msg);
            }
        } else {
            qCDebug(Backend) << "Qt3D: OpenGL debug logging requested but GL_KHR_debug not supported";
        }
    }


    // Set Vendor and Extensions of reference GraphicsApiFilter
    // TO DO: would that vary like the glHelper ?

    QStringList extensions;
    const auto exts = m_gl->extensions();
    for (const QByteArray &ext : exts)
        extensions << QString::fromUtf8(ext);
    m_contextInfo.m_major = m_gl->format().version().first;
    m_contextInfo.m_minor = m_gl->format().version().second;
    m_contextInfo.m_api = m_gl->isOpenGLES() ? QGraphicsApiFilter::OpenGLES : QGraphicsApiFilter::OpenGL;
    m_contextInfo.m_profile = static_cast<QGraphicsApiFilter::OpenGLProfile>(m_gl->format().profile());
    m_contextInfo.m_extensions = extensions;
    m_contextInfo.m_vendor = QString::fromUtf8(reinterpret_cast<const char *>(m_gl->functions()->glGetString(GL_VENDOR)));

    return glHelper;
}

const GraphicsApiFilterData *GraphicsContext::contextInfo() const
{
    return &m_contextInfo;
}

bool GraphicsContext::supportsDrawBuffersBlend() const
{
    return m_glHelper->supportsFeature(GraphicsHelperInterface::DrawBuffersBlend);
}

/*!
 * \internal
 * Wraps an OpenGL call to glDrawElementsInstanced.
 * If the call is not supported by the system's OpenGL version,
 * it is simulated with a loop.
 */
void GraphicsContext::drawElementsInstancedBaseVertexBaseInstance(GLenum primitiveType,
                                                                  GLsizei primitiveCount,
                                                                  GLint indexType,
                                                                  void *indices,
                                                                  GLsizei instances,
                                                                  GLint baseVertex,
                                                                  GLint baseInstance)
{
    m_glHelper->drawElementsInstancedBaseVertexBaseInstance(primitiveType,
                                                            primitiveCount,
                                                            indexType,
                                                            indices,
                                                            instances,
                                                            baseVertex,
                                                            baseInstance);
}

/*!
 * \internal
 * Wraps an OpenGL call to glDrawArraysInstanced.
 */
void GraphicsContext::drawArraysInstanced(GLenum primitiveType,
                                          GLint first,
                                          GLsizei count,
                                          GLsizei instances)
{
    m_glHelper->drawArraysInstanced(primitiveType,
                                    first,
                                    count,
                                    instances);
}

void GraphicsContext::drawArraysInstancedBaseInstance(GLenum primitiveType, GLint first, GLsizei count, GLsizei instances, GLsizei baseinstance)
{
    m_glHelper->drawArraysInstancedBaseInstance(primitiveType,
                                                first,
                                                count,
                                                instances,
                                                baseinstance);
}

/*!
 * \internal
 * Wraps an OpenGL call to glDrawElements.
 */
void GraphicsContext::drawElements(GLenum primitiveType,
                                   GLsizei primitiveCount,
                                   GLint indexType,
                                   void *indices,
                                   GLint baseVertex)
{
    m_glHelper->drawElements(primitiveType,
                             primitiveCount,
                             indexType,
                             indices,
                             baseVertex);
}

void GraphicsContext::drawElementsIndirect(GLenum mode,
                                           GLenum type,
                                           void *indirect)
{
    m_glHelper->drawElementsIndirect(mode, type, indirect);
}

/*!
 * \internal
 * Wraps an OpenGL call to glDrawArrays.
 */
void GraphicsContext::drawArrays(GLenum primitiveType,
                                 GLint first,
                                 GLsizei count)
{
    m_glHelper->drawArrays(primitiveType,
                           first,
                           count);
}

void GraphicsContext::drawArraysIndirect(GLenum mode, void *indirect)
{
    m_glHelper->drawArraysIndirect(mode, indirect);
}

void GraphicsContext::setVerticesPerPatch(GLint verticesPerPatch)
{
    m_glHelper->setVerticesPerPatch(verticesPerPatch);
}

void GraphicsContext::blendEquation(GLenum mode)
{
    m_glHelper->blendEquation(mode);
}

void GraphicsContext::blendFunci(GLuint buf, GLenum sfactor, GLenum dfactor)
{
    m_glHelper->blendFunci(buf, sfactor, dfactor);
}

void GraphicsContext::blendFuncSeparatei(GLuint buf, GLenum sRGB, GLenum dRGB, GLenum sAlpha, GLenum dAlpha)
{
    m_glHelper->blendFuncSeparatei(buf, sRGB, dRGB, sAlpha, dAlpha);
}

void GraphicsContext::alphaTest(GLenum mode1, GLenum mode2)
{
    m_glHelper->alphaTest(mode1, mode2);
}

void GraphicsContext::bindFramebuffer(GLuint fbo, GraphicsHelperInterface::FBOBindMode mode)
{
    m_glHelper->bindFrameBufferObject(fbo, mode);
}

void GraphicsContext::depthTest(GLenum mode)
{
    m_glHelper->depthTest(mode);
}

void GraphicsContext::depthMask(GLenum mode)
{
    m_glHelper->depthMask(mode);
}

void GraphicsContext::frontFace(GLenum mode)
{
    m_glHelper->frontFace(mode);
}

void GraphicsContext::bindFragOutputs(GLuint shader, const QHash<QString, int> &outputs)
{
    if (m_glHelper->supportsFeature(GraphicsHelperInterface::MRT) &&
            m_glHelper->supportsFeature(GraphicsHelperInterface::BindableFragmentOutputs))
        m_glHelper->bindFragDataLocation(shader, outputs);
}

void GraphicsContext::bindUniformBlock(GLuint programId, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    m_glHelper->bindUniformBlock(programId, uniformBlockIndex, uniformBlockBinding);
}

void GraphicsContext::bindShaderStorageBlock(GLuint programId, GLuint shaderStorageBlockIndex, GLuint shaderStorageBlockBinding)
{
    m_glHelper->bindShaderStorageBlock(programId, shaderStorageBlockIndex, shaderStorageBlockBinding);
}

void GraphicsContext::bindBufferBase(GLenum target, GLuint bindingIndex, GLuint buffer)
{
    m_glHelper->bindBufferBase(target, bindingIndex, buffer);
}

void GraphicsContext::buildUniformBuffer(const QVariant &v, const ShaderUniform &description, QByteArray &buffer)
{
    m_glHelper->buildUniformBuffer(v, description, buffer);
}

void GraphicsContext::setMSAAEnabled(bool enabled)
{
    m_glHelper->setMSAAEnabled(enabled);
}

void GraphicsContext::setAlphaCoverageEnabled(bool enabled)
{
    m_glHelper->setAlphaCoverageEnabled(enabled);
}

void GraphicsContext::clearBufferf(GLint drawbuffer, const QVector4D &values)
{
    m_glHelper->clearBufferf(drawbuffer, values);
}

GLuint GraphicsContext::boundFrameBufferObject()
{
    return m_glHelper->boundFrameBufferObject();
}

void GraphicsContext::clearColor(const QColor &color)
{
    m_gl->functions()->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void GraphicsContext::clearDepthValue(float depth)
{
    m_gl->functions()->glClearDepthf(depth);
}

void GraphicsContext::clearStencilValue(int stencil)
{
    m_gl->functions()->glClearStencil(stencil);
}

void GraphicsContext::enableClipPlane(int clipPlane)
{
    m_glHelper->enableClipPlane(clipPlane);
}

void GraphicsContext::disableClipPlane(int clipPlane)
{
    m_glHelper->disableClipPlane(clipPlane);
}

void GraphicsContext::setClipPlane(int clipPlane, const QVector3D &normal, float distance)
{
    m_glHelper->setClipPlane(clipPlane, normal, distance);
}

GLint GraphicsContext::maxClipPlaneCount()
{
    return m_glHelper->maxClipPlaneCount();
}

GLint GraphicsContext::maxTextureUnitsCount()
{
    return m_maxTextureUnits;
}

void GraphicsContext::enablePrimitiveRestart(int restartIndex)
{
    if (m_glHelper->supportsFeature(GraphicsHelperInterface::PrimitiveRestart))
        m_glHelper->enablePrimitiveRestart(restartIndex);
}

void GraphicsContext::disablePrimitiveRestart()
{
    if (m_glHelper->supportsFeature(GraphicsHelperInterface::PrimitiveRestart))
        m_glHelper->disablePrimitiveRestart();
}

void GraphicsContext::pointSize(bool programmable, GLfloat value)
{
    m_glHelper->pointSize(programmable, value);
}

void GraphicsContext::dispatchCompute(int x, int y, int z)
{
    if (m_glHelper->supportsFeature(GraphicsHelperInterface::Compute))
        m_glHelper->dispatchCompute(x, y, z);
}

GLboolean GraphicsContext::unmapBuffer(GLenum target)
{
    return m_glHelper->unmapBuffer(target);
}

char *GraphicsContext::mapBuffer(GLenum target, GLsizeiptr size)
{
    return m_glHelper->mapBuffer(target, size);
}

void GraphicsContext::enablei(GLenum cap, GLuint index)
{
    m_glHelper->enablei(cap, index);
}

void GraphicsContext::disablei(GLenum cap, GLuint index)
{
    m_glHelper->disablei(cap, index);
}

void GraphicsContext::setSeamlessCubemap(bool enable)
{
    m_glHelper->setSeamlessCubemap(enable);
}

void GraphicsContext::readBuffer(GLenum mode)
{
    m_glHelper->readBuffer(mode);
}

void GraphicsContext::drawBuffer(GLenum mode)
{
    m_glHelper->drawBuffer(mode);
}

void GraphicsContext::drawBuffers(GLsizei n, const int *bufs)
{
    m_glHelper->drawBuffers(n, bufs);
}

void GraphicsContext::applyUniform(const ShaderUniform &description, const UniformValue &v)
{
    const UniformType type = m_glHelper->uniformTypeFromGLType(description.m_type);

    switch (type) {
    case UniformType::Float:
        // See QTBUG-57510 and uniform_p.h
        if (v.storedType() == Int) {
            float value = float(*v.constData<int>());
            UniformValue floatV(value);
            applyUniformHelper<UniformType::Float>(description, floatV);
        } else {
            applyUniformHelper<UniformType::Float>(description, v);
        }
        break;
    case UniformType::Vec2:
        applyUniformHelper<UniformType::Vec2>(description, v);
        break;
    case UniformType::Vec3:
        applyUniformHelper<UniformType::Vec3>(description, v);
        break;
    case UniformType::Vec4:
        applyUniformHelper<UniformType::Vec4>(description, v);
        break;

    case UniformType::Double:
        applyUniformHelper<UniformType::Double>(description, v);
        break;
    case UniformType::DVec2:
        applyUniformHelper<UniformType::DVec2>(description, v);
        break;
    case UniformType::DVec3:
        applyUniformHelper<UniformType::DVec3>(description, v);
        break;
    case UniformType::DVec4:
        applyUniformHelper<UniformType::DVec4>(description, v);
        break;

    case UniformType::Sampler:
    case UniformType::Int:
        applyUniformHelper<UniformType::Int>(description, v);
        break;
    case UniformType::IVec2:
        applyUniformHelper<UniformType::IVec2>(description, v);
        break;
    case UniformType::IVec3:
        applyUniformHelper<UniformType::IVec3>(description, v);
        break;
    case UniformType::IVec4:
        applyUniformHelper<UniformType::IVec4>(description, v);
        break;

    case UniformType::UInt:
        applyUniformHelper<UniformType::UInt>(description, v);
        break;
    case UniformType::UIVec2:
        applyUniformHelper<UniformType::UIVec2>(description, v);
        break;
    case UniformType::UIVec3:
        applyUniformHelper<UniformType::UIVec3>(description, v);
        break;
    case UniformType::UIVec4:
        applyUniformHelper<UniformType::UIVec4>(description, v);
        break;

    case UniformType::Bool:
        applyUniformHelper<UniformType::Bool>(description, v);
        break;
    case UniformType::BVec2:
        applyUniformHelper<UniformType::BVec2>(description, v);
        break;
    case UniformType::BVec3:
        applyUniformHelper<UniformType::BVec3>(description, v);
        break;
    case UniformType::BVec4:
        applyUniformHelper<UniformType::BVec4>(description, v);
        break;

    case UniformType::Mat2:
        applyUniformHelper<UniformType::Mat2>(description, v);
        break;
    case UniformType::Mat3:
        applyUniformHelper<UniformType::Mat3>(description, v);
        break;
    case UniformType::Mat4:
        applyUniformHelper<UniformType::Mat4>(description, v);
        break;
    case UniformType::Mat2x3:
        applyUniformHelper<UniformType::Mat2x3>(description, v);
        break;
    case UniformType::Mat3x2:
        applyUniformHelper<UniformType::Mat3x2>(description, v);
        break;
    case UniformType::Mat2x4:
        applyUniformHelper<UniformType::Mat2x4>(description, v);
        break;
    case UniformType::Mat4x2:
        applyUniformHelper<UniformType::Mat4x2>(description, v);
        break;
    case UniformType::Mat3x4:
        applyUniformHelper<UniformType::Mat3x4>(description, v);
        break;
    case UniformType::Mat4x3:
        applyUniformHelper<UniformType::Mat4x3>(description, v);
        break;

    default:
        break;
    }
}

void GraphicsContext::memoryBarrier(QMemoryBarrier::Operations barriers)
{
    m_glHelper->memoryBarrier(barriers);
}

GLint GraphicsContext::elementType(GLint type)
{
    switch (type) {
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
        return GL_FLOAT;

#ifndef QT_OPENGL_ES_2 // Otherwise compile error as Qt defines GL_DOUBLE as GL_FLOAT when using ES2
    case GL_DOUBLE:
#ifdef GL_DOUBLE_VEC3 // For compiling on pre GL 4.1 systems
    case GL_DOUBLE_VEC2:
    case GL_DOUBLE_VEC3:
    case GL_DOUBLE_VEC4:
#endif
        return GL_DOUBLE;
#endif
    default:
        qWarning() << Q_FUNC_INFO << "unsupported:" << QString::number(type, 16);
    }

    return GL_INVALID_VALUE;
}

GLint GraphicsContext::tupleSizeFromType(GLint type)
{
    switch (type) {
    case GL_FLOAT:
#ifndef QT_OPENGL_ES_2 // Otherwise compile error as Qt defines GL_DOUBLE as GL_FLOAT when using ES2
    case GL_DOUBLE:
#endif
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_INT:
        break; // fall through

    case GL_FLOAT_VEC2:
#ifdef GL_DOUBLE_VEC2 // For compiling on pre GL 4.1 systems.
    case GL_DOUBLE_VEC2:
#endif
        return 2;

    case GL_FLOAT_VEC3:
#ifdef GL_DOUBLE_VEC3 // For compiling on pre GL 4.1 systems.
    case GL_DOUBLE_VEC3:
#endif
        return 3;

    case GL_FLOAT_VEC4:
#ifdef GL_DOUBLE_VEC4 // For compiling on pre GL 4.1 systems.
    case GL_DOUBLE_VEC4:
#endif
        return 4;

    default:
        qWarning() << Q_FUNC_INFO << "unsupported:" << QString::number(type, 16);
    }

    return 1;
}

GLuint GraphicsContext::byteSizeFromType(GLint type)
{
    switch (type) {
    case GL_FLOAT:          return sizeof(float);
#ifndef QT_OPENGL_ES_2 // Otherwise compile error as Qt defines GL_DOUBLE as GL_FLOAT when using ES2
    case GL_DOUBLE:         return sizeof(double);
#endif
    case GL_UNSIGNED_BYTE:  return sizeof(unsigned char);
    case GL_UNSIGNED_INT:   return sizeof(GLuint);

    case GL_FLOAT_VEC2:     return sizeof(float) * 2;
    case GL_FLOAT_VEC3:     return sizeof(float) * 3;
    case GL_FLOAT_VEC4:     return sizeof(float) * 4;
#ifdef GL_DOUBLE_VEC3 // Required to compile on pre GL 4.1 systems
    case GL_DOUBLE_VEC2:    return sizeof(double) * 2;
    case GL_DOUBLE_VEC3:    return sizeof(double) * 3;
    case GL_DOUBLE_VEC4:    return sizeof(double) * 4;
#endif
    default:
        qWarning() << Q_FUNC_INFO << "unsupported:" << QString::number(type, 16);
    }

    return 0;
}

GLint GraphicsContext::glDataTypeFromAttributeDataType(QAttribute::VertexBaseType dataType)
{
    switch (dataType) {
    case QAttribute::Byte:
        return GL_BYTE;
    case QAttribute::UnsignedByte:
        return GL_UNSIGNED_BYTE;
    case QAttribute::Short:
        return GL_SHORT;
    case QAttribute::UnsignedShort:
        return GL_UNSIGNED_SHORT;
    case QAttribute::Int:
        return GL_INT;
    case QAttribute::UnsignedInt:
        return GL_UNSIGNED_INT;
    case QAttribute::HalfFloat:
#ifdef GL_HALF_FLOAT
        return GL_HALF_FLOAT;
#endif
#ifndef QT_OPENGL_ES_2 // Otherwise compile error as Qt defines GL_DOUBLE as GL_FLOAT when using ES2
    case QAttribute::Double:
        return GL_DOUBLE;
#endif
    case QAttribute::Float:
        break;
    default:
        qWarning() << Q_FUNC_INFO << "unsupported dataType:" << dataType;
    }
    return GL_FLOAT;
}

QT3D_UNIFORM_TYPE_IMPL(UniformType::Float, float, glUniform1fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Vec2, float, glUniform2fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Vec3, float, glUniform3fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Vec4, float, glUniform4fv)

// OpenGL expects int* as values for booleans
QT3D_UNIFORM_TYPE_IMPL(UniformType::Bool, int, glUniform1iv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::BVec2, int, glUniform2iv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::BVec3, int, glUniform3iv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::BVec4, int, glUniform4iv)

QT3D_UNIFORM_TYPE_IMPL(UniformType::Int, int, glUniform1iv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::IVec2, int, glUniform2iv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::IVec3, int, glUniform3iv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::IVec4, int, glUniform4iv)

QT3D_UNIFORM_TYPE_IMPL(UniformType::UInt, uint, glUniform1uiv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::UIVec2, uint, glUniform2uiv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::UIVec3, uint, glUniform3uiv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::UIVec4, uint, glUniform4uiv)

QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat2, float, glUniformMatrix2fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat3, float, glUniformMatrix3fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat4, float, glUniformMatrix4fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat2x3, float, glUniformMatrix2x3fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat3x2, float, glUniformMatrix3x2fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat2x4, float, glUniformMatrix2x4fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat4x2, float, glUniformMatrix4x2fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat3x4, float, glUniformMatrix3x4fv)
QT3D_UNIFORM_TYPE_IMPL(UniformType::Mat4x3, float, glUniformMatrix4x3fv)

} // namespace Render
} // namespace Qt3DRender of namespace

QT_END_NAMESPACE
