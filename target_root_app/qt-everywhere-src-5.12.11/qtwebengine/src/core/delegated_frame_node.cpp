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

// On Mac we need to reset this define in order to prevent definition
// of "check" macros etc. The "check" macro collides with a member function name in QtQuick.
// See AssertMacros.h in the Mac SDK.
#include <QtGlobal> // We need this for the Q_OS_MAC define.
#if defined(Q_OS_MAC)
#undef __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#endif

#include "delegated_frame_node.h"

#include "chromium_gpu_helper.h"
#include "ozone/gl_surface_qt.h"
#include "stream_video_node.h"
#include "type_conversion.h"
#include "yuv_video_node.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread_task_runner_handle.h"
#include "cc/base/math_util.h"
#include "components/viz/common/quads/compositor_frame.h"
#include "components/viz/common/quads/compositor_frame_metadata.h"
#include "components/viz/common/quads/debug_border_draw_quad.h"
#include "components/viz/common/quads/draw_quad.h"
#include "components/viz/common/quads/render_pass_draw_quad.h"
#include "components/viz/common/quads/solid_color_draw_quad.h"
#include "components/viz/common/quads/stream_video_draw_quad.h"
#include "components/viz/common/quads/texture_draw_quad.h"
#include "components/viz/common/quads/tile_draw_quad.h"
#include "components/viz/common/quads/yuv_video_draw_quad.h"
#include "components/viz/common/resources/returned_resource.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "components/viz/service/display/bsp_tree.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "content/browser/browser_main_loop.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_fence.h"

#ifndef QT_NO_OPENGL
# include <QOpenGLContext>
# include <QOpenGLFunctions>
# include <QSGFlatColorMaterial>
#endif
#include <QSGTexture>
#include <private/qsgadaptationlayer_p.h>

#include <QSGImageNode>
#include <QSGRectangleNode>

#if !defined(QT_NO_EGL)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#ifndef GL_TIMEOUT_IGNORED
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#endif

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE              0x84F5
#endif

#ifndef GL_NEAREST
#define GL_NEAREST                        0x2600
#endif

#ifndef GL_LINEAR
#define GL_LINEAR                         0x2601
#endif

#ifndef GL_RGBA
#define GL_RGBA                           0x1908
#endif

#ifndef GL_RGB
#define GL_RGB                            0x1907
#endif

#ifndef GL_LINE_LOOP
#define GL_LINE_LOOP                      0x0002
#endif

#ifndef QT_NO_OPENGL
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE
#endif

namespace QtWebEngineCore {
#ifndef QT_NO_OPENGL
class MailboxTexture : public QSGTexture, protected QOpenGLFunctions {
public:
    MailboxTexture(const gpu::MailboxHolder &mailboxHolder, const QSize textureSize);
    ~MailboxTexture();
    // QSGTexture:
    int textureId() const override { return m_textureId; }
    QSize textureSize() const override { return m_textureSize; }
    bool hasAlphaChannel() const override { return m_hasAlpha; }
    bool hasMipmaps() const override { return false; }
    void bind() override;

    void setHasAlphaChannel(bool hasAlpha) { m_hasAlpha = hasAlpha; }
    gpu::MailboxHolder &mailboxHolder() { return m_mailboxHolder; }
    void fetchTexture(gpu::MailboxManager *mailboxManager);
    void setTarget(GLenum target);

private:
    gpu::MailboxHolder m_mailboxHolder;
    int m_textureId;
    QSize m_textureSize;
    bool m_hasAlpha;
    GLenum m_target;
#if defined(USE_OZONE)
    bool m_ownsTexture;
#endif
#ifdef Q_OS_QNX
    EGLStreamData m_eglStreamData;
#endif
    friend class DelegatedFrameNode;
};
#endif // QT_NO_OPENGL
class ResourceHolder {
public:
    ResourceHolder(const viz::TransferableResource &resource);
    QSharedPointer<QSGTexture> initTexture(bool quadIsAllOpaque, RenderWidgetHostViewQtDelegate *apiDelegate = 0);
    QSGTexture *texture() const { return m_texture.data(); }
    viz::ReturnedResource returnResource();
    void incImportCount() { ++m_importCount; }
    bool needsToFetch() const { return !m_resource.is_software && m_texture && !m_texture.data()->textureId(); }

private:
    QWeakPointer<QSGTexture> m_texture;
    viz::TransferableResource m_resource;
    int m_importCount;
};

class RectClipNode : public QSGClipNode
{
public:
    RectClipNode(const QRectF &);
private:
    QSGGeometry m_geometry;
};

class DelegatedNodeTreeHandler
{
public:
    DelegatedNodeTreeHandler(QVector<QSGNode*> *sceneGraphNodes)
        : m_sceneGraphNodes(sceneGraphNodes)
    {
    }

    virtual ~DelegatedNodeTreeHandler(){}

    virtual void setupRenderPassNode(QSGTexture *, const QRect &, const QRectF &, QSGNode *) = 0;
    virtual void setupTextureContentNode(QSGTexture *, const QRect &, const QRectF &,
                                         QSGImageNode::TextureCoordinatesTransformMode,
                                         QSGNode *) = 0;
    virtual void setupSolidColorNode(const QRect &, const QColor &, QSGNode *) = 0;

#ifndef QT_NO_OPENGL
    virtual void setupDebugBorderNode(QSGGeometry *, QSGFlatColorMaterial *, QSGNode *) = 0;
    virtual void setupYUVVideoNode(QSGTexture *, QSGTexture *, QSGTexture *, QSGTexture *,
                           const QRectF &, const QRectF &, const QSizeF &, const QSizeF &,
                           gfx::ColorSpace, float, float, const QRectF &,
                                   QSGNode *) = 0;
#ifdef GL_OES_EGL_image_external
    virtual void setupStreamVideoNode(MailboxTexture *, const QRectF &,
                                      const QMatrix4x4 &, QSGNode *) = 0;
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL
protected:
    QVector<QSGNode*> *m_sceneGraphNodes;
};

class DelegatedNodeTreeUpdater : public DelegatedNodeTreeHandler
{
public:
    DelegatedNodeTreeUpdater(QVector<QSGNode*> *sceneGraphNodes)
        : DelegatedNodeTreeHandler(sceneGraphNodes)
        , m_nodeIterator(sceneGraphNodes->begin())
    {
    }

    void setupRenderPassNode(QSGTexture *layer, const QRect &rect, const QRectF &sourceRect, QSGNode *) override
    {
        Q_ASSERT(layer);
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
        QSGInternalImageNode *imageNode = static_cast<QSGInternalImageNode*>(*m_nodeIterator++);
        imageNode->setTargetRect(rect);
        imageNode->setInnerTargetRect(rect);
        imageNode->setSubSourceRect(layer->convertToNormalizedSourceRect(sourceRect));
        imageNode->setTexture(layer);
        imageNode->update();
    }

    void setupTextureContentNode(QSGTexture *texture, const QRect &rect, const QRectF &sourceRect,
                                 QSGImageNode::TextureCoordinatesTransformMode texCoordTransForm,
                                 QSGNode *) override
    {
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
        QSGImageNode *textureNode = static_cast<QSGImageNode*>(*m_nodeIterator++);
        if (textureNode->texture() != texture) {
            // Chromium sometimes uses textures that doesn't completely fit
            // in which case the geometry needs to be recalculated even if
            // rect and src-rect matches.
            if (textureNode->texture()->textureSize() != texture->textureSize())
                textureNode->markDirty(QSGImageNode::DirtyGeometry);
            textureNode->setTexture(texture);
        }
        if (textureNode->textureCoordinatesTransform() != texCoordTransForm)
            textureNode->setTextureCoordinatesTransform(texCoordTransForm);
        if (textureNode->rect() != rect)
            textureNode->setRect(rect);
        if (textureNode->sourceRect() != sourceRect)
            textureNode->setSourceRect(sourceRect);
        if (textureNode->filtering() != texture->filtering())
            textureNode->setFiltering(texture->filtering());
    }
    void setupSolidColorNode(const QRect &rect, const QColor &color, QSGNode *) override
    {
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
         QSGRectangleNode *rectangleNode = static_cast<QSGRectangleNode*>(*m_nodeIterator++);

         if (rectangleNode->rect() != rect)
             rectangleNode->setRect(rect);
         if (rectangleNode->color() != color)
             rectangleNode->setColor(color);
    }
#ifndef QT_NO_OPENGL
    void setupDebugBorderNode(QSGGeometry *geometry, QSGFlatColorMaterial *material,
                              QSGNode *) override
    {
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
        QSGGeometryNode *geometryNode = static_cast<QSGGeometryNode*>(*m_nodeIterator++);

        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);
    }

    void setupYUVVideoNode(QSGTexture *, QSGTexture *, QSGTexture *, QSGTexture *,
                           const QRectF &, const QRectF &, const QSizeF &, const QSizeF &,
                           gfx::ColorSpace, float, float, const QRectF &,
                           QSGNode *) override
    {
        Q_UNREACHABLE();
    }
#ifdef GL_OES_EGL_image_external
    void setupStreamVideoNode(MailboxTexture *, const QRectF &,
                              const QMatrix4x4 &, QSGNode *) override
    {
        Q_UNREACHABLE();
    }
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL

private:
    QVector<QSGNode*>::iterator m_nodeIterator;
};

class DelegatedNodeTreeCreator : public DelegatedNodeTreeHandler
{
public:
    DelegatedNodeTreeCreator(QVector<QSGNode*> *sceneGraphNodes,
                             RenderWidgetHostViewQtDelegate *apiDelegate)
        : DelegatedNodeTreeHandler(sceneGraphNodes)
        , m_apiDelegate(apiDelegate)
    {
    }

    void setupRenderPassNode(QSGTexture *layer, const QRect &rect, const QRectF &sourceRect,
                             QSGNode *layerChain) override
    {
        Q_ASSERT(layer);
        // Only QSGInternalImageNode currently supports QSGLayer textures.
        QSGInternalImageNode *imageNode = m_apiDelegate->createInternalImageNode();
        imageNode->setTargetRect(rect);
        imageNode->setInnerTargetRect(rect);
        imageNode->setSubSourceRect(layer->convertToNormalizedSourceRect(sourceRect));
        imageNode->setTexture(layer);
        imageNode->update();

        layerChain->appendChildNode(imageNode);
        m_sceneGraphNodes->append(imageNode);
    }

    void setupTextureContentNode(QSGTexture *texture, const QRect &rect, const QRectF &sourceRect,
                                 QSGImageNode::TextureCoordinatesTransformMode texCoordTransForm,
                                 QSGNode *layerChain) override
    {
        QSGImageNode *textureNode = m_apiDelegate->createImageNode();
        textureNode->setTextureCoordinatesTransform(texCoordTransForm);
        textureNode->setRect(rect);
        textureNode->setSourceRect(sourceRect);
        textureNode->setTexture(texture);
        textureNode->setFiltering(texture->filtering());

        layerChain->appendChildNode(textureNode);
        m_sceneGraphNodes->append(textureNode);
    }

    void setupSolidColorNode(const QRect &rect, const QColor &color,
                             QSGNode *layerChain) override
    {
        QSGRectangleNode *rectangleNode = m_apiDelegate->createRectangleNode();
        rectangleNode->setRect(rect);
        rectangleNode->setColor(color);

        layerChain->appendChildNode(rectangleNode);
        m_sceneGraphNodes->append(rectangleNode);
    }

#ifndef QT_NO_OPENGL
    void setupDebugBorderNode(QSGGeometry *geometry, QSGFlatColorMaterial *material,
                              QSGNode *layerChain) override
    {
        QSGGeometryNode *geometryNode = new QSGGeometryNode;
        geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);

        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);

        layerChain->appendChildNode(geometryNode);
        m_sceneGraphNodes->append(geometryNode);
    }

    void setupYUVVideoNode(QSGTexture *yTexture, QSGTexture *uTexture, QSGTexture *vTexture,
                           QSGTexture *aTexture, const QRectF &yaTexCoordRect,
                           const QRectF &uvTexCoordRect, const QSizeF &yaTexSize,
                           const QSizeF &uvTexSize, gfx::ColorSpace colorspace,
                           float rMul, float rOff, const QRectF &rect,
                           QSGNode *layerChain) override
    {
        YUVVideoNode *videoNode = new YUVVideoNode(
                    yTexture,
                    uTexture,
                    vTexture,
                    aTexture,
                    yaTexCoordRect,
                    uvTexCoordRect,
                    yaTexSize,
                    uvTexSize,
                    colorspace,
                    rMul,
                    rOff);
        videoNode->setRect(rect);

        layerChain->appendChildNode(videoNode);
        m_sceneGraphNodes->append(videoNode);
    }
#ifdef GL_OES_EGL_image_external
    void setupStreamVideoNode(MailboxTexture *texture, const QRectF &rect,
                              const QMatrix4x4 &textureMatrix, QSGNode *layerChain) override
    {
        StreamVideoNode *svideoNode = new StreamVideoNode(texture, false, ExternalTarget);
        svideoNode->setRect(rect);
        svideoNode->setTextureMatrix(textureMatrix);
        layerChain->appendChildNode(svideoNode);
        m_sceneGraphNodes->append(svideoNode);
    }
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL

private:
    RenderWidgetHostViewQtDelegate *m_apiDelegate;
};


static inline QSharedPointer<QSGLayer> findRenderPassLayer(const int &id, const QVector<QPair<int, QSharedPointer<QSGLayer> > > &list)
{
    typedef QPair<int, QSharedPointer<QSGLayer> > Pair;
    for (const Pair &pair : list)
        if (pair.first == id)
            return pair.second;
    return QSharedPointer<QSGLayer>();
}

static QSGNode *buildRenderPassChain(QSGNode *chainParent)
{
    // Chromium already ordered the quads from back to front for us, however the
    // Qt scene graph layers individual geometries in their own z-range and uses
    // the depth buffer to visually stack nodes according to their item tree order.

    // This gets rid of the z component of all quads, once any x and y perspective
    // transformation has been applied to vertices not on the z=0 plane. Qt will
    // use an orthographic projection to render them.
    QSGTransformNode *zCompressNode = new QSGTransformNode;
    QMatrix4x4 zCompressMatrix;
    zCompressMatrix.scale(1, 1, 0);
    zCompressNode->setMatrix(zCompressMatrix);
    chainParent->appendChildNode(zCompressNode);
    return zCompressNode;
}

static QSGNode *buildLayerChain(QSGNode *chainParent, const viz::SharedQuadState *layerState)
{
    QSGNode *layerChain = chainParent;
    if (layerState->is_clipped) {
        RectClipNode *clipNode = new RectClipNode(toQt(layerState->clip_rect));
        layerChain->appendChildNode(clipNode);
        layerChain = clipNode;
    }
    if (!layerState->quad_to_target_transform.IsIdentity()) {
        QSGTransformNode *transformNode = new QSGTransformNode;
        transformNode->setMatrix(toQt(layerState->quad_to_target_transform.matrix()));
        layerChain->appendChildNode(transformNode);
        layerChain = transformNode;
    }
    if (layerState->opacity < 1.0) {
        QSGOpacityNode *opacityNode = new QSGOpacityNode;
        opacityNode->setOpacity(layerState->opacity);
        layerChain->appendChildNode(opacityNode);
        layerChain = opacityNode;
    }
    return layerChain;
}

#ifndef QT_NO_OPENGL
static void waitChromiumSync(gl::TransferableFence *sync)
{
    // Chromium uses its own GL bindings and stores in in thread local storage.
    // For that reason, let chromium_gpu_helper.cpp contain the producing code that will run in the Chromium
    // GPU thread, and put the sync consuming code here that will run in the QtQuick SG or GUI thread.
    switch (sync->type) {
    case gl::TransferableFence::NoSync:
        break;
    case gl::TransferableFence::EglSync:
#ifdef EGL_KHR_reusable_sync
    {
        static bool resolved = false;
        static PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR = 0;

        if (!resolved) {
            if (gl::GLSurfaceQt::HasEGLExtension("EGL_KHR_fence_sync")) {
                QOpenGLContext *context = QOpenGLContext::currentContext();
                eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)context->getProcAddress("eglClientWaitSyncKHR");
            }
            resolved = true;
        }

        if (eglClientWaitSyncKHR)
            // FIXME: Use the less wasteful eglWaitSyncKHR once we have a device that supports EGL_KHR_wait_sync.
            eglClientWaitSyncKHR(sync->egl.display, sync->egl.sync, 0, EGL_FOREVER_KHR);
    }
#endif
        break;
    case gl::TransferableFence::ArbSync:
        typedef void (QOPENGLF_APIENTRYP WaitSyncPtr)(GLsync sync, GLbitfield flags, GLuint64 timeout);
        static WaitSyncPtr glWaitSync_ = 0;
        if (!glWaitSync_) {
            QOpenGLContext *context = QOpenGLContext::currentContext();
            glWaitSync_ = (WaitSyncPtr)context->getProcAddress("glWaitSync");
            Q_ASSERT(glWaitSync_);
        }
        glWaitSync_(sync->arb.sync, 0, GL_TIMEOUT_IGNORED);
        break;
    }
}

static void deleteChromiumSync(gl::TransferableFence *sync)
{
    // Chromium uses its own GL bindings and stores in in thread local storage.
    // For that reason, let chromium_gpu_helper.cpp contain the producing code that will run in the Chromium
    // GPU thread, and put the sync consuming code here that will run in the QtQuick SG or GUI thread.
    switch (sync->type) {
    case gl::TransferableFence::NoSync:
        break;
    case gl::TransferableFence::EglSync:
#ifdef EGL_KHR_reusable_sync
    {
        static bool resolved = false;
        static PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR = 0;

        if (!resolved) {
            if (gl::GLSurfaceQt::HasEGLExtension("EGL_KHR_fence_sync")) {
                QOpenGLContext *context = QOpenGLContext::currentContext();
                eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)context->getProcAddress("eglDestroySyncKHR");
            }
            resolved = true;
        }

        if (eglDestroySyncKHR) {
            // FIXME: Use the less wasteful eglWaitSyncKHR once we have a device that supports EGL_KHR_wait_sync.
            eglDestroySyncKHR(sync->egl.display, sync->egl.sync);
            sync->reset();
        }
    }
#endif
        break;
    case gl::TransferableFence::ArbSync:
        typedef void (QOPENGLF_APIENTRYP DeleteSyncPtr)(GLsync sync);
        static DeleteSyncPtr glDeleteSync_ = 0;
        if (!glDeleteSync_) {
            QOpenGLContext *context = QOpenGLContext::currentContext();
            glDeleteSync_ = (DeleteSyncPtr)context->getProcAddress("glDeleteSync");
            Q_ASSERT(glDeleteSync_);
        }
        glDeleteSync_(sync->arb.sync);
        sync->reset();
        break;
    }
    // If Chromium was able to create a sync, we should have been able to handle its type here too.
    Q_ASSERT(!*sync);
}

MailboxTexture::MailboxTexture(const gpu::MailboxHolder &mailboxHolder, const QSize textureSize)
    : m_mailboxHolder(mailboxHolder)
    , m_textureId(0)
    , m_textureSize(textureSize)
    , m_hasAlpha(false)
    , m_target(GL_TEXTURE_2D)
#if defined(USE_OZONE)
    , m_ownsTexture(false)
#endif
{
    initializeOpenGLFunctions();

    // Assume that resources without a size will be used with a full source rect.
    // Setting a size of 1x1 will let any texture node compute a normalized source
    // rect of (0, 0) to (1, 1) while an empty texture size would set (0, 0) on all corners.
    if (m_textureSize.isEmpty())
        m_textureSize = QSize(1, 1);
}

MailboxTexture::~MailboxTexture()
{
#if defined(USE_OZONE)
   // This is rare case, where context is not shared
   // we created extra texture in current context, so
   // delete it now
   if (m_ownsTexture) {
       QOpenGLContext *currentContext = QOpenGLContext::currentContext() ;
       QOpenGLFunctions *funcs = currentContext->functions();
       GLuint id(m_textureId);
       funcs->glDeleteTextures(1, &id);
   }
#endif
}

void MailboxTexture::bind()
{
    glBindTexture(m_target, m_textureId);
#ifdef Q_OS_QNX
    if (m_target == GL_TEXTURE_EXTERNAL_OES) {
        static bool resolved = false;
        static PFNEGLSTREAMCONSUMERACQUIREKHRPROC eglStreamConsumerAcquire = 0;

        if (!resolved) {
            QOpenGLContext *context = QOpenGLContext::currentContext();
            eglStreamConsumerAcquire = (PFNEGLSTREAMCONSUMERACQUIREKHRPROC)context->getProcAddress("eglStreamConsumerAcquireKHR");
            resolved = true;
        }
        if (eglStreamConsumerAcquire)
            eglStreamConsumerAcquire(m_eglStreamData.egl_display, m_eglStreamData.egl_str_handle);
    }
#endif
}

void MailboxTexture::setTarget(GLenum target)
{
    m_target = target;
}

void MailboxTexture::fetchTexture(gpu::MailboxManager *mailboxManager)
{
    gpu::TextureBase *tex = ConsumeTexture(mailboxManager, m_target, m_mailboxHolder.mailbox);

    // The texture might already have been deleted (e.g. when navigating away from a page).
    if (tex) {
        m_textureId = service_id(tex);
#ifdef Q_OS_QNX
        if (m_target == GL_TEXTURE_EXTERNAL_OES) {
            m_eglStreamData = eglstream_connect_consumer(tex);
        }
#endif
    }
}
#endif //QT_NO_OPENGL

ResourceHolder::ResourceHolder(const viz::TransferableResource &resource)
    : m_resource(resource)
    , m_importCount(1)
{
}

QSharedPointer<QSGTexture> ResourceHolder::initTexture(bool quadNeedsBlending, RenderWidgetHostViewQtDelegate *apiDelegate)
{
    QSharedPointer<QSGTexture> texture = m_texture.toStrongRef();
    if (!texture) {
        if (m_resource.is_software) {
            Q_ASSERT(apiDelegate);
            std::unique_ptr<viz::SharedBitmap> sharedBitmap =
                    content::BrowserMainLoop::GetInstance()->GetServerSharedBitmapManager()->GetSharedBitmapFromId(
                        m_resource.size, viz::BGRA_8888, m_resource.mailbox_holder.mailbox);
            // QSG interprets QImage::hasAlphaChannel meaning that a node should enable blending
            // to draw it but Chromium keeps this information in the quads.
            // The input format is currently always Format_ARGB32_Premultiplied, so assume that all
            // alpha bytes are 0xff if quads aren't requesting blending and avoid the conversion
            // from Format_ARGB32_Premultiplied to Format_RGB32 just to get hasAlphaChannel to
            // return false.
            QImage::Format format = quadNeedsBlending ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;
            QImage image = sharedBitmap
                         ? QImage(sharedBitmap->pixels(), m_resource.size.width(), m_resource.size.height(), format)
                         : QImage(m_resource.size.width(), m_resource.size.height(), format);
            texture.reset(apiDelegate->createTextureFromImage(image.copy()));
        } else {
#ifndef QT_NO_OPENGL
            texture.reset(new MailboxTexture(m_resource.mailbox_holder, toQt(m_resource.size)));
            static_cast<MailboxTexture *>(texture.data())->setHasAlphaChannel(quadNeedsBlending);
#else
            Q_UNREACHABLE();
#endif
        }
        if (m_resource.filter == GL_NEAREST)
            texture->setFiltering(QSGTexture::Nearest);
        else if (m_resource.filter == GL_LINEAR)
            texture->setFiltering(QSGTexture::Linear);
        else {
            // Depends on qtdeclarative fix, see QTBUG-71322
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 1)
            texture->setFiltering(QSGTexture::Linear);
#else
            texture->setFiltering(QSGTexture::Nearest);
#endif
        }
        m_texture = texture;
    }
    // All quads using a resource should request the same blending state.
    Q_ASSERT(texture->hasAlphaChannel() || !quadNeedsBlending);
    return texture;
}

viz::ReturnedResource ResourceHolder::returnResource()
{
    viz::ReturnedResource returned;
    // The ResourceProvider ensures that the resource isn't used by the parent compositor's GL
    // context in the GPU process by inserting a sync point to be waited for by the child
    // compositor's GL context. We don't need this since we are triggering the delegated frame
    // ack directly from our rendering thread. At this point (in updatePaintNode) we know that
    // a frame that was compositing any of those resources has already been swapped and we thus
    // don't need to use this mechanism.
    returned.id = m_resource.id;
    returned.count = m_importCount;
    m_importCount = 0;
    return returned;
}


RectClipNode::RectClipNode(const QRectF &rect)
    : m_geometry(QSGGeometry::defaultAttributes_Point2D(), 4)
{
    QSGGeometry::updateRectGeometry(&m_geometry, rect);
    setGeometry(&m_geometry);
    setClipRect(rect);
    setIsRectangular(true);
}

DelegatedFrameNode::DelegatedFrameNode()
    : m_numPendingSyncPoints(0)
#if defined(USE_OZONE) && !defined(QT_NO_OPENGL)
    , m_contextShared(true)
#endif
{
    setFlag(UsePreprocess);
#if defined(USE_OZONE) && !defined(QT_NO_OPENGL)
    QOpenGLContext *currentContext = QOpenGLContext::currentContext() ;
    QOpenGLContext *sharedContext = qt_gl_global_share_context();
    if (currentContext && sharedContext && !QOpenGLContext::areSharing(currentContext, sharedContext)) {
        static bool allowNotSharedContextWarningShown = true;
        if (allowNotSharedContextWarningShown) {
            allowNotSharedContextWarningShown = false;
            qWarning("Context is not shared, textures will be copied between contexts.");
        }
        m_offsurface.reset(new QOffscreenSurface);
        m_offsurface->create();
        m_contextShared = false;
    }
#endif
}

DelegatedFrameNode::~DelegatedFrameNode()
{
}

void DelegatedFrameNode::preprocess()
{
#ifndef QT_NO_OPENGL
    // With the threaded render loop the GUI thread has been unlocked at this point.
    // We can now wait for the Chromium GPU thread to produce textures that will be
    // rendered on our quads and fetch the IDs from the mailboxes we were given.
    QList<MailboxTexture *> mailboxesToFetch;
    typedef QHash<unsigned, QSharedPointer<ResourceHolder> >::const_iterator ResourceHolderIterator;
    ResourceHolderIterator end = m_chromiumCompositorData->resourceHolders.constEnd();
    for (ResourceHolderIterator it = m_chromiumCompositorData->resourceHolders.constBegin(); it != end ; ++it) {
        if ((*it)->needsToFetch())
            mailboxesToFetch.append(static_cast<MailboxTexture *>((*it)->texture()));
    }

    if (!mailboxesToFetch.isEmpty())
        fetchAndSyncMailboxes(mailboxesToFetch);
#endif

    // Then render any intermediate RenderPass in order.
    typedef QPair<int, QSharedPointer<QSGLayer> > Pair;
    for (const Pair &pair : qAsConst(m_sgObjects.renderPassLayers)) {
        // The layer is non-live, request a one-time update here.
        pair.second->scheduleUpdate();
        // Proceed with the actual update.
        pair.second->updateTexture();
    }
}

static bool areSharedQuadStatesEqual(const viz::SharedQuadState *layerState,
                                     const viz::SharedQuadState *prevLayerState)
{
    if (layerState->sorting_context_id != 0 || prevLayerState->sorting_context_id != 0)
        return false;
    if (layerState->is_clipped != prevLayerState->is_clipped
        || layerState->clip_rect != prevLayerState->clip_rect)
        return false;
    if (layerState->quad_to_target_transform != prevLayerState->quad_to_target_transform)
        return false;
    return qFuzzyCompare(layerState->opacity, prevLayerState->opacity);
}

// Compares if the frame data that we got from the Chromium Compositor is
// *structurally* equivalent to the one of the previous frame.
// If it is, we will just reuse and update the old nodes where necessary.
static bool areRenderPassStructuresEqual(viz::CompositorFrame *frameData,
                                         viz::CompositorFrame *previousFrameData, const gfx::Rect &viewportRect)
{
    if (!previousFrameData)
        return false;

    if (previousFrameData->render_pass_list.size() != frameData->render_pass_list.size())
        return false;

    auto rootRenderPass = frameData->render_pass_list.back().get();

    for (unsigned i = 0; i < frameData->render_pass_list.size(); ++i) {
        viz::RenderPass *newPass = frameData->render_pass_list.at(i).get();
        viz::RenderPass *prevPass = previousFrameData->render_pass_list.at(i).get();

        if (newPass->id != prevPass->id)
            return false;

        if (newPass->quad_list.size() != prevPass->quad_list.size())
            return false;

        auto &&scissorRect = newPass == rootRenderPass ? viewportRect : newPass->output_rect;
        auto &&prevScissorRect = newPass == rootRenderPass ? viewportRect : prevPass->output_rect;
        if (newPass != rootRenderPass) {
            if (scissorRect.IsEmpty() != prevScissorRect.IsEmpty())
                return false;
        }

        viz::QuadList::ConstBackToFrontIterator it = newPass->quad_list.BackToFrontBegin();
        viz::QuadList::ConstBackToFrontIterator end = newPass->quad_list.BackToFrontEnd();
        viz::QuadList::ConstBackToFrontIterator prevIt = prevPass->quad_list.BackToFrontBegin();
        viz::QuadList::ConstBackToFrontIterator prevEnd = prevPass->quad_list.BackToFrontEnd();
        for (; it != end && prevIt != prevEnd; ++it, ++prevIt) {
            const viz::DrawQuad *quad = *it;
            const viz::DrawQuad *prevQuad = *prevIt;
            if (quad->material != prevQuad->material)
                return false;
#ifndef QT_NO_OPENGL
            if (quad->material == viz::DrawQuad::YUV_VIDEO_CONTENT)
                return false;
#ifdef GL_OES_EGL_image_external
            if (quad->material == viz::DrawQuad::STREAM_VIDEO_CONTENT)
                return false;
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL

            auto sharedState = quad->shared_quad_state, prevSharedState = prevQuad->shared_quad_state;
            if (!areSharedQuadStatesEqual(sharedState, prevSharedState))
                return false;

            auto &&transform = quad->shared_quad_state->quad_to_target_transform;

            gfx::Rect targetRect1 = cc::MathUtil::MapEnclosingClippedRect(transform, quad->visible_rect);
            if (sharedState->is_clipped)
                targetRect1.Intersect(sharedState->clip_rect);
            targetRect1.Intersect(scissorRect);

            gfx::Rect targetRect2 = cc::MathUtil::MapEnclosingClippedRect(transform, prevQuad->visible_rect);
            if (prevSharedState->is_clipped)
                targetRect2.Intersect(prevSharedState->clip_rect);
            targetRect2.Intersect(scissorRect);

            if (targetRect1.IsEmpty() != targetRect2.IsEmpty())
                return false;
        }
    }
    return true;
}

void DelegatedFrameNode::commit(ChromiumCompositorData *chromiumCompositorData,
                                std::vector<viz::ReturnedResource> *resourcesToRelease,
                                RenderWidgetHostViewQtDelegate *apiDelegate)
{
    m_chromiumCompositorData = chromiumCompositorData;
    viz::CompositorFrame* frameData = &m_chromiumCompositorData->frameData;
    if (frameData->render_pass_list.empty())
        return;

    // DelegatedFrameNode is a transform node only for the purpose of
    // countering the scale of devicePixel-scaled tiles when rendering them
    // to the final surface.
    QMatrix4x4 matrix;
    const float devicePixelRatio = m_chromiumCompositorData->frameDevicePixelRatio;
    matrix.scale(1 / devicePixelRatio, 1 / devicePixelRatio);
    if (QSGTransformNode::matrix() != matrix)
        setMatrix(matrix);

    QHash<unsigned, QSharedPointer<ResourceHolder> > resourceCandidates;
    qSwap(m_chromiumCompositorData->resourceHolders, resourceCandidates);

    // A frame's resource_list only contains the new resources to be added to the scene. Quads can
    // still reference resources that were added in previous frames. Add them to the list of
    // candidates to be picked up by quads, it's then our responsibility to return unused resources
    // to the producing child compositor.
    for (unsigned i = 0; i < frameData->resource_list.size(); ++i) {
        const viz::TransferableResource &res = frameData->resource_list.at(i);
        if (QSharedPointer<ResourceHolder> resource = resourceCandidates.value(res.id))
            resource->incImportCount();
        else
            resourceCandidates[res.id] = QSharedPointer<ResourceHolder>(new ResourceHolder(res));
    }

    frameData->resource_list.clear();

    // The RenderPasses list is actually a tree where a parent RenderPass is connected
    // to its dependencies through a RenderPassId reference in one or more RenderPassQuads.
    // The list is already ordered with intermediate RenderPasses placed before their
    // parent, with the last one in the list being the root RenderPass, the one
    // that we displayed to the user.
    // All RenderPasses except the last one are rendered to an FBO.
    viz::RenderPass *rootRenderPass = frameData->render_pass_list.back().get();

    const QSizeF viewportSizeInPt = apiDelegate->screenRect().size();
    const QSizeF viewportSizeF = viewportSizeInPt * devicePixelRatio;
    const QSize viewportSize(std::ceil(viewportSizeF.width()), std::ceil(viewportSizeF.height()));
    gfx::Rect viewportRect(toGfx(viewportSize));
    viewportRect += rootRenderPass->output_rect.OffsetFromOrigin();

    // We first compare if the render passes from the previous frame data are structurally
    // equivalent to the render passes in the current frame data. If they are, we are going
    // to reuse the old nodes. Otherwise, we will delete the old nodes and build a new tree.
    //
    // Additionally, because we clip (i.e. don't build scene graph nodes for) quads outside
    // of the visible area, we also have to rebuild the tree whenever the window is resized.
    const bool buildNewTree =
        m_sceneGraphNodes.empty() ||
        viewportSize != m_previousViewportSize ||
        !areRenderPassStructuresEqual(frameData, &m_chromiumCompositorData->previousFrameData, viewportRect);

    QScopedPointer<DelegatedNodeTreeHandler> nodeHandler;
    m_chromiumCompositorData->previousFrameData = viz::CompositorFrame();
    SGObjects previousSGObjects;
    QVector<QSharedPointer<QSGTexture> > textureStrongRefs;
    if (buildNewTree) {
        // Keep the old objects in scope to hold a ref on layers, resources and textures
        // that we can re-use. Destroy the remaining objects before returning.
        qSwap(m_sgObjects, previousSGObjects);
        // Discard the scene graph nodes from the previous frame.
        while (QSGNode *oldChain = firstChild())
            delete oldChain;
        m_sceneGraphNodes.clear();
        nodeHandler.reset(new DelegatedNodeTreeCreator(&m_sceneGraphNodes, apiDelegate));
    } else {
        // Save the texture strong refs so they only go out of scope when the method returns and
        // the new vector of texture strong refs has been filled.
        qSwap(m_sgObjects.textureStrongRefs, textureStrongRefs);
        nodeHandler.reset(new DelegatedNodeTreeUpdater(&m_sceneGraphNodes));
    }

    for (unsigned i = 0; i < frameData->render_pass_list.size(); ++i) {
        viz::RenderPass *pass = frameData->render_pass_list.at(i).get();

        QSGNode *renderPassParent = 0;
        auto &&scissorRect = pass != rootRenderPass ? pass->output_rect : viewportRect;
        if (pass != rootRenderPass) {
            QSharedPointer<QSGLayer> rpLayer;
            if (buildNewTree) {
                rpLayer = findRenderPassLayer(pass->id, previousSGObjects.renderPassLayers);
                if (!rpLayer) {
                    rpLayer = QSharedPointer<QSGLayer>(apiDelegate->createLayer());
                    // Avoid any premature texture update since we need to wait
                    // for the GPU thread to produce the dependent resources first.
                    rpLayer->setLive(false);
                }
                QSharedPointer<QSGRootNode> rootNode(new QSGRootNode);
                rpLayer->setItem(rootNode.data());
                m_sgObjects.renderPassLayers.append(QPair<int,
                                                    QSharedPointer<QSGLayer> >(pass->id, rpLayer));
                m_sgObjects.renderPassRootNodes.append(rootNode);
                renderPassParent = rootNode.data();
            } else
                rpLayer = findRenderPassLayer(pass->id, m_sgObjects.renderPassLayers);

            rpLayer->setRect(toQt(pass->output_rect));
            rpLayer->setSize(toQt(pass->output_rect.size()));
            rpLayer->setFormat(pass->has_transparent_background ? GL_RGBA : GL_RGB);
            rpLayer->setHasMipmaps(pass->generate_mipmap);
            rpLayer->setMirrorVertical(true);
        } else {
            renderPassParent = this;
        }

        if (scissorRect.IsEmpty()) {
            holdResources(pass, resourceCandidates);
            continue;
        }

        QSGNode *renderPassChain = nullptr;
        if (buildNewTree)
            renderPassChain = buildRenderPassChain(renderPassParent);

        base::circular_deque<std::unique_ptr<viz::DrawPolygon>> polygonQueue;
        int nextPolygonId = 0;
        int currentSortingContextId = 0;
        const viz::SharedQuadState *currentLayerState = nullptr;
        QSGNode *currentLayerChain = nullptr;
        const auto quadListBegin = pass->quad_list.BackToFrontBegin();
        const auto quadListEnd = pass->quad_list.BackToFrontEnd();
        for (auto it = quadListBegin; it != quadListEnd; ++it) {
            const viz::DrawQuad *quad = *it;
            const viz::SharedQuadState *quadState = quad->shared_quad_state;

            gfx::Rect targetRect =
                cc::MathUtil::MapEnclosingClippedRect(quadState->quad_to_target_transform,
                                                      quad->visible_rect);
            if (quadState->is_clipped)
                targetRect.Intersect(quadState->clip_rect);
            targetRect.Intersect(scissorRect);
            if (targetRect.IsEmpty()) {
                holdResources(quad, resourceCandidates);
                continue;
            }

            if (quadState->sorting_context_id != currentSortingContextId) {
                flushPolygons(&polygonQueue, renderPassChain,
                              nodeHandler.data(), resourceCandidates, apiDelegate);
                currentSortingContextId = quadState->sorting_context_id;
            }

            if (currentSortingContextId != 0) {
                std::unique_ptr<viz::DrawPolygon> polygon(
                    new viz::DrawPolygon(
                        quad,
                        gfx::RectF(quad->visible_rect),
                        quadState->quad_to_target_transform,
                        nextPolygonId++));
                if (polygon->points().size() > 2u)
                    polygonQueue.push_back(std::move(polygon));
                continue;
            }

            if (renderPassChain && currentLayerState != quadState) {
                currentLayerState = quadState;
                currentLayerChain = buildLayerChain(renderPassChain, quadState);
            }

            handleQuad(quad, currentLayerChain,
                       nodeHandler.data(), resourceCandidates, apiDelegate);
        }
        flushPolygons(&polygonQueue, renderPassChain,
                      nodeHandler.data(), resourceCandidates, apiDelegate);
    }
    // Send resources of remaining candidates back to the child compositors so that
    // they can be freed or reused.
    typedef QHash<unsigned, QSharedPointer<ResourceHolder> >::const_iterator
            ResourceHolderIterator;

    ResourceHolderIterator end = resourceCandidates.constEnd();
    for (ResourceHolderIterator it = resourceCandidates.constBegin(); it != end ; ++it)
        resourcesToRelease->push_back((*it)->returnResource());

    m_previousViewportSize = viewportSize;
}

void DelegatedFrameNode::flushPolygons(
    base::circular_deque<std::unique_ptr<viz::DrawPolygon>> *polygonQueue,
    QSGNode *renderPassChain,
    DelegatedNodeTreeHandler *nodeHandler,
    QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    if (polygonQueue->empty())
        return;

    const auto actionHandler = [&](viz::DrawPolygon *polygon) {
        const viz::DrawQuad *quad = polygon->original_ref();
        const viz::SharedQuadState *quadState = quad->shared_quad_state;

        QSGNode *currentLayerChain = nullptr;
        if (renderPassChain)
            currentLayerChain = buildLayerChain(renderPassChain, quad->shared_quad_state);

        gfx::Transform inverseTransform;
        bool invertible = quadState->quad_to_target_transform.GetInverse(&inverseTransform);
        DCHECK(invertible);
        polygon->TransformToLayerSpace(inverseTransform);

        handlePolygon(polygon, currentLayerChain,
                      nodeHandler, resourceCandidates, apiDelegate);
    };

    viz::BspTree(polygonQueue).TraverseWithActionHandler(&actionHandler);
}

void DelegatedFrameNode::handlePolygon(
    const viz::DrawPolygon *polygon,
    QSGNode *currentLayerChain,
    DelegatedNodeTreeHandler *nodeHandler,
    QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    const viz::DrawQuad *quad = polygon->original_ref();

    if (!polygon->is_split()) {
        handleQuad(quad, currentLayerChain,
                   nodeHandler, resourceCandidates, apiDelegate);
    } else {
        std::vector<gfx::QuadF> clipRegionList;
        polygon->ToQuads2D(&clipRegionList);
        for (const auto & clipRegion : clipRegionList)
            handleClippedQuad(quad, clipRegion, currentLayerChain,
                              nodeHandler, resourceCandidates, apiDelegate);
    }
}

void DelegatedFrameNode::handleClippedQuad(
    const viz::DrawQuad *quad,
    const gfx::QuadF &clipRegion,
    QSGNode *currentLayerChain,
    DelegatedNodeTreeHandler *nodeHandler,
    QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    if (currentLayerChain) {
        auto clipGeometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        auto clipGeometryVertices = clipGeometry->vertexDataAsPoint2D();
        clipGeometryVertices[0].set(clipRegion.p1().x(), clipRegion.p1().y());
        clipGeometryVertices[1].set(clipRegion.p2().x(), clipRegion.p2().y());
        clipGeometryVertices[2].set(clipRegion.p4().x(), clipRegion.p4().y());
        clipGeometryVertices[3].set(clipRegion.p3().x(), clipRegion.p3().y());
        auto clipNode = new QSGClipNode;
        clipNode->setGeometry(clipGeometry);
        clipNode->setIsRectangular(false);
        clipNode->setFlag(QSGNode::OwnsGeometry);
        currentLayerChain->appendChildNode(clipNode);
        currentLayerChain = clipNode;
    }
    handleQuad(quad, currentLayerChain,
               nodeHandler, resourceCandidates, apiDelegate);
}

void DelegatedFrameNode::handleQuad(
    const viz::DrawQuad *quad,
    QSGNode *currentLayerChain,
    DelegatedNodeTreeHandler *nodeHandler,
    QHash<unsigned, QSharedPointer<ResourceHolder> > &resourceCandidates,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    switch (quad->material) {
    case viz::DrawQuad::RENDER_PASS: {
        const viz::RenderPassDrawQuad *renderPassQuad = viz::RenderPassDrawQuad::MaterialCast(quad);
        if (!renderPassQuad->mask_texture_size.IsEmpty()) {
            ResourceHolder *resource = findAndHoldResource(renderPassQuad->mask_resource_id(), resourceCandidates);
            Q_UNUSED(resource); // FIXME: QTBUG-67652
        }
        QSGLayer *layer =
            findRenderPassLayer(renderPassQuad->render_pass_id, m_sgObjects.renderPassLayers).data();

        if (layer)
            nodeHandler->setupRenderPassNode(layer, toQt(quad->rect), toQt(renderPassQuad->tex_coord_rect), currentLayerChain);

        break;
    }
    case viz::DrawQuad::TEXTURE_CONTENT: {
        const viz::TextureDrawQuad *tquad = viz::TextureDrawQuad::MaterialCast(quad);
        ResourceHolder *resource = findAndHoldResource(tquad->resource_id(), resourceCandidates);
        QSGTexture *texture =
            initAndHoldTexture(resource, quad->ShouldDrawWithBlending(), apiDelegate);
        QSizeF textureSize;
        if (texture)
            textureSize = texture->textureSize();
        gfx::RectF uv_rect =
            gfx::ScaleRect(gfx::BoundingRect(tquad->uv_top_left, tquad->uv_bottom_right),
                           textureSize.width(), textureSize.height());

        nodeHandler->setupTextureContentNode(
            texture, toQt(quad->rect), toQt(uv_rect),
            tquad->y_flipped ? QSGImageNode::MirrorVertically : QSGImageNode::NoTransform,
            currentLayerChain);
        break;
    }
    case viz::DrawQuad::SOLID_COLOR: {
        const viz::SolidColorDrawQuad *scquad = viz::SolidColorDrawQuad::MaterialCast(quad);
        // Qt only supports MSAA and this flag shouldn't be needed.
        // If we ever want to use QSGRectangleNode::setAntialiasing for this we should
        // try to see if we can do something similar for tile quads first.
        Q_UNUSED(scquad->force_anti_aliasing_off);
        nodeHandler->setupSolidColorNode(toQt(quad->rect), toQt(scquad->color), currentLayerChain);
        break;
#ifndef QT_NO_OPENGL
    }
    case viz::DrawQuad::DEBUG_BORDER: {
        const viz::DebugBorderDrawQuad *dbquad = viz::DebugBorderDrawQuad::MaterialCast(quad);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        geometry->setDrawingMode(GL_LINE_LOOP);
        geometry->setLineWidth(dbquad->width);
        // QSGGeometry::updateRectGeometry would actually set the
        // corners in the following order:
        // top-left, bottom-left, top-right, bottom-right, leading to a nice criss cross,
        // instead of having a closed loop.
        const gfx::Rect &r(dbquad->rect);
        geometry->vertexDataAsPoint2D()[0].set(r.x(), r.y());
        geometry->vertexDataAsPoint2D()[1].set(r.x() + r.width(), r.y());
        geometry->vertexDataAsPoint2D()[2].set(r.x() + r.width(), r.y() + r.height());
        geometry->vertexDataAsPoint2D()[3].set(r.x(), r.y() + r.height());

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
        material->setColor(toQt(dbquad->color));

        nodeHandler->setupDebugBorderNode(geometry, material, currentLayerChain);
        break;
#endif
    }
    case viz::DrawQuad::TILED_CONTENT: {
        const viz::TileDrawQuad *tquad = viz::TileDrawQuad::MaterialCast(quad);
        ResourceHolder *resource = findAndHoldResource(tquad->resource_id(), resourceCandidates);
        nodeHandler->setupTextureContentNode(
            initAndHoldTexture(resource, quad->ShouldDrawWithBlending(), apiDelegate),
            toQt(quad->rect), toQt(tquad->tex_coord_rect),
            QSGImageNode::NoTransform, currentLayerChain);
        break;
#ifndef QT_NO_OPENGL
    }
    case viz::DrawQuad::YUV_VIDEO_CONTENT: {
        const viz::YUVVideoDrawQuad *vquad = viz::YUVVideoDrawQuad::MaterialCast(quad);
        ResourceHolder *yResource =
            findAndHoldResource(vquad->y_plane_resource_id(), resourceCandidates);
        ResourceHolder *uResource =
            findAndHoldResource(vquad->u_plane_resource_id(), resourceCandidates);
        ResourceHolder *vResource =
            findAndHoldResource(vquad->v_plane_resource_id(), resourceCandidates);
        ResourceHolder *aResource = 0;
        // This currently requires --enable-vp8-alpha-playback and
        // needs a video with alpha data to be triggered.
        if (vquad->a_plane_resource_id())
            aResource = findAndHoldResource(vquad->a_plane_resource_id(), resourceCandidates);

        nodeHandler->setupYUVVideoNode(
            initAndHoldTexture(yResource, quad->ShouldDrawWithBlending()),
            initAndHoldTexture(uResource, quad->ShouldDrawWithBlending()),
            initAndHoldTexture(vResource, quad->ShouldDrawWithBlending()),
            aResource ? initAndHoldTexture(aResource, quad->ShouldDrawWithBlending()) : 0,
            toQt(vquad->ya_tex_coord_rect), toQt(vquad->uv_tex_coord_rect),
            toQt(vquad->ya_tex_size), toQt(vquad->uv_tex_size), vquad->video_color_space,
            vquad->resource_multiplier, vquad->resource_offset, toQt(quad->rect),
            currentLayerChain);
        break;
#ifdef GL_OES_EGL_image_external
    }
    case viz::DrawQuad::STREAM_VIDEO_CONTENT: {
        const viz::StreamVideoDrawQuad *squad = viz::StreamVideoDrawQuad::MaterialCast(quad);
        ResourceHolder *resource = findAndHoldResource(squad->resource_id(), resourceCandidates);
        MailboxTexture *texture = static_cast<MailboxTexture *>(
            initAndHoldTexture(resource, quad->ShouldDrawWithBlending()));
        // since this is not default TEXTURE_2D type
        texture->setTarget(GL_TEXTURE_EXTERNAL_OES);

        nodeHandler->setupStreamVideoNode(texture, toQt(squad->rect), toQt(squad->matrix.matrix()),
                                          currentLayerChain);
        break;
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL
    }
    case viz::DrawQuad::SURFACE_CONTENT:
        Q_UNREACHABLE();
    default:
        qWarning("Unimplemented quad material: %d", quad->material);
    }
}

ResourceHolder *DelegatedFrameNode::findAndHoldResource(unsigned resourceId, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates)
{
    // ResourceHolders must survive when the scene graph destroys our node branch
    QSharedPointer<ResourceHolder> &resource = m_chromiumCompositorData->resourceHolders[resourceId];
    if (!resource)
        resource = candidates.take(resourceId);
    Q_ASSERT(resource);
    return resource.data();
}

void DelegatedFrameNode::holdResources(const viz::DrawQuad *quad, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates)
{
    for (auto resource : quad->resources)
        findAndHoldResource(resource, candidates);
}

void DelegatedFrameNode::holdResources(const viz::RenderPass *pass, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates)
{
    for (const auto &quad : pass->quad_list)
        holdResources(quad, candidates);
}

QSGTexture *DelegatedFrameNode::initAndHoldTexture(ResourceHolder *resource, bool quadIsAllOpaque, RenderWidgetHostViewQtDelegate *apiDelegate)
{
    // QSGTextures must be destroyed in the scene graph thread as part of the QSGNode tree,
    // so we can't store them with the ResourceHolder in m_chromiumCompositorData.
    // Hold them through a QSharedPointer solely on the root DelegatedFrameNode of the web view
    // and access them through a QWeakPointer from the resource holder to find them later.
    m_sgObjects.textureStrongRefs.append(resource->initTexture(quadIsAllOpaque, apiDelegate));
    return m_sgObjects.textureStrongRefs.last().data();
}

void DelegatedFrameNode::fetchAndSyncMailboxes(QList<MailboxTexture *> &mailboxesToFetch)
{
#ifndef QT_NO_OPENGL
    QList<gl::TransferableFence> transferredFences;
    {
        QMutexLocker lock(&m_mutex);
        QVector<MailboxTexture *> mailboxesToPull;
        mailboxesToPull.reserve(mailboxesToFetch.size());

        gpu::SyncPointManager *syncPointManager = sync_point_manager();
        scoped_refptr<base::SingleThreadTaskRunner> gpuTaskRunner = gpu_task_runner();
        Q_ASSERT(m_numPendingSyncPoints == 0);
        m_numPendingSyncPoints = mailboxesToFetch.count();
        for (MailboxTexture *mailboxTexture : qAsConst(mailboxesToFetch)) {
            gpu::SyncToken &syncToken = mailboxTexture->mailboxHolder().sync_token;
            const auto task = base::Bind(&DelegatedFrameNode::pullTexture, this, mailboxTexture);
            if (!syncPointManager->WaitOutOfOrder(syncToken, std::move(task)))
                mailboxesToPull.append(mailboxTexture);
        }
        if (!mailboxesToPull.isEmpty()) {
            auto task = base::BindOnce(&DelegatedFrameNode::pullTextures, this, std::move(mailboxesToPull));
            gpuTaskRunner->PostTask(FROM_HERE, std::move(task));
        }

        m_mailboxesFetchedWaitCond.wait(&m_mutex);
        m_textureFences.swap(transferredFences);
    }

    for (gl::TransferableFence sync : qAsConst(transferredFences)) {
        // We need to wait on the fences on the Qt current context, and
        // can therefore not use GLFence routines that uses a different
        // concept of current context.
        waitChromiumSync(&sync);
        deleteChromiumSync(&sync);
    }

#if defined(USE_OZONE) && !defined(QT_NO_OPENGL)
    // Workaround when context is not shared QTBUG-48969
    // Make slow copy between two contexts.
    if (!m_contextShared) {
        QOpenGLContext *currentContext = QOpenGLContext::currentContext() ;
        QOpenGLContext *sharedContext = qt_gl_global_share_context();

        QSurface *surface = currentContext->surface();
        Q_ASSERT(m_offsurface);
        sharedContext->makeCurrent(m_offsurface.data());
        QOpenGLFunctions *funcs = sharedContext->functions();

        GLuint fbo = 0;
        funcs->glGenFramebuffers(1, &fbo);

        for (MailboxTexture *mailboxTexture : qAsConst(mailboxesToFetch)) {
            // Read texture into QImage from shared context.
            // Switch to shared context.
            sharedContext->makeCurrent(m_offsurface.data());
            funcs = sharedContext->functions();
            QImage img(mailboxTexture->textureSize(), QImage::Format_RGBA8888_Premultiplied);
            funcs->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mailboxTexture->m_textureId, 0);
            GLenum status = funcs->glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                qWarning("fbo error, skipping slow copy...");
                continue;
            }
            funcs->glReadPixels(0, 0, mailboxTexture->textureSize().width(), mailboxTexture->textureSize().height(),
                                GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

            // Restore current context.
            // Create texture from QImage in current context.
            currentContext->makeCurrent(surface);
            GLuint texture = 0;
            funcs = currentContext->functions();
            funcs->glGenTextures(1, &texture);
            funcs->glBindTexture(GL_TEXTURE_2D, texture);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mailboxTexture->textureSize().width(), mailboxTexture->textureSize().height(), 0,
                                GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
            mailboxTexture->m_textureId = texture;
            mailboxTexture->m_ownsTexture = true;
        }
        // Cleanup allocated resources
        sharedContext->makeCurrent(m_offsurface.data());
        funcs = sharedContext->functions();
        funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        funcs->glDeleteFramebuffers(1, &fbo);
        currentContext->makeCurrent(surface);
    }
#endif
#else
    Q_UNUSED(mailboxesToFetch)
#endif //QT_NO_OPENGL
}


void DelegatedFrameNode::pullTextures(DelegatedFrameNode *frameNode, const QVector<MailboxTexture *> textures)
{
#ifndef QT_NO_OPENGL
    gpu::MailboxManager *mailboxManager = mailbox_manager();
    for (MailboxTexture *texture : textures) {
        gpu::SyncToken &syncToken = texture->mailboxHolder().sync_token;
        if (syncToken.HasData())
            mailboxManager->PullTextureUpdates(syncToken);
        texture->fetchTexture(mailboxManager);
        --frameNode->m_numPendingSyncPoints;
    }

    fenceAndUnlockQt(frameNode);
#else
    Q_UNUSED(frameNode)
    Q_UNUSED(textures)
#endif
}

void DelegatedFrameNode::pullTexture(DelegatedFrameNode *frameNode, MailboxTexture *texture)
{
#ifndef QT_NO_OPENGL
    gpu::MailboxManager *mailboxManager = mailbox_manager();
    gpu::SyncToken &syncToken = texture->mailboxHolder().sync_token;
    if (syncToken.HasData())
        mailboxManager->PullTextureUpdates(syncToken);
    texture->fetchTexture(mailboxManager);
    --frameNode->m_numPendingSyncPoints;

    fenceAndUnlockQt(frameNode);
#else
    Q_UNUSED(frameNode)
    Q_UNUSED(texture)
#endif
}

void DelegatedFrameNode::fenceAndUnlockQt(DelegatedFrameNode *frameNode)
{
#ifndef QT_NO_OPENGL
    if (!!gl::GLContext::GetCurrent() && gl::GLFence::IsSupported()) {
        // Create a fence on the Chromium GPU-thread and context
        std::unique_ptr<gl::GLFence> fence = gl::GLFence::Create();
        // But transfer it to something generic since we need to read it using Qt's OpenGL.
        frameNode->m_textureFences.append(fence->Transfer());
    }
    if (frameNode->m_numPendingSyncPoints == 0)
        base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, base::Bind(&DelegatedFrameNode::unlockQt, frameNode));
#else
    Q_UNUSED(frameNode)
#endif
}

void DelegatedFrameNode::unlockQt(DelegatedFrameNode *frameNode)
{
    QMutexLocker lock(&frameNode->m_mutex);
    // Signal preprocess() the textures are ready
    frameNode->m_mailboxesFetchedWaitCond.wakeOne();
}

} // namespace QtWebEngineCore
