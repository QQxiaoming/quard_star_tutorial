/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
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

#include "scene3drenderer_p.h"

#include <Qt3DCore/qaspectengine.h>
#include <Qt3DRender/qrenderaspect.h>
#include <QtCore/qthread.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglframebufferobject.h>
#include <QtQuick/qquickwindow.h>

#include <Qt3DRender/private/qrenderaspect_p.h>
#include <Qt3DCore/private/qaspectengine_p.h>

#include <scene3ditem_p.h>
#include <scene3dlogging_p.h>
#include <scene3dsgnode_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace {

inline QMetaMethod setItemAreaAndDevicePixelRatioMethod()
{
    const int idx = Scene3DItem::staticMetaObject.indexOfMethod("setItemAreaAndDevicePixelRatio(QSize,qreal)");
    Q_ASSERT(idx != -1);
    return Scene3DItem::staticMetaObject.method(idx);
}

} // anonymous

class ContextSaver
{
public:
    explicit ContextSaver(QOpenGLContext *context = QOpenGLContext::currentContext())
        : m_context(context),
          m_surface(context ? context->surface() : nullptr)
    {
    }

    ~ContextSaver()
    {
        if (m_context && m_context->surface() != m_surface)
            m_context->makeCurrent(m_surface);
    }

    QOpenGLContext *context() const { return m_context; }
    QSurface *surface() const { return m_surface; }

private:
    QOpenGLContext * const m_context;
    QSurface * const m_surface;
};

/*!
    \class Qt3DRender::Scene3DRenderer
    \internal

    \brief The Scene3DRenderer class takes care of rendering a Qt3D scene
    within a Framebuffer object to be used by the QtQuick 2 renderer.

    The Scene3DRenderer class renders a Qt3D scene as provided by a Scene3DItem.
    It owns the aspectEngine even though it doesn't instantiate it.

    The shutdown procedure is a two steps process that goes as follow:

    \list
    \li The window is closed

    \li This triggers the windowsChanged signal which the Scene3DRenderer
    uses to perform the necessary cleanups in the QSGRenderThread (destroys
    DebugLogger ...) with the shutdown slot (queued connection).

    \li The destroyed signal of the window is also connected to the
    Scene3DRenderer. When triggered in the context of the main thread, the
    cleanup slot is called.
    \endlist

    There is an alternate shutdown procedure in case the QQuickItem is
    destroyed with an active window which can happen in the case where the
    Scene3D is used with a QtQuick Loader

    In that case the shutdown procedure goes the same except that the destroyed
    signal of the window is not called. Therefore the cleanup method is invoked
    to properly destroy the aspect engine.
 */
Scene3DRenderer::Scene3DRenderer()
    : QObject()
    , m_item(nullptr)
    , m_aspectEngine(nullptr)
    , m_renderAspect(nullptr)
    , m_multisampledFBO(nullptr)
    , m_finalFBO(nullptr)
    , m_texture(nullptr)
    , m_node(nullptr)
    , m_window(nullptr)
    , m_multisample(false) // this value is not used, will be synced from the Scene3DItem instead
    , m_lastMultisample(false)
    , m_needsShutdown(true)
    , m_blocking(false)
    , m_forceRecreate(false)
{
}

void Scene3DRenderer::init(Scene3DItem *item, Qt3DCore::QAspectEngine *aspectEngine,
                           QRenderAspect *renderAspect)
{
    m_item = item;
    m_window = m_item->window();
    m_aspectEngine = aspectEngine;
    m_renderAspect = renderAspect;
    m_needsShutdown = true;

    Q_CHECK_PTR(m_item);
    Q_CHECK_PTR(m_item->window());

    m_window = m_item->window();
    QObject::connect(m_item->window(), &QQuickWindow::afterSynchronizing, this, &Scene3DRenderer::synchronize, Qt::DirectConnection);
    QObject::connect(m_item->window(), &QQuickWindow::beforeRendering, this, &Scene3DRenderer::render, Qt::DirectConnection);
    QObject::connect(m_item->window(), &QQuickWindow::sceneGraphInvalidated, this, &Scene3DRenderer::onSceneGraphInvalidated, Qt::DirectConnection);
    // So that we can schedule the cleanup
    QObject::connect(m_item, &QQuickItem::windowChanged, this, &Scene3DRenderer::onWindowChanged, Qt::QueuedConnection);
    // Main thread -> updates the rendering window
    QObject::connect(m_item, &QQuickItem::windowChanged, this, [this] (QQuickWindow *w) {
        QMutexLocker l(&m_windowMutex);
        m_window = w;
    });

    Q_ASSERT(QOpenGLContext::currentContext());
    ContextSaver saver;
    static_cast<QRenderAspectPrivate*>(QRenderAspectPrivate::get(m_renderAspect))->renderInitialize(saver.context());
    scheduleRootEntityChange();

    const bool blockingRendermode = !qgetenv("SCENE3D_BLOCKING_RENDERMODE").isEmpty();
    m_blocking = blockingRendermode;
}

Scene3DRenderer::~Scene3DRenderer()
{
    qCDebug(Scene3D) << Q_FUNC_INFO << QThread::currentThread();
}

QOpenGLFramebufferObject *Scene3DRenderer::createMultisampledFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    int samples = QSurfaceFormat::defaultFormat().samples();
    if (samples == -1)
        samples = 4;
    format.setSamples(samples);
    return new QOpenGLFramebufferObject(size, format);
}

QOpenGLFramebufferObject *Scene3DRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    return new QOpenGLFramebufferObject(size, format);
}

void Scene3DRenderer::scheduleRootEntityChange()
{
    QMetaObject::invokeMethod(m_item, "applyRootEntityChange", Qt::QueuedConnection);
}

// Executed in the QtQuick render thread (which may even be the gui/main with QQuickWidget / RenderControl).
void Scene3DRenderer::shutdown()
{
    qCDebug(Scene3D) << Q_FUNC_INFO << QThread::currentThread();

    // In case the same item is rendered on another window reset it
    m_resetRequested = true;

    // Set to null so that subsequent calls to render
    // would return early
    m_item = nullptr;

    // Exit the simulation loop so no more jobs are asked for. Once this
    // returns it is safe to shutdown the renderer.
    if (m_aspectEngine) {
        auto engineD = Qt3DCore::QAspectEnginePrivate::get(m_aspectEngine);
        engineD->exitSimulationLoop();
    }

    // Shutdown the Renderer Aspect while the OpenGL context
    // is still valid
    if (m_renderAspect) {
        static_cast<QRenderAspectPrivate*>(QRenderAspectPrivate::get(m_renderAspect))->renderShutdown();
        m_renderAspect = nullptr;
    }
    m_aspectEngine = nullptr;
    m_finalFBO.reset();
    m_multisampledFBO.reset();
}

// QtQuick render thread (which may also be the gui/main thread with QQuickWidget / RenderControl)
void Scene3DRenderer::onSceneGraphInvalidated()
{
    qCDebug(Scene3D) << Q_FUNC_INFO << QThread::currentThread();
    if (m_needsShutdown) {
        m_needsShutdown = false;
        shutdown();
    }
}

void Scene3DRenderer::onWindowChanged(QQuickWindow *w)
{
    qCDebug(Scene3D) << Q_FUNC_INFO << QThread::currentThread() << w;
    if (!w) {
        if (m_needsShutdown) {
            m_needsShutdown = false;
            shutdown();
        }
    }
}

void Scene3DRenderer::synchronize()
{
    if (m_item && m_window) {
        m_multisample = m_item->multisample();

        if (m_aspectEngine->rootEntity() != m_item->entity()) {
            scheduleRootEntityChange();
        }

        const QSize boundingRectSize = m_item->boundingRect().size().toSize();
        const QSize currentSize = boundingRectSize * m_window->effectiveDevicePixelRatio();
        const bool sizeHasChanged = currentSize != m_lastSize;
        const bool multisampleHasChanged = m_multisample != m_lastMultisample;
        m_forceRecreate = sizeHasChanged || multisampleHasChanged;

        if (sizeHasChanged) {
            static const QMetaMethod setItemAreaAndDevicePixelRatio = setItemAreaAndDevicePixelRatioMethod();
            setItemAreaAndDevicePixelRatio.invoke(m_item, Qt::QueuedConnection, Q_ARG(QSize, boundingRectSize),
                                                  Q_ARG(qreal, m_window->effectiveDevicePixelRatio()));
        }

        // Store the current size as a comparison
        // point for the next frame
        m_lastSize = currentSize;
        m_lastMultisample = m_multisample;
    }
}

void Scene3DRenderer::setSGNode(Scene3DSGNode *node)
{
    m_node = node;
    if (!m_texture.isNull())
        node->setTexture(m_texture.data());
}

void Scene3DRenderer::render()
{
    QMutexLocker l(&m_windowMutex);
    // Lock to ensure the window doesn't change while we are rendering
    if (!m_window)
        return;

    ContextSaver saver;

    // The OpenGL state may be dirty from the previous QtQuick nodes, so reset
    // it here to give Qt3D the clean state it expects
    m_window->resetOpenGLState();

    // Rebuild FBO and textures if never created or a resize has occurred
    if ((m_multisampledFBO.isNull() || m_forceRecreate) && m_multisample) {
        m_multisampledFBO.reset(createMultisampledFramebufferObject(m_lastSize));
        if (m_multisampledFBO->format().samples() == 0 || !QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) {
            m_multisample = false;
            m_multisampledFBO.reset(nullptr);
        }
    }

    if (m_finalFBO.isNull() || m_forceRecreate) {
        m_finalFBO.reset(createFramebufferObject(m_lastSize));
        m_texture.reset(m_window->createTextureFromId(m_finalFBO->texture(), m_finalFBO->size(), QQuickWindow::TextureHasAlphaChannel));
        m_node->setTexture(m_texture.data());
    }

    // Bind FBO
    if (m_multisample) //Only try to use MSAA when available
        m_multisampledFBO->bind();
    else
        m_finalFBO->bind();

    // Render Qt3D Scene
    static_cast<QRenderAspectPrivate*>(QRenderAspectPrivate::get(m_renderAspect))->renderSynchronous(m_blocking);

    // We may have called doneCurrent() so restore the context if the rendering surface was changed
    // Note: keep in mind that the ContextSave also restores the surface when destroyed
    if (saver.context()->surface() != saver.surface())
        saver.context()->makeCurrent(saver.surface());

    if (m_multisample) {
        // Blit multisampled FBO with non multisampled FBO with texture attachment
        const QRect dstRect(QPoint(0, 0), m_finalFBO->size());
        const QRect srcRect(QPoint(0, 0), m_multisampledFBO->size());
        QOpenGLFramebufferObject::blitFramebuffer(m_finalFBO.data(), dstRect,
                                                  m_multisampledFBO.data(), srcRect,
                                                  GL_COLOR_BUFFER_BIT,
                                                  GL_NEAREST,
                                                  0, 0,
                                                  QOpenGLFramebufferObject::DontRestoreFramebufferBinding);
    }

    // Restore QtQuick FBO
    QOpenGLFramebufferObject::bindDefault();

    // Reset the state used by the Qt Quick scenegraph to avoid any
    // interference when rendering the rest of the UI.
    m_window->resetOpenGLState();

    // Mark material as dirty to request a new frame
    m_node->markDirty(QSGNode::DirtyMaterial);

    // Request next frame
    m_window->update();
}

} // namespace Qt3DRender

QT_END_NAMESPACE
