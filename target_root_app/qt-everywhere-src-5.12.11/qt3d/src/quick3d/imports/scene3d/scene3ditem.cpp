/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
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

#include "scene3ditem_p.h"

#include <Qt3DCore/qt3dcore_global.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/QAspectEngine>

#if QT_CONFIG(qt3d_input)
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/qinputsettings.h>
#endif

#if QT_CONFIG(qt3d_logic)
#include <Qt3DLogic/qlogicaspect.h>
#endif

#if QT_CONFIG(qt3d_animation)
#include <Qt3DAnimation/qanimationaspect.h>
#endif

#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/qcamera.h>
#include <Qt3DRender/qrendersurfaceselector.h>

#include <QtGui/qguiapplication.h>
#include <QtGui/qoffscreensurface.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qquickrendercontrol.h>

#include <Qt3DRender/private/qrendersurfaceselector_p.h>
#include <Qt3DRender/private/qrenderaspect_p.h>
#include <scene3dlogging_p.h>
#include <scene3drenderer_p.h>
#include <scene3dsgnode_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

/*!
    \class Qt3DRender::Scene3DItem
    \internal

    \brief The Scene3DItem class is a QQuickItem subclass used to integrate
    a Qt3D scene into a QtQuick 2 scene.

    The Scene3DItem class renders a Qt3D scene, provided by a Qt3DCore::QEntity
    into a multisampled Framebuffer object that is later blitted into a
    non-multisampled Framebuffer object to be then rendered through the use of a
    Qt3DCore::Scene3DSGNode with premultiplied alpha.
 */

/*!
    \qmltype Scene3D
    \inherits Item
    \inqmlmodule QtQuick.Scene3D

    \preliminary

    \brief The Scene3D type is used to integrate a Qt3D scene into a QtQuick 2
    scene.

    The Scene3D type renders a Qt3D scene, provided by an \l Entity, into a
    multisampled Framebuffer object. This object is later blitted into a
    non-multisampled Framebuffer object, which is then rendered with
    premultiplied alpha. If multisampling is not required, it can be avoided
    by setting the \l multisample property to \c false. In this case
    Scene3D will render directly into the non-multisampled Framebuffer object.

    If the scene to be rendered includes non-opaque materials, you may need to
    modify those materials with custom blend arguments in order for them to be
    rendered correctly. For example, if working with a \l PhongAlphaMaterial and
    a scene with an opaque clear color, you will likely want to add:

    \qml
    sourceAlphaArg: BlendEquationArguments.Zero
    destinationAlphaArg: BlendEquationArguments.One
    \endqml

    to that material.
 */
Scene3DItem::Scene3DItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_entity(nullptr)
    , m_aspectEngine(new Qt3DCore::QAspectEngine())
    , m_renderAspect(nullptr)
    , m_aspectToDelete(nullptr)
    , m_renderer(nullptr)
    , m_multisample(true)
    , m_cameraAspectRatioMode(AutomaticAspectRatio)
    , m_dummySurface(nullptr)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::MouseButtonMask);
    // TO DO: register the event source in the main thread
}

Scene3DItem::~Scene3DItem()
{
    // When the window is closed, it first destroys all of its children. At
    // this point, Scene3DItem is destroyed but the Renderer, AspectEngine and
    // Scene3DSGNode still exist and will perform their cleanup on their own.
    m_aspectEngine->deleteLater();
    m_renderer->deleteLater();
    if (m_dummySurface)
        m_dummySurface->deleteLater();
}

/*!
    \qmlproperty list<string> Scene3D::aspects

    The list of aspects that should be registered for the 3D scene.

    For example, if the scene makes use of FrameAction, the \c "logic" aspect should be included in the list.

    The \c "render" aspect is hardwired and does not need to be explicitly listed.
*/
QStringList Scene3DItem::aspects() const
{
    return m_aspects;
}

/*!
    \qmlproperty Entity Scene3D::entity

    \default

    The root entity of the 3D scene to be displayed.
 */
Qt3DCore::QEntity *Scene3DItem::entity() const
{
    return m_entity.data();
}

void Scene3DItem::applyAspects()
{
    // Aspects are owned by the aspect engine
    for (const QString &aspect : qAsConst(m_aspects)) {
        if (aspect == QLatin1String("render")) // This one is hardwired anyway
            continue;
        if (aspect == QLatin1String("input"))  {
#if QT_CONFIG(qt3d_input)
            m_aspectEngine->registerAspect(new Qt3DInput::QInputAspect);
            continue;
#else
            qFatal("Scene3D requested the Qt 3D input aspect but Qt 3D wasn't configured to build the Qt 3D Input aspect");
#endif
        }
        if (aspect == QLatin1String("logic"))  {
#if QT_CONFIG(qt3d_logic)
            m_aspectEngine->registerAspect(new Qt3DLogic::QLogicAspect);
            continue;
#else
            qFatal("Scene3D requested the Qt 3D logic aspect but Qt 3D wasn't configured to build the Qt 3D Logic aspect");
#endif
        }
        if (aspect == QLatin1String("animation"))  {
#if QT_CONFIG(qt3d_animation)
            m_aspectEngine->registerAspect(new Qt3DAnimation::QAnimationAspect);
            continue;
#else
            qFatal("Scene3D requested the Qt 3D animation aspect but Qt 3D wasn't configured to build the Qt 3D Animation aspect");
#endif
        }
        m_aspectEngine->registerAspect(aspect);
    }
}

void Scene3DItem::setAspects(const QStringList &aspects)
{
    if (!m_aspects.isEmpty()) {
        qWarning() << "Aspects already set on the Scene3D, ignoring";
        return;
    }

    m_aspects = aspects;
    applyAspects();

    emit aspectsChanged();
}

void Scene3DItem::setEntity(Qt3DCore::QEntity *entity)
{
    if (entity == m_entity.data())
        return;

    m_entity.reset(entity);
    emit entityChanged();
}

void Scene3DItem::setCameraAspectRatioMode(CameraAspectRatioMode mode)
{
    if (m_cameraAspectRatioMode == mode)
        return;

    m_cameraAspectRatioMode = mode;
    setCameraAspectModeHelper();
    emit cameraAspectRatioModeChanged(mode);
}

void Scene3DItem::setHoverEnabled(bool enabled)
{
    if (enabled != acceptHoverEvents()) {
        setAcceptHoverEvents(enabled);
        emit hoverEnabledChanged();
    }
}

/*!
    \qmlproperty enumeration Scene3D::cameraAspectRatioMode

    \value Scene3D.AutomaticAspectRatio
           Automatic aspect ratio.

    \value Scene3D.UserAspectRatio
           User defined aspect ratio.
    \brief \TODO
 */
Scene3DItem::CameraAspectRatioMode Scene3DItem::cameraAspectRatioMode() const
{
    return m_cameraAspectRatioMode;
}

void Scene3DItem::applyRootEntityChange()
{
    if (m_aspectEngine->rootEntity() != m_entity.data()) {
        m_aspectEngine->setRootEntity(m_entity);

        /* If we changed window, the old aspect engine must be deleted only after we have set
           the root entity for the new one so that it doesn't delete the root node. */
        if (m_aspectToDelete) {
            delete m_aspectToDelete;
            m_aspectToDelete = nullptr;
        }

        // Set the render surface
        if (!m_entity)
            return;

        setWindowSurface(entity());

        if (m_cameraAspectRatioMode == AutomaticAspectRatio) {
            // Set aspect ratio of first camera to match the window
            QList<Qt3DRender::QCamera *> cameras
                = m_entity->findChildren<Qt3DRender::QCamera *>();
            if (cameras.isEmpty()) {
                qCDebug(Scene3D) << "No camera found and automatic aspect ratio requested";
            } else {
                m_camera = cameras.first();
                setCameraAspectModeHelper();
            }
        }

#if QT_CONFIG(qt3d_input)
        // Set ourselves up as a source of input events for the input aspect
        Qt3DInput::QInputSettings *inputSettings = m_entity->findChild<Qt3DInput::QInputSettings *>();
        if (inputSettings) {
            inputSettings->setEventSource(this);
        } else {
            qCDebug(Scene3D) << "No Input Settings found, keyboard and mouse events won't be handled";
        }
#endif
    }
}

void Scene3DItem::updateWindowSurface()
{
    if (!m_entity || !m_dummySurface)
        return;
    Qt3DRender::QRenderSurfaceSelector *surfaceSelector =
        Qt3DRender::QRenderSurfaceSelectorPrivate::find(entity());
    if (surfaceSelector) {
        if (QWindow *rw = QQuickRenderControl::renderWindowFor(this->window())) {
            m_dummySurface->deleteLater();
            createDummySurface(rw, surfaceSelector);
        }
    }
}

void Scene3DItem::setWindowSurface(QObject *rootObject)
{
    Qt3DRender::QRenderSurfaceSelector *surfaceSelector = Qt3DRender::QRenderSurfaceSelectorPrivate::find(rootObject);

    // Set the item's window surface if it appears
    // the surface wasn't set on the surfaceSelector
    if (surfaceSelector && !surfaceSelector->surface()) {
        // We may not have a real, exposed QQuickWindow when the Quick rendering
        // is redirected via QQuickRenderControl (f.ex. QQuickWidget).
        if (QWindow *rw = QQuickRenderControl::renderWindowFor(this->window())) {
            createDummySurface(rw, surfaceSelector);
        } else {
            surfaceSelector->setSurface(this->window());
        }
    }
}

void Scene3DItem::createDummySurface(QWindow *rw, Qt3DRender::QRenderSurfaceSelector *surfaceSelector)
{
    // rw is the top-level window that is backed by a native window. Do
    // not use that though since we must not clash with e.g. the widget
    // backingstore compositor in the gui thread.
    m_dummySurface = new QOffscreenSurface;
    m_dummySurface->setParent(qGuiApp); // parent to something suitably long-living
    m_dummySurface->setFormat(rw->format());
    m_dummySurface->setScreen(rw->screen());
    m_dummySurface->create();
    surfaceSelector->setSurface(m_dummySurface);
}
/*!
    \qmlmethod void Scene3D::setItemAreaAndDevicePixelRatio(size area, real devicePixelRatio)

    \brief \TODO
 */
void Scene3DItem::setItemAreaAndDevicePixelRatio(QSize area, qreal devicePixelRatio)
{
    Qt3DRender::QRenderSurfaceSelector *surfaceSelector
            = Qt3DRender::QRenderSurfaceSelectorPrivate::find(entity());
    if (surfaceSelector) {
        surfaceSelector->setExternalRenderTargetSize(area);
        surfaceSelector->setSurfacePixelRatio(devicePixelRatio);
    }
}

/*!
    \qmlproperty bool Scene3D::hoverEnabled

    \c true if hover events are accepted.
 */
bool Scene3DItem::isHoverEnabled() const
{
    return acceptHoverEvents();
}

void Scene3DItem::setCameraAspectModeHelper()
{
    switch (m_cameraAspectRatioMode) {
    case AutomaticAspectRatio:
        connect(this, &Scene3DItem::widthChanged, this, &Scene3DItem::updateCameraAspectRatio);
        connect(this, &Scene3DItem::heightChanged, this, &Scene3DItem::updateCameraAspectRatio);
        // Update the aspect ratio the first time the surface is set
        updateCameraAspectRatio();
        break;
    case UserAspectRatio:
        disconnect(this, &Scene3DItem::widthChanged, this, &Scene3DItem::updateCameraAspectRatio);
        disconnect(this, &Scene3DItem::heightChanged, this, &Scene3DItem::updateCameraAspectRatio);
        break;
    }
}

void Scene3DItem::updateCameraAspectRatio()
{
    if (m_camera) {
        m_camera->setAspectRatio(static_cast<float>(width()) /
                                 static_cast<float>(height()));
    }
}

/*!
    \qmlproperty bool Scene3D::multisample

    \c true if a multisample render buffer is requested.

    By default multisampling is enabled. If the OpenGL implementation has no
    support for multisample renderbuffers or framebuffer blits, the request to
    use multisampling is ignored.

    \note Refrain from changing the value frequently as it involves expensive
    and potentially slow initialization of framebuffers and other OpenGL
    resources.
 */
bool Scene3DItem::multisample() const
{
    return m_multisample;
}

void Scene3DItem::setMultisample(bool enable)
{
    if (m_multisample != enable) {
        m_multisample = enable;
        emit multisampleChanged();
        update();
    }
}

QSGNode *Scene3DItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    // m_resetRequested is set to true by Scene3DRenderer::shutdown()
    if (m_renderer && m_renderer->m_resetRequested) {
        qCWarning(Scene3D) << "Renderer for Scene3DItem has requested a reset due to the item "
                              "moving to another window";
        m_aspectEngine->unregisterAspect(m_renderAspect); // Deletes the renderAspect
        m_renderAspect = nullptr;
        m_aspectToDelete = m_aspectEngine;
        m_aspectEngine = new Qt3DCore::QAspectEngine();
        applyAspects();
        // Needs to belong in the same thread as the item which is the same as the original
        // QAspectEngine
        m_aspectEngine->moveToThread(thread());
        m_renderer->m_resetRequested = false;
    }
    // If the render aspect wasn't created yet, do so now
    if (m_renderAspect == nullptr) {
        m_renderAspect = new QRenderAspect(QRenderAspect::Synchronous);
        auto *rw = QQuickRenderControl::renderWindowFor(window());
        static_cast<Qt3DRender::QRenderAspectPrivate *>(Qt3DRender::QRenderAspectPrivate::get(m_renderAspect))->m_screen =
                (rw ? rw->screen() : window()->screen());
        m_aspectEngine->registerAspect(m_renderAspect);
    }

    if (m_renderer == nullptr) {
        m_renderer = new Scene3DRenderer();
        m_renderer->init(this, m_aspectEngine, m_renderAspect);
    } else if (m_renderer->renderAspect() != m_renderAspect) {
        // If the renderer's renderAspect is not equal to the aspect used
        // by the item, then it means that we have created a new one due to
        // the fact that shutdown() was called on the renderer previously.
        // This is a typical situation when the window the item is in has
        // moved from one screen to another.
        updateWindowSurface();
        m_renderer->init(this, m_aspectEngine, m_renderAspect);
    }

    Scene3DSGNode *fboNode = static_cast<Scene3DSGNode *>(node);
    if (fboNode == nullptr) {
        fboNode = new Scene3DSGNode();
        m_renderer->setSGNode(fboNode);
    }
    fboNode->setRect(boundingRect());

    return fboNode;
}

void Scene3DItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //Prevent subsequent move and release events being disregarded my the default event->ignore() from QQuickItem
}

} // namespace Qt3DRender

QT_END_NAMESPACE
