/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include "submissioncontext_p.h"

#include <Qt3DRender/qgraphicsapifilter.h>
#include <Qt3DRender/qparameter.h>
#include <Qt3DRender/qcullface.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/shader_p.h>
#include <Qt3DRender/private/material_p.h>
#include <Qt3DRender/private/gltexture_p.h>
#include <Qt3DRender/private/buffer_p.h>
#include <Qt3DRender/private/attribute_p.h>
#include <Qt3DRender/private/rendercommand_p.h>
#include <Qt3DRender/private/renderstates_p.h>
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
#include <Qt3DRender/private/stringtoint_p.h>
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

#include <private/qdebug_p.h>
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

namespace Qt3DRender {
namespace Render {


static QHash<unsigned int, SubmissionContext*> static_contexts;

unsigned int nextFreeContextId()
{
    for (unsigned int i=0; i < 0xffff; ++i) {
        if (!static_contexts.contains(i))
            return i;
    }

    qFatal("Couldn't find free context ID");
    return 0;
}

namespace {

GLBuffer::Type attributeTypeToGLBufferType(QAttribute::AttributeType type)
{
    switch (type) {
    case QAttribute::VertexAttribute:
        return GLBuffer::ArrayBuffer;
    case QAttribute::IndexAttribute:
        return GLBuffer::IndexBuffer;
    case QAttribute::DrawIndirectAttribute:
        return GLBuffer::DrawIndirectBuffer;
    default:
        Q_UNREACHABLE();
    }
}

void copyGLFramebufferDataToImage(QImage &img, const uchar *srcData, uint stride, uint width, uint height, QAbstractTexture::TextureFormat format)
{
    switch (format) {
    case QAbstractTexture::RGBA32F:
        {
            uchar *srcScanline = (uchar *)srcData + stride * (height - 1);
            for (uint i = 0; i < height; ++i) {
                uchar *dstScanline = img.scanLine(i);
                float *pSrc = (float*)srcScanline;
                for (uint j = 0; j < width; j++) {
                    *dstScanline++ = (uchar)(255.0f * qBound(0.0f, pSrc[4*j+2], 1.0f));
                    *dstScanline++ = (uchar)(255.0f * qBound(0.0f, pSrc[4*j+1], 1.0f));
                    *dstScanline++ = (uchar)(255.0f * qBound(0.0f, pSrc[4*j+0], 1.0f));
                    *dstScanline++ = (uchar)(255.0f * qBound(0.0f, pSrc[4*j+3], 1.0f));
                }
                srcScanline -= stride;
            }
        } break;
    default:
        {
            uchar* srcScanline = (uchar *)srcData + stride * (height - 1);
            for (uint i = 0; i < height; ++i) {
                memcpy(img.scanLine(i), srcScanline, stride);
                srcScanline -= stride;
            }
        } break;
    }
}

// Render States Helpers
template<typename GenericState>
void applyStateHelper(const GenericState *state, SubmissionContext *gc)
{
    Q_UNUSED(state);
    Q_UNUSED(gc);
}

template<>
void applyStateHelper<AlphaFunc>(const AlphaFunc *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->alphaTest(std::get<0>(values), std::get<1>(values));
}

template<>
void applyStateHelper<BlendEquationArguments>(const BlendEquationArguments *state, SubmissionContext *gc)
{
    const auto values = state->values();
    // Un-indexed BlendEquationArguments -> Use normal GL1.0 functions
    if (std::get<5>(values) < 0) {
        if (std::get<4>(values)) {
            gc->openGLContext()->functions()->glEnable(GL_BLEND);
            gc->openGLContext()->functions()->glBlendFuncSeparate(std::get<0>(values), std::get<1>(values), std::get<2>(values), std::get<3>(values));
        } else {
            gc->openGLContext()->functions()->glDisable(GL_BLEND);
        }
    }
    // BlendEquationArguments for a particular Draw Buffer. Different behaviours for
    //  (1) 3.0-3.3: only enablei/disablei supported.
    //  (2) 4.0+: all operations supported.
    // We just ignore blend func parameter for (1), so no warnings get
    // printed.
    else {
        if (std::get<4>(values)) {
            gc->enablei(GL_BLEND, std::get<5>(values));
            if (gc->supportsDrawBuffersBlend()) {
                gc->blendFuncSeparatei(std::get<5>(values), std::get<0>(values), std::get<1>(values), std::get<2>(values), std::get<3>(values));
            }
        } else {
            gc->disablei(GL_BLEND, std::get<5>(values));
        }
    }
}

template<>
void applyStateHelper<BlendEquation>(const BlendEquation *state, SubmissionContext *gc)
{
    gc->blendEquation(std::get<0>(state->values()));
}

template<>
void applyStateHelper<MSAAEnabled>(const MSAAEnabled *state, SubmissionContext *gc)
{
    gc->setMSAAEnabled(std::get<0>(state->values()));
}


template<>
void applyStateHelper<DepthTest>(const DepthTest *state, SubmissionContext *gc)
{
    gc->depthTest(std::get<0>(state->values()));
}


template<>
void applyStateHelper<NoDepthMask>(const NoDepthMask *state, SubmissionContext *gc)
{
    gc->depthMask(std::get<0>(state->values()));
}


template<>
void applyStateHelper<CullFace>(const CullFace *state, SubmissionContext *gc)
{
    const auto values = state->values();
    if (std::get<0>(values) == QCullFace::NoCulling) {
        gc->openGLContext()->functions()->glDisable(GL_CULL_FACE);
    } else {
        gc->openGLContext()->functions()->glEnable(GL_CULL_FACE);
        gc->openGLContext()->functions()->glCullFace(std::get<0>(values));
    }
}

template<>
void applyStateHelper<FrontFace>(const FrontFace *state, SubmissionContext *gc)
{
    gc->frontFace(std::get<0>(state->values()));
}

template<>
void applyStateHelper<ScissorTest>(const ScissorTest *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->openGLContext()->functions()->glEnable(GL_SCISSOR_TEST);
    gc->openGLContext()->functions()->glScissor(std::get<0>(values), std::get<1>(values), std::get<2>(values), std::get<3>(values));
}

template<>
void applyStateHelper<StencilTest>(const StencilTest *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->openGLContext()->functions()->glEnable(GL_STENCIL_TEST);
    gc->openGLContext()->functions()->glStencilFuncSeparate(GL_FRONT, std::get<0>(values), std::get<1>(values), std::get<2>(values));
    gc->openGLContext()->functions()->glStencilFuncSeparate(GL_BACK, std::get<3>(values), std::get<4>(values), std::get<5>(values));
}

template<>
void applyStateHelper<AlphaCoverage>(const AlphaCoverage *, SubmissionContext *gc)
{
    gc->setAlphaCoverageEnabled(true);
}

template<>
void applyStateHelper<PointSize>(const PointSize *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->pointSize(std::get<0>(values), std::get<1>(values));
}


template<>
void applyStateHelper<PolygonOffset>(const PolygonOffset *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->openGLContext()->functions()->glEnable(GL_POLYGON_OFFSET_FILL);
    gc->openGLContext()->functions()->glPolygonOffset(std::get<0>(values), std::get<1>(values));
}

template<>
void applyStateHelper<ColorMask>(const ColorMask *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->openGLContext()->functions()->glColorMask(std::get<0>(values), std::get<1>(values), std::get<2>(values), std::get<3>(values));
}

template<>
void applyStateHelper<ClipPlane>(const ClipPlane *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->enableClipPlane(std::get<0>(values));
    gc->setClipPlane(std::get<0>(values), std::get<1>(values), std::get<2>(values));
}

template<>
void applyStateHelper<SeamlessCubemap>(const SeamlessCubemap *, SubmissionContext *gc)
{
    gc->setSeamlessCubemap(true);
}

template<>
void applyStateHelper<StencilOp>(const StencilOp *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->openGLContext()->functions()->glStencilOpSeparate(GL_FRONT, std::get<0>(values), std::get<1>(values), std::get<2>(values));
    gc->openGLContext()->functions()->glStencilOpSeparate(GL_BACK, std::get<3>(values), std::get<4>(values), std::get<5>(values));
}

template<>
void applyStateHelper<StencilMask>(const StencilMask *state, SubmissionContext *gc)
{
    const auto values = state->values();
    gc->openGLContext()->functions()->glStencilMaskSeparate(GL_FRONT, std::get<0>(values));
    gc->openGLContext()->functions()->glStencilMaskSeparate(GL_BACK, std::get<1>(values));
}

template<>
void applyStateHelper<Dithering>(const Dithering *, SubmissionContext *gc)
{
    gc->openGLContext()->functions()->glEnable(GL_DITHER);
}

#ifndef GL_LINE_SMOOTH
#define GL_LINE_SMOOTH 0x0B20
#endif

template<>
void applyStateHelper<LineWidth>(const LineWidth *state, SubmissionContext *gc)
{
    const auto values = state->values();
    if (std::get<1>(values))
        gc->openGLContext()->functions()->glEnable(GL_LINE_SMOOTH);
    else
        gc->openGLContext()->functions()->glDisable(GL_LINE_SMOOTH);

    gc->openGLContext()->functions()->glLineWidth(std::get<0>(values));
}

} // anonymous


SubmissionContext::SubmissionContext()
    : GraphicsContext()
    , m_ownCurrent(true)
    , m_id(nextFreeContextId())
    , m_surface(nullptr)
    , m_activeShader(nullptr)
    , m_activeShaderDNA(0)
    , m_renderTargetFormat(QAbstractTexture::NoFormat)
    , m_currClearStencilValue(0)
    , m_currClearDepthValue(1.f)
    , m_currClearColorValue(0,0,0,0)
    , m_material(nullptr)
    , m_activeFBO(0)
    , m_boundArrayBuffer(nullptr)
    , m_stateSet(nullptr)
    , m_renderer(nullptr)
    , m_uboTempArray(QByteArray(1024, 0))
{
    static_contexts[m_id] = this;
}

SubmissionContext::~SubmissionContext()
{
    releaseOpenGL();

    Q_ASSERT(static_contexts[m_id] == this);
    static_contexts.remove(m_id);
}

void SubmissionContext::initialize()
{
    GraphicsContext::initialize();
    m_textureContext.initialize(this);
}

void SubmissionContext::resolveRenderTargetFormat()
{
    const QSurfaceFormat format = m_gl->format();
    const uint a = (format.alphaBufferSize() == -1) ? 0 : format.alphaBufferSize();
    const uint r = format.redBufferSize();
    const uint g = format.greenBufferSize();
    const uint b = format.blueBufferSize();

#define RGBA_BITS(r,g,b,a) (r | (g << 6) | (b << 12) | (a << 18))

    const uint bits = RGBA_BITS(r,g,b,a);
    switch (bits) {
    case RGBA_BITS(8,8,8,8):
        m_renderTargetFormat = QAbstractTexture::RGBA8_UNorm;
        break;
    case RGBA_BITS(8,8,8,0):
        m_renderTargetFormat = QAbstractTexture::RGB8_UNorm;
        break;
    case RGBA_BITS(5,6,5,0):
        m_renderTargetFormat = QAbstractTexture::R5G6B5;
        break;
    }
#undef RGBA_BITS
}

bool SubmissionContext::beginDrawing(QSurface *surface)
{
    Q_ASSERT(surface);
    Q_ASSERT(m_gl);

    m_surface = surface;

    // TO DO: Find a way to make to pause work if the window is not exposed
    //    if (m_surface && m_surface->surfaceClass() == QSurface::Window) {
    //        qDebug() << Q_FUNC_INFO << 1;
    //        if (!static_cast<QWindow *>(m_surface)->isExposed())
    //            return false;
    //        qDebug() << Q_FUNC_INFO << 2;
    //    }

    // Makes the surface current on the OpenGLContext
    // and sets the right glHelper
    m_ownCurrent = !(m_gl->surface() == m_surface);
    if (m_ownCurrent && !makeCurrent(m_surface))
        return false;

    // TODO: cache surface format somewhere rather than doing this every time render surface changes
    resolveRenderTargetFormat();

#if defined(QT3D_RENDER_ASPECT_OPENGL_DEBUG)
    GLint err = m_gl->functions()->glGetError();
    if (err != 0) {
        qCWarning(Backend) << Q_FUNC_INFO << "glGetError:" << err;
    }
#endif

    if (!isInitialized())
        initialize();
    initializeHelpers(m_surface);

    // need to reset these values every frame, may get overwritten elsewhere
    m_gl->functions()->glClearColor(m_currClearColorValue.redF(), m_currClearColorValue.greenF(), m_currClearColorValue.blueF(), m_currClearColorValue.alphaF());
    m_gl->functions()->glClearDepthf(m_currClearDepthValue);
    m_gl->functions()->glClearStencil(m_currClearStencilValue);

    if (m_activeShader) {
        m_activeShader = nullptr;
        m_activeShaderDNA = 0;
    }

    m_boundArrayBuffer = nullptr;

    static int callCount = 0;
    ++callCount;
    const int shaderPurgePeriod = 600;
    if (callCount % shaderPurgePeriod == 0)
        m_shaderCache->purge();

    return true;
}

void SubmissionContext::endDrawing(bool swapBuffers)
{
    if (swapBuffers)
        m_gl->swapBuffers(m_surface);
    if (m_ownCurrent)
        m_gl->doneCurrent();
    m_textureContext.endDrawing();
}

void SubmissionContext::activateRenderTarget(Qt3DCore::QNodeId renderTargetNodeId, const AttachmentPack &attachments, GLuint defaultFboId)
{
    GLuint fboId = defaultFboId; // Default FBO
    if (renderTargetNodeId) {
        // New RenderTarget
        if (!m_renderTargets.contains(renderTargetNodeId)) {
            if (m_defaultFBO && fboId == m_defaultFBO) {
                // this is the default fbo that some platforms create (iOS), we never
                // register it
            } else {
                fboId = createRenderTarget(renderTargetNodeId, attachments);
            }
        } else {
            fboId = updateRenderTarget(renderTargetNodeId, attachments, true);
        }
    }
    m_activeFBO = fboId;
    m_activeFBONodeId = renderTargetNodeId;
    m_glHelper->bindFrameBufferObject(m_activeFBO, GraphicsHelperInterface::FBODraw);
    // Set active drawBuffers
    activateDrawBuffers(attachments);
}

void SubmissionContext::releaseRenderTarget(const Qt3DCore::QNodeId id)
{
    if (m_renderTargets.contains(id)) {
        const RenderTargetInfo targetInfo = m_renderTargets.take(id);
        const GLuint fboId = targetInfo.fboId;
        m_glHelper->releaseFrameBufferObject(fboId);
    }
}

GLuint SubmissionContext::createRenderTarget(Qt3DCore::QNodeId renderTargetNodeId, const AttachmentPack &attachments)
{
    const GLuint fboId = m_glHelper->createFrameBufferObject();
    if (fboId) {
        // The FBO is created and its attachments are set once
        // Bind FBO
        m_glHelper->bindFrameBufferObject(fboId, GraphicsHelperInterface::FBODraw);
        // Insert FBO into hash
        const RenderTargetInfo info = bindFrameBufferAttachmentHelper(fboId, attachments);
        m_renderTargets.insert(renderTargetNodeId, info);
    } else {
        qCritical("Failed to create FBO");
    }
    return fboId;
}

GLuint SubmissionContext::updateRenderTarget(Qt3DCore::QNodeId renderTargetNodeId, const AttachmentPack &attachments, bool isActiveRenderTarget)
{
    const RenderTargetInfo fboInfo = m_renderTargets.value(renderTargetNodeId);
    const GLuint fboId =fboInfo.fboId;

    // We need to check if  one of the attachnent have changed QTBUG-64757
    bool needsRebuild = attachments != fboInfo.attachments;

    // Even if the attachment packs are the same, one of the inner texture might have
    // been resized or recreated, we need to check for that
    if (!needsRebuild) {
        // render target exists, has attachment been resized?
        GLTextureManager *glTextureManager = m_renderer->nodeManagers()->glTextureManager();
        const QSize s = fboInfo.size;

        const auto attachments_ = attachments.attachments();
        for (const Attachment &attachment : attachments_) {
            const bool textureWasUpdated = m_updateTextureIds.contains(attachment.m_textureUuid);
            GLTexture *rTex = glTextureManager->lookupResource(attachment.m_textureUuid);
            if (rTex) {
                const bool sizeHasChanged = rTex->size() != s;
                needsRebuild |= sizeHasChanged;
                if (isActiveRenderTarget && attachment.m_point == QRenderTargetOutput::Color0)
                    m_renderTargetFormat = rTex->properties().format;
            }
            needsRebuild |= textureWasUpdated;
        }
    }

    if (needsRebuild) {
        m_glHelper->bindFrameBufferObject(fboId, GraphicsHelperInterface::FBODraw);
        const RenderTargetInfo updatedInfo = bindFrameBufferAttachmentHelper(fboId, attachments);
        // Update our stored Render Target Info
        m_renderTargets.insert(renderTargetNodeId, updatedInfo);
    }

    return fboId;
}

void SubmissionContext::releaseRenderTargets()
{
    const auto keys = m_renderTargets.keys();
    for (Qt3DCore::QNodeId renderTargetId : keys)
        releaseRenderTarget(renderTargetId);
}

QSize SubmissionContext::renderTargetSize(const QSize &surfaceSize) const
{
    QSize renderTargetSize;
    if (m_activeFBO != m_defaultFBO) {
        // For external FBOs we may not have a m_renderTargets entry.
        if (m_renderTargets.contains(m_activeFBONodeId)) {
            renderTargetSize = m_renderTargets[m_activeFBONodeId].size;
        } else if (surfaceSize.isValid()) {
            renderTargetSize = surfaceSize;
        } else {
            // External FBO (when used with QtQuick2 Scene3D)

            // Query FBO color attachment 0 size
            GLint attachmentObjectType = GL_NONE;
            GLint attachment0Name = 0;
            m_gl->functions()->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                                     GL_COLOR_ATTACHMENT0,
                                                                     GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                                                     &attachmentObjectType);
            m_gl->functions()->glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                                     GL_COLOR_ATTACHMENT0,
                                                                     GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                                                     &attachment0Name);

            if (attachmentObjectType == GL_RENDERBUFFER && m_glHelper->supportsFeature(GraphicsHelperInterface::RenderBufferDimensionRetrieval))
                renderTargetSize = m_glHelper->getRenderBufferDimensions(attachment0Name);
            else if (attachmentObjectType == GL_TEXTURE && m_glHelper->supportsFeature(GraphicsHelperInterface::TextureDimensionRetrieval))
                // Assumes texture level 0 and GL_TEXTURE_2D target
                renderTargetSize = m_glHelper->getTextureDimensions(attachment0Name, GL_TEXTURE_2D);
            else
                return renderTargetSize;
        }
    } else {
        renderTargetSize = surfaceSize;
        if (m_surface->surfaceClass() == QSurface::Window) {
            const float dpr = static_cast<QWindow *>(m_surface)->devicePixelRatio();
            renderTargetSize *= dpr;
        }
    }
    return renderTargetSize;
}

QImage SubmissionContext::readFramebuffer(const QRect &rect)
{
    QImage img;
    const unsigned int area = rect.width() * rect.height();
    unsigned int bytes;
    GLenum format, type;
    QImage::Format imageFormat;
    uint stride;

    /* format value should match GL internalFormat */
    GLenum internalFormat = m_renderTargetFormat;

    switch (m_renderTargetFormat) {
    case QAbstractTexture::RGBAFormat:
    case QAbstractTexture::RGBA8_SNorm:
    case QAbstractTexture::RGBA8_UNorm:
    case QAbstractTexture::RGBA8U:
    case QAbstractTexture::SRGB8_Alpha8:
#ifdef QT_OPENGL_ES_2
        format = GL_RGBA;
        imageFormat = QImage::Format_RGBA8888_Premultiplied;
#else
        format = GL_BGRA;
        imageFormat = QImage::Format_ARGB32_Premultiplied;
        internalFormat = GL_RGBA8;
#endif
        type = GL_UNSIGNED_BYTE;
        bytes = area * 4;
        stride = rect.width() * 4;
        break;
    case QAbstractTexture::SRGB8:
    case QAbstractTexture::RGBFormat:
    case QAbstractTexture::RGB8U:
    case QAbstractTexture::RGB8_UNorm:
#ifdef QT_OPENGL_ES_2
        format = GL_RGBA;
        imageFormat = QImage::Format_RGBX8888;
#else
        format = GL_BGRA;
        imageFormat = QImage::Format_RGB32;
        internalFormat = GL_RGB8;
#endif
        type = GL_UNSIGNED_BYTE;
        bytes = area * 4;
        stride = rect.width() * 4;
        break;
#ifndef QT_OPENGL_ES_2
    case QAbstractTexture::RG11B10F:
        bytes = area * 4;
        format = GL_RGB;
        type = GL_UNSIGNED_INT_10F_11F_11F_REV;
        imageFormat = QImage::Format_RGB30;
        stride = rect.width() * 4;
        break;
    case QAbstractTexture::RGB10A2:
        bytes = area * 4;
        format = GL_RGBA;
        type = GL_UNSIGNED_INT_2_10_10_10_REV;
        imageFormat = QImage::Format_A2BGR30_Premultiplied;
        stride = rect.width() * 4;
        break;
    case QAbstractTexture::R5G6B5:
        bytes = area * 2;
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT;
        internalFormat = GL_UNSIGNED_SHORT_5_6_5_REV;
        imageFormat = QImage::Format_RGB16;
        stride = rect.width() * 2;
        break;
    case QAbstractTexture::RGBA16F:
    case QAbstractTexture::RGBA16U:
    case QAbstractTexture::RGBA32F:
    case QAbstractTexture::RGBA32U:
        bytes = area * 16;
        format = GL_RGBA;
        type = GL_FLOAT;
        imageFormat = QImage::Format_ARGB32_Premultiplied;
        stride = rect.width() * 16;
        break;
#endif
    default:
        auto warning = qWarning();
        warning << "Unable to convert";
        QtDebugUtils::formatQEnum(warning, m_renderTargetFormat);
        warning << "render target texture format to QImage.";
        return img;
    }

    GLint samples = 0;
    m_gl->functions()->glGetIntegerv(GL_SAMPLES, &samples);
    if (samples > 0 && !m_glHelper->supportsFeature(GraphicsHelperInterface::BlitFramebuffer)) {
        qWarning () << Q_FUNC_INFO << "Unable to capture multisampled framebuffer; "
                       "Required feature BlitFramebuffer is missing.";
        return img;
    }

    img = QImage(rect.width(), rect.height(), imageFormat);

    QScopedArrayPointer<uchar> data(new uchar [bytes]);

    if (samples > 0) {
        // resolve multisample-framebuffer to renderbuffer and read pixels from it
        GLuint fbo, rb;
        QOpenGLFunctions *gl = m_gl->functions();
        gl->glGenFramebuffers(1, &fbo);
        gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        gl->glGenRenderbuffers(1, &rb);
        gl->glBindRenderbuffer(GL_RENDERBUFFER, rb);
        gl->glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, rect.width(), rect.height());
        gl->glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);

        const GLenum status = gl->glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            gl->glDeleteRenderbuffers(1, &rb);
            gl->glDeleteFramebuffers(1, &fbo);
            qWarning () << Q_FUNC_INFO << "Copy-framebuffer not complete: " << status;
            return img;
        }

        m_glHelper->blitFramebuffer(rect.x(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height(),
                                    0, 0, rect.width(), rect.height(),
                                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
        gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        gl->glReadPixels(0,0,rect.width(), rect.height(), format, type, data.data());

        copyGLFramebufferDataToImage(img, data.data(), stride, rect.width(), rect.height(), m_renderTargetFormat);

        gl->glBindRenderbuffer(GL_RENDERBUFFER, rb);
        gl->glDeleteRenderbuffers(1, &rb);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, m_activeFBO);
        gl->glDeleteFramebuffers(1, &fbo);
    } else {
        // read pixels directly from framebuffer
        m_gl->functions()->glReadPixels(rect.x(), rect.y(), rect.width(), rect.height(), format, type, data.data());
        copyGLFramebufferDataToImage(img, data.data(), stride, rect.width(), rect.height(), m_renderTargetFormat);
    }

    return img;
}

void SubmissionContext::setViewport(const QRectF &viewport, const QSize &surfaceSize)
{
    //    // save for later use; this has nothing to do with the viewport but it is
    //    // here that we get to know the surfaceSize from the RenderView.
        m_surfaceSize = surfaceSize;

    m_viewport = viewport;
    QSize size = renderTargetSize(surfaceSize);

    // Check that the returned size is before calling glViewport
    if (size.isEmpty())
        return;

    // Qt3D 0------------------> 1  OpenGL  1^
    //      |                                |
    //      |                                |
    //      |                                |
    //      V                                |
    //      1                                0---------------------> 1
    // The Viewport is defined between 0 and 1 which allows us to automatically
    // scale to the size of the provided window surface
    m_gl->functions()->glViewport(m_viewport.x() * size.width(),
                                  (1.0 - m_viewport.y() - m_viewport.height()) * size.height(),
                                  m_viewport.width() * size.width(),
                                  m_viewport.height() * size.height());
}

void SubmissionContext::releaseOpenGL()
{
    m_shaderCache->clear();
    m_renderBufferHash.clear();

    // Stop and destroy the OpenGL logger
    if (m_debugLogger) {
        m_debugLogger->stopLogging();
        m_debugLogger.reset(nullptr);
    }
}

// The OpenGLContext is not current on any surface at this point
void SubmissionContext::setOpenGLContext(QOpenGLContext* ctx)
{
    Q_ASSERT(ctx && m_shaderCache);

    releaseOpenGL();
    m_gl = ctx;
}

// Called only from RenderThread
bool SubmissionContext::activateShader(ProgramDNA shaderDNA)
{
    if (shaderDNA != m_activeShaderDNA) {
        // Ensure material uniforms are re-applied
        m_material = nullptr;

        m_activeShader = m_shaderCache->getShaderProgramForDNA(shaderDNA);
        if (Q_LIKELY(m_activeShader != nullptr)) {
            m_activeShader->bind();
            m_activeShaderDNA = shaderDNA;
        } else {
            m_glHelper->useProgram(0);
            qWarning() << "No shader program found for DNA";
            m_activeShaderDNA = 0;
            return false;
        }
    }
    return true;
}

SubmissionContext::RenderTargetInfo SubmissionContext::bindFrameBufferAttachmentHelper(GLuint fboId, const AttachmentPack &attachments)
{
    // Set FBO attachments. These are normally textures, except that on Open GL
    // ES <= 3.1 we must use a renderbuffer if a combined depth+stencil is
    // desired since this cannot be achieved neither with a single texture (not
    // before GLES 3.2) nor with separate textures (no suitable format for
    // stencil before 3.1 with the appropriate extension).

    QSize fboSize;
    GLTextureManager *glTextureManager = m_renderer->nodeManagers()->glTextureManager();
    const auto attachments_ = attachments.attachments();
    for (const Attachment &attachment : attachments_) {
        GLTexture *rTex = glTextureManager->lookupResource(attachment.m_textureUuid);
        if (!m_glHelper->frameBufferNeedsRenderBuffer(attachment)) {
            QOpenGLTexture *glTex = rTex ? rTex->getGLTexture() : nullptr;
            if (glTex != nullptr) {
                // The texture can not be rendered simultaniously by another renderer
                Q_ASSERT(!rTex->isExternalRenderingEnabled());
                if (fboSize.isEmpty())
                    fboSize = QSize(glTex->width(), glTex->height());
                else
                    fboSize = QSize(qMin(fboSize.width(), glTex->width()), qMin(fboSize.height(), glTex->height()));
                m_glHelper->bindFrameBufferAttachment(glTex, attachment);
            }
        } else {
            RenderBuffer *renderBuffer = rTex ? rTex->getOrCreateRenderBuffer() : nullptr;
            if (renderBuffer) {
                if (fboSize.isEmpty())
                    fboSize = QSize(renderBuffer->width(), renderBuffer->height());
                else
                    fboSize = QSize(qMin(fboSize.width(), renderBuffer->width()), qMin(fboSize.height(), renderBuffer->height()));
                m_glHelper->bindFrameBufferAttachment(renderBuffer, attachment);
            }
        }
    }
    return {fboId, fboSize, attachments};
}

void SubmissionContext::activateDrawBuffers(const AttachmentPack &attachments)
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


void SubmissionContext::setActiveMaterial(Material *rmat)
{
    if (m_material == rmat)
        return;

    m_textureContext.deactivateTexturesWithScope(TextureSubmissionContext::TextureScopeMaterial);
    m_material = rmat;
}

void SubmissionContext::setCurrentStateSet(RenderStateSet *ss)
{
    if (ss == m_stateSet)
        return;
    if (ss)
        applyStateSet(ss);
    m_stateSet = ss;
}

RenderStateSet *SubmissionContext::currentStateSet() const
{
    return m_stateSet;
}

void SubmissionContext::applyState(const StateVariant &stateVariant)
{
    switch (stateVariant.type) {

    case AlphaCoverageStateMask: {
        applyStateHelper<AlphaCoverage>(static_cast<const AlphaCoverage *>(stateVariant.constState()), this);
        break;
    }
    case AlphaTestMask: {
        applyStateHelper<AlphaFunc>(static_cast<const AlphaFunc *>(stateVariant.constState()), this);
        break;
    }
    case BlendStateMask: {
        applyStateHelper<BlendEquation>(static_cast<const BlendEquation *>(stateVariant.constState()), this);
        break;
    }
    case BlendEquationArgumentsMask: {
        applyStateHelper<BlendEquationArguments>(static_cast<const BlendEquationArguments *>(stateVariant.constState()), this);
        break;
    }
    case MSAAEnabledStateMask: {
        applyStateHelper<MSAAEnabled>(static_cast<const MSAAEnabled *>(stateVariant.constState()), this);
        break;
    }

    case CullFaceStateMask: {
        applyStateHelper<CullFace>(static_cast<const CullFace *>(stateVariant.constState()), this);
        break;
    }

    case DepthWriteStateMask: {
        applyStateHelper<NoDepthMask>(static_cast<const NoDepthMask *>(stateVariant.constState()), this);
        break;
    }

    case DepthTestStateMask: {
        applyStateHelper<DepthTest>(static_cast<const DepthTest *>(stateVariant.constState()), this);
        break;
    }

    case FrontFaceStateMask: {
        applyStateHelper<FrontFace>(static_cast<const FrontFace *>(stateVariant.constState()), this);
        break;
    }

    case ScissorStateMask: {
        applyStateHelper<ScissorTest>(static_cast<const ScissorTest *>(stateVariant.constState()), this);
        break;
    }

    case StencilTestStateMask: {
        applyStateHelper<StencilTest>(static_cast<const StencilTest *>(stateVariant.constState()), this);
        break;
    }

    case PointSizeMask: {
        applyStateHelper<PointSize>(static_cast<const PointSize *>(stateVariant.constState()), this);
        break;
    }

    case PolygonOffsetStateMask: {
        applyStateHelper<PolygonOffset>(static_cast<const PolygonOffset *>(stateVariant.constState()), this);
        break;
    }

    case ColorStateMask: {
        applyStateHelper<ColorMask>(static_cast<const ColorMask *>(stateVariant.constState()), this);
        break;
    }

    case ClipPlaneMask: {
        applyStateHelper<ClipPlane>(static_cast<const ClipPlane *>(stateVariant.constState()), this);
        break;
    }

    case SeamlessCubemapMask: {
        applyStateHelper<SeamlessCubemap>(static_cast<const SeamlessCubemap *>(stateVariant.constState()), this);
        break;
    }

    case StencilOpMask: {
        applyStateHelper<StencilOp>(static_cast<const StencilOp *>(stateVariant.constState()), this);
        break;
    }

    case StencilWriteStateMask: {
        applyStateHelper<StencilMask>(static_cast<const StencilMask *>(stateVariant.constState()), this);
        break;
    }

    case DitheringStateMask: {
        applyStateHelper<Dithering>(static_cast<const Dithering *>(stateVariant.constState()), this);
        break;
    }

    case LineWidthMask: {
        applyStateHelper<LineWidth>(static_cast<const LineWidth *>(stateVariant.constState()), this);
        break;
    }
    default:
        Q_UNREACHABLE();
    }
}

void SubmissionContext::resetMasked(qint64 maskOfStatesToReset)
{
    // TO DO -> Call gcHelper methods instead of raw GL
    // QOpenGLFunctions shouldn't be used here directly
    QOpenGLFunctions *funcs = m_gl->functions();

    if (maskOfStatesToReset & ScissorStateMask)
        funcs->glDisable(GL_SCISSOR_TEST);

    if (maskOfStatesToReset & BlendStateMask)
        funcs->glDisable(GL_BLEND);

    if (maskOfStatesToReset & StencilWriteStateMask)
        funcs->glStencilMask(0);

    if (maskOfStatesToReset & StencilTestStateMask)
        funcs->glDisable(GL_STENCIL_TEST);

    if (maskOfStatesToReset & DepthTestStateMask)
        funcs->glDisable(GL_DEPTH_TEST);

    if (maskOfStatesToReset & DepthWriteStateMask)
        funcs->glDepthMask(GL_TRUE); // reset to default

    if (maskOfStatesToReset & FrontFaceStateMask)
        funcs->glFrontFace(GL_CCW); // reset to default

    if (maskOfStatesToReset & CullFaceStateMask)
        funcs->glDisable(GL_CULL_FACE);

    if (maskOfStatesToReset & DitheringStateMask)
        funcs->glDisable(GL_DITHER);

    if (maskOfStatesToReset & AlphaCoverageStateMask)
        setAlphaCoverageEnabled(false);

    if (maskOfStatesToReset & PointSizeMask)
        pointSize(false, 1.0f);    // reset to default

    if (maskOfStatesToReset & PolygonOffsetStateMask)
        funcs->glDisable(GL_POLYGON_OFFSET_FILL);

    if (maskOfStatesToReset & ColorStateMask)
        funcs->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if (maskOfStatesToReset & ClipPlaneMask) {
        GLint max = maxClipPlaneCount();
        for (GLint i = 0; i < max; ++i)
            disableClipPlane(i);
    }

    if (maskOfStatesToReset & SeamlessCubemapMask)
        setSeamlessCubemap(false);

    if (maskOfStatesToReset & StencilOpMask)
        funcs->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    if (maskOfStatesToReset & LineWidthMask)
        funcs->glLineWidth(1.0f);
}

void SubmissionContext::applyStateSet(RenderStateSet *ss)
{
    RenderStateSet* previousStates = currentStateSet();

    const StateMaskSet invOurState = ~ss->stateMask();
    // generate a mask for each set bit in previous, where we do not have
    // the corresponding bit set.

    StateMaskSet stateToReset = 0;
    if (previousStates) {
        stateToReset = previousStates->stateMask() & invOurState;
        qCDebug(RenderStates) << "previous states " << QString::number(previousStates->stateMask(), 2);
    }
    qCDebug(RenderStates) << " current states " << QString::number(ss->stateMask(), 2)  << "inverse " << QString::number(invOurState, 2) << " -> states to change:  " << QString::number(stateToReset, 2);

    // Reset states that aren't active in the current state set
    resetMasked(stateToReset);

    // Apply states that weren't in the previous state or that have
    // different values
    const QVector<StateVariant> statesToSet = ss->states();
    for (const StateVariant &ds : statesToSet) {
        if (previousStates && previousStates->contains(ds))
            continue;
        applyState(ds);
    }
}

void SubmissionContext::clearColor(const QColor &color)
{
    if (m_currClearColorValue != color) {
        m_currClearColorValue = color;
        m_gl->functions()->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
}

void SubmissionContext::clearDepthValue(float depth)
{
    if (m_currClearDepthValue != depth) {
        m_currClearDepthValue = depth;
        m_gl->functions()->glClearDepthf(depth);
    }
}

void SubmissionContext::clearStencilValue(int stencil)
{
    if (m_currClearStencilValue != stencil) {
        m_currClearStencilValue = stencil;
        m_gl->functions()->glClearStencil(stencil);
    }
}

void SubmissionContext::setUpdatedTexture(const Qt3DCore::QNodeIdVector &updatedTextureIds)
{
    m_updateTextureIds = updatedTextureIds;
}

// It will be easier if the QGraphicContext applies the QUniformPack
// than the other way around
bool SubmissionContext::setParameters(ShaderParameterPack &parameterPack)
{
    static const int irradianceId = StringToInt::lookupId(QLatin1String("envLight.irradiance"));
    static const int specularId = StringToInt::lookupId(QLatin1String("envLight.specular"));
    // Activate textures and update TextureUniform in the pack
    // with the correct textureUnit

    // Set the pinned texture of the previous material texture
    // to pinable so that we should easily find an available texture unit
    NodeManagers *manager = m_renderer->nodeManagers();

    m_textureContext.deactivateTexturesWithScope(TextureSubmissionContext::TextureScopeMaterial);
    // Update the uniforms with the correct texture unit id's
    PackUniformHash &uniformValues = parameterPack.uniforms();

    for (int i = 0; i < parameterPack.textures().size(); ++i) {
        const ShaderParameterPack::NamedTexture &namedTex = parameterPack.textures().at(i);
        // Given a Texture QNodeId, we retrieve the associated shared GLTexture
        if (uniformValues.contains(namedTex.glslNameId)) {
            GLTexture *t = manager->glTextureManager()->lookupResource(namedTex.texId);
            if (t != nullptr) {
                UniformValue &texUniform = uniformValues[namedTex.glslNameId];
                if (texUniform.valueType() == UniformValue::TextureValue) {
                    const int texUnit = m_textureContext.activateTexture(TextureSubmissionContext::TextureScopeMaterial, t);
                    texUniform.data<int>()[namedTex.uniformArrayIndex] = texUnit;
                    if (texUnit == -1) {
                        if (namedTex.glslNameId != irradianceId &&
                            namedTex.glslNameId != specularId) {
                            // Only return false if we are not dealing with env light textures
                            return false;
                        }
                    }
                }
            }
        }
    }

    QOpenGLShaderProgram *shader = activeShader();

    // TO DO: We could cache the binding points somehow and only do the binding when necessary
    // for SSBO and UBO

    // Bind Shader Storage block to SSBO and update SSBO
    const QVector<BlockToSSBO> blockToSSBOs = parameterPack.shaderStorageBuffers();
    for (const BlockToSSBO b : blockToSSBOs) {
        Buffer *cpuBuffer = m_renderer->nodeManagers()->bufferManager()->lookupResource(b.m_bufferID);
        GLBuffer *ssbo = glBufferForRenderBuffer(cpuBuffer, GLBuffer::ShaderStorageBuffer);

        // bindShaderStorageBlock
        // This is currently not required as we are introspecting the bindingIndex
        // value from the shaders and not replacing them, making such a call useless
        // bindShaderStorageBlock(shader->programId(), b.m_blockIndex, b.m_bindingIndex);

        // Needed to avoid conflict where the buffer would already
        // be bound as a VertexArray
        bindGLBuffer(ssbo, GLBuffer::ShaderStorageBuffer);
        ssbo->bindBufferBase(this, b.m_bindingIndex, GLBuffer::ShaderStorageBuffer);
        // TO DO: Make sure that there's enough binding points
    }

    // Bind UniformBlocks to UBO and update UBO from Buffer
    // TO DO: Convert ShaderData to Buffer so that we can use that generic process
    const QVector<BlockToUBO> blockToUBOs = parameterPack.uniformBuffers();
    int uboIndex = 0;
    for (const BlockToUBO &b : blockToUBOs) {
        Buffer *cpuBuffer = m_renderer->nodeManagers()->bufferManager()->lookupResource(b.m_bufferID);
        GLBuffer *ubo = glBufferForRenderBuffer(cpuBuffer, GLBuffer::UniformBuffer);
        bindUniformBlock(shader->programId(), b.m_blockIndex, uboIndex);
        // Needed to avoid conflict where the buffer would already
        // be bound as a VertexArray
        bindGLBuffer(ubo, GLBuffer::UniformBuffer);
        ubo->bindBufferBase(this, uboIndex++, GLBuffer::UniformBuffer);
        // TO DO: Make sure that there's enough binding points
    }

    // Update uniforms in the Default Uniform Block
    const PackUniformHash values = parameterPack.uniforms();
    const QVector<ShaderUniform> activeUniforms = parameterPack.submissionUniforms();

    for (const ShaderUniform &uniform : activeUniforms) {
        // We can use [] as we are sure the the uniform wouldn't
        // be un activeUniforms if there wasn't a matching value
        const UniformValue &v = values[uniform.m_nameId];

        // skip invalid textures
        if (v.valueType() == UniformValue::TextureValue && *v.constData<int>() == -1)
            continue;

        applyUniform(uniform, v);
    }
    // if not all data is valid, the next frame will be rendered immediately
    return true;
}

void SubmissionContext::enableAttribute(const VAOVertexAttribute &attr)
{
    // Bind buffer within the current VAO
    GLBuffer *buf = m_renderer->nodeManagers()->glBufferManager()->data(attr.bufferHandle);
    Q_ASSERT(buf);
    bindGLBuffer(buf, attr.attributeType);

    // Don't use QOpenGLShaderProgram::setAttributeBuffer() because of QTBUG-43199.
    // Use the introspection data and set the attribute explicitly
    m_glHelper->enableVertexAttributeArray(attr.location);
    m_glHelper->vertexAttributePointer(attr.shaderDataType,
                                       attr.location,
                                       attr.vertexSize,
                                       attr.dataType,
                                       GL_TRUE, // TODO: Support normalization property on QAttribute
                                       attr.byteStride,
                                       reinterpret_cast<const void *>(qintptr(attr.byteOffset)));


    // Done by the helper if it supports it
    if (attr.divisor != 0)
        m_glHelper->vertexAttribDivisor(attr.location, attr.divisor);
}

void SubmissionContext::disableAttribute(const SubmissionContext::VAOVertexAttribute &attr)
{
    QOpenGLShaderProgram *prog = activeShader();
    prog->disableAttributeArray(attr.location);
}

// Note: needs to be called while VAO is bound
void SubmissionContext::specifyAttribute(const Attribute *attribute,
                                       Buffer *buffer,
                                       const ShaderAttribute *attributeDescription)
{
    const int location = attributeDescription->m_location;
    if (location < 0) {
        qCWarning(Backend) << "failed to resolve location for attribute:" << attribute->name();
        return;
    }

    const GLint attributeDataType = glDataTypeFromAttributeDataType(attribute->vertexBaseType());
    const HGLBuffer glBufferHandle = m_renderer->nodeManagers()->glBufferManager()->lookupHandle(buffer->peerId());
    Q_ASSERT(!glBufferHandle.isNull());
    const GLBuffer::Type attributeType = attributeTypeToGLBufferType(attribute->attributeType());

    int typeSize = 0;
    int attrCount = 0;

    if (attribute->vertexSize() >= 1 && attribute->vertexSize() <= 4) {
        attrCount = 1;
    } else if (attribute->vertexSize() == 9) {
        typeSize = byteSizeFromType(attributeDataType);
        attrCount = 3;
    } else if (attribute->vertexSize() == 16) {
        typeSize = byteSizeFromType(attributeDataType);
        attrCount = 4;
    } else {
        Q_UNREACHABLE();
    }

    VAOVertexAttribute attr;
    attr.bufferHandle = glBufferHandle;
    attr.attributeType = attributeType;
    attr.dataType = attributeDataType;
    attr.divisor = attribute->divisor();
    attr.vertexSize = attribute->vertexSize() / attrCount;
    attr.byteStride = (attribute->byteStride() != 0) ? attribute->byteStride() : (attrCount * attrCount * typeSize);
    attr.shaderDataType = attributeDescription->m_type;

    for (int i = 0; i < attrCount; i++) {
        attr.location = location + i;
        attr.byteOffset = attribute->byteOffset() + (i * attrCount * typeSize);

        enableAttribute(attr);

        // Save this in the current emulated VAO
        if (m_currentVAO)
            m_currentVAO->saveVertexAttribute(attr);
    }
}

void SubmissionContext::specifyIndices(Buffer *buffer)
{
    GLBuffer *buf = glBufferForRenderBuffer(buffer, GLBuffer::IndexBuffer);
    if (!bindGLBuffer(buf, GLBuffer::IndexBuffer))
        qCWarning(Backend) << Q_FUNC_INFO << "binding index buffer failed";

    // bound within the current VAO
    // Save this in the current emulated VAO
    if (m_currentVAO)
        m_currentVAO->saveIndexAttribute(m_renderer->nodeManagers()->glBufferManager()->lookupHandle(buffer->peerId()));
}

void SubmissionContext::updateBuffer(Buffer *buffer)
{
    const QHash<Qt3DCore::QNodeId, HGLBuffer>::iterator it = m_renderBufferHash.find(buffer->peerId());
    if (it != m_renderBufferHash.end())
        uploadDataToGLBuffer(buffer, m_renderer->nodeManagers()->glBufferManager()->data(it.value()));
}

QByteArray SubmissionContext::downloadBufferContent(Buffer *buffer)
{
    const QHash<Qt3DCore::QNodeId, HGLBuffer>::iterator it = m_renderBufferHash.find(buffer->peerId());
    if (it != m_renderBufferHash.end())
        return downloadDataFromGLBuffer(buffer, m_renderer->nodeManagers()->glBufferManager()->data(it.value()));
    return QByteArray();
}

void SubmissionContext::releaseBuffer(Qt3DCore::QNodeId bufferId)
{
    auto it = m_renderBufferHash.find(bufferId);
    if (it != m_renderBufferHash.end()) {
        HGLBuffer glBuffHandle = it.value();
        GLBuffer *glBuff = m_renderer->nodeManagers()->glBufferManager()->data(glBuffHandle);

        Q_ASSERT(glBuff);
        // Destroy the GPU resource
        glBuff->destroy(this);
        // Destroy the GLBuffer instance
        m_renderer->nodeManagers()->glBufferManager()->releaseResource(bufferId);
        // Remove Id - HGLBuffer entry
        m_renderBufferHash.erase(it);
    }
}

bool SubmissionContext::hasGLBufferForBuffer(Buffer *buffer)
{
    const QHash<Qt3DCore::QNodeId, HGLBuffer>::iterator it = m_renderBufferHash.find(buffer->peerId());
    return (it != m_renderBufferHash.end());
}

GLBuffer *SubmissionContext::glBufferForRenderBuffer(Buffer *buf, GLBuffer::Type type)
{
    if (!m_renderBufferHash.contains(buf->peerId()))
        m_renderBufferHash.insert(buf->peerId(), createGLBufferFor(buf, type));
    return m_renderer->nodeManagers()->glBufferManager()->data(m_renderBufferHash.value(buf->peerId()));
}

HGLBuffer SubmissionContext::createGLBufferFor(Buffer *buffer, GLBuffer::Type type)
{
    GLBuffer *b = m_renderer->nodeManagers()->glBufferManager()->getOrCreateResource(buffer->peerId());
    //    b.setUsagePattern(static_cast<QOpenGLBuffer::UsagePattern>(buffer->usage()));
    Q_ASSERT(b);
    if (!b->create(this))
        qCWarning(Render::Io) << Q_FUNC_INFO << "buffer creation failed";

    if (!bindGLBuffer(b, type))
        qCWarning(Render::Io) << Q_FUNC_INFO << "buffer binding failed";

    return m_renderer->nodeManagers()->glBufferManager()->lookupHandle(buffer->peerId());
}

bool SubmissionContext::bindGLBuffer(GLBuffer *buffer, GLBuffer::Type type)
{
    if (type == GLBuffer::ArrayBuffer && buffer == m_boundArrayBuffer)
        return true;

    if (buffer->bind(this, type)) {
        if (type == GLBuffer::ArrayBuffer)
            m_boundArrayBuffer = buffer;
        return true;
    }
    return false;
}

void SubmissionContext::uploadDataToGLBuffer(Buffer *buffer, GLBuffer *b, bool releaseBuffer)
{
    if (!bindGLBuffer(b, GLBuffer::ArrayBuffer)) // We're uploading, the type doesn't matter here
        qCWarning(Render::Io) << Q_FUNC_INFO << "buffer bind failed";
    // If the buffer is dirty (hence being called here)
    // there are two possible cases
    // * setData was called changing the whole data or functor (or the usage pattern)
    // * partial buffer updates where received

    // TO DO: Handle usage pattern
    QVector<Qt3DRender::QBufferUpdate> updates = std::move(buffer->pendingBufferUpdates());
    for (auto it = updates.begin(); it != updates.end(); ++it) {
        auto update = it;
        // We have a partial update
        if (update->offset >= 0) {
            //accumulate sequential updates as single one
            int bufferSize = update->data.size();
            auto it2 = it + 1;
            while ((it2 != updates.end())
                   && (it2->offset - update->offset == bufferSize)) {
                bufferSize += it2->data.size();
                ++it2;
            }
            update->data.resize(bufferSize);
            while (it + 1 != it2) {
                ++it;
                update->data.replace(it->offset - update->offset, it->data.size(), it->data);
                it->data.clear();
            }
            // TO DO: based on the number of updates .., it might make sense to
            // sometime use glMapBuffer rather than glBufferSubData
            b->update(this, update->data.constData(), update->data.size(), update->offset);
        } else {
            // We have an update that was done by calling QBuffer::setData
            // which is used to resize or entirely clear the buffer
            // Note: we use the buffer data directly in that case
            const int bufferSize = buffer->data().size();
            b->allocate(this, bufferSize, false); // orphan the buffer
            b->allocate(this, buffer->data().constData(), bufferSize, false);
        }
    }

    if (releaseBuffer) {
        b->release(this);
        m_boundArrayBuffer = nullptr;
    }
    qCDebug(Render::Io) << "uploaded buffer size=" << buffer->data().size();
}

QByteArray SubmissionContext::downloadDataFromGLBuffer(Buffer *buffer, GLBuffer *b)
{
    if (!bindGLBuffer(b, GLBuffer::ArrayBuffer)) // We're downloading, the type doesn't matter here
        qCWarning(Render::Io) << Q_FUNC_INFO << "buffer bind failed";

    QByteArray data = b->download(this, buffer->data().size());
    return data;
}

void SubmissionContext::blitFramebuffer(Qt3DCore::QNodeId inputRenderTargetId,
                                        Qt3DCore::QNodeId outputRenderTargetId,
                                        QRect inputRect, QRect outputRect,
                                        uint defaultFboId,
                                        QRenderTargetOutput::AttachmentPoint inputAttachmentPoint,
                                        QRenderTargetOutput::AttachmentPoint outputAttachmentPoint,
                                        QBlitFramebuffer::InterpolationMethod interpolationMethod)
{
    GLuint inputFboId = defaultFboId;
    bool inputBufferIsDefault = true;
    if (!inputRenderTargetId.isNull()) {
        RenderTarget *renderTarget = m_renderer->nodeManagers()->renderTargetManager()->lookupResource(inputRenderTargetId);
        if (renderTarget) {
            AttachmentPack attachments(renderTarget, m_renderer->nodeManagers()->attachmentManager());
            if (m_renderTargets.contains(inputRenderTargetId))
                inputFboId = updateRenderTarget(inputRenderTargetId, attachments, false);
            else
                inputFboId = createRenderTarget(inputRenderTargetId, attachments);
        }
        inputBufferIsDefault = false;
    }

    GLuint outputFboId = defaultFboId;
    bool outputBufferIsDefault = true;
    if (!outputRenderTargetId.isNull()) {
        RenderTarget *renderTarget = m_renderer->nodeManagers()->renderTargetManager()->lookupResource(outputRenderTargetId);
        if (renderTarget) {
            AttachmentPack attachments(renderTarget, m_renderer->nodeManagers()->attachmentManager());
            if (m_renderTargets.contains(outputRenderTargetId))
                outputFboId = updateRenderTarget(outputRenderTargetId, attachments, false);
            else
                outputFboId = createRenderTarget(outputRenderTargetId, attachments);
        }
        outputBufferIsDefault = false;
    }

    // Up until this point the input and output rects are normal Qt rectangles.
    // Convert them to GL rectangles (Y at bottom).
    const int inputFboHeight = inputFboId == defaultFboId ? m_surfaceSize.height() : m_renderTargets[inputRenderTargetId].size.height();
    const GLint srcX0 = inputRect.left();
    const GLint srcY0 = inputFboHeight - (inputRect.top() + inputRect.height());
    const GLint srcX1 = srcX0 + inputRect.width();
    const GLint srcY1 = srcY0 + inputRect.height();

    const int outputFboHeight = outputFboId == defaultFboId ? m_surfaceSize.height() : m_renderTargets[outputRenderTargetId].size.height();
    const GLint dstX0 = outputRect.left();
    const GLint dstY0 = outputFboHeight - (outputRect.top() + outputRect.height());
    const GLint dstX1 = dstX0 + outputRect.width();
    const GLint dstY1 = dstY0 + outputRect.height();

    //Get the last bounded framebuffers
    const GLuint lastDrawFboId = boundFrameBufferObject();

    // Activate input framebuffer for reading
    bindFramebuffer(inputFboId, GraphicsHelperInterface::FBORead);

    // Activate output framebuffer for writing
    bindFramebuffer(outputFboId, GraphicsHelperInterface::FBODraw);

    //Bind texture
    if (!inputBufferIsDefault)
        readBuffer(GL_COLOR_ATTACHMENT0 + inputAttachmentPoint);

    if (!outputBufferIsDefault) {
        // Note that we use glDrawBuffers, not glDrawBuffer. The
        // latter is not available with GLES.
        const int buf = outputAttachmentPoint;
        drawBuffers(1, &buf);
    }

    // Blit framebuffer
    const GLenum mode = interpolationMethod ? GL_NEAREST : GL_LINEAR;
    m_glHelper->blitFramebuffer(srcX0, srcY0, srcX1, srcY1,
                                dstX0, dstY0, dstX1, dstY1,
                                GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT,
                                mode);

    // Reset draw buffer
    bindFramebuffer(lastDrawFboId, GraphicsHelperInterface::FBOReadAndDraw);
    if (outputAttachmentPoint != QRenderTargetOutput::Color0) {
        const int buf = QRenderTargetOutput::Color0;
        drawBuffers(1, &buf);
    }
}

} // namespace Render
} // namespace Qt3DRender of namespace

QT_END_NAMESPACE
